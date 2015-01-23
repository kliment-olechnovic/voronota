#ifndef MODESCOMMON_TMPFUNCS_H_
#define MODESCOMMON_TMPFUNCS_H_

namespace auxiliaries
{

template<typename T>
inline std::set<T> read_set_from_string(const std::string& input_str, const std::string& separators)
{
	std::set<T> result;
	if(input_str.find_first_not_of(separators)!=std::string::npos)
	{
		std::string str=input_str;
		for(std::size_t i=0;i<str.size();i++)
		{
			if(separators.find(str[i])!=std::string::npos)
			{
				str[i]=' ';
			}
		}
		std::istringstream input(str);
		while(input.good())
		{
			std::string token;
			input >> token;
			if(!token.empty())
			{
				result.insert(token);
			}
		}
	}
	return result;
}

template<typename T>
inline void print_set_to_stream(const std::set<T>& set, const std::string& sep, std::ostream& output)
{
	for(typename std::set<T>::const_iterator it=set.begin();it!=set.end();++it)
	{
		output << (it==set.begin() ? std::string() : sep) << (*it);
	}
}

template<typename A, typename B>
inline std::map<A, B> read_map_from_string(const std::string& input_str, const std::string& separators)
{
	std::map<A, B> result;
	const std::set<std::string> set=read_set_from_string<std::string>(input_str, separators);
	for(std::set<std::string>::const_iterator it=set.begin();it!=set.end();++it)
	{
		const std::size_t op_pos=it->find('=');
		if(op_pos<it->size())
		{
			std::string str=(*it);
			str[op_pos]=' ';
			std::istringstream input(str);
			if(input.good())
			{
				A a;
				B b;
				input >> a >> b;
				if(!input.fail())
				{
					result[a]=b;
				}
			}
		}
	}
	return result;
}

template<typename A, typename B>
inline void print_map_to_stream(const std::map<A, B>& map, const std::string& sep, std::ostream& output)
{
	for(typename std::map<A, B>::const_iterator it=map.begin();it!=map.end();++it)
	{
		output << (it==map.begin() ? std::string() : sep) << it->first << "=" << it->second;
	}
}

}

#endif /* MODESCOMMON_TMPFUNCS_H_ */
