#ifndef COMMON_CONTACT_VALUE_H_
#define COMMON_CONTACT_VALUE_H_

#include "properties_value.h"

namespace voronota
{

namespace common
{

struct ContactValue
{
	double area;
	double dist;
	bool accumulated;
	PropertiesValue props;
	std::string graphics;

	ContactValue() : area(0.0), dist(0.0), accumulated(false)
	{
	}

	void add(const ContactValue& v)
	{
		area+=v.area;
		dist=(!accumulated ? v.dist : std::min(dist, v.dist));
		props.tags.insert(v.props.tags.begin(), v.props.tags.end());
		for(std::map<std::string, double>::const_iterator it=v.props.adjuncts.begin();it!=v.props.adjuncts.end();++it)
		{
			props.adjuncts[it->first]+=it->second;
		}
		if(!v.graphics.empty())
		{
			if(graphics.empty())
			{
				graphics=v.graphics;
			}
			else
			{
				if(graphics[graphics.size()-1]!=' ' || v.graphics[0]!=' ')
				{
					graphics+=" ";
				}
				graphics+=v.graphics;
			}
		}
		accumulated=true;
	}
};

inline bool& enabled_output_of_ContactValue_graphics()
{
	static bool enabled_output=true;
	return enabled_output;
}

inline std::ostream& operator<<(std::ostream& output, const ContactValue& value)
{
	output << value.area << " " << value.dist << " " << value.props;
	if(enabled_output_of_ContactValue_graphics() && !value.graphics.empty())
	{
		output << " \"";
		output << value.graphics;
		output << "\"";
	}
	return output;
}

inline std::istream& operator>>(std::istream& input, ContactValue& value)
{
	input >> value.area >> value.dist >> value.props;
	if(input.good())
	{
		std::getline(input, value.graphics, '"');
		std::getline(input, value.graphics, '"');
	}
	return input;
}

}

}

#endif /* COMMON_CONTACT_VALUE_H_ */
