#ifndef PROPERTIES_VALUE_H_
#define PROPERTIES_VALUE_H_

#include <set>
#include <map>

#include "../auxiliaries/io_utilities.h"

namespace
{

struct PropertiesValue
{
	std::set<std::string> tags;
	std::map<std::string, double> adjuncts;

	void update_tags(const std::string& str)
	{
		if(!str.empty() && str[0]!='.')
		{
			std::set<std::string> input_tags;
			auxiliaries::IOUtilities(';').read_string_lines_to_set(str, input_tags);
			if(!input_tags.empty())
			{
				tags.insert(input_tags.begin(), input_tags.end());
			}
		}
	}

	void update_adjuncts(const std::string& str)
	{
		if(!str.empty() && str[0]!='.')
		{
			std::map<std::string, double> input_adjuncts;
			auxiliaries::IOUtilities(';', '=').read_string_lines_to_map(str, input_adjuncts);
			for(std::map<std::string, double>::const_iterator it=input_adjuncts.begin();it!=input_adjuncts.end();++it)
			{
				adjuncts[it->first]=it->second;
			}
		}
	}
};

inline std::ostream& operator<<(std::ostream& output, const PropertiesValue& value)
{
	if(value.tags.empty())
	{
		output << ".";
	}
	else
	{
		auxiliaries::IOUtilities(';').write_set(value.tags, output);
	}
	output << " ";
	if(value.adjuncts.empty())
	{
		output << ".";
	}
	else
	{
		auxiliaries::IOUtilities(';', '=').write_map(value.adjuncts, output);
	}
	return output;
}

inline std::istream& operator>>(std::istream& input, PropertiesValue& value)
{
	std::string tags_str;
	std::string adjuncts_str;
	input >> tags_str >> adjuncts_str;
	if(!input.fail())
	{
		value.update_tags(tags_str);
		value.update_adjuncts(adjuncts_str);
	}
	return input;
}

}

#endif /* PROPERTIES_VALUE_H_ */
