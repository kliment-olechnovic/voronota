#ifndef SCRIPTING_OPERATORS_SPLIT_PDB_FILE_H_
#define SCRIPTING_OPERATORS_SPLIT_PDB_FILE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SplitPDBFile : public operators::OperatorBase<SplitPDBFile>
{
public:
	struct Result : public operators::OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	SplitPDBFile()
	{
	}

	std::string input_file;
	std::string output_file;
	std::string prefix;
	std::string postfix;

	void initialize(CommandInput& input)
	{
		input_file=input.get_value<std::string>("input-file");
		output_file=input.get_value_or_default<std::string>("output-file", "");
		prefix=input.get_value<std::string>("prefix");
		postfix=input.get_value_or_default<std::string>("postfix", "");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("input-file", CDOD::DATATYPE_STRING, "path to input file to split"));
		doc.set_option_decription(CDOD("output-file", CDOD::DATATYPE_STRING, "path to output file with list of written files", ""));
		doc.set_option_decription(CDOD("prefix", CDOD::DATATYPE_STRING, "prefix for file paths"));
		doc.set_option_decription(CDOD("postfix", CDOD::DATATYPE_STRING, "postfix for file paths", ""));
	}

	Result run(void*) const
	{
		assert_file_name_input(input_file, false);

		InputSelector finput_selector(input_file);
		std::istream& finput=finput_selector.stream();
		assert_io_stream(input_file, finput);

		std::string current_model_id;
		std::vector<std::string> current_model_lines;
		std::vector<std::string> result_filenames;

		while(finput.good())
		{
			std::string line;
			std::getline(finput, line);
			if(!line.empty())
			{
				const bool model_start=(line.rfind("MODEL ", 0)==0);
				const bool model_end=!model_start && (line.rfind("END", 0)==0);

				if(!current_model_id.empty())
				{
					if(!model_start)
					{
						current_model_lines.push_back(line);
					}

					if(model_start || model_end)
					{
						const std::string filename=prefix+current_model_id+postfix;
						OutputSelector foutput_selector(filename);
						std::ostream& foutput=foutput_selector.stream();
						for(std::size_t i=0;i<current_model_lines.size();i++)
						{
							foutput << current_model_lines[i] << "\n";
						}
						result_filenames.push_back(filename);
						current_model_id.clear();
						current_model_lines.clear();
					}
				}

				if(model_start)
				{
					std::istringstream line_input(line);
					std::string model_label;
					std::string model_id;
					line_input >> model_label >> model_id;
					if(!model_id.empty())
					{
						current_model_id=model_id;
					}
					else
					{
						current_model_id="unnamed";
					}
					current_model_lines.clear();
					current_model_lines.push_back(line);
				}
			}
		}

		if(!output_file.empty())
		{
			OutputSelector foutput_selector(output_file);
			std::ostream& foutput=foutput_selector.stream();
			assert_io_stream(output_file, foutput);

			voronota::auxiliaries::IOUtilities().write_set(result_filenames, foutput);
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SPLIT_PDB_FILE_H_ */

