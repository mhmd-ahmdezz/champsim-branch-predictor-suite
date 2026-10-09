#ifndef BRANCH_PERCEPTRON_H
#define BRANCH_PERCEPTRON_H

#include <array>
#include "modules.h"
#include "address.h"
#include <bits/stdc++.h>

class perceptron: champsim::modules::branch_predictor
{
	[[nodiscard]] static constexpr auto hash(champsim::address ip) {return ip.to<unsigned long>() % PRIME;}
	
	static constexpr std::size_t GHR_SIZE = 24; // Global History Register Size
	static constexpr std::size_t PWT_SIZE = 16384; // Perceptron Weights Table
	static constexpr std::size_t PRIME = 16381;
	static constexpr double theta = (1.93 * GHR_SIZE ) + 14.0 ; // Training Threshold, this relation based on Dynamic Branch Prediction with Perceptrons Paper
	
	std::bitset<GHR_SIZE> GHR;
	std::array< std::array<int, GHR_SIZE+1>, PWT_SIZE > PWT; //Perceptron Table

public:
	using branch_predictor::branch_predictor;
	//void initialize_branch_predictor();
	bool predict_branch(champsim::address ip);
	void last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type);
	int compute_sum(champsim::address ip); //Prediction Function: Compute the y = w0 + xi * wi
};


#endif
