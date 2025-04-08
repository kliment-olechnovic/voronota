#ifndef SCRIPTING_VVBLOCKS_OF_DATA_MANAGER_H_
#define SCRIPTING_VVBLOCKS_OF_DATA_MANAGER_H_

#include "data_manager.h"

namespace voronota
{

namespace scripting
{

class VVBlocksOfDataManager
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

	struct VVBlock
	{
		std::size_t residue_ids_main[4];
		std::size_t residue_ids_side[4];
		std::size_t rr_contact_descriptor_ids_main[6];
		std::size_t rr_contact_descriptor_ids_side[12];
	};

	struct Result
	{
		std::vector<RRContactDescriptor> rr_contact_descriptors;
		std::vector<VVBlock> vvblocks;

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

		std::vector<SortableIDOfRRContactDescriptors> sortable_ids_of_rr_contact_descriptors(result.rr_contact_descriptors.size());

		for(std::size_t i=0;i<result.rr_contact_descriptors.size();i++)
		{
			const RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
			SortableIDOfRRContactDescriptors& sid_rr=sortable_ids_of_rr_contact_descriptors[i];
			const common::ConstructionOfPrimaryStructure::Residue& residue_a=data_manager.primary_structure_info().residues[rrcd.rr_pair[0]];
			const common::ConstructionOfPrimaryStructure::Residue& residue_b=data_manager.primary_structure_info().residues[rrcd.rr_pair[1]];
			sid_rr.seq_sep=std::abs(common::ConstructionOfPrimaryStructure::Residue::distance_in_segment(residue_a, residue_b));
			sid_rr.seq_sep=std::max(1, std::min(sid_rr.seq_sep, 5));
			sid_rr.area=0.0;
			for(std::vector<std::size_t>::const_iterator it=rrcd.aa_contact_ids.begin();it!=rrcd.aa_contact_ids.end();++it)
			{
				const Contact& contact=data_manager.contacts()[*it];
				sid_rr.area+=contact.value.area;
			}
			sid_rr.residue_types.resize(2);
			sid_rr.residue_types[0]=residue_a.chain_residue_descriptor.resName;
			sid_rr.residue_types[1]=residue_b.chain_residue_descriptor.resName;
			sid_rr.global_id=i;
			sid_rr.ensure_validity();
		}

		std::vector< std::set<std::size_t> > graph_of_residues(data_manager.primary_structure_info().residues.size());

		for(std::size_t i=0;i<result.rr_contact_descriptors.size();i++)
		{
			const RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
			graph_of_residues[rrcd.rr_pair[0]].insert(rrcd.rr_pair[1]);
			graph_of_residues[rrcd.rr_pair[1]].insert(rrcd.rr_pair[0]);
		}

		std::vector< std::vector<std::size_t> > rrrr_quadruples;

		{
			std::set< std::vector<std::size_t> > set_of_rrrr_quadruples;
			for(std::size_t i=0;i<graph_of_residues.size();i++)
			{
				const std::set<std::size_t>& neighbors=graph_of_residues[i];
				for(std::set<std::size_t>::const_iterator it1=neighbors.begin();it1!=neighbors.end();++it1)
				{
					std::set<std::size_t>::const_iterator it2=it1;
					++it2;
					for(;it2!=neighbors.end();++it2)
					{
						if(graph_of_residues[*it1].count(*it2)>0)
						{
							std::set<std::size_t>::const_iterator it3=it2;
							++it3;
							for(;it3!=neighbors.end();++it3)
							{
								if(graph_of_residues[*it1].count(*it3)>0 && graph_of_residues[*it2].count(*it3)>0)
								{
									std::vector<std::size_t> q(4);
									q[0]=i;
									q[1]=(*it1);
									q[2]=(*it2);
									q[3]=(*it3);
									std::sort(q.begin(), q.end());
									set_of_rrrr_quadruples.insert(q);
								}
							}
						}
					}
				}
			}
			rrrr_quadruples.insert(rrrr_quadruples.end(), set_of_rrrr_quadruples.begin(), set_of_rrrr_quadruples.end());
		}

		std::map< std::vector<std::size_t>, std::set<std::size_t> > map_of_rrr_triples_to_r_neigbors;

		for(std::size_t i=0;i<rrrr_quadruples.size();i++)
		{
			const std::vector<std::size_t>& q=rrrr_quadruples[i];
			for(std::size_t j=0;j<4;j++)
			{
				std::vector<std::size_t> t;
				t.reserve(3);
				for(std::size_t k=0;k<4;k++)
				{
					if(k!=j)
					{
						t.push_back(q[k]);
					}
				}
				std::sort(t.begin(), t.end());
				map_of_rrr_triples_to_r_neigbors[t].insert(q[j]);
			}
		}

		result.vvblocks.resize(rrrr_quadruples.size());

