#ifndef BRANCH_THREE_BC_BASED_PREDICTOR_H
#define BRANCH_THREE_BC_BASED_PREDICTOR_H

#include <array>
#include "modules.h"
#include "address.h"
#include "msl/fwcounter.h"

class three_bc_based_predictor: champsim::modules::branch_predictor
{
	[[nodiscard]] static constexpr auto hash(champsim::address ip) {return ip.to<unsigned long>() % PRIME;}
	
	// Prime number is chosen for modulus, since power-two will create collisions (due to power-two modulos take the LSBs only)
	// Compare between this hashing techniques and that in the real implementation (taking some bits of IP and checks the tag (rest bits))
	static constexpr std::size_t TABLE_SIZE = 16384;
	static constexpr std::size_t PRIME = 16381;
	
	static constexpr std::size_t BITS = 3;
	std::array<champsim::msl::fwcounter<BITS>,TABLE_SIZE> bht;


public: 
	using branch_predictor::branch_predictor;
	//void initialize_branch_predictor();
	bool predict_branch(champsim::address ip);
	void last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type);

};


#endif
