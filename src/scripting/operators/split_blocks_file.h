#ifndef SCRIPTING_OPERATORS_SPLIT_BLOCKS_FILE_H_
#define SCRIPTING_OPERATORS_SPLIT_BLOCKS_FILE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SplitBlocksFile : public OperatorBase<SplitBlocksFile>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<std::string> result_filenames;

		void store(HeterogeneousStorage& heterostorage) const
		{
			std::vector<VariantValue>& result_filenames_array=heterostorage.variant_object.values_array("result_filenames");
			for(std::size_t i=0;i<result_filenames.size();i++)
			{
				result_filenames_array.push_back(VariantValue(result_filenames[i]));
			}
		}
	};

	SplitBlocksFile()
	{
	}

	std::string input_file;
	std::string output_file;
	std::string block_start;
	std::string block_substart;
	std::string block_end;
	std::string prefix;
	std::string postfix;

	void initialize(CommandInput& input)
	{
		input_file=input.get_value<std::string>("input-file");
		output_file=input.get_value_or_default<std::string>("output-file", "");
		block_start=input.get_value_or_default<std::string>("block-start", "");
		block_substart=input.get_value_or_default<std::string>("block-substart", "");
		block_end=input.get_value_or_default<std::string>("block-end", "");
		prefix=input.get_value<std::string>("prefix");
		postfix=input.get_value_or_default<std::string>("postfix", "");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("input-file", CDOD::DATATYPE_STRING, "path to input file to split"));
		doc.set_option_decription(CDOD("output-file", CDOD::DATATYPE_STRING, "path to output file with list of written files", ""));
		doc.set_option_decription(CDOD("block-start", CDOD::DATATYPE_STRING_ARRAY, "beginning of a block-starting line", ""));
		doc.set_option_decription(CDOD("block-substart", CDOD::DATATYPE_STRING_ARRAY, "beginning of a block-substarting line", ""));
		doc.set_option_decription(CDOD("block-end", CDOD::DATATYPE_STRING, "beginning of a block-ending line", ""));
		doc.set_option_decription(CDOD("prefix", CDOD::DATATYPE_STRING, "prefix for file paths"));
		doc.set_option_decription(CDOD("postfix", CDOD::DATATYPE_STRING, "postfix for file paths", ""));
	}

	Result run(void*) const
	{
		assert_file_name_input(input_file, false);

		if(block_start.empty() && block_end.empty())
		{
			throw std::runtime_error(std::string("Both block start and end were not unspecified."));
		}

		if(block_start.empty() && !block_substart.empty())
		{
			throw std::runtime_error(std::string("Block substart line beginning was specified without the start line beginning."));
		}

		InputSelector finput_selector(input_file);
		std::istream& finput=finput_selector.stream();
		assert_io_stream(input_file, finput);

		std::vector<std::string> current_block_id;
		std::vector<std::string> current_block_lines;
		std::vector<std::string> result_filenames;

		while(finput.good())
		{
			std::string line;
			std::getline(finput, line);
			if(!block_start.empty() && (line.rfind(block_start, 0)==0))
			{
				if(block_end.empty())
				{
					write_current_block_if_valid(current_block_id, current_block_lines, result_filenames);
				}
				current_block_id.clear();
				current_block_id.push_back(line);
			}
			else if(!block_substart.empty() && (line.rfind(block_substart, 0)==0) && (current_block_id.size()==1 || current_block_id.size()==2))
			{
				if(current_block_id.size()==2)
				{
					if(block_end.empty())
					{
						write_current_block_if_valid(current_block_id, current_block_lines, result_filenames);
					}
				}
				if(current_block_id.size()==1)
				{
					current_block_id.push_back(line);
				}
			}
			else if(!block_end.empty() && (line.rfind(block_end, 0)==0))
			{
				current_block_lines.push_back(line);
				write_current_block_if_valid(current_block_id, current_block_lines, result_filenames);
			}
			else if((current_block_id.size())==(block_substart.empty() ? 1 : 2))
			{
				current_block_lines.push_back(line);
			}
		}

		if(block_end.empty())
		{
			write_current_block_if_valid(current_block_id, current_block_lines, result_filenames);
		}

		if(!output_file.empty())
		{
			OutputSelector foutput_selector(output_file);
			std::ostream& foutput=foutput_selector.stream();
			assert_io_stream(output_file, foutput);

			voronota::auxiliaries::IOUtilities().write_set(result_filenames, foutput);
		}

		Result result;
		result.result_filenames=result_filenames;

		return result;
	}

private:
	void write_current_block_if_valid(std::vector<std::string>& current_block_id, std::vector<std::string>& current_block_lines, std::vector<std::string>& result_filenames) const
	{
		if(!current_block_lines.empty() && (current_block_id.size())==(block_substart.empty() ? 1 : 2))
		{
			std::string filename=prefix;
			for(std::size_t i=0;i<current_block_id.size();i++)
			{
				if(i>0)
				{
					filename+="__";
				}
				for(std::size_t j=0;j<current_block_id[i].size();j++)
				{
					const char c=current_block_id[i][j];
					filename.push_back(static_cast<int>(c)<=32 ? '_' : c);
				}
			}
			filename+=postfix;
			OutputSelector foutput_selector(filename);
			std::ostream& foutput=foutput_selector.stream();
			for(std::size_t i=0;i<current_block_lines.size();i++)
			{
				foutput << current_block_lines[i] << "\n";
			}
			result_filenames.push_back(filename);

			if(current_block_id.size()==2)
			{
				current_block_id.pop_back();
			}
			else
			{
				current_block_id.clear();
			}
		}
		else
		{
			current_block_id.clear();
		}
		current_block_lines.clear();
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SPLIT_BLOCKS_FILE_H_ */
