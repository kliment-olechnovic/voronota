#ifndef MODESCOMMON_HANDLE_CONTACT_H_
#define MODESCOMMON_HANDLE_CONTACT_H_

#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/io_utilities.h"

namespace modescommon
{

namespace contact
{

typedef auxiliaries::ChainResidueAtomDescriptor Comment;

struct ContactValue
{
	double area;
	double dist;
	std::set<std::string> tags;
	std::string graphics;

	ContactValue() : area(0.0), dist(0.0)
	{
	}

	void add(const ContactValue& v)
	{
		area+=v.area;
		dist=(dist<=0.0 ? v.dist : std::min(dist, v.dist));
		tags.insert(v.tags.begin(), v.tags.end());
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
	}

	void tag(const std::string& str)
	{
		if(!str.empty())
		{
			std::string refined_input_str=str;
			bool filled=false;
			for(std::size_t i=0;i<refined_input_str.size();i++)
			{
				char& s=refined_input_str[i];
				if(s=='.' || s==';' || s==',')
				{
					s=' ';
				}
				else
				{
					filled=true;
				}
			}
			if(filled)
			{
				std::istringstream input(refined_input_str);
				while(input.good())
				{
					std::string marker;
					input >> marker;
					if(!marker.empty())
					{
						tags.insert(marker);
					}
				}
			}
		}
	}
};

template<typename T>
inline T refine_pair(const T& p, const bool reverse)
{
	if(reverse)
	{
		return T(p.second, p.first);
	}
	else
	{
		return p;
	}
}

template<typename T>
inline T refine_pair_by_ordering(const T& p)
{
	return refine_pair(p, p.second<p.first);
}

inline void print_contact_record(const std::pair<Comment, Comment>& comments, const ContactValue& value, const bool preserve_graphics, std::ostream& output)
{
	output << comments.first.str() << " " << comments.second.str() << " " << value.area << " " << value.dist << " " << (value.tags.empty() ? "." : "");
	for(std::set<std::string>::const_iterator it=value.tags.begin();it!=value.tags.end();++it)
	{
		output << (it==value.tags.begin() ? "" : ";") << (*it);
	}
	if(preserve_graphics && !value.graphics.empty())
	{
		output << " \"";
		output << value.graphics;
		output << "\"";
	}
	output << "\n";
}

inline bool add_contacts_record_from_stream_to_map(std::istream& input, std::map< std::pair<Comment, Comment>, ContactValue >& map_of_records)
{
	std::pair<std::string, std::string> comment_strings;
	ContactValue value;
	input >> comment_strings.first >> comment_strings.second >> value.area >> value.dist;
	{
		std::string tags;
		input >> tags;
		value.tag(tags);
	}
	if(input.good())
	{
		std::getline(input, value.graphics, '"');
		std::getline(input, value.graphics, '"');
	}
	if(!input.fail() && !comment_strings.first.empty() && !comment_strings.second.empty())
	{
		const std::pair<Comment, Comment> comments(Comment::from_str(comment_strings.first), Comment::from_str(comment_strings.second));
		if(comments.first.valid() && comments.second.valid())
		{
			map_of_records[refine_pair_by_ordering(comments)]=value;
			return true;
		}
	}
	return false;
}

inline bool add_contacts_name_pair_from_stream_to_set(std::istream& input, std::set< std::pair<Comment, Comment> >& set_of_name_pairs)
{
	std::pair<std::string, std::string> comment_strings;
	input >> comment_strings.first >> comment_strings.second;
	if(!input.fail() && !comment_strings.first.empty() && !comment_strings.second.empty())
	{
		const std::pair<Comment, Comment> comments(Comment::from_str(comment_strings.first), Comment::from_str(comment_strings.second));
		if(comments.first.valid() && comments.second.valid())
		{
			set_of_name_pairs.insert(refine_pair_by_ordering(comments));
			return true;
		}
	}
	return false;
}

}

}

#endif /* MODESCOMMON_HANDLE_CONTACT_H_ */