		for(std::size_t i=0;i<rrrr_quadruples.size();i++)
		{
			VVBlock& vvblock=result.vvblocks[i];
			{
				const std::vector<std::size_t>& q=rrrr_quadruples[i];
				std::vector<SortableIDOfResidue> sortable_ids_from_q(4);
				for(std::size_t j=0;j<4;j++)
				{
					SortableIDOfResidue& sid_r=sortable_ids_from_q[j];
					sid_r.rr_contacts_sortable_ids.reserve(3);
					for(std::size_t k=0;k<4;k++)
					{
						if(k!=j)
						{
							std::vector<std::size_t> rr_pair(2);
							rr_pair[0]=q[j];
							rr_pair[1]=q[k];
							std::sort(rr_pair.begin(), rr_pair.end());
							sid_r.rr_contacts_sortable_ids.push_back(sortable_ids_of_rr_contact_descriptors[map_of_rr_pairs_to_rr_contact_descriptors[rr_pair]]);
						}
					}
					const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[q[j]];
					sid_r.residue_type=residue.chain_residue_descriptor.resName;
					sid_r.global_id=q[j];
					sid_r.ensure_validity();
				}
				std::sort(sortable_ids_from_q.begin(), sortable_ids_from_q.end());

				for(std::size_t j=0;j<4;j++)
				{
					vvblock.residue_ids_main[j]=sortable_ids_from_q[j].global_id;
				}
			}
			{
				std::size_t e=0;
				for(std::size_t j=0;j<4;j++)
				{
					for(std::size_t k=j+1;k<4;k++)
					{
						std::vector<std::size_t> rr_pair(2);
						rr_pair[0]=vvblock.residue_ids_main[j];
						rr_pair[1]=vvblock.residue_ids_main[k];
						std::sort(rr_pair.begin(), rr_pair.end());
						vvblock.rr_contact_descriptor_ids_main[e++]=map_of_rr_pairs_to_rr_contact_descriptors[rr_pair];
					}
				}
			}
			{
				std::size_t e=0;
				for(std::size_t j=0;j<4;j++)
				{
					vvblock.residue_ids_side[j]=null_id();
					std::vector<std::size_t> t;
					t.reserve(3);
					for(std::size_t k=0;k<4;k++)
					{
						if(k!=j)
						{
							t.push_back(vvblock.residue_ids_main[k]);
						}
					}
					std::sort(t.begin(), t.end());
					std::map< std::vector<std::size_t>, std::set<std::size_t> >::const_iterator it=map_of_rrr_triples_to_r_neigbors.find(t);
					if(it!=map_of_rrr_triples_to_r_neigbors.end())
					{
						const std::set<std::size_t>& r_neighbors=it->second;
						for(std::set<std::size_t>::const_iterator jt=r_neighbors.begin();jt!=r_neighbors.end() && vvblock.residue_ids_side[j]==null_id();++jt)
						{
							if((*jt)!=vvblock.residue_ids_main[j])
							{
								vvblock.residue_ids_side[j]=(*jt);
							}
						}
					}
					if(vvblock.residue_ids_side[j]==null_id())
					{
						vvblock.rr_contact_descriptor_ids_side[e++]=null_id();
						vvblock.rr_contact_descriptor_ids_side[e++]=null_id();
						vvblock.rr_contact_descriptor_ids_side[e++]=null_id();
					}
					else
					{
						for(std::size_t k=0;k<3;k++)
						{
							std::vector<std::size_t> rr_pair(2);
							rr_pair[0]=vvblock.residue_ids_side[j];
							rr_pair[1]=t[k];
							std::sort(rr_pair.begin(), rr_pair.end());
							vvblock.rr_contact_descriptor_ids_side[e++]=map_of_rr_pairs_to_rr_contact_descriptors[rr_pair];
						}
					}
				}
			}
		}
	}

private:
	struct SortableIDOfRRContactDescriptors
	{
		int seq_sep;
		double area;
		std::vector<std::string> residue_types;
		std::size_t global_id;

		SortableIDOfRRContactDescriptors() : seq_sep(0), area(0.0), global_id(0)
		{
		}

		void ensure_validity()
		{
			std::sort(residue_types.begin(), residue_types.end());
		}

		bool operator<(const SortableIDOfRRContactDescriptors& v) const
		{
			if(seq_sep>v.seq_sep)
			{
				return true;
			}
			else if(seq_sep==v.seq_sep)
			{
				if(area>v.area)
				{
					return true;
				}
				else if(area==v.area)
				{
					if(residue_types<v.residue_types)
					{
						return true;
					}
					else if(residue_types==v.residue_types)
					{
						return (global_id<v.global_id);
					}
				}
			}
			return false;
		}

		bool operator==(const SortableIDOfRRContactDescriptors& v) const
		{
			return (seq_sep==v.seq_sep && area==v.area && residue_types==v.residue_types && global_id==v.global_id);
		}
	};

	struct SortableIDOfResidue
	{
		std::vector<SortableIDOfRRContactDescriptors> rr_contacts_sortable_ids;
		std::string residue_type;
		std::size_t global_id;

		SortableIDOfResidue() : global_id(0)
		{
		}

		void ensure_validity()
		{
			std::sort(rr_contacts_sortable_ids.begin(), rr_contacts_sortable_ids.end());
		}

		bool operator<(const SortableIDOfResidue& v) const
		{
			if(rr_contacts_sortable_ids<v.rr_contacts_sortable_ids)
			{
				return true;
			}
			else if(rr_contacts_sortable_ids==v.rr_contacts_sortable_ids)
			{
				if(residue_type<v.residue_type)
				{
					return true;
				}
				else if(residue_type==v.residue_type)
				{
					return (global_id<v.global_id);
				}
			}
			return false;
		}
	};
};

}

}


#endif /* SCRIPTING_VVBLOCKS_OF_DATA_MANAGER_H_ */
