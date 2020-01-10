#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

void split_atoms_file(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "file in PDB format");
	pohw.describe_io("stdout", false, true, "list of result files");

	const std::string prefix=poh.argument<std::string>(pohw.describe_option("--prefix", "string", "prefix for output files"));
	const std::string postfix=poh.argument<std::string>(pohw.describe_option("--postfix", "string", "prefix for output files"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::string current_model_id;
	std::vector<std::string> current_model_lines;
	std::vector<std::string> result_filenames;

	while(std::cin.good())
	{
		std::string line;
		std::getline(std::cin, line);
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
					std::ofstream foutput(filename.c_str(), std::ios::out);
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

	voronota::auxiliaries::IOUtilities().write_set(result_filenames, std::cout);
}
