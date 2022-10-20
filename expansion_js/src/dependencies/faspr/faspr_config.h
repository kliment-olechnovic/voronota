#ifndef DEPENDENCIES_FASPR_FASPR_CONFIG_H_
#define DEPENDENCIES_FASPR_FASPR_CONFIG_H_

#include <string>

struct FASPRConfig
{
	std::string PROGRAM_PATH;
	std::string ROTLIB2010;

	FASPRConfig() : PROGRAM_PATH("."), ROTLIB2010("dun2010bbdep.bin")
	{
	}
};

#endif /* DEPENDENCIES_FASPR_FASPR_CONFIG_H_ */

