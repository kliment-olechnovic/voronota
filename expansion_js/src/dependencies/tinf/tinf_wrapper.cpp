#include <vector>
#include <iostream>
#include <fstream>

#include "tinf.h"
#include "tinf_wrapper.h"

//the uncompression workflow here is based on the example from https://github.com/jibsen/tinf/blob/master/examples/tgunzip/tgunzip.c

namespace TinfWrapper
{

bool check_if_string_gzipped(const std::string& potentially_gzipped_string)
{
	return (potentially_gzipped_string.size()>=18 && static_cast<unsigned char>(potentially_gzipped_string[0])==0x1F && static_cast<unsigned char>(potentially_gzipped_string[1])==0x8B);
}

inline unsigned int read_le32(const unsigned char* p)
{
	return ((static_cast<unsigned int>(p[0])) | (static_cast<unsigned int>(p[1]) << 8) | (static_cast<unsigned int>(p[2]) << 16) | (static_cast<unsigned int>(p[3]) << 24));
}

bool uncompress_gzipped_string(const std::string& gzipped_string, std::string& uncompressed_string)
{
	if(!check_if_string_gzipped(gzipped_string))
	{
		return false;
	}

	tinf_init();

	const unsigned int len=static_cast<unsigned int>(gzipped_string.size());
	const unsigned char* source=reinterpret_cast<const unsigned char*>(gzipped_string.data());
	const unsigned int dlen=read_le32(&source[len-4]);

	if(dlen<1)
	{
		return false;
	}

	std::vector<unsigned char> dest_vector(dlen);
	unsigned char* dest=&dest_vector[0];

	unsigned int outlen=dlen;

	const int res=tinf_gzip_uncompress(dest, &outlen, source, len);

	if((res!=TINF_OK) || (outlen!=dlen))
	{
		return false;
	}

	std::string result(reinterpret_cast<const char*>(dest), static_cast<std::size_t>(outlen));
	uncompressed_string.swap(result);
	return true;
}

}

