#ifndef SCRIPTING_OPERATORS_EXPORT_DIRECTIONAL_ATOM_TYPE_TUPLES_H_
#define SCRIPTING_OPERATORS_EXPORT_DIRECTIONAL_ATOM_TYPE_TUPLES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportDirectionalAtomTypeTuples : public OperatorBase<ExportDirectionalAtomTypeTuples>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
		}
	};

    std::string file;

	ExportDirectionalAtomTypeTuples()
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_primary_structure_info_valid();

		assert_file_name_input(file, false);

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
						if(apollota::distance_from_point_to_point(data_manager.atoms()[atom_id_a].value, data_manager.atoms()[atom_id_b].value)<1.7)
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

		std::vector< std::vector<std::size_t> > directional_neighbors(data_manager.atoms().size());
		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			const std::set<std::size_t>& direct_neighbors=graph_direct[i];
			const std::set<std::size_t>& indirect_neighbors=graph_indirect[i];
			const std::set<std::size_t>& far_neighbors=graph_far[i];
			std::vector< std::pair< std::pair<int, std::string>, std::size_t > > neighbors_in_residue;
			for(std::set<std::size_t>::const_iterator it=direct_neighbors.begin();it!=direct_neighbors.end();++it)
			{
				neighbors_in_residue.push_back(std::pair< std::pair<int, std::string>, std::size_t >(std::pair<int, std::string>(1, data_manager.atoms()[*it].crad.name), *it));
			}
			for(std::set<std::size_t>::const_iterator it=indirect_neighbors.begin();it!=indirect_neighbors.end() && neighbors_in_residue.size()<2;++it)
			{
				neighbors_in_residue.push_back(std::pair< std::pair<int, std::string>, std::size_t >(std::pair<int, std::string>(2, data_manager.atoms()[*it].crad.name), *it));
			}
			for(std::set<std::size_t>::const_iterator it=far_neighbors.begin();it!=far_neighbors.end() && neighbors_in_residue.size()<2;++it)
			{
				neighbors_in_residue.push_back(std::pair< std::pair<int, std::string>, std::size_t >(std::pair<int, std::string>(3, data_manager.atoms()[*it].crad.name), *it));
			}
			std::sort(neighbors_in_residue.begin(), neighbors_in_residue.end());
			for(std::size_t l=0;l<2 && l<neighbors_in_residue.size();l++)
			{
				directional_neighbors[i].push_back(neighbors_in_residue[l].second);
			}
		}

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		for(std::size_t i=0;i<directional_neighbors.size();i++)
		{
			const Atom& central_atom=data_manager.atoms()[i];
			output << central_atom.crad.resName << " " << central_atom.crad.name;
			for(std::size_t l=0;l<2;l++)
			{
				if(l<directional_neighbors[i].size())
				{
					output << " " << data_manager.atoms()[directional_neighbors[i][l]].crad.name;
				}
				else
				{
					output << " XXX";
				}
			}
			output << "\n";
		}

		Result result;
		result.file=file;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_DIRECTIONAL_ATOM_TYPE_TUPLES_H_ */
