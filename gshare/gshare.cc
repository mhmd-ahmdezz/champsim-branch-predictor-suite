#include "gshare.h"

std::size_t gshare::gs_table_hash(champsim::address ip, std::bitset<GHR_LENGTH>bh_vector)
{
	//constexpr champsim::data::bits LOG2_PHT_SIZE{champsim::lg2(PHT_SIZE)}; //bits used since slice accepts bits only
	constexpr champsim::data::bits LENGTH{GHR_LENGTH};
	
	std::size_t hash = bh_vector.to_ullong();
	
	//Folding the PC, to avoid inter-branch aliasing due to same lower bits as GHR Length will be always less than 
	// Size of PC
	
	//First Chunk PC[0 ,GHR_LENGTH]
	hash ^= ip.slice<LENGTH, champsim::data::bits{}>().to<std::size_t>();
	
	// Second Chunk PC[GHR_LENGTH, 2*GHR_LENGTH] (e.g. PC[14, 2*14] = PC[14, 28])
	hash ^= ip.slice<2*LENGTH, LENGTH>().to<std::size_t>();
	
	// Third Chunk PC[2*GHR_LENGTH, 3*GHR_LENGTH] (e.g. PC[2*14, 3*14] = PC[28, 42])
	hash ^= ip.slice<3*LENGTH, 2*LENGTH>().to<std::size_t>();
	
	//Fourth Chunk PC[3*GHR_LENGTH, 4*GHR_LENGTH] (e.g. PC[3*14, 4*14] = PC[42, 56])
	hash ^= ip.slice<4*LENGTH, 3*LENGTH>().to<std::size_t>();
	
	return hash;
}

bool gshare::predict_branch(champsim::address ip)
{
	//Hasing between the GHR and PC
	auto gs_hash = gs_table_hash(ip, GHR);
	auto value = PHT[gs_hash];
	return value.value() > (value.maximum/2);
}


void gshare::last_branch_result(champsim::address ip, champsim::address branch_target, uint8_t taken, uint8_t branch_type)
{
	auto gs_hash = gs_table_hash(ip, GHR);
	if(taken)
	{
		//Update the PHT entry
		PHT[gs_hash] += 1 ; // Increase the degree of takenness 
		//Update the GHR
		t_nt.set(0);
		GHR = (GHR<<1) | t_nt ;
	}
	else 
	{
		//Update the PHT entry
		PHT[gs_hash] -= 1 ; // Increase the degree of nottakenness 
		//Update the GHR
		t_nt.reset(0);
		GHR = (GHR<<1) | t_nt ;
	}


}
