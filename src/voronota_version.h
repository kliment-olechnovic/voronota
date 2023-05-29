#ifndef VORONOTA_VERSION_H_
#define VORONOTA_VERSION_H_

#include <string>

namespace voronota
{

inline std::string version()
{
	static const std::string version_str="1.26";
	return version_str;
}

}

#endif /* VORONOTA_VERSION_H_ */
