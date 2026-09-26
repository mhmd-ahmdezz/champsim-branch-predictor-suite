#ifndef BRANCH_ALWAYS_NOT_TAKEN_H
#define BRANCH_ALWAYS_NOT_TAKEN_H

#include <array>

#include "address.h"
#include "modules.h"

class always_not_taken : champsim::modules::branch_predictor
{

public:
  using branch_predictor::branch_predictor;
  void initialize_branch_predictor();
  bool predict_branch(champsim::address ip);
  void last_branch_result(champsim::address ip, champsim::address branch_target, bool taken, uint8_t branch_type);
   

};


#endif
