#ifndef MODESCOMMON_HANDLE_BALL_H_
#define MODESCOMMON_HANDLE_BALL_H_

#include "auxiliaries/io_utilities.h"

namespace modescommon
{

struct BallValue
{
	double x;
	double y;
	double z;
	double r;
	std::set<std::string> tags;
	std::map<std::string, double> adjuncts;

	BallValue() : x(0.0), y(0.0), z(0.0), r(0.0)
	{
	}

	void set_tags(const std::string& str)
	{
		if(!str.empty())
		{
			const std::set<std::string> input_tags=auxiliaries::read_set_from_string<std::string>(str, ".;,");
			if(!input_tags.empty())
			{
				tags.insert(input_tags.begin(), input_tags.end());
			}
		}
	}

	void set_adjuncts(const std::string& str)
	{
		if(!str.empty())
		{
			const std::map<std::string, double> input_adjuncts=auxiliaries::read_map_from_string<std::string, double>(str, ".;,");
			for(std::map<std::string, double>::const_iterator it=input_adjuncts.begin();it!=input_adjuncts.end();++it)
			{
				adjuncts[it->first]=it->second;
			}
		}
	}
};

template<typename T>
inline void print_ball_record(const T& comment, const BallValue& value, std::ostream& output)
{
	output << comment.str() << " " << value.x << " " << value.y << " " << value.z << " " << value.r;
	output << " " << (value.tags.empty() ? std::string(".") : auxiliaries::print_set_to_string(value.tags, ";"));
	output << " " << (value.adjuncts.empty() ? std::string(".") : auxiliaries::print_map_to_string(value.adjuncts, ";"));
	output << "\n";
}

template<typename T>
inline bool add_ball_record_from_stream_to_vector(std::istream& input, std::vector< std::pair<T, BallValue> >& vector_of_records)
{
	std::string comment_string;
	BallValue value;
	input >> comment_string >> value.x >> value.y >> value.z >> value.r;
	{
		std::string tags;
		input >> tags;
		value.set_tags(tags);
	}
	{
		std::string adjuncts;
		input >> adjuncts;
		value.set_adjuncts(adjuncts);
	}
	if(!input.fail() && !comment_string.empty())
	{
		const T comment=T::from_str(comment_string);
		if(comment.valid())
		{
			vector_of_records.push_back(std::make_pair(comment, value));
			return true;
		}
	}
	return false;
}

template<typename T, typename SphereType>
inline void collect_spheres_from_vector_of_ball_records(const std::vector< std::pair<T, BallValue> >& vector_of_records, std::vector<SphereType>& spheres)
{
	spheres.resize(vector_of_records.size());
	for(std::size_t i=0;i<vector_of_records.size();i++)
	{
		spheres[i]=SphereType(vector_of_records[i].second);
	}
}

}

#endif /* MODESCOMMON_HANDLE_BALL_H_ */
