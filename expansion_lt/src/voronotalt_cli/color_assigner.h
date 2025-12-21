#ifndef VORONOTALT_CLI_COLOR_ASSIGNER_H_
#define VORONOTALT_CLI_COLOR_ASSIGNER_H_

#include <map>
#include <cctype>

#include "filtering_by_sphere_labels.h"

namespace voronotalt
{

class ColorAssigner
{
public:
	ColorAssigner() : default_color_(0x00FFFFFF)
	{
	}

	bool add_rules(const std::string& input_string) noexcept
	{
		if(input_string.empty())
		{
			return false;
		}
		int count=0;
		std::istringstream input(input_string);
		while(input.good())
		{
			std::string line;
			std::getline(input, line);
			if(!line.empty())
			{
				if(line.find_first_of(";")==std::string::npos)
				{
					if(add_rule(line))
					{
						++count;
					}
					else
					{
						return false;
					}
				}
				else
				{
					std::istringstream subinput(line);
					while(subinput.good())
					{
						std::string subline;
						std::getline(subinput, subline, ';');
						if(!subline.empty())
						{
							if(add_rule(subline))
							{
								++count;
							}
							else
							{
								return false;
							}
						}
					}
				}
			}
		}
		return (count>0);
	}

	bool add_rule(const std::string& category, const std::string& rep, const unsigned int color, const std::string& expression) noexcept
	{
		if(category.empty() || rep.empty())
		{
			return false;
		}

		if(category=="contact")
		{
			if(expression.empty())
			{
				map_for_rep_defaults_for_pairs_[rep]=color;
			}
			else
			{
				FilteringBySphereLabels::ExpressionForPair efp(expression);
				if(!efp.valid())
				{
					return false;
				}
				map_for_pairs_[rep].push_back(std::pair<FilteringBySphereLabels::ExpressionForPair, unsigned int>(efp, color));
			}
		}
		else if(category=="ball")
		{
			if(expression.empty())
			{
				map_for_rep_defaults_for_singles_[rep]=color;
			}
			else
			{
				FilteringBySphereLabels::ExpressionForSingle efs(expression);
				if(!efs.valid())
				{
					return false;
				}
				map_for_singles_[rep].push_back(std::pair<FilteringBySphereLabels::ExpressionForSingle, unsigned int>(efs, color));
			}
		}
		else if(category=="other" && expression.empty())
		{
			map_for_rep_defaults_for_other_[rep]=color;
		}
		else
		{
			return false;
		}

		return true;
	}

	bool add_rule(const std::string& category, const std::string& rep, const unsigned int color) noexcept
	{
		return add_rule(category, rep, color, std::string());
	}

	bool add_rule(const std::string& input) noexcept
	{
		std::string category;
		std::string rep;
		unsigned int color;
		std::string expression;

		if(!read_config_line(input, category, rep, color, expression))
		{
			return false;
		}

		return add_rule(category, rep, color, expression);
	}

	unsigned int get_color(const std::string& rep) const noexcept
	{
		{
			std::map<std::string, unsigned int>::const_iterator it=map_for_rep_defaults_for_pairs_.find(rep);
			if(it!=map_for_rep_defaults_for_pairs_.end())
			{
				return (it->second);
			}
		}

		{
			std::map<std::string, unsigned int>::const_iterator it=map_for_rep_defaults_for_singles_.find(rep);
			if(it!=map_for_rep_defaults_for_singles_.end())
			{
				return (it->second);
			}
		}

		{
			std::map<std::string, unsigned int>::const_iterator it=map_for_rep_defaults_for_other_.find(rep);
			if(it!=map_for_rep_defaults_for_other_.end())
			{
				return (it->second);
			}
		}

		return default_color_;
	}

	unsigned int get_color(const std::string& rep, const SphereLabeling::SphereLabel& a) const noexcept
	{
		std::map< std::string, std::vector< std::pair<FilteringBySphereLabels::ExpressionForSingle, unsigned int> > >::const_iterator it=map_for_singles_.find(rep);
		if(it==map_for_singles_.end())
		{
			return get_color(rep);
		}
		const std::vector< std::pair<FilteringBySphereLabels::ExpressionForSingle, unsigned int> >& v=it->second;
		for(std::vector< std::pair<FilteringBySphereLabels::ExpressionForSingle, unsigned int> >::const_reverse_iterator rit=v.rbegin();rit!=v.rend();++rit)
		{
			if(rit->first.filter(a).expression_matched)
			{
				return rit->second;
			}
		}
		return get_color(rep);
	}

