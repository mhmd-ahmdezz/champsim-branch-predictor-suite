#include "tage.h"
void tage::compute_GEHL_series()
{
 	L[0]   = min_hist_length ;
    L[M-2] = max_hist_length ;
    
    for(std::size_t i=0;i<M-1;i++)
    {
        L[i] = static_cast<std::size_t>((1.0*(pow(alpha, static_cast<double>(i)) * L[0])) + 0.5);
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

std::size_t tage::fold_ip(champsim::address ip, std::size_t width) const
{
	//Assume that ip will be always 64-bit 
	uint64_t raw = ip.to<uint64_t>() ;
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
	
	result ^= fold_ip(ip, tables_index_width);
	result &= mask ;
	
	return result;
}

std::size_t tage::hash_tag(champsim::address ip, std::size_t table) const
{
	const std::size_t mask = (std::size_t{1} << tag_width) - 1 ;
	std::size_t result = tag_csr[table].comp ;
	
	result ^= fold_ip(ip, tag_width);
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
	
	std::size_t Tpred_tag_index;
	std::size_t Taltr_tag_index;
	
	bool provider_prediction;
	bool alternate_prediction;
	
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
			Tpred = i ;
		}
		
	}
	
	
	if(!hit)
	{
		Tpred = -1 ; // Idicates that the tagless predictor used
		//The provider component is the base predictor or tagless predictor
		provider_prediction = base_predictor_value.value() > (base_predictor_value.maximum/2);
	}
	else 
	{
		//Tpred
		Tpred_tag_index = hash_index(ip,Tpred);
		auto prediction_value = tagged_tables[Tpred][Tpred_tag_index].ctr;
		provider_prediction = prediction_value.value() > 0 ;
		
		//Taltr
		Taltr_tag_index = hash_index(ip,Taltr);
		prediction_value = tagged_tables[Taltr][Taltr_tag_index].ctr;
		alternate_prediction = prediction_value.value() > 0 ;
	}
	
	
	// record the various values needed to update the predictor
  	tage_state_buf.push_back({ip,provider_prediction,alternate_prediction, Tpred, Taltr, Tpred_tag_index, Taltr_tag_index});
 	if(std::size(tage_state_buf) > BUF_ENTRIES)
 	{
 		tage_state_buf.pop_front();
 	}
	
	return provider_prediction;
	
}
void tage::last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type)
{
	auto state = std::find_if(std::begin(tage_state_buf), std::end(tage_state_buf), [ip](auto x) {return x.ip == ip;}) ;
  	if (state == std::end(tage_state_buf))
    	return; // Skip update because state was lost
    	
	auto [_ip, provider_prediction, alternate_prediction, Tpred, Taltr,Tpred_tag_index, Taltr_tag_index] = *state;
	tage_state_buf.erase(state);
	
	
	bool u_not_equal_zero_is_found = false;
	
	
	//Update the CSR
	for(std::size_t i=0;i<M-1;i++)
	{
		idx_csr[i].update(GHR, static_cast<bool>(taken));
		tag_csr[i].update(GHR, static_cast<bool>(taken));
	}
	
	//Update the Global History Register
	GHR <<= 1 ;
	GHR.set(0,taken);
	
	if(Tpred == -1)
	{
		//Tagless predictor used so we need to update its prediction counter
		T0[hash(ip)] += (taken) ? 1 : -1;
	}
	else 
	{
		//Tagged Component Used: 
		
		//Update ctr of the provider component:
		tagged_tables[Tpred][Tpred_tag_index].ctr += (taken) ? 1 : -1;
		
		//Check which one is correct Tpred or Taltr:
		if(provider_prediction != alternate_prediction)
		{
			// Tpred - Was Correct | Talter - Was Wrong
			if(provider_prediction == taken && alternate_prediction != taken)
			{
				//Update U - Counter
				tagged_tables[Tpred][Tpred_tag_index].u += 1 ; // The Long History is valuable
			}
			// Tpred - Was Wrong | Talter - Was Correct
			else if(provider_prediction != taken && alternate_prediction == taken)
			{
				tagged_tables[Tpred][Tpred_tag_index].u -= 1 ; // The Long History is misleading or corrupted by noise 
			}
		} 
		
		
		//Misprediction Update: 
		if(provider_prediction != taken)
		{
			//Allocate New Entry:
			// Search in a longer history table at the same index for a u == 0, if multiple founds pick the shortest one.
			for(std::size_t i = Tpred+1 ;i<M-1; i++)
			{
				if(tagged_tables[i][Tpred_tag_index].u == 0)
				{
					tagged_tables[i][Tpred_tag_index].tag = tagged_tables[Tpred][Tpred_tag_index].tag;
					tagged_tables[i][Tpred_tag_index].ctr = 0 ; //weak correct
					tagged_tables[i][Tpred_tag_index].u = 0 ; // Strongly Not Useful
					
					u_not_equal_zero_is_found = true;
					break;
				}
			}
			
			if(!u_not_equal_zero_is_found)
			{
				//Decrement all the tables - u - counters
				for(std::size_t i = Tpred + 1; i<M-1 ;i++)
				{
					tagged_tables[i][Tpred_tag_index].u -= 1;
				}
			}	
		}
	}
	
	
	
	
}
