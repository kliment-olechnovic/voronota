#ifndef SCRIPTING_OPERATORS_TOURNAMENT_SORT_H_
#define SCRIPTING_OPERATORS_TOURNAMENT_SORT_H_

#include <algorithm>

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class TournamentSort : public OperatorBase<TournamentSort>
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
	std::vector<double> multipliers;
	std::vector<double> tolerances;
	std::string add_win_score_column;
	std::string add_rank_column;
	bool no_tournament;

	TournamentSort() : no_tournament(false)
	{
	}

	void initialize(CommandInput& input)
	{
		input_file=input.get_value<std::string>("input-file");
		output_file=input.get_value<std::string>("output-file");
		columns=input.get_value_vector<std::string>("columns");
		multipliers=input.get_value_vector_or_default<double>("multipliers", std::vector<double>());
		tolerances=input.get_value_vector_or_default<double>("tolerances", std::vector<double>());
		add_win_score_column=input.get_value_or_default<std::string>("add-win-score-column", "");
		add_rank_column=input.get_value_or_default<std::string>("add-rank-column", "");
		no_tournament=input.get_flag("no-tournament");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("input-file", CDOD::DATATYPE_STRING, "path to input file"));
		doc.set_option_decription(CDOD("output-file", CDOD::DATATYPE_STRING, "path to output file"));
		doc.set_option_decription(CDOD("columns", CDOD::DATATYPE_STRING_ARRAY, "column names"));
		doc.set_option_decription(CDOD("multipliers", CDOD::DATATYPE_FLOAT_ARRAY, "multipliers for column values", ""));
		doc.set_option_decription(CDOD("tolerances", CDOD::DATATYPE_FLOAT_ARRAY, "tolerances for column values", ""));
		doc.set_option_decription(CDOD("add-win-score-column", CDOD::DATATYPE_STRING, "new column name for win scores", ""));
		doc.set_option_decription(CDOD("add-rank-column", CDOD::DATATYPE_STRING, "new column name for win score-based ranks", ""));
		doc.set_option_decription(CDOD("no-tournament", CDOD::DATATYPE_BOOL, "flag to sort simply without tournament"));
	}

	Result run(void*) const
	{
		assert_file_name_input(input_file, false);
		assert_file_name_input(output_file, false);

		if(columns.empty())
		{
			throw std::runtime_error(std::string("No column names specified."));
		}

		std::vector<double> usable_multipliers;
		std::vector<double> usable_tolerances;

		if(multipliers.empty())
		{
			usable_multipliers.resize(columns.size(), 1.0);
		}
		else if(multipliers.size()==columns.size())
		{
			usable_multipliers=multipliers;
		}
		else
		{
			throw std::runtime_error(std::string("Number of multipliers does not equal number of columns."));
		}

		if(tolerances.empty())
		{
			usable_tolerances.resize(columns.size(), 0.0);
		}
		else if(tolerances.size()==columns.size())
		{
			usable_tolerances=tolerances;
		}
		else
		{
			throw std::runtime_error(std::string("Number of tolerances does not equal number of columns."));
		}

		InputSelector finput_selector(input_file);
		std::istream& finput=finput_selector.stream();
		assert_io_stream(input_file, finput);

		OutputSelector foutput_selector(output_file);
		std::ostream& foutput=foutput_selector.stream();
		assert_io_stream(output_file, foutput);

		std::string table_header_line;
		std::vector<std::string> table_header_split;
		std::vector<std::string> table_row_lines;
		std::vector<std::size_t> column_positions(columns.size(), 0);
		std::vector< std::vector<double> > column_values(columns.size(), std::vector<double>());

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
						column_values[i].push_back(value*usable_multipliers[i]);
					}
				}
			}
		}

		const std::size_t N=table_row_lines.size();

		if(N<1)
		{
			throw std::runtime_error(std::string("No value rows in table."));
		}

		const bool possible_to_sort_simply=((column_values.size()==1 || no_tournament) && add_win_score_column.empty());

		if(possible_to_sort_simply)
		{
			typedef std::pair< std::vector<double>, std::size_t> SortableID;
			std::vector<SortableID> sortable_ids(N);

			for(std::size_t a=0;a<N;a++)
			{
				SortableID& sid=sortable_ids[a];
				sid.first.resize(column_values.size());
				for(std::size_t i=0;i<column_values.size();i++)
				{
					sid.first[i]=(0.0-column_values[i][a]);
				}
				sid.second=a;
			}

			std::sort(sortable_ids.begin(), sortable_ids.end());

			{
				foutput << table_header_line;
				if(!add_rank_column.empty())
				{
					foutput << " " << add_rank_column;
				}
				foutput << "\n";
				for(std::size_t a=0;a<N;a++)
				{
					foutput << table_row_lines[sortable_ids[a].second];
					if(!add_rank_column.empty())
					{
						foutput << " " << (a+1);
					}
					foutput << "\n";
				}
			}
		}
		else
		{
			typedef std::pair< std::pair<std::size_t, std::size_t>, std::size_t > SortableID;
			std::vector<SortableID> sortable_ids(N);

			for(std::size_t a=0;a<N;a++)
			{
				SortableID& sid=sortable_ids[a];
				sid.first.first=0;
				sid.first.second=0;
				sid.second=a;
			}

			for(std::size_t a=0;a<N;a++)
			{
				for(std::size_t b=(a+1);b<N;b++)
				{
					bool win_a=true;
					bool win_b=true;

					if(!no_tournament)
					{
						for(std::size_t i=0;i<column_values.size() && win_a;i++)
						{
							win_a=(win_a && ((column_values[i][a]+usable_tolerances[i])>column_values[i][b]));
						}

						for(std::size_t i=0;i<column_values.size() && win_b;i++)
						{
							win_b=(win_b && ((column_values[i][b]+usable_tolerances[i])>column_values[i][a]));
						}
					}
					else
					{
						bool finished=false;
						for(std::size_t i=0;i<column_values.size() && !finished;i++)
						{
							if(column_values[i][a]>column_values[i][b])
							{
								win_a=true;
								win_b=false;
								finished=true;
							}
							else if(column_values[i][a]<column_values[i][b])
							{
								win_a=false;
								win_b=true;
								finished=true;
							}
						}
					}

					if(win_a==win_b)
					{
						sortable_ids[a].first.second++;
						sortable_ids[b].first.second++;
					}
					else if(win_a)
					{
						sortable_ids[a].first.first++;
					}
					else if(win_b)
					{
						sortable_ids[b].first.first++;
					}
				}
			}

			std::sort(sortable_ids.begin(), sortable_ids.end());
			std::reverse(sortable_ids.begin(), sortable_ids.end());

			{
				foutput << table_header_line;
				if(!add_win_score_column.empty())
				{
					foutput << " " << add_win_score_column;
				}
				if(!add_rank_column.empty())
				{
					foutput << " " << add_rank_column;
				}
				foutput << "\n";
				for(std::size_t a=0;a<N;a++)
				{
					foutput << table_row_lines[sortable_ids[a].second];
					if(!add_win_score_column.empty())
					{
						foutput << " " << sortable_ids[a].first.first << "." << sortable_ids[a].first.second;
					}
					if(!add_rank_column.empty())
					{
						foutput << " " << (a+1);
					}
					foutput << "\n";
				}
			}
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_TOURNAMENT_SORT_H_ */
