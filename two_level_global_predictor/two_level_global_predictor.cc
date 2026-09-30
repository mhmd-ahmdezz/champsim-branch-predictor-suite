#include "two_level_global_predictor.h"

bool two_level_global_predictor::predict_branch(champsim::address ip)
{
	auto value = PHT[GHR.to_ulong()];
	return value.value() > (value.maximum/ 2);
}

void two_level_global_predictor::last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type)
{
	if(taken)
	{
		//Update the PHT entry
		PHT[GHR.to_ulong()] += 1 ; // Increase the degree of takenness 
		//Update the GHR
		t_nt.set(0);
		GHR = (GHR<<1) | t_nt ;
	}
	else 
	{
		//Update the PHT entry
		PHT[GHR.to_ulong()] -= 1 ; // Increase the degree of nottakenness 
		//Update the GHR
		t_nt.reset(0);
		GHR = (GHR<<1) | t_nt ;
	}
}
