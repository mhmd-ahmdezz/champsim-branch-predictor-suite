#include "bimodal.h"

bool bimodal::predict_branch(champsim::address ip)
{
	auto value = bht[hash(ip)];
	return value.value() > (value.maximum/2); //if greater than the threshold (hysteresis threshold) considered as taken (either Strongly or Weakly)
}

void bimodal::last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type)
{
	if(taken)
	{
		// if predicted taken, increase the degree of takenness 
		bht[hash(ip)] += 1 ;
	}
	else 
	{	// if predicted not taken, increase the degreee of nottakenness
		bht[hash(ip)] -= 1 ;
	}
}
