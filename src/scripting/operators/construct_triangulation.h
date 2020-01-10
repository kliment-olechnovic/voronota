#ifndef SCRIPTING_OPERATORS_CONSTRUCT_TRIANGULATION_H_
#define SCRIPTING_OPERATORS_CONSTRUCT_TRIANGULATION_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ConstructTriangulation : public OperatorBase<ConstructTriangulation>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfTriangulation triangulation_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(triangulation_summary, heterostorage.variant_object.object("triangulation_summary"));
		}
	};

	common::ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation parameters_to_construct_triangulation;
	bool force;

	ConstructTriangulation() : force(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_to_construct_triangulation=common::ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation();
		parameters_to_construct_triangulation.artificial_boundary_shift=input.get_value_or_default<double>("boundary-shift", 5.0);
		parameters_to_construct_triangulation.init_radius_for_BSH=input.get_value_or_default<double>("init-radius-for-BSH", parameters_to_construct_triangulation.init_radius_for_BSH);
		parameters_to_construct_triangulation.exclude_hidden_balls=input.get_flag("exclude-hidden-balls");
		force=input.get_flag("force");
	}

	void document(CommandDocumentation& doc) const
	{
		common::ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation params;
		doc.set_option_decription(CDOD("boundary-shift", CDOD::DATATYPE_FLOAT, "artificial boundary offset distance", 5.0));
		doc.set_option_decription(CDOD("init-radius-for-BSH", CDOD::DATATYPE_FLOAT, "init radius for a bounding sphere hierarchy", params.init_radius_for_BSH));
		doc.set_option_decription(CDOD("exclude-hidden-balls", CDOD::DATATYPE_BOOL, "flag to exclude balls that are completely inside other balls"));
		doc.set_option_decription(CDOD("force", CDOD::DATATYPE_BOOL, "flag to force construction even if there is a cached version"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(force)
		{
			data_manager.remove_triangulation_info();
		}

		data_manager.reset_triangulation_info_by_creating(parameters_to_construct_triangulation);

		Result result;
		result.triangulation_summary=SummaryOfTriangulation(data_manager.triangulation_info());

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CONSTRUCT_TRIANGULATION_H_ */
