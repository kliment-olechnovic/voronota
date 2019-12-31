#ifndef SCRIPTING_OPERATORS_PRINT_VIRTUAL_FILE_H_
#define SCRIPTING_OPERATORS_PRINT_VIRTUAL_FILE_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class PrintVirtualFile
{
public:
	struct Result
	{
		std::string name;
		bool line_by_line;
		std::string data;
		std::vector<std::string> lines;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("name")=name;
			if(line_by_line)
			{
				heterostorage.variant_object.values_array("lines");
				for(std::size_t i=0;i<lines.size();i++)
				{
					heterostorage.variant_object.values_array("lines").push_back(VariantValue(lines[i]));
				}
			}
			else
			{
				heterostorage.variant_object.value("data")=data;
			}
			return (*this);
		}
	};

	std::string filename;
	bool line_by_line;

	PrintVirtualFile() : line_by_line(false)
	{
	}

	PrintVirtualFile& init(CommandInput& input)
	{
		filename=input.get_value_or_first_unused_unnamed_value("name");
		line_by_line=input.get_flag("line-by-line");
		return (*this);
	}

	Result run() const
	{
		Result result;
		result.name=filename;
		result.line_by_line=line_by_line;
		if(line_by_line)
		{
			std::istringstream input(VirtualFileStorage::get_file(filename));
			while(input.good())
			{
				std::string line;
				std::getline(input, line);
				result.lines.push_back(line);
			}
		}
		else
		{
			result.data=VirtualFileStorage::get_file(filename);
		}
		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_PRINT_VIRTUAL_FILE_H_ */
