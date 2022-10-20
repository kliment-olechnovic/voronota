#ifndef DEPENDENCIES_FASPR_FASPR_CONFIG_H_
#define DEPENDENCIES_FASPR_FASPR_CONFIG_H_

#include <string>

class FASPRConfig
{
public:
	static std::string& PROGRAM_PATH()
	{
		static std::string value=".";
		return value;
	}

	static std::string& ROTLIB2010()
	{
		static std::string value="dun2010bbdep.bin";
		return value;
	}
};



#endif /* DEPENDENCIES_FASPR_FASPR_CONFIG_H_ */
