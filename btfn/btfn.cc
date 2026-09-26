#include "btfn.h"
//#include <iostream>
#include "instruction.h"


bool btfn::predict_branch(champsim::address ip, champsim::address predicted_target, bool always_taken, uint8_t branch_type)
{
	//std::cerr << "ip=" << ip << " target=" << predicted_target << " taken=" << (predicted_target < ip) << "\n";
	
	//BTFN: predicting taken for Backward conditions and not taken for forward conditions,
	//Which happens mostly with loops.
	
	if(branch_type != BRANCH_CONDITIONAL)
	{
		return true;
	}
	
	
	// if it was the first time "cold start" to a branch, the BTB returns a (0x00) as a target address,
	// and (0x00) is always less than ip (0x00 < ip) so it will predict taken, which is not necessarily right.
	// From the last simulation results for always_not_taken and always_taken, it's obvious that predicting not taken
	// is better than predicting taken (I'm not sure is it correct for any other benchmarks other than "600.perlbench_s-210B")
	if (predicted_target == champsim::address{})
	{
    	return false;
  	}
	
	return predicted_target < ip;
}
