#ifndef BRANCH_TOURNAMENT_H
#define BRANCH_TOURNAMENT_H

#include <array>
#include "address.h"
#include "modules.h"
#include "msl/fwcounter.h"

#include <bits/stdc++.h>
#include <cmath>

class tournament: champsim::modules::branch_predictor
{
	//Local Branch Predictor: PAg
	[[nodiscard]] static constexpr auto hash(champsim::address ip) {return ip.to<unsigned long>() % LP_PRIME;}
	
	// Prime number is chosen for modulus, since power-two will create collisions (due to power-two modulos take the LSBs only)
	
	//History Registers Table
	static constexpr std::size_t LP_HR_LENGTH = 14; // HR width
	static constexpr std::size_t LP_HRT_SIZE = 16384; // History Registers Table Size
	static constexpr std::size_t LP_PRIME = 16381;
	
	std::array<std::bitset<LP_HR_LENGTH>, LP_HRT_SIZE> LP_HRT;
	std::bitset<LP_HR_LENGTH> LP_t_nt; // Temp register used in ORing or Masking at shift left operation
	
	// Pattern History Table
	static constexpr std::size_t LP_PHT_SIZE = std::pow(2,LP_HR_LENGTH); // PHT width
	static constexpr std::size_t LP_BITS = 2;
	
	std::array<champsim::msl::fwcounter<LP_BITS>, LP_PHT_SIZE > LP_PHT; // Pattern History Table
	
	//Global Branch Predictor: GAg
	
	//static constexpr std::size_t GHR_LENGTH = 4; // GHR size
	static constexpr std::size_t GP_GHR_LENGTH = 14; // GHR size
	static constexpr std::size_t GP_PHT_SIZE   = std::pow(2,GP_GHR_LENGTH); // PHT size
	static constexpr std::size_t GP_BITS = 2;
	
	std::bitset<GP_GHR_LENGTH> GP_GHR; // Global History Register
	std::bitset<GP_GHR_LENGTH> GP_t_nt; // Temp register used in ORing or Masking at shift left operation
	std::array<champsim::msl::fwcounter<GP_BITS>, GP_PHT_SIZE > GP_PHT; // Pattern History Table
	
	
	//Chooser or Meta-Predictor:
	static constexpr std::size_t CPT_SIZE = std::pow(2, GP_GHR_LENGTH); // Size of Choice Prediction Table
	static constexpr std::size_t CP_BITS = 2; 
	
	std::array<champsim::msl::fwcounter<CP_BITS>, CPT_SIZE > CPT;
	


public:
	using branch_predictor::branch_predictor;
	//void initialize_branch_predictor();
	bool predict_branch(champsim::address ip);
	void last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type);
};


#endif
