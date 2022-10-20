#ifndef DEPENDENCIES_FASPR_FASPR_CONFIG_H_
#define DEPENDENCIES_FASPR_FASPR_CONFIG_H_

#include <string>
#include <fstream>

struct FASPRConfig
{
	std::string PROGRAM_PATH;
	std::string ROTLIB2010;

	FASPRConfig() : PROGRAM_PATH("."), ROTLIB2010("dun2010bbdep.bin")
	{
	}

	bool is_library_file_available() const
	{
		std::string filepath=PROGRAM_PATH+"/"+ROTLIB2010;
		std::fstream input(filepath.c_str(), std::ios::in|std::ios::binary);
		if(!input)
		{
			return false;
		}
		return true;
	}
};

#endif /* DEPENDENCIES_FASPR_FASPR_CONFIG_H_ */

