#ifndef SCRIPTING_OPERATORS_EXPORT_GLOBAL_ADJUNCTS_H_
#define SCRIPTING_OPERATORS_EXPORT_GLOBAL_ADJUNCTS_H_

#include <algorithm>

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportGlobalAdjuncts : public OperatorBase<ExportGlobalAdjuncts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string file;
	CongregationOfDataManagers::ObjectQuery query;
	bool all;
	std::vector<std::string> adjuncts;
	std::string sort_inc;
	std::string sort_dec;
	bool format_columns;
	bool no_header;

	ExportGlobalAdjuncts() : all(false), format_columns(false), no_header(false)
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		all=input.get_flag("all");
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
		sort_inc=input.get_value_or_default<std::string>("sort-inc", "");
		sort_dec=input.get_value_or_default<std::string>("sort-dec", "");
		format_columns=input.get_flag("format-columns");
		no_header=input.get_flag("no-header");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		doc.set_option_decription(CDOD("all", CDOD::DATATYPE_BOOL, "flag to export all adjuncts"));
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "adjunct names", ""));
		doc.set_option_decription(CDOD("sort-inc", CDOD::DATATYPE_STRING, "adjunct to sort in increasing order", ""));
		doc.set_option_decription(CDOD("sort-dec", CDOD::DATATYPE_STRING, "adjunct to sort in decreasing order", ""));
		doc.set_option_decription(CDOD("format-columns", CDOD::DATATYPE_BOOL, "flag to format columns"));
		doc.set_option_decription(CDOD("no-header", CDOD::DATATYPE_BOOL, "flag to not output header"));
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		assert_file_name_input(file, false);

		if(!all && adjuncts.empty())
		{
			throw std::runtime_error(std::string("No adjuncts specified."));
		}

		if(all && !adjuncts.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of adjuncts."));
		}

		if(!sort_inc.empty() && !sort_dec.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of sorting adjunct."));
		}

		const std::vector<DataManager*> objects=congregation_of_data_managers.get_objects(query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		std::vector<std::string> adjuncts_filled;

		if(all)
		{
			std::set<std::string> all_adjuncts;
			for(std::size_t i=0;i<objects.size();i++)
			{
				const DataManager& dm=(*(objects[i]));
				for(std::map<std::string, double>::const_iterator it=dm.global_numeric_adjuncts().begin();it!=dm.global_numeric_adjuncts().end();++it)
				{
					all_adjuncts.insert(it->first);
				}
			}
			if(all_adjuncts.empty())
			{
				throw std::runtime_error(std::string("Selected objects have no adjuncts."));
			}
			adjuncts_filled=std::vector<std::string>(all_adjuncts.begin(), all_adjuncts.end());
		}
		else
		{
			adjuncts_filled=adjuncts;
		}

		if(adjuncts_filled.empty())
		{
			throw std::runtime_error(std::string("No adjuncts specified."));
		}

		if(!sort_inc.empty() && std::find(adjuncts_filled.begin(), adjuncts_filled.end(), sort_inc)==adjuncts_filled.end())
		{
			throw std::runtime_error(std::string("No adjunct for sorting."));
		}

		if(!sort_dec.empty() && std::find(adjuncts_filled.begin(), adjuncts_filled.end(), sort_dec)==adjuncts_filled.end())
		{
			throw std::runtime_error(std::string("No adjunct for sorting."));
		}

		std::vector<std::string> header;
		header.push_back("ID");
		header.insert(header.end(), adjuncts_filled.begin(), adjuncts_filled.end());

		std::vector<ContentRow> rows;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const DataManager& dm=(*(objects[i]));
			rows.push_back(ContentRow(i));
			ContentRow& row=rows.back();
			row.add_value(congregation_of_data_managers.get_object_attributes(objects[i]).name);
			for(std::size_t j=0;j<adjuncts_filled.size();j++)
			{
				std::map<std::string, double>::const_iterator it=dm.global_numeric_adjuncts().find(adjuncts_filled[j]);
				if(it!=dm.global_numeric_adjuncts().end())
				{
					row.add_value(it->second);
					if(sort_inc==it->first)
					{
						row.sort_value=it->second;
					}
					if(sort_dec==it->first)
					{
						row.sort_value=0.0-(it->second);
					}
				}
				else
				{
					row.add_value("NA");
				}
			}
		}

		std::sort(rows.begin(), rows.end());

		std::vector<std::size_t> column_widths(header.size(), 0);

		if(format_columns)
		{
			update_column_widths(column_widths, header);
			for(std::size_t i=0;i<rows.size();i++)
			{
				update_column_widths(column_widths, rows[i].values);
			}
		}

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		if(!no_header)
		{
			output_vector_columns(column_widths, header, output);
		}

		for(std::size_t i=0;i<rows.size();i++)
		{
			output_vector_columns(column_widths, rows[i].values, output);
		}

		Result result;

		return result;
	}

private:
	class ContentRow
	{
	public:
		std::size_t original_row_number;
		double sort_value;
		std::vector<std::string> values;

		explicit ContentRow(const std::size_t index) : original_row_number(index), sort_value(std::numeric_limits<double>::max())
		{
		}

		template<typename T>
		void add_value(const T& value)
		{
			std::ostringstream output;
			output << value;
			values.push_back(output.str());
		}

		bool operator<(const ContentRow& cr) const
		{
			if(sort_value<cr.sort_value)
			{
				return true;
			}
			else if(sort_value==cr.sort_value && original_row_number<cr.original_row_number)
			{
				return true;
			}
			return false;
		}
	};

	static void update_column_widths(std::vector<std::size_t>& column_widths, const std::vector<std::string>& values)
	{
		for(std::size_t j=0;j<column_widths.size() && j<values.size();j++)
		{
			column_widths[j]=std::max(column_widths[j], values[j].size());
		}
	}

	static void output_vector_columns(const std::vector<std::size_t>& column_widths, const std::vector<std::string>& values, std::ostream& output)
	{
		for(std::size_t j=0;j<column_widths.size() && j<values.size();j++)
		{
			output << values[j];
			if(column_widths[j]>values[j].size())
			{
				std::size_t padding=(column_widths[j]-values[j].size());
				output << std::string(padding, ' ');
			}
			if(j+1<values.size())
			{
				output << "  ";
			}
		}
		output << "\n";
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_GLOBAL_ADJUNCTS_H_ */
