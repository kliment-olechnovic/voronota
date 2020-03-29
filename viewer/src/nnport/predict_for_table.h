#ifndef NNPORT_PREDICT_FOR_TABLE_H_
#define NNPORT_PREDICT_FOR_TABLE_H_

#include "predict_for_vector.h"
#include "table.h"
#include "statistics_map.h"

namespace voronota
{

namespace nnport
{

inline std::size_t predict_for_table(
		const std::string& input_label_column_name,
		const std::vector<std::string>& input_value_column_names,
		const StatisticsMap& statistics_map,
		const std::vector<fdeep::model>& models,
		const std::string& output_label_column_name,
		const std::vector<std::string>& output_value_column_names,
		std::istream& input_stream,
		std::ostream& output_stream)
{
	if(input_label_column_name.empty())
	{
		throw std::runtime_error(std::string("No input label column name"));
	}

	if(input_value_column_names.empty())
	{
		throw std::runtime_error(std::string("No input value column names"));
	}

	if(models.empty())
	{
		throw std::runtime_error(std::string("No models"));
	}

	if(output_label_column_name.empty())
	{
		throw std::runtime_error(std::string("No output label column name"));
	}

	if(output_value_column_names.empty())
	{
		throw std::runtime_error(std::string("No output value column names"));
	}

	nnport::Table input_table(input_stream, 1, 1);
	const std::size_t input_label_column_id=input_table.select_column(input_label_column_name);
	const std::size_t category_column_id=input_table.select_column("category");
	const std::vector<std::size_t> input_values_column_ids=input_table.select_columns(input_value_column_names);

	std::size_t output_counter=0;

	while(input_table.rows().size()==1)
	{
		const std::string category=input_table.get_value<std::string>(category_column_id, 0);
		const std::vector<float> raw_values=input_table.get_row_values<float>(input_values_column_ids, 0);
		std::vector<float> normalized_values;
		normalized_values.reserve(raw_values.size());
		for(std::size_t i=0;i<raw_values.size();i++)
		{
			const std::string& value_name=input_value_column_names[i];
			const StatisticsMap::Statistics svs=statistics_map.get_statistics(category, value_name);
			const double normalized_value=((raw_values[i]-svs.mean)/svs.sd);
			normalized_values.push_back(normalized_value);
		}
		const std::vector<float> output_vector=predict_for_vector(models, normalized_values);
		if(output_vector.size()!=output_value_column_names.size())
		{
			throw std::runtime_error(std::string("Invalid number of output columns"));
		}
		if(output_counter==0)
		{
			output_stream << output_label_column_name;
			for(std::size_t i=0;i<output_value_column_names.size();i++)
			{
				output_stream << " " << output_value_column_names[i];
			}
			output_stream << "\n";
		}
		{
			output_stream << input_table.get_value<std::string>(input_label_column_id, 0);
			for(std::size_t i=0;i<output_vector.size();i++)
			{
				output_stream << " " << output_vector[i];
			}
			output_stream << "\n";
		}
		output_counter++;
		input_table.clear_rows();
		input_table.add_row(input_stream);
	}

	return output_counter;
}

}

}

#endif /* NNPORT_PREDICT_FOR_TABLE_H_ */
