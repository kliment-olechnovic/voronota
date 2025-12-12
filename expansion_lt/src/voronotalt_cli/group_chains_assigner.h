#ifndef VORONOTALT_CLI_GROUP_CHAINS_ASSIGNER_H_
#define VORONOTALT_CLI_GROUP_CHAINS_ASSIGNER_H_

#include <map>
#include <cctype>

#include "filtering_by_sphere_labels.h"

namespace voronotalt
{

class GroupChainsAssigner
{
public:
	GroupChainsAssigner()
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

	bool add_rule(const std::string& groupname, const std::string& expression) noexcept
	{
		if(groupname.empty() || expression.empty())
		{
			return false;
		}

		FilteringBySphereLabels::ExpressionForSingle efs(expression);
		if(!efs.valid())
		{
			return false;
		}

		map_for_singles_.push_back(std::pair<FilteringBySphereLabels::ExpressionForSingle, std::string>(efs, groupname));

		return true;
	}

	bool add_rule(const std::string& input) noexcept
	{
		std::string groupname;
		std::string expression;

		if(!read_config_line(input, groupname, expression))
		{
			return false;
		}

		return add_rule(groupname, expression);
	}

	bool empty() const noexcept
	{
		return map_for_singles_.empty();
	}

	bool assign_group_chains(std::vector<SphereLabeling::SphereLabel>& sphere_labels) const noexcept
	{
		if(empty())
		{
			return false;
		}
		bool some_groups_assigned=false;
		for(std::size_t i=0;i<sphere_labels.size();i++)
		{
			SphereLabeling::SphereLabel& sl=sphere_labels[i];
			std::string groupname;
			for(std::vector< std::pair<FilteringBySphereLabels::ExpressionForSingle, std::string> >::const_reverse_iterator rit=map_for_singles_.rbegin();rit!=map_for_singles_.rend();++rit)
			{
				if(rit->first.filter(sl).expression_matched)
				{
					groupname=rit->second;
				}
			}
			if(!groupname.empty())
			{
				if(!sl.expanded_residue_id.parsed)
				{
					sl.expanded_residue_id=SphereLabeling::parse_expanded_residue_id(sl.residue_id);
				}
				SphereLabeling::form_residue_id_string(sl.expanded_residue_id.rnum, (sl.chain_id+"_"+sl.expanded_residue_id.icode), sl.expanded_residue_id.rname, sl.residue_id);
				sl.chain_id=groupname;
				sl.expanded_residue_id=SphereLabeling::parse_expanded_residue_id(sl.residue_id);
				some_groups_assigned=true;
			}
		}
		return some_groups_assigned;
	}

private:
	static bool read_config_line(const std::string& input, std::string& groupname, std::string& expression) noexcept
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
	    groupname=input.substr(start, pos-start);

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

	    return (!groupname.empty() && !expression.empty());
	}

	std::vector< std::pair<FilteringBySphereLabels::ExpressionForSingle, std::string> > map_for_singles_;
};

}

#endif /* VORONOTALT_CLI_GROUP_CHAINS_ASSIGNER_H_ */
