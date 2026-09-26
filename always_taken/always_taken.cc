#include "always_taken.h"

void always_taken::initialize_branch_predictor(){}
bool always_taken::predict_branch(champsim::address ip){
	return true;
}
void always_taken::last_branch_result(champsim::address ip, champsim::address branch_target, bool taken, uint8_t branch_type){}

