#ifndef BRANCH_BTFN_H
#define BRANCH_BTFN_H

#include "address.h"
#include "modules.h"


class btfn : champsim::modules::branch_predictor
{

public: 
	using branch_predictor::branch_predictor;
	//void initialize_branch_predictor();
	bool predict_branch(champsim::address ip, champsim::address predicted_target, bool always_taken, uint8_t branch_type);
	//void last_branch_result(champsim::address ip, champsim::address branch_target, bool taken, uint8_t branch_type);

};

#endif
