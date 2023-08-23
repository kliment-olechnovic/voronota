#ifndef SCRIPTING_OPERATORS_REDUCE_TABLE_REDUNDANCY_BY_BUCKETING_H_
#define SCRIPTING_OPERATORS_REDUCE_TABLE_REDUNDANCY_BY_BUCKETING_H_

#include <algorithm>

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ReduceTableRedundancyByBucketing : public OperatorBase<ReduceTableRedundancyByBucketing>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string input_file;
	std::string output_file;
	std::vector<std::string> columns;
	std::vector<int> bucket_counts;
	int max_presence;

	ReduceTableRedundancyByBucketing() : max_presence(1)
	{
	}

	void initialize(CommandInput& input)
	{
		input_file=input.get_value<std::string>("input-file");
		output_file=input.get_value<std::string>("output-file");
		columns=input.get_value_vector<std::string>("bucket-columns");
		bucket_counts=input.get_value_vector<int>("bucket-counts");
		max_presence=input.get_value_or_default<int>("max-presence", 1);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("input-file", CDOD::DATATYPE_STRING, "path to input file"));
		doc.set_option_decription(CDOD("output-file", CDOD::DATATYPE_STRING, "path to output file"));
		doc.set_option_decription(CDOD("bucket-columns", CDOD::DATATYPE_STRING_ARRAY, "names of columns to use for bucketing"));
		doc.set_option_decription(CDOD("bucket-counts", CDOD::DATATYPE_INT_ARRAY, "bucket counts for columns"));
		doc.set_option_decription(CDOD("max-presence", CDOD::DATATYPE_INT, "max number of occurrences for a bucketed vector", 1));
	}

	Result run(void*) const
	{
		assert_file_name_input(input_file, false);
		assert_file_name_input(output_file, false);

		if(columns.empty())
		{
			throw std::runtime_error(std::string("No column names specified."));
		}

		if(bucket_counts.empty())
		{
			throw std::runtime_error(std::string("No bucket counts specified."));
		}

		if(bucket_counts.size()!=columns.size())
		{
			throw std::runtime_error(std::string("Number of bucket counts is not the same as number of columns for bucketing."));
		}

		for(std::size_t i=0;i<bucket_counts.size();i++)
		{
			if(bucket_counts[i]<1)
			{
				throw std::runtime_error(std::string("Not all bucket counts are positive."));
			}
		}

		if(max_presence<1)
		{
			throw std::runtime_error(std::string("Max presence value is not positive."));
		}

		InputSelector finput_selector(input_file);
		std::istream& finput=finput_selector.stream();
		assert_io_stream(input_file, finput);

		OutputSelector foutput_selector(output_file);
		std::ostream& foutput=foutput_selector.stream();
		assert_io_stream(output_file, foutput);

		std::string table_header_line;
		std::vector<std::string> table_row_lines;
		std::vector< std::vector<double> > columns_values(columns.size(), std::vector<double>());

		{
			std::vector<std::string> table_header_split;
			std::vector<std::size_t> column_positions(columns.size(), 0);
			while(finput.good())
			{
				std::string line;
				std::getline(finput, line);
				if(!line.empty())
				{
					const bool in_header=table_header_line.empty();

					if(in_header)
					{
						table_header_line=line;
					}
					else
					{
						table_row_lines.push_back(line);
					}

					std::vector<std::string> line_split;

					{
						std::istringstream line_input(line);
						while(line_input.good())
						{
							std::string token;
							line_input >> token;
							if(!token.empty())
							{
								line_split.push_back(token);
							}
						}
					}

					if(in_header)
					{
						for(std::size_t i=0;i<columns.size();i++)
						{
							bool found=false;
							for(std::size_t j=0;j<line_split.size() && !found;j++)
							{
								if(line_split[j]==columns[i])
								{
									column_positions[i]=j;
									found=true;
								}
							}
							if(!found)
							{
								throw std::runtime_error(std::string("No column '")+columns[i]+"'.");
							}
						}
						table_header_split=line_split;
					}
					else
					{
						if(line_split.size()!=table_header_split.size())
						{
							throw std::runtime_error(std::string("Invalid number of values in row '")+line+"'.");
						}
						for(std::size_t i=0;i<column_positions.size();i++)
						{
							const std::string& str_value=line_split[column_positions[i]];
							std::istringstream value_input(str_value);
							double value=0.0;
							value_input >> value;
							if(value_input.fail())
							{
								throw std::runtime_error(std::string("Failed to convert string '")+str_value+"' to numeric value.");
							}
							columns_values[i].push_back(value);
						}
					}
				}
			}
		}

		const std::size_t N=table_row_lines.size();

		if(N<1)
		{
			throw std::runtime_error(std::string("No value rows in table."));
		}

		std::vector< std::pair<double, double> > columns_values_ranges(columns.size());
		std::vector<double> columns_values_bucket_size(columns.size());
		for(std::size_t c=0;c<columns.size();c++)
		{
			const std::vector<double>& values=columns_values[c];
			std::pair<double, double>& range=columns_values_ranges[c];
			range.first=(*(std::min_element(values.begin(), values.end())));
			range.second=(*(std::max_element(values.begin(), values.end())));
			columns_values_bucket_size[c]=(range.second-range.first)/static_cast<double>(bucket_counts[c]);
		}

		std::vector<std::size_t> selected_row_ids;
		selected_row_ids.reserve(N);
		{
			std::map< std::vector<int>, int > map_of_multibucket_presence;
			for(std::size_t r=0;r<N;r++)
			{
				std::vector<int> multibucket(columns.size());
				for(std::size_t c=0;c<columns.size();c++)
				{
					const int bucket_id=(columns_values_bucket_size[c]>0.0 ? static_cast<int>((columns_values[c][r]-columns_values_ranges[c].first)/columns_values_bucket_size[c]) : 0);
					multibucket[c]=std::max(0, std::min(bucket_id, bucket_counts[c]-1));
				}
				int& presence=map_of_multibucket_presence[multibucket];
				if(presence<max_presence)
				{
					selected_row_ids.push_back(r);
				}
				presence++;
			}
		}

		{
			foutput << table_header_line << "\n";
			for(std::size_t i=0;i<selected_row_ids.size();i++)
			{
				foutput << table_row_lines[selected_row_ids[i]] << "\n";
			}
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_REDUCE_TABLE_REDUNDANCY_BY_BUCKETING_H_ */

