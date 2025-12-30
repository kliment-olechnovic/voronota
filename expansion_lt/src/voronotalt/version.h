#ifndef VORONOTALT_VERSION_H_
#define VORONOTALT_VERSION_H_

#include <string>

namespace voronotalt
{

inline const std::string& version()
{
	static const std::string version_str="1.1";
	return version_str;
}

}

#endif /* VORONOTALT_VERSION_H_ */
