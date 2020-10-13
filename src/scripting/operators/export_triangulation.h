#ifndef SCRIPTING_OPERATORS_EXPORT_TRIANGULATION_H_
#define SCRIPTING_OPERATORS_EXPORT_TRIANGULATION_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportTriangulation : public OperatorBase<ExportTriangulation>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;
		SummaryOfTriangulation full_triangulation_summary;
		SummaryOfAtoms atoms_summary;
		std::size_t number_of_relevant_voronoi_vertices;
		double total_relevant_tetrahedron_volume;

		Result() : number_of_relevant_voronoi_vertices(0), total_relevant_tetrahedron_volume(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			VariantSerialization::write(full_triangulation_summary, heterostorage.variant_object.object("full_triangulation_summary"));
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("number_of_relevant_voronoi_vertices")=number_of_relevant_voronoi_vertices;
			heterostorage.variant_object.value("total_relevant_tetrahedron_volume")=total_relevant_tetrahedron_volume;
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	FilteringOfTriangulation::Query filtering_query_without_ids;
	std::string file;
	std::vector<std::string> adjuncts;

	ExportTriangulation()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		filtering_query_without_ids=OperatorsUtilities::read_filtering_of_triangulation_query(input);
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		OperatorsUtilities::document_read_filtering_of_triangulation_query(doc);
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "names of atom adjuncts to output", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_triangulation_info_availability();

		assert_file_name_input(file, false);

		FilteringOfTriangulation::Query filtering_query=filtering_query_without_ids;
		filtering_query.atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

		if(filtering_query.atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const FilteringOfTriangulation::MatchingResult filtering_result=FilteringOfTriangulation::match_vertices(data_manager.triangulation_info(), filtering_query);

		if(filtering_result.vertices_info.empty())
		{
			throw std::runtime_error(std::string("No triangulation parts selected."));
		}

		OutputSelector output_selector(file);

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
			{
				const FilteringOfTriangulation::VertexInfo& vi=filtering_result.vertices_info[i];
				for(int j=0;j<4;j++)
				{
					output << vi.quadruple.get(j) << " ";
				}
				output << vi.sphere.x << " " << vi.sphere.y << " " << vi.sphere.z << " " << vi.sphere.r << " " << vi.tetrahedron_volume;
				for(std::size_t a=0;a<adjuncts.size();a++)
				{
					const std::string& adjunct_name=adjuncts[a];
					for(int j=0;j<4;j++)
					{
						const std::map<std::string, double>& adjuncts_map=data_manager.atoms().at(vi.quadruple.get(j)).value.props.adjuncts;
						std::map<std::string, double>::const_iterator adjunct_it=adjuncts_map.find(adjunct_name);
						if(adjunct_it!=adjuncts_map.end())
						{
							output << " " << (adjunct_it->second);
						}
						else
						{
							output << " 0";
						}
					}
				}
				output << "\n";
			}
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.full_triangulation_summary=SummaryOfTriangulation(data_manager.triangulation_info());
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), filtering_query.atom_ids);
		result.number_of_relevant_voronoi_vertices=filtering_result.vertices_info.size();
		result.total_relevant_tetrahedron_volume=filtering_result.total_relevant_tetrahedron_volume;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_TRIANGULATION_H_ */
