#ifndef SCRIPTING_OPERATORS_IMPORT_FIGURE_VOXELS_H_
#define SCRIPTING_OPERATORS_IMPORT_FIGURE_VOXELS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ImportFigureVoxels : public OperatorBase<ImportFigureVoxels>
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

	std::string file;
	double voxel_diameter;
	std::vector<std::string> figure_name;

	ImportFigureVoxels() : voxel_diameter(0.2)
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		voxel_diameter=input.get_value_or_default<double>("voxel-diameter", 0.2);
		figure_name=input.get_value_vector<std::string>("figure-name");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		doc.set_option_decription(CDOD("voxel-diameter", CDOD::DATATYPE_FLOAT, "voxel diameter", 0.2));
		doc.set_option_decription(CDOD("figure-name", CDOD::DATATYPE_STRING_ARRAY, "figure name"));
	}

	Result run(DataManager& data_manager) const
	{
		if(voxel_diameter<0.1)
		{
			throw std::runtime_error(std::string("Voxel diameter is too small, need to be not less than 0.1"));
		}

		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input file name."));
		}

		InputSelector finput_selector(file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		Figure figure;
		figure.name=LongName(figure_name);

		Result result;

		while(finput.good())
		{
			std::string line;
			std::getline(finput, line);
			if(!line.empty())
			{
				apollota::SimplePoint center;
				std::istringstream linput(line);
				linput >> center.x >> center.y >> center.z;
				if(linput.fail())
				{
					throw std::runtime_error(std::string("Invalid coordinates in file line '")+line+"'");
				}
				figure.add_voxel(center, voxel_diameter);
				result.total_voxels++;
			}
		}

		data_manager.add_figure(figure);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_FIGURE_VOXELS_H_ */

