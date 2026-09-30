#include "two_level_local_predictor.h"

bool two_level_local_predictor::predict_branch(champsim::address ip)
{
	auto value = PHT[HRT[hash(ip)].to_ulong()];
	return value.value() > (value.maximum/2);
}

void two_level_local_predictor::last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type)
{
	if(taken)
	{
		//Update the PHT entry
		PHT[HRT[hash(ip)].to_ulong()] += 1 ; // Increase the degree of takenness 
		//Update the HRT entry
		t_nt.set(0);
		HRT[hash(ip)] = (HRT[hash(ip)]<<1) | t_nt ;
	}
	else 
	{
		//Update the PHT entry
		PHT[HRT[hash(ip)].to_ulong()] -= 1 ; // Increase the degree of nottakenness 
		//Update the GHR
		t_nt.reset(0);
		HRT[hash(ip)] = (HRT[hash(ip)]<<1) | t_nt ;
	}

}
