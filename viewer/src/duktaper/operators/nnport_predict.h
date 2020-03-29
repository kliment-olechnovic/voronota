#ifndef DUKTAPER_OPERATORS_NNPORT_PREDICT_H_
#define DUKTAPER_OPERATORS_NNPORT_PREDICT_H_

#include "../operators_common.h"

#include "../../nnport/predict_for_table.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class NNPortPredict : public scripting::operators::OperatorBase<NNPortPredict>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		std::size_t number_of_output_lines;

		Result() : number_of_output_lines(0)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			scripting::VariantObject& info=heterostorage.variant_object.object("output_summary");
			info.value("number_of_output_lines")=number_of_output_lines;
		}
	};

	std::string input_label_column_name;
	std::vector<std::string> input_value_column_names;
	std::string input_statistics_file;
	std::vector<std::string> input_model_files;
	std::string output_label_column_name;
	std::vector<std::string> output_value_column_names;
	std::string input_data_file;
	std::string output_data_file;

	NNPortPredict()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		input_label_column_name=input.get_value<std::string>("input-label-column-name");
		input_value_column_names=input.get_value_vector<std::string>("input-value-column-names");
		input_statistics_file=input.get_value<std::string>("input-statistics-file");
		input_model_files=input.get_value_vector<std::string>("input-model-files");
		output_label_column_name=input.get_value<std::string>("output-label-column-name");
		output_value_column_names=input.get_value_vector<std::string>("output-value-column-names");
		input_data_file=input.get_value<std::string>("input-data-file");
		output_data_file=input.get_value<std::string>("output-data-file");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("input-label-column-name", scripting::CDOD::DATATYPE_STRING, "input label column name"));
		doc.set_option_decription(scripting::CDOD("input-value-column-names", scripting::CDOD::DATATYPE_STRING_ARRAY, "input value column names"));
		doc.set_option_decription(scripting::CDOD("input-statistics-file", scripting::CDOD::DATATYPE_STRING, "input statistics file"));
		doc.set_option_decription(scripting::CDOD("input-model-files", scripting::CDOD::DATATYPE_STRING_ARRAY, "input model files"));
		doc.set_option_decription(scripting::CDOD("output-label-column-name", scripting::CDOD::DATATYPE_STRING, "output label column name"));
		doc.set_option_decription(scripting::CDOD("output-value-column-names", scripting::CDOD::DATATYPE_STRING_ARRAY, "output value column names"));
		doc.set_option_decription(scripting::CDOD("input-data-file", scripting::CDOD::DATATYPE_STRING, "input data file"));
		doc.set_option_decription(scripting::CDOD("output-data-file", scripting::CDOD::DATATYPE_STRING, "output data file"));
	}

	Result run(void*&) const
	{
		nnport::StatisticsMap statistics_map;
		{
			scripting::InputSelector finput_selector(input_statistics_file);
			std::istream& finput=finput_selector.stream();
			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read statistics file '")+input_statistics_file+"'.");
			}
			statistics_map=nnport::StatisticsMap(finput);
		}

		std::vector<fdeep::model> models;
		for(std::size_t i=0;i<input_model_files.size();i++)
		{
			scripting::InputSelector finput_selector(input_model_files[i]);
			std::istream& finput=finput_selector.stream();
			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read model file '")+input_model_files[i]+"'.");
			}
			models.push_back(fdeep::read_model(finput, true, nnport::silent_logger));
		}

		scripting::InputSelector finput_selector(input_data_file);
		std::istream& finput=finput_selector.stream();
		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read data file '")+input_data_file+"'.");
		}

		scripting::OutputSelector foutput_selector(output_data_file);
		std::ostream& foutput=foutput_selector.stream();
		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to write data file '")+output_data_file+"'.");
		}

		Result result;

		result.number_of_output_lines=nnport::predict_for_table(
				input_label_column_name, input_value_column_names, statistics_map, models, output_label_column_name, output_value_column_names, finput, foutput);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_NNPORT_PREDICT_H_ */
