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
		double max_bond_distance_common;
		double max_bond_distance_extended;

		Parameters() :
			max_bond_distance_common(1.7),
			max_bond_distance_extended(2.0)
		{
		}
	};

	struct Result
	{
		std::vector< std::vector<std::size_t> > directional_neighbors;
		std::vector< std::vector<apollota::SimplePoint> > basic_directions;

		Result()
		{
		}
	};

	static void construct_result(const Parameters& params, const DataManager& data_manager, Result& result)
	{
		result=Result();

		data_manager.assert_atoms_availability();
		data_manager.assert_primary_structure_info_valid();

		std::vector< std::set<std::size_t> > graph_direct(data_manager.atoms().size());
		for(std::size_t j=0;j<data_manager.primary_structure_info().residues.size();j++)
		{
			const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[j];
			for(std::size_t id_a=0;id_a<residue.atom_ids.size();id_a++)
			{
				const std::size_t atom_id_a=residue.atom_ids[id_a];
				const Atom& atom_a=data_manager.atoms()[atom_id_a];
				for(std::size_t k=((j>0) ? (j-1) : j);k<=((j+1<data_manager.primary_structure_info().residues.size()) ? (j+1) : j);k++)
				{
					const common::ConstructionOfPrimaryStructure::Residue& context_residue=data_manager.primary_structure_info().residues[k];
					for(std::size_t id_b=0;id_b<context_residue.atom_ids.size();id_b++)
					{
						const std::size_t atom_id_b=context_residue.atom_ids[id_b];
						if(atom_id_a!=atom_id_b)
						{
							const Atom& atom_b=data_manager.atoms()[atom_id_b];
							double max_bond_distance=params.max_bond_distance_common;
							if((atom_a.crad.name.size()>0 && atom_a.crad.name[0]=='S') || (atom_b.crad.name.size()>0 && atom_b.crad.name[0]=='S'))
							{
								max_bond_distance=params.max_bond_distance_extended;
							}
							if(apollota::distance_from_point_to_point(atom_a.value, atom_b.value)<max_bond_distance)
							{
								graph_direct[atom_id_a].insert(atom_id_b);
							}
						}
					}
				}
			}
		}

		result.directional_neighbors.resize(data_manager.atoms().size());
		result.basic_directions.resize(data_manager.atoms().size());
		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			const std::set<std::size_t>& direct_neighbors=graph_direct[i];
			if(!direct_neighbors.empty())
			{
				std::vector< std::pair<std::string, std::size_t> > neighbors_in_residue;

				for(std::set<std::size_t>::const_iterator it=direct_neighbors.begin();it!=direct_neighbors.end();++it)
				{
					neighbors_in_residue.push_back(std::pair<std::string, std::size_t>(data_manager.atoms()[*it].crad.name, *it));
				}

				std::sort(neighbors_in_residue.begin(), neighbors_in_residue.end());

				for(std::size_t l=0;l<neighbors_in_residue.size();l++)
				{
					result.directional_neighbors[i].push_back(neighbors_in_residue[l].second);
				}
			}

			const std::vector<std::size_t>& ordered_direct_neighbors=result.directional_neighbors[i];
			if(!ordered_direct_neighbors.empty())
			{
				const apollota::SimplePoint o(data_manager.atoms()[i].value);

				std::vector<apollota::SimplePoint>& directions=result.basic_directions[i];

				if(ordered_direct_neighbors.size()==1)
				{
					const apollota::SimplePoint a(data_manager.atoms()[ordered_direct_neighbors[0]].value);
					directions.resize(1);
					directions[0]=((o-a).unit());
				}
				else if(ordered_direct_neighbors.size()==2)
				{
					const apollota::SimplePoint a(data_manager.atoms()[ordered_direct_neighbors[0]].value);
					const apollota::SimplePoint b(data_manager.atoms()[ordered_direct_neighbors[1]].value);
					directions.resize(1);
					directions[0]=((o-a).unit()+(o-b).unit()).unit();
				}
				else if(ordered_direct_neighbors.size()==3)
				{
					const apollota::SimplePoint a(data_manager.atoms()[ordered_direct_neighbors[0]].value);
					const apollota::SimplePoint b(data_manager.atoms()[ordered_direct_neighbors[1]].value);
					const apollota::SimplePoint c(data_manager.atoms()[ordered_direct_neighbors[2]].value);
					const double normalized_volume=std::abs(apollota::signed_volume_of_tetrahedron(apollota::SimplePoint(), (a-o).unit(), (b-o).unit(), (c-o).unit()));
					if(normalized_volume>0.1)
					{
						directions.resize(1);
						directions[0]=((o-a).unit()+(o-b).unit()+(o-c).unit()).unit();
					}
				}
			}
		}
	}

	static void construct_result(const DataManager& data_manager, Result& result)
	{
		construct_result(Parameters(), data_manager, result);
	}
};

}

}

#endif /* SCRIPTING_PRIMITIVE_ATOM_DIRECTIONS_ASSIGNMENT_H_ */
