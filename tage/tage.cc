#include "tage.h"
void tage::compute_GEHL_series()
{
 	const double ratio = static_cast<double>(max_hist_length) / static_cast<double>(min_hist_length);
 	const double alpha = std::pow(ratio, 1.0/static_cast<double>(M-2)); // Geometric Multiplier
 	
    for(std::size_t i=0;i<M-1;i++)
    {
        L[i] = static_cast<std::size_t>( (std::pow(alpha, static_cast<double>(i)) * static_cast<double>(min_hist_length) )+ 0.5);
    }
}


void tage::initialize_branch_predictor()
{
	//Compute the Geometric History Length series
	compute_GEHL_series();
	
	//Create the CSR with at which widths folds and the history length for each table
	//This code assumes that the tag_width is equal for all tables and the tables are equal sizes.
	for(std::size_t i=0;i<M-1;i++)
	{
		idx_csr[i].init(L[i], tables_index_width);
		tag_csr[i].init(L[i], tag_width);
	}
	
}

std::size_t tage::fold_ip(uint64_t pc, std::size_t width) const
{
	//Assume that ip will be always 64-bit 
	//uint64_t raw = ip.to<uint64_t>() ;
	uint64_t raw = pc ;
	raw &= (uint64_t{1} << 63) - 1 ;
	
	const uint64_t mask = (uint64_t{1} << width) - 1;
	std::size_t result =0;
	for(std::size_t i =0; i<64 ;i+=width)
	{
		result ^= (raw >> i) & mask ;
	}
	return result;
}

std::size_t tage::hash_index(champsim::address ip, std::size_t table) const
{
	const std::size_t mask = (std::size_t{1} <<tables_index_width) - 1 ;
	std::size_t result = idx_csr[table].comp ;
	
	result ^= fold_ip(ip.to<uint64_t>(), tables_index_width);
	result &= mask ;
	
	return result;
}

std::size_t tage::hash_tag(champsim::address ip, std::size_t table) const
{
	const std::size_t mask = (std::size_t{1} << tag_width) - 1 ;
	std::size_t result = tag_csr[table].comp ;
	
	// result ^= fold_ip((ip.to<uint64_t>()>>4), tag_width);
	result ^= fold_ip((ip.to<uint64_t>()>>2), tag_width);
	result &= mask;
	
	return result;
}

