#ifndef MODESCOMMON_HANDLE_BALL_H_
#define MODESCOMMON_HANDLE_BALL_H_

#include "handle_annotations.h"

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
};

template<typename T>
inline void print_ball_record(const T& name, const BallValue& value, std::ostream& output)
{
	output << name.str() << " " << value.x << " " << value.y << " " << value.z << " " << value.r;
	output << " " << (value.tags.empty() ? std::string(".") : auxiliaries::print_set_to_string(value.tags, ";"));
	output << " " << (value.adjuncts.empty() ? std::string(".") : auxiliaries::print_map_to_string(value.adjuncts, ";"));
	output << "\n";
}

template<typename T>
inline bool add_ball_record_from_stream_to_vector(std::istream& input, std::vector< std::pair<T, BallValue> >& vector_of_records)
{
	std::string name_string;
	BallValue value;
	input >> name_string >> value.x >> value.y >> value.z >> value.r;
	{
		std::string tags;
		input >> tags;
		update_set_of_tags(value.tags, tags);
	}
	{
		std::string adjuncts;
		input >> adjuncts;
		update_map_of_adjuncts(value.adjuncts, adjuncts);
	}
	if(!input.fail() && !name_string.empty())
	{
		const T name=T::from_str(name_string);
		if(name.valid())
		{
			vector_of_records.push_back(std::make_pair(name, value));
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
