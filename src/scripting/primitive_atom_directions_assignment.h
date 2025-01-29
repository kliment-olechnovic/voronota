#ifndef SCRIPTING_PRIMITIVE_ATOM_DIRECTIONS_ASSIGNMENT_H_
#define SCRIPTING_PRIMITIVE_ATOM_DIRECTIONS_ASSIGNMENT_H_

#include "data_manager.h"

namespace voronota
{

namespace scripting
{

class PrimitiveAtomDirectionsAssignment
{
public:
	struct Parameters
	{
		double max_bond_distance;
		std::size_t max_number_of_directional_neighbors;
		apollota::SimplePoint default_fallback_direction;

		Parameters() :
			max_bond_distance(1.7),
			max_number_of_directional_neighbors(2),
			default_fallback_direction(apollota::SimplePoint(1.0, 1.0, 1.0).unit())
		{
		}

		Parameters(const double max_bond_distance, const std::size_t max_number_of_directional_neighbors) :
			max_bond_distance(max_bond_distance),
			max_number_of_directional_neighbors(max_number_of_directional_neighbors)
		{
		}
	};

	struct Result
	{
		std::vector<std::size_t> counts_of_bonds;
		std::vector< std::vector<std::size_t> > directional_neighbors;
		std::vector< std::vector<apollota::SimplePoint> > basic_directions;

		Result()
		{
		}
	};

	static void construct_result(const Parameters& params, const DataManager& data_manager, Result& result)
	{
		result=Result();

		if(!(params.max_number_of_directional_neighbors>0))
		{
			throw std::runtime_error(std::string("Invalid max number of directional neighbors, must greater than 0."));
		}

		data_manager.assert_atoms_availability();
		data_manager.assert_primary_structure_info_valid();

		std::vector< std::set<std::size_t> > graph_direct(data_manager.atoms().size());
		for(std::size_t j=0;j<data_manager.primary_structure_info().residues.size();j++)
		{
			const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[j];
			for(std::size_t id_a=0;id_a<residue.atom_ids.size();id_a++)
			{
				for(std::size_t id_b=0;id_b<residue.atom_ids.size();id_b++)
				{
					const std::size_t atom_id_a=residue.atom_ids[id_a];
					const std::size_t atom_id_b=residue.atom_ids[id_b];
					if(atom_id_a!=atom_id_b)
					{
						if(apollota::distance_from_point_to_point(data_manager.atoms()[atom_id_a].value, data_manager.atoms()[atom_id_b].value)<params.max_bond_distance)
						{
							graph_direct[atom_id_a].insert(atom_id_b);
							graph_direct[atom_id_b].insert(atom_id_a);
						}
					}
				}
			}
		}

		std::vector< std::set<std::size_t> > graph_indirect(graph_direct.size());
		for(std::size_t i=0;i<graph_direct.size();i++)
		{
			const std::set<std::size_t>& direct_neighbors=graph_direct[i];
			std::set<std::size_t>& indirect_neighbors=graph_indirect[i];
			for(std::set<std::size_t>::const_iterator it1=direct_neighbors.begin();it1!=direct_neighbors.end();++it1)
			{
				const std::set<std::size_t>& neigbors_of_direct_neighbor=graph_direct[*it1];
				for(std::set<std::size_t>::const_iterator it2=neigbors_of_direct_neighbor.begin();it2!=neigbors_of_direct_neighbor.end();++it2)
				{
					if((*it2)!=i && direct_neighbors.count(*it2)==0)
					{
						indirect_neighbors.insert(*it2);
					}
				}
			}
		}

		std::vector< std::set<std::size_t> > graph_far(graph_direct.size());
		for(std::size_t j=0;j<data_manager.primary_structure_info().residues.size();j++)
		{
			const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[j];
			for(std::size_t id_a=0;id_a<residue.atom_ids.size();id_a++)
			{
				for(std::size_t id_b=0;id_b<residue.atom_ids.size();id_b++)
				{
					const std::size_t atom_id_a=residue.atom_ids[id_a];
					const std::size_t atom_id_b=residue.atom_ids[id_b];
					if(atom_id_a!=atom_id_b)
					{
						if(graph_direct[atom_id_a].count(atom_id_b)==0 || graph_indirect[atom_id_a].count(atom_id_b)==0)
						{
							graph_far[atom_id_a].insert(atom_id_b);
							graph_far[atom_id_b].insert(atom_id_a);
						}
					}
				}
			}
		}

		result.counts_of_bonds.resize(data_manager.atoms().size());
		result.directional_neighbors.resize(data_manager.atoms().size());
		result.basic_directions.resize(data_manager.atoms().size(), std::vector<apollota::SimplePoint>(3, params.default_fallback_direction));
		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			const std::set<std::size_t>& direct_neighbors=graph_direct[i];
			const std::set<std::size_t>& indirect_neighbors=graph_indirect[i];
			const std::set<std::size_t>& far_neighbors=graph_far[i];

			result.counts_of_bonds[i]=direct_neighbors.size();

			{
				std::vector< std::pair< std::pair<int, std::string>, std::size_t > > neighbors_in_residue;

				for(std::set<std::size_t>::const_iterator it=direct_neighbors.begin();it!=direct_neighbors.end();++it)
				{
					neighbors_in_residue.push_back(std::pair< std::pair<int, std::string>, std::size_t >(std::pair<int, std::string>(1, data_manager.atoms()[*it].crad.name), *it));
				}

				for(std::set<std::size_t>::const_iterator it=indirect_neighbors.begin();it!=indirect_neighbors.end() && neighbors_in_residue.size()<params.max_number_of_directional_neighbors;++it)
				{
					neighbors_in_residue.push_back(std::pair< std::pair<int, std::string>, std::size_t >(std::pair<int, std::string>(2, data_manager.atoms()[*it].crad.name), *it));
				}

				for(std::set<std::size_t>::const_iterator it=far_neighbors.begin();it!=far_neighbors.end() && neighbors_in_residue.size()<params.max_number_of_directional_neighbors;++it)
				{
					neighbors_in_residue.push_back(std::pair< std::pair<int, std::string>, std::size_t >(std::pair<int, std::string>(3, data_manager.atoms()[*it].crad.name), *it));
				}

				std::sort(neighbors_in_residue.begin(), neighbors_in_residue.end());

				for(std::size_t l=0;l<params.max_number_of_directional_neighbors && l<neighbors_in_residue.size();l++)
				{
					result.directional_neighbors[i].push_back(neighbors_in_residue[l].second);
				}
			}

			{
				const std::vector<std::size_t>& dneighbors=result.directional_neighbors[i];

				const apollota::SimplePoint o(data_manager.atoms()[i].value);
				const apollota::SimplePoint a=(dneighbors.size()>0 ? apollota::SimplePoint(data_manager.atoms()[dneighbors[0]].value) : o+params.default_fallback_direction);
				const apollota::SimplePoint b=(dneighbors.size()>1 ? apollota::SimplePoint(data_manager.atoms()[dneighbors[1]].value) : a);

				const apollota::SimplePoint d1=(direct_neighbors.size()<2 ? (o-a).unit() : ((o-a)+(o-b)).unit());
				const apollota::SimplePoint d2=((o-a)&(o-b)).unit();
				const apollota::SimplePoint d3=(d1&d2).unit();

				result.basic_directions[i][0]=d1;
				result.basic_directions[i][1]=d2;
				result.basic_directions[i][2]=d3;
			}
		}
	}
};

}

}

#endif /* SCRIPTING_PRIMITIVE_ATOM_DIRECTIONS_ASSIGNMENT_H_ */
