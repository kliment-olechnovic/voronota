#ifndef SCRIPTING_OPERATORS_CONSTRUCT_TRIANGULATION_H_
#define SCRIPTING_OPERATORS_CONSTRUCT_TRIANGULATION_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class ConstructTriangulation
{
public:
	struct Result
	{
		SummaryOfTriangulation triangulation_summary;

		Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(triangulation_summary, heterostorage.variant_object.object("triangulation_summary"));
			return (*this);
		}
	};

	common::ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation parameters_to_construct_triangulation;
	bool force;

	ConstructTriangulation() : force(false)
	{
	}

	ConstructTriangulation& init(CommandInput& input)
	{
		parameters_to_construct_triangulation.artificial_boundary_shift=input.get_value_or_default<double>("boundary-shift", 5.0);
		parameters_to_construct_triangulation.init_radius_for_BSH=input.get_value_or_default<double>("init-radius-for-BSH", parameters_to_construct_triangulation.init_radius_for_BSH);
		parameters_to_construct_triangulation.exclude_hidden_balls=input.get_flag("exclude-hidden-balls");
		force=input.get_flag("force");
		return (*this);
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

#endif /* SCRIPTING_OPERATORS_CONSTRUCT_TRIANGULATION_H_ */
