#ifndef AUXILIARIES_ATOM_RADIUS_ASSIGNER_H_
#define AUXILIARIES_ATOM_RADIUS_ASSIGNER_H_

#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace voronota
{

namespace auxiliaries
{

class AtomRadiusAssigner
{
public:
	explicit AtomRadiusAssigner(const double default_radius) : default_radius(default_radius)
	{
	}

	bool add_radius_by_descriptor(const std::string& resName_pattern, const std::string& name_pattern, const double radius)
	{
		if(!resName_pattern.empty() && !name_pattern.empty())
		{
			radii_by_descriptors[Descriptor(resName_pattern, name_pattern)]=radius;
			return true;
		}
		return false;
	}

	double get_atom_radius(const std::string& resName, const std::string& name) const
	{
		std::vector<std::string> ordered_resName_patterns;
		ordered_resName_patterns.push_back(resName);
		ordered_resName_patterns.push_back("*");

		std::vector<std::string> ordered_name_patterns;
		ordered_name_patterns.push_back(name);
		for(std::size_t i=0;i<name.size();i++)
		{
			ordered_name_patterns.push_back(name.substr(0, name.size()-i)+"*");
		}
		ordered_name_patterns.push_back("*");

		for(std::size_t i=0;i<ordered_resName_patterns.size();i++)
		{
			for(std::size_t j=0;j<ordered_name_patterns.size();j++)
			{
				std::map<Descriptor, double>::const_iterator it=radii_by_descriptors.find(Descriptor(ordered_resName_patterns[i], ordered_name_patterns[j]));
				if(it!=radii_by_descriptors.end())
				{
					return it->second;
				}
			}
		}
		return default_radius;
	}

	static bool add_descriptor_and_radius_from_stream_to_atom_radius_assigner(std::istream& input, AtomRadiusAssigner& atom_radius_assigner)
	{
		std::string resName;
		std::string name;
		double radius=0.0;
		input >> resName >> name >> radius;
		if(!input.fail())
		{
			atom_radius_assigner.add_radius_by_descriptor(resName, name, radius);
			return true;
		}
		return false;
	}

private:
	struct Descriptor
	{
		std::string resName_pattern;
		std::string name_pattern;

		Descriptor(const std::string& resName_pattern, const std::string& name_pattern) : resName_pattern(resName_pattern), name_pattern(name_pattern)
		{
		}

		bool operator<(const Descriptor& d) const
		{
			return ( resName_pattern<d.resName_pattern || (resName_pattern==d.resName_pattern && (name_pattern<d.name_pattern)) );
		}
	};

	double default_radius;
	std::map<Descriptor, double> radii_by_descriptors;
};

}

}

#endif /* AUXILIARIES_ATOM_RADIUS_ASSIGNER_H_ */