bool tage::predict_branch(champsim::address ip)
{
	bool hit = false;
	//Base Predictor
	auto base_predictor_value = T0[hash(ip)];
	
	
	//Tagged Tables
	std::size_t index;
	std::size_t expected_tag ;
	
	int Tpred = -1;
	int Taltr = -1;
	
	std::size_t Tpred_tag_index = 0;
	std::size_t Taltr_tag_index = 0;
	
	bool provider_prediction = false;
	bool alternate_prediction = false;
	
	for(std::size_t i=0;i<M-1;i++)
	{
		index = hash_index(ip,i);
		expected_tag = hash_tag(ip,i);
		
		if(tagged_tables[i][index].tag == expected_tag)
		{
			//Hit is happened
			hit = true;
			if(i>0)
			{
				Taltr = Tpred ;
			}
			Tpred = static_cast<int>(i) ;
		}
		
	}
	
	
	if(!hit)
	{
		Tpred = -1 ; // Idicates that the tagless predictor used
		Taltr = -1 ;
		
		//The provider component is the base predictor or tagless predictor
		provider_prediction = base_predictor_value.value() > (base_predictor_value.maximum/2);
		alternate_prediction = provider_prediction;
	}
	else 
	{
		//Tpred
		Tpred_tag_index = hash_index(ip,Tpred);
		auto prediction_value = tagged_tables[Tpred][Tpred_tag_index].ctr;
		provider_prediction = prediction_value.value() > 0 ;
		
		//Taltr
		if(Taltr != -1)
		{
			Taltr_tag_index = hash_index(ip,Taltr);
			prediction_value = tagged_tables[Taltr][Taltr_tag_index].ctr;
			alternate_prediction = prediction_value.value() > 0 ;
		}
		else
		{
			alternate_prediction = base_predictor_value.value() > (base_predictor_value.maximum/2) ;
		}
	}
	
	//Take a snapshot of history register, idx_csr, and tag_csr before updating them speculatively for speculative recovery
	// record the various values needed to update the predictor
	
	auto spec_GHR = GHR;
	auto spec_idx_csr = idx_csr;
	auto spec_tag_csr = tag_csr;
	
  	tage_state_buf.push_back({ip,provider_prediction,alternate_prediction, 
  		Tpred, Taltr, Tpred_tag_index, Taltr_tag_index,
  		spec_GHR, spec_idx_csr, spec_tag_csr});
  		
 	if(std::size(tage_state_buf) > BUF_ENTRIES)
 	{
		tage_state_buf.pop_front();
 	}
	
	
	//Update Index and Tag CSR with predicted outcome.
	for(std::size_t i=0;i<M-1;i++)
	{
		spec_idx_csr[i].update(spec_GHR, provider_prediction);
		spec_tag_csr[i].update(spec_GHR, provider_prediction);
	}
	
	//Update the GHR speculatively with predicted outcome.
	spec_GHR <<= 1;
	spec_GHR.set(0, provider_prediction);
	
	
	return provider_prediction;
	
}
void tage::last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type)
{
	auto state = std::find_if(std::begin(tage_state_buf), std::end(tage_state_buf), [ip](auto x) {return x.ip == ip;}) ;
  	if (state == std::end(tage_state_buf))
    	return; // Skip update because state was lost
    	
	auto [_ip, provider_prediction, alternate_prediction, 
		Tpred, Taltr,Tpred_tag_index, Taltr_tag_index,
		spec_GHR, spec_idx_csr, spec_tag_csr] = *state;
		
	tage_state_buf.erase(state);
	
	bool u_not_equal_zero_is_found = false;
	const bool outcome = (taken != 0);
	
	//Check whether the speculative prediction is correct or wrong. If it was wrong
	// rollback to the last state (GHR, idx_csr, and tag_csr) and then update them with
	// correct values.
	
	if(provider_prediction != outcome)
	{
		//Restore the last state, which's correct
		GHR = spec_GHR;
		idx_csr = spec_idx_csr;
		tag_csr = spec_tag_csr;
	}
	
	
	/*
	auto get_prediction_time_index = [&](std::size_t table) {
		const std::size_t mask = (std::size_t{1} << tables_index_width) - 1 ;
		std::size_t result = spec_idx_csr[table].comp ;
		result ^= fold_ip(_ip, tables_index_width);
		
		result &= mask;
		return result;
	};
	
	
	auto get_prediction_time_tag = [&](std::size_t table) {
		const std::size_t mask = (std::size_t{1} << tag_width) - 1 ;
		std::size_t result = spec_tag_csr[table].comp ;
		result ^= fold_ip(_ip, tag_width);
		
		result &= mask;
		return result;
	};
	*/
	
	
	if(Tpred == -1)
	{
		//Tagless predictor used so we need to update its prediction counter
		T0[hash(_ip)] += (outcome) ? 1 : -1;
	}
	else 
	{
		//Tagged Component Used: 
		
		//Update ctr of the provider component:
		tagged_tables[Tpred][Tpred_tag_index].ctr += (outcome) ? 1 : -1;
		
		//Check which one is correct Tpred or Taltr:
		if(provider_prediction != alternate_prediction)
		{
			// Tpred - Was Correct | Talter - Was Wrong
			if(provider_prediction == outcome && alternate_prediction != outcome)
			{
				//Update U - Counter
				tagged_tables[Tpred][Tpred_tag_index].u += 1 ; // The Long History is valuable
			}
			// Tpred - Was Wrong | Talter - Was Correct
			else if(provider_prediction != outcome && alternate_prediction == outcome)
			{
				tagged_tables[Tpred][Tpred_tag_index].u -= 1 ; // The Long History is misleading or corrupted by noise 
			}
		} 	
	}
	
	//Misprediction Update: 
	if(provider_prediction != outcome)
	{
		//Allocate New Entry:
		// Search in a longer history table at the same index for a u == 0, if multiple founds pick the shortest one.
		for(std::size_t i = Tpred+1 ;i<M-1; i++)
		{
			std::size_t table_index = hash_index(_ip, i);
			if(tagged_tables[i][table_index].u == 0)
			{
				tagged_tables[i][table_index].tag = hash_tag(_ip, i);
				tagged_tables[i][table_index].ctr = 0 ; //weak correct
				tagged_tables[i][table_index].u = 0 ; // Strongly Not Useful
				
				u_not_equal_zero_is_found = true;
				break;
			}
		}
		
		if(!u_not_equal_zero_is_found)
		{
			//Decrement all the tables - u - counters
			for(std::size_t i = Tpred + 1; i<M-1 ;i++)
			{
				tagged_tables[i][hash_index(_ip, i)].u -= 1;
			}
		}
		
	}
	
	
	for(std::size_t i=0;i<M-1;i++)
	{
		idx_csr[i].update(GHR, outcome);
		tag_csr[i].update(GHR, outcome);
	}
		
	//Update the Global History Register
	GHR <<= 1 ;
	GHR.set(0,outcome);
	
	
}
