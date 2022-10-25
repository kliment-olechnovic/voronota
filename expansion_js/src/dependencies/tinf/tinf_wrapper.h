#ifndef DEPENDENCIES_TINF_TINF_WRAPPER_H_
#define DEPENDENCIES_TINF_TINF_WRAPPER_H_

#include <string>

namespace TinfWrapper
{

bool check_if_string_gzipped(const std::string& potentially_gzipped_string);

bool uncompress_gzipped_string(const std::string& gzipped_string, std::string& uncompressed_string);

}

#endif /* DEPENDENCIES_TINF_TINF_WRAPPER_H_ */