	unsigned int get_color(const std::string& rep, const std::vector<SphereLabeling::SphereLabel>& sphere_labels, const unsigned int base_color, const UnsignedInt id) const noexcept
	{
		return (id<sphere_labels.size() ? get_color(rep, sphere_labels[id]) : base_color);
	}

	unsigned int get_color(const std::string& rep, const SphereLabeling::SphereLabel& a, const SphereLabeling::SphereLabel& b, const SimpleSphere& sa, const SimpleSphere& sb) const noexcept
	{
		std::map< std::string, std::vector< std::pair<FilteringBySphereLabels::ExpressionForPair, unsigned int> > >::const_iterator it=map_for_pairs_.find(rep);
		if(it==map_for_pairs_.end())
		{
			return get_color(rep);
		}
		const std::vector< std::pair<FilteringBySphereLabels::ExpressionForPair, unsigned int> >& v=it->second;
		for(std::vector< std::pair<FilteringBySphereLabels::ExpressionForPair, unsigned int> >::const_reverse_iterator rit=v.rbegin();rit!=v.rend();++rit)
		{
			if(rit->first.filter(a, b, sa, sb).expression_matched)
			{
				return rit->second;
			}
		}
		return get_color(rep);
	}

	unsigned int get_color(const std::string& rep, const std::vector<SphereLabeling::SphereLabel>& sphere_labels, const std::vector<SimpleSphere>& spheres, const unsigned int base_color, const UnsignedInt id_a, const UnsignedInt id_b) const noexcept
	{
		return ((id_a<sphere_labels.size() && id_b<sphere_labels.size() && id_a<spheres.size() && id_b<spheres.size()) ? get_color(rep, sphere_labels[id_a], sphere_labels[id_b], spheres[id_a], spheres[id_b]) : base_color);
	}

private:
	static bool read_unsigned_int_from_string_hex(const std::string& s, unsigned int& value) noexcept
	{
	    if(!(s.size()==6 || s.size()==8))
	    {
	        return false;
	    }
	    value=0;
	    for(std::size_t i=0;i<s.size();i++)
	    {
	        char c=s[i];
	        unsigned int digit=0;
	        if(c>='0' && c<='9')
	        {
	            digit=static_cast<unsigned int>(c-'0');
	        }
	        else if(c>='A' && c<='F')
	        {
	            digit=static_cast<unsigned int>(10)+static_cast<unsigned int>(c-'A');
	        }
	        else if(c>='a' && c<='f')
	        {
	            digit=static_cast<unsigned int>(10)+static_cast<unsigned int>(c-'a');
	        }
	        else
	        {
	            return false;
	        }
	        value=(value << 4)+digit;
	    }
	    return true;
	}

	static bool read_config_line(const std::string& input, std::string& category, std::string& rep, unsigned int& color, std::string& expression) noexcept
	{
	    const std::string::size_type n=input.size();
	    std::string::size_type start=0;
	    std::string::size_type pos=0;

	    while(pos<n && std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    if(pos>=n)
	    {
	        return false;
	    }
	    start=pos;
	    while(pos<n && !std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    category=input.substr(start, pos-start);

	    while(pos<n && std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    if(pos>=n)
	    {
	        return false;
	    }
	    start=pos;
	    while(pos<n && !std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    rep=input.substr(start, pos-start);

	    while(pos<n && std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    if(pos>=n)
	    {
	        return false;
	    }
	    start=pos;
		while(pos<n && !std::isspace(static_cast<unsigned char>(input[pos])))
		{
			++pos;
		}
		if(!read_unsigned_int_from_string_hex(input.substr(start, pos-start), color))
		{
			return false;
		}

	    while(pos<n && std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    if(pos>=n)
	    {
	    	expression.clear();
	    }
	    else
	    {
	    	expression=input.substr(pos);
	    }

	    return (color<=static_cast<unsigned int>(0xFFFFFFFF) && !category.empty() && !rep.empty());
	}

	std::map<std::string, unsigned int> map_for_rep_defaults_for_singles_;
	std::map<std::string, unsigned int> map_for_rep_defaults_for_pairs_;
	std::map<std::string, unsigned int> map_for_rep_defaults_for_other_;
	std::map< std::string, std::vector< std::pair<FilteringBySphereLabels::ExpressionForSingle, unsigned int> > > map_for_singles_;
	std::map< std::string, std::vector< std::pair<FilteringBySphereLabels::ExpressionForPair, unsigned int> > > map_for_pairs_;
	unsigned int default_color_;
};

}


#endif /* VORONOTALT_CLI_COLOR_ASSIGNER_H_ */
