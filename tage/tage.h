#ifndef BRANCH_TAGE_H
#define BRANCH_TAGE_H

#include <array>
#include <bitset>
#include <cmath>
#include <deque>
#include <algorithm>

#include "modules.h"
#include "address.h"

#include "msl/fwcounter.h"


class tage: champsim::modules::branch_predictor
{

  	[[nodiscard]] static constexpr auto hash(champsim::address ip) { return ip.to<unsigned long>() % PRIME; }
  	static constexpr std::size_t PRIME = 2039;	  
	static constexpr std::size_t N = 15 ; // To determine the storage budget
	static constexpr std::size_t M = 5  ; // To determine the number of components 
	
	static constexpr std::size_t tables_index_width = N-6 ; // Number of bits to index tagged tables
	static constexpr std::size_t min_hist_length = 5 ; // Min Length of History Lengths
	static constexpr std::size_t max_hist_length = 130 ; // Max Length of History Lengths
 	static constexpr  double alpha = pow((1.0*max_hist_length / min_hist_length), (1.0/(M-2))); //Geometric mutliplier
 	
	static constexpr std::size_t t0_counter_bits = 2 ;
	static constexpr std::size_t STORAGE_BUDGET = (1<<N) ; // Storage Budget is 32k bits
	static constexpr std::size_t base_predictor_entries = (1<< (N-4) ); // number of tagless table - T0 component entries
	
	static constexpr std::size_t tag_width = 9 ;
	static constexpr std::size_t ctr_width = 3 ; // Prediction counter width
	static constexpr std::size_t u_width = 2   ; // Useful Counter width
	static constexpr std::size_t tagged_entries = (1 << (N-6)) ; // number of tagged Ti entries
	
	static constexpr std::size_t BUF_ENTRIES = 100;
	
	struct tagged_table_entry{
		std::bitset<tag_width> tag;
		champsim::msl::sfwcounter<ctr_width> ctr ;
		champsim::msl::fwcounter<u_width> u ;
	};
	
	struct tage_state
	{
		champsim::address ip{};
		
		bool provider_prediction;
		bool alternate_prediction;
		
		int Tpred;
		int Taltr;
	
		std::size_t Tpred_tag_index;
		std::size_t Taltr_tag_index;
	};
	
	std::deque<tage_state> tage_state_buf;
	
	
	std::array<std::size_t, M-1> L; // Array contains the History Lengths
	std::bitset<max_hist_length> GHR; // Global History Register
	std::array<champsim::msl::fwcounter<t0_counter_bits>, base_predictor_entries> T0; // Tagless or base predictor or fallback (default) predictor - T0 Component
	std::array< std::array<tagged_table_entry, tagged_entries>, M-1> tagged_tables; //Tagged components - Ti (1 <= i < M)
	
	struct folded_history
	{
		std::size_t comp = 0 ; // The folded value, W-bits wide
		std::size_t width = 0 ; // Width
		std::size_t length = 0 ; // Length of the required part from GHR
		std::size_t outpoint = 0 ; // L mod W, the position where the leaving bits sits
		
		void init(std::size_t L, std::size_t W)
		{
			length = L;
			width = W ;
			outpoint = L % W;
			comp = 0 ;
		}
		
		template <std::size_t K>
		void update (const std::bitset<K> &ghr, bool new_bit)
		{
			const bool old_bit = ghr[length - 1]; // Oldest bit in the window which leaves the window		
			comp  = (comp << 1) | new_bit; // Shift Left and insert the new-bit in position-0
			comp ^= static_cast<std::size_t>(old_bit) << outpoint; //// Handle the evict bit or Last element
			comp ^= comp >> width ;//Handle the first element
			comp &= (std::size_t{1} << width) - 1 ; // Mask to extract the W-bits
		}
	};
	
	std::array<folded_history, M-1> idx_csr; //CSR for index, width = tables_index_width
	std::array<folded_history, M-1> tag_csr; //CSR for Tag, width = tag_width
public:
	using branch_predictor::branch_predictor;
	void initialize_branch_predictor();
	bool predict_branch(champsim::address ip);
	void last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type);
	void compute_GEHL_series();
	std::size_t fold_ip(champsim::address ip, std::size_t width)const;
	std::size_t hash_index(champsim::address ip, std::size_t table) const;
	std::size_t hash_tag(champsim::address ip, std::size_t table) const;
};


#endif
