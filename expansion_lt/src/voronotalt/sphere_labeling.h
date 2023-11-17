#ifndef VORONOTALT_SPHERE_LABELING_H_
#define VORONOTALT_SPHERE_LABELING_H_

#include <string>
#include <vector>
#include <map>

namespace voronotalt
{

struct SphereLabel
{
	std::string chain_id;
	std::string residue_id;
	std::string atom_name;
};

inline int assign_groups_to_sphere_labels_by_chain(const std::vector<SphereLabel>& sphere_labels, std::vector<int>& groups)
{
	groups.clear();
	groups.resize(sphere_labels.size(), 0);
	std::map<std::string, int> map_of_chains_to_groups;
	for(std::size_t i=0;i<sphere_labels.size();i++)
	{
		const SphereLabel& sl=sphere_labels[i];
		std::map<std::string, int>::const_iterator it=map_of_chains_to_groups.find(sl.chain_id);
		if(it==map_of_chains_to_groups.end())
		{
			groups[i]=static_cast<int>(map_of_chains_to_groups.size());
			map_of_chains_to_groups[sl.chain_id]=groups[i];
		}
		else
		{
			groups[i]=it->second;
		}
	}

	return static_cast<int>(map_of_chains_to_groups.size());
}

inline int assign_groups_to_sphere_labels_by_residue(const std::vector<SphereLabel>& sphere_labels, std::vector<int>& groups)
{
	groups.clear();
	groups.resize(sphere_labels.size(), 0);
	std::map< std::pair<std::string, std::string>, int> map_of_residues_to_groups;
	for(std::size_t i=0;i<sphere_labels.size();i++)
	{
		const SphereLabel& sl=sphere_labels[i];
		const std::pair<std::string, std::string> chain_residue_id(sl.chain_id, sl.residue_id);
		std::map< std::pair<std::string, std::string>, int>::const_iterator it=map_of_residues_to_groups.find(chain_residue_id);
		if(it==map_of_residues_to_groups.end())
		{
			groups[i]=static_cast<int>(map_of_residues_to_groups.size());
			map_of_residues_to_groups[chain_residue_id]=groups[i];
		}
		else
		{
			groups[i]=it->second;
		}
	}

	return static_cast<int>(map_of_residues_to_groups.size());
}

}

#endif /* VORONOTALT_SPHERE_LABELING_H_ */
