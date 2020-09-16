#ifndef SCRIPTING_OPERATORS_PRINT_TRIANGULATION_H_
#define SCRIPTING_OPERATORS_PRINT_TRIANGULATION_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class PrintTriangulation : public OperatorBase<PrintTriangulation>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<VariantObject> vertices;
		SummaryOfTriangulation full_triangulation_summary;
		SummaryOfAtoms atoms_summary;
		std::size_t number_of_relevant_voronoi_vertices;
		double total_relevant_tetrahedron_volume;

		Result() : number_of_relevant_voronoi_vertices(0), total_relevant_tetrahedron_volume(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			if(!vertices.empty())
			{
				heterostorage.variant_object.objects_array("vertices")=vertices;
			}
			VariantSerialization::write(full_triangulation_summary, heterostorage.variant_object.object("full_triangulation_summary"));
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("number_of_relevant_voronoi_vertices")=number_of_relevant_voronoi_vertices;
			heterostorage.variant_object.value("total_relevant_tetrahedron_volume")=total_relevant_tetrahedron_volume;
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	FilteringOfTriangulation::Query filtering_query_without_ids;
	bool only_summary;

	PrintTriangulation() : only_summary(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		filtering_query_without_ids=OperatorsUtilities::read_filtering_of_triangulation_query(input);
		only_summary=input.get_flag("only-summary");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		OperatorsUtilities::document_read_filtering_of_triangulation_query(doc);
		doc.set_option_decription(CDOD("only-summary", CDOD::DATATYPE_BOOL, "flag to output only summary"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_triangulation_info_availability();

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

		Result result;

		if(!only_summary)
		{
			for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
			{
				const FilteringOfTriangulation::VertexInfo& vi=filtering_result.vertices_info[i];

				VariantObject info;
				info.value("id")=vi.id;

				{
					std::vector<VariantValue>& suboutput=info.values_array("quadruple");
					for(std::size_t j=0;j<4;j++)
					{
						suboutput.push_back(VariantValue(vi.quadruple.get(j)));
					}
				}

				{
					std::vector<VariantValue>& suboutput=info.values_array("tangent_sphere");
					suboutput.push_back(VariantValue(vi.sphere.x));
					suboutput.push_back(VariantValue(vi.sphere.y));
					suboutput.push_back(VariantValue(vi.sphere.z));
					suboutput.push_back(VariantValue(vi.sphere.r));
				}

				info.value("tetrahedron_volume")=vi.tetrahedron_volume;

				result.vertices.push_back(info);
			}
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

#endif /* SCRIPTING_OPERATORS_PRINT_TRIANGULATION_H_ */
