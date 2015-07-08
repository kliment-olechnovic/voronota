#ifndef GENERIC_UTILITIES_H_
#define GENERIC_UTILITIES_H_

#include <map>

namespace
{

class GenericUtilities
{
public:
	template<typename T>
	static std::map<typename T::key_type, std::pair<typename T::mapped_type, typename T::mapped_type> > merge_two_maps(const T& a, const T& b)
	{
		std::map<typename T::key_type, std::pair<typename T::mapped_type, typename T::mapped_type> > result;
		typename T::const_iterator a_it=a.begin();
		typename T::const_iterator b_it=b.begin();
		while(a_it!=a.end() && b_it!=b.end())
		{
			if(a_it->first==b_it->first)
			{
				result.insert(result.end(), std::make_pair(a_it->first, std::make_pair(a_it->second, b_it->second)));
				++a_it;
				++b_it;
			}
			else if(a_it->first<b_it->first)
			{
				++a_it;
			}
			else if(b_it->first<a_it->first)
			{
				++b_it;
			}
		}
		return result;
	}
};

}


#endif /* GENERIC_UTILITIES_H_ */
