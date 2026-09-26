#include "last_time.h"

bool last_time::predict_branch(champsim::address ip)
{
	return bht[hash(ip)];
}

void last_time::last_branch_result(champsim::address ip, champsim::address branch_target, bool taken, uint8_t branch_type)
{
	bht[hash(ip)] = taken;
}
