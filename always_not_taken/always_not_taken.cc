#include "always_not_taken.h"


void always_not_taken::initialize_branch_predictor(){};

bool always_not_taken::predict_branch(champsim::address ip)
{
   return 0;
};

void always_not_taken::last_branch_result(champsim::address ip, champsim::address branch_target, bool taken, uint8_t branch_type){};
