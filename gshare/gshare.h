#ifndef BRANCH_GSHARE_H
#define BRANCH_GSHARE_H

#include <array>
#include <cmath>
#include <bits/stdc++.h>
#include "modules.h"
#include "address.h"
#include "msl/fwcounter.h"

class gshare: champsim::modules::branch_predictor
{
	//static constexpr std::size_t GHR_LENGTH = 4; // GHR size
	static constexpr std::size_t GHR_LENGTH = 14; // GHR size
	static constexpr std::size_t PHT_SIZE   = std::pow(2,GHR_LENGTH); // GHR size
	static constexpr std::size_t BITS = 2;
	
	std::bitset<GHR_LENGTH> GHR; // Global History Register
	std::bitset<GHR_LENGTH> t_nt; // Temp register used in ORing or Masking at shift left operation
	std::array<champsim::msl::fwcounter<BITS>, PHT_SIZE > PHT; // Pattern History Table
	
	
public:
	using branch_predictor::branch_predictor;
	//void initialize_branch_predictor();
	bool predict_branch(champsim::address ip);
	void last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type);
	std::size_t gs_table_hash(champsim::address ip, std::bitset<GHR_LENGTH>GHR);

};

#endif
