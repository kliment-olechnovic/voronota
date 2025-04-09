#ifndef SCRIPTING_VCBLOCKS_OF_DATA_MANAGER_H_
#define SCRIPTING_VCBLOCKS_OF_DATA_MANAGER_H_

#include "data_manager.h"

namespace voronota
{

namespace scripting
{

class VCBlocksOfDataManager
{
public:
	static std::size_t null_id()
	{
		return std::numeric_limits<std::size_t>::max();
	}

	struct Parameters
	{
		Parameters()
		{
		}
	};

	struct RRContactDescriptor
	{
		std::size_t rr_pair[2];
		std::vector<std::size_t> aa_contact_ids;
	};

	struct VCBlock
	{
		std::size_t residue_id_main[2];
		std::vector<std::size_t> residue_ids_surrounding;
		std::size_t rr_contact_descriptor_id_main;
		std::vector<std::size_t> rr_contact_descriptor_ids_surrounding[2];
		std::vector<std::size_t> rr_contact_descriptor_ids_capping[2];
	};

	struct Result
	{
		std::vector<RRContactDescriptor> rr_contact_descriptors;
		std::vector<VCBlock> vcblocks;

		Result()
		{
		}
	};

	static void construct_result(const Parameters& /*params*/, const DataManager& data_manager, Result& result)
	{
		result=Result();

		data_manager.assert_primary_structure_info_valid();
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_adjacencies_availability();

		std::map< std::vector<std::size_t>,  std::vector<std::size_t> > map_of_rr_pairs_to_aa_contact_ids;

		for(std::size_t i=0;i<data_manager.contacts().size();i++)
		{
			const Contact& contact=data_manager.contacts()[i];
			if(!contact.solvent())
			{
				std::vector<std::size_t> residue_pair_ids(2);
				residue_pair_ids[0]=data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[0]];
				residue_pair_ids[1]=data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[1]];
				if(residue_pair_ids[0]!=residue_pair_ids[1])
				{
					std::sort(residue_pair_ids.begin(), residue_pair_ids.end());
					map_of_rr_pairs_to_aa_contact_ids[residue_pair_ids].push_back(i);
				}
			}
		}

		result.rr_contact_descriptors.resize(map_of_rr_pairs_to_aa_contact_ids.size());

		std::map< std::vector<std::size_t>, std::size_t > map_of_rr_pairs_to_rr_contact_descriptors;

		{
			std::size_t i=0;
			for(std::map< std::vector<std::size_t>,  std::vector<std::size_t> >::const_iterator it=map_of_rr_pairs_to_aa_contact_ids.begin();it!=map_of_rr_pairs_to_aa_contact_ids.end();++it)
			{
				RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
				rrcd.rr_pair[0]=it->first[0];
				rrcd.rr_pair[1]=it->first[1];
				rrcd.aa_contact_ids=it->second;
				map_of_rr_pairs_to_rr_contact_descriptors[it->first]=i;
				i++;
			}
		}

		std::vector< std::set<std::size_t> > graph_of_residues(data_manager.primary_structure_info().residues.size());

		for(std::size_t i=0;i<result.rr_contact_descriptors.size();i++)
		{
			const RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
			graph_of_residues[rrcd.rr_pair[0]].insert(rrcd.rr_pair[1]);
			graph_of_residues[rrcd.rr_pair[1]].insert(rrcd.rr_pair[0]);
		}

		result.vcblocks.resize(result.rr_contact_descriptors.size());

		for(std::size_t i=0;i<result.rr_contact_descriptors.size();i++)
		{
			const RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
			VCBlock& vcblock=result.vcblocks[i];
			vcblock.residue_id_main[0]=rrcd.rr_pair[0];
			vcblock.residue_id_main[1]=rrcd.rr_pair[1];
			vcblock.rr_contact_descriptor_id_main=i;

			std::set<std::size_t> shared_neighbors;
			std::set<std::size_t> unique_neighbors[2];
			{
				std::map<std::size_t, int> neighboring_residue_counts;
				for(int j=0;j<2;j++)
				{
					std::set<std::size_t>& neighbors=graph_of_residues[vcblock.residue_id_main[j]];
					for(std::set<std::size_t>::const_iterator it=neighbors.begin();it!=neighbors.end();++it)
					{
						neighboring_residue_counts[*it]++;
					}
				}
				for(std::map<std::size_t, int>::const_iterator it=neighboring_residue_counts.begin();it!=neighboring_residue_counts.end();++it)
				{
					if(it->second==2)
					{
						shared_neighbors.insert(it->first);
					}
				}
				for(int j=0;j<2;j++)
				{
					std::set<std::size_t>& neighbors=graph_of_residues[vcblock.residue_id_main[j]];
					for(std::set<std::size_t>::const_iterator it=neighbors.begin();it!=neighbors.end();++it)
					{
						if((*it)!=vcblock.residue_id_main[j==0 ? 1 : 0] && shared_neighbors.count(*it)==0)
						{
							unique_neighbors[j].insert(*it);
						}
					}
				}
			}

			vcblock.residue_ids_surrounding.insert(vcblock.residue_ids_surrounding.end(), shared_neighbors.begin(), shared_neighbors.end());

			for(int j=0;j<2;j++)
			{
				vcblock.rr_contact_descriptor_ids_surrounding[j].reserve(shared_neighbors.size());
				for(std::set<std::size_t>::const_iterator it=shared_neighbors.begin();it!=shared_neighbors.end();++it)
				{
					std::vector<std::size_t> residue_pair_ids(2);
					residue_pair_ids[0]=vcblock.residue_id_main[j];
					residue_pair_ids[1]=(*it);
					std::sort(residue_pair_ids.begin(), residue_pair_ids.end());
					vcblock.rr_contact_descriptor_ids_surrounding[j].push_back(map_of_rr_pairs_to_rr_contact_descriptors[residue_pair_ids]);
				}
				vcblock.rr_contact_descriptor_ids_capping[j].reserve(unique_neighbors[j].size());
				for(std::set<std::size_t>::const_iterator it=unique_neighbors[j].begin();it!=unique_neighbors[j].end();++it)
				{
					std::vector<std::size_t> residue_pair_ids(2);
					residue_pair_ids[0]=vcblock.residue_id_main[j];
					residue_pair_ids[1]=(*it);
					std::sort(residue_pair_ids.begin(), residue_pair_ids.end());
					vcblock.rr_contact_descriptor_ids_capping[j].push_back(map_of_rr_pairs_to_rr_contact_descriptors[residue_pair_ids]);
				}
			}
		}
	}
};

}

}

#endif /* SCRIPTING_VCBLOCKS_OF_DATA_MANAGER_H_ */
