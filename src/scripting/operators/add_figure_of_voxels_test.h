#ifndef SCRIPTING_OPERATORS_ADD_FIGURE_OF_VOXELS_TEST_H_
#define SCRIPTING_OPERATORS_ADD_FIGURE_OF_VOXELS_TEST_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class AddFigureOfVoxelsTest : public OperatorBase<AddFigureOfVoxelsTest>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		int total_voxels;

		Result() : total_voxels(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("total_voxels")=total_voxels;
		}
	};

	double voxel_diameter;
	double figure_radius;
	std::vector<std::string> figure_name;

	AddFigureOfVoxelsTest() : voxel_diameter(0.2), figure_radius(5.0)
	{
	}

	void initialize(CommandInput& input)
	{
		voxel_diameter=input.get_value_or_default<double>("voxel-diameter", 0.2);
		figure_radius=input.get_value_or_default<double>("figure-radius", 5.0);
		figure_name=input.get_value_vector<std::string>("figure-name");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("voxel-diameter", CDOD::DATATYPE_FLOAT, "voxel diameter", 0.2));
		doc.set_option_decription(CDOD("figure-radius", CDOD::DATATYPE_FLOAT, "demo figure radius", 5.0));
		doc.set_option_decription(CDOD("figure-name", CDOD::DATATYPE_STRING_ARRAY, "figure name"));
	}

	Result run(DataManager& data_manager) const
	{
		if(voxel_diameter<0.1)
		{
			throw std::runtime_error(std::string("Voxel diameter is too small, need to be not less than 0.1"));
		}

		if(figure_radius<voxel_diameter*10)
		{
			throw std::runtime_error(std::string("Figure radius is too small, need to be more than 10 times voxel radius"));
		}

		Figure figure;
		figure.name=LongName(figure_name);

		Result result;

		const int steps=static_cast<int>(std::ceil(figure_radius/voxel_diameter));

		for(int xi=(0-steps);xi<=steps;xi++)
		{
			const double x=static_cast<double>(xi)*voxel_diameter;
			for(int yi=0;yi<=steps;yi++)
			{
				const double y=static_cast<double>(yi)*voxel_diameter;
				for(int zi=0;zi<=steps;zi++)
				{
					const double z=static_cast<double>(zi)*voxel_diameter;
					{
						const double r=sqrt(x*x+y*y+z*z);
						if(r<=figure_radius && r>=(figure_radius*0.9))
						{
							figure.add_voxel(apollota::SimplePoint(x, y, z), voxel_diameter);
							result.total_voxels++;
						}
					}
				}
			}
		}

		data_manager.add_figure(figure);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURE_OF_VOXELS_TEST_H_ */
