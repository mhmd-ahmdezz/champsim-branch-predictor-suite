#include "perceptron.h"
int perceptron::compute_sum(champsim::address ip)
{
	int sum = PWT[hash(ip)][0] ; // w0 * x0 = w0 * 1 = Bias
	
	for(size_t i=0;i< GHR_SIZE;i++)
	{
		int temp0 = (GHR[i]) ? 1 : -1 ; 
		sum += PWT[hash(ip)][i+1] * temp0 ;
	}
	return sum ;
}
bool perceptron::predict_branch(champsim::address ip)
{
	return ( compute_sum(ip) > 0) ;
}

void perceptron::last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type)
{
	int y = compute_sum(ip) ;
	bool prediction_correct = (y > 0) ;
	
	if(prediction_correct != static_cast<bool>(taken) || ((prediction_correct) ? y : -1*y)  <= static_cast<int>(theta)) // Fix the abs() problem
	{
		int t = taken ? 1 : -1 ;
		// wi = wi + t * xi, where (t) is the actual outcome
		PWT[hash(ip)][0] += t * 1 ; // w0 = w0 + t * x0 = w0 + t * 1
		for(size_t i=0;i< GHR_SIZE;i++)
		{
			int temp1 = (GHR[i]) ? 1 : -1 ;
			PWT[hash(ip)][i+1] += t * temp1 ;  
		}
	}
	
	//Update the GHR
	GHR <<= 1 ;
	GHR[0] = taken ;
}
