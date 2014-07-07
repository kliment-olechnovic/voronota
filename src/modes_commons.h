#ifndef MODES_COMMONS_H_
#define MODES_COMMONS_H_

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

namespace modes_commons
{

template<typename SphereType>
inline bool add_sphere_from_stream_to_vector(std::istream& input, std::vector<SphereType>& spheres)
{
	SphereType sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	if(!input.fail())
	{
		spheres.push_back(sphere);
		return true;
	}
	return false;
}

inline bool assert_options(
		const auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions& basic_map_of_option_descriptions,
		const auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions& advanced_map_of_option_descriptions,
		const auxiliaries::ProgramOptionsHandler& poh,
		const bool allow_unrecognized_options)
{
	if(poh.contains_option("--help"))
	{
		if(!basic_map_of_option_descriptions.empty())
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions("", basic_map_of_option_descriptions, std::cerr);
		}
		if(!advanced_map_of_option_descriptions.empty())
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions("", advanced_map_of_option_descriptions, std::cerr);
		}
		return false;
	}
	else
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions.insert(advanced_map_of_option_descriptions.begin(), advanced_map_of_option_descriptions.end());
		poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions, allow_unrecognized_options);
		return true;
	}
}

}

#endif /* MODES_COMMONS_H_ */
