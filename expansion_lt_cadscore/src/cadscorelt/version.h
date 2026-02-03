#ifndef CADSCORELT_VERSION_H_
#define CADSCORELT_VERSION_H_

#include <string>

namespace cadscorelt
{

inline const std::string& version()
{
	static const std::string version_str="0.8";
	return version_str;
}

}

#endif /* CADSCORELT_VERSION_H_ */
