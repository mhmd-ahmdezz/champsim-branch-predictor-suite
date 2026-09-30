#ifndef BRANCH_TWO_LEVEL_LOCAL_PREDICTOR_H
#define BRANCH_TWO_LEVEL_LOCAL_PREDICTOR_H

#include <array>
#include "address.h"
#include "modules.h"
#include "msl/fwcounter.h"

#include <bits/stdc++.h>
#include <cmath>

class two_level_local_predictor: champsim::modules::branch_predictor
{
	[[nodiscard]] static constexpr auto hash(champsim::address ip) {return ip.to<unsigned long>() % PRIME;}
	
	// Prime number is chosen for modulus, since power-two will create collisions (due to power-two modulos take the LSBs only)
	
	//History Registers Table
	static constexpr std::size_t HR_LENGTH = 14; // HR width
	static constexpr std::size_t HRT_SIZE = 16384; // History Registers Table Size
	static constexpr std::size_t PRIME = 16381;
	
	std::array<std::bitset<HR_LENGTH>, HRT_SIZE> HRT;
	std::bitset<HR_LENGTH> t_nt; // Temp register used in ORing or Masking at shift left operation
	
	// Pattern History Table
	static constexpr std::size_t PHT_SIZE = std::pow(2,HR_LENGTH); // PHT width
	static constexpr std::size_t BITS = 2;
	
	std::array<champsim::msl::fwcounter<BITS>, PHT_SIZE > PHT; // Pattern History Table
	

public:
	using branch_predictor::branch_predictor;

	//void initialize_branch_predictor();
	bool predict_branch(champsim::address ip);
	void last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type);

};

#endif
