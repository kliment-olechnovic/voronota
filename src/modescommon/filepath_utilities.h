#ifndef FILEPATH_UTILITIES_H_
#define FILEPATH_UTILITIES_H_

#include <string>
#include <set>

namespace
{

std::size_t calc_common_path_start_length(const std::set<std::string>& filenames)
{
	std::size_t common_path_start_length=0;
	if(filenames.size()>1)
	{
		const std::string& first_filename=(*filenames.begin());
		bool common_start=true;
		for(std::size_t pos=0;(common_start && pos<first_filename.size());pos++)
		{
			for(std::set<std::string>::const_iterator it=filenames.begin();(common_start && it!=filenames.end());++it)
			{
				const std::string& filename=(*it);
				if(!(pos<filename.size() && filename[pos]==first_filename[pos]))
				{
					common_start=false;
				}
			}
			if(common_start && (first_filename[pos]=='/' || first_filename[pos]=='\\'))
			{
				common_path_start_length=(pos+1);
			}
		}
	}
	return common_path_start_length;
}

}

#endif /* FILEPATH_UTILITIES_H_ */
