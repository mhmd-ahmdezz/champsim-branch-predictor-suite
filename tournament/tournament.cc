#include "tournament.h"
bool tournament::predict_branch(champsim::address ip)
{
	//Local Predictor
	auto LP_value = LP_PHT[LP_HRT[hash(ip)].to_ulong()];
	
	//Global Predictor
	auto GP_value = GP_PHT[GP_GHR.to_ulong()];
	
	//Choice Predictor
	auto CP_value = CPT[GP_GHR.to_ulong()];
	
	if(CP_value.value() > (CP_value.maximum/2))
	{
		return GP_value.value() > (GP_value.maximum/2);
	}
	else 
	{
		return LP_value.value() > (LP_value.maximum/2);
	}

}


void tournament::last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type)
{
	//Local Predictor
	auto LP_value = LP_PHT[LP_HRT[hash(ip)].to_ulong()];
	bool lp_pred  = LP_value.value() > (LP_value.maximum/2) ;
	 
	//Global Predictor
	auto GP_value = GP_PHT[GP_GHR.to_ulong()];
	bool gp_pred = GP_value.value() > (GP_value.maximum/2) ;
	

	if(taken)
	{
		//Update the Choice Prediction Table CPT
		if(lp_pred && !gp_pred) //Local Predictor prediction is correct
		{
			//Update the Choice predictor toward the local predictor
			CPT[GP_GHR.to_ulong()] -= 1; 
		} 
		else if(!lp_pred && gp_pred) //Global Predictor prediction is correct
		{
			//Update the Choice predictor toward the global predictor
			CPT[GP_GHR.to_ulong()] += 1; 
		}
		
		//Update the PHT entry
		LP_PHT[LP_HRT[hash(ip)].to_ulong()] += 1 ; // Increase the degree of takenness 
		GP_PHT[GP_GHR.to_ulong()] += 1 ; // Increase the degree of takenness 
		
		//Update the GHR
		LP_t_nt.set(0);
		GP_t_nt.set(0);
		
		LP_HRT[hash(ip)] = (LP_HRT[hash(ip)]<<1) | LP_t_nt ;
		GP_GHR = (GP_GHR<<1) | GP_t_nt ;
	}
	else
	{
		//Update the Choice Prediction Table CPT
		if(!lp_pred && gp_pred) //Local Predictor prediction is correct
		{
			//Update the Choice predictor toward the local predictor
			CPT[GP_GHR.to_ulong()] -= 1; 
		} 
		else if(lp_pred && !gp_pred) //Global Predictor prediction is correct
		{
			//Update the Choice predictor toward the global predictor
			CPT[GP_GHR.to_ulong()] += 1; 	
		}
		
		//Update the PHT entry
		LP_PHT[LP_HRT[hash(ip)].to_ulong()] -= 1 ; // Increase the degree of nottakenness
		GP_PHT[GP_GHR.to_ulong()] -= 1 ; // Increase the degree of nottakenness 
		
		//Update the GHR
		LP_t_nt.reset(0);
		GP_t_nt.reset(0);
		
		LP_HRT[hash(ip)] = (LP_HRT[hash(ip)]<<1) | LP_t_nt ;
		GP_GHR = (GP_GHR<<1) | GP_t_nt ;
	
	}
}
