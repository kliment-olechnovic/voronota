#ifndef SCRIPTING_OPERATORS_EXPORT_GLOBAL_ADJUNCTS_H_
#define SCRIPTING_OPERATORS_EXPORT_GLOBAL_ADJUNCTS_H_

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

	ExportGlobalAdjuncts() : all(false)
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		query=Utilities::read_congregation_of_data_managers_object_query(input);
		all=input.get_flag("all");
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		Utilities::document_read_congregation_of_data_managers_object_query(doc);
		doc.set_option_decription(CDOD("all", CDOD::DATATYPE_BOOL, "flag to export all adjuncts"));
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "adjunct names", ""));
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
				throw std::runtime_error(std::string("Selected atoms have no adjuncts."));
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

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		output << "ID";
		for(std::size_t i=0;i<adjuncts_filled.size();i++)
		{
			output << " " << adjuncts_filled[i];
		}
		output << "\n";

		for(std::size_t i=0;i<objects.size();i++)
		{
			output << congregation_of_data_managers.get_object_attributes(objects[i]).name;
			const DataManager& dm=(*(objects[i]));
			for(std::size_t j=0;j<adjuncts_filled.size();j++)
			{
				std::map<std::string, double>::const_iterator it=dm.global_numeric_adjuncts().find(adjuncts_filled[j]);
				output << " ";
				if(it!=dm.global_numeric_adjuncts().end())
				{
					output << (it->second);
				}
				else
				{
					output << "NA";
				}
			}
			output << "\n";
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_GLOBAL_ADJUNCTS_H_ */
