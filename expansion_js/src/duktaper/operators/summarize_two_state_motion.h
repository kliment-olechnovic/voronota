#ifndef DUKTAPER_OPERATORS_SUMMARIZE_TWO_STATE_MOTION_H_
#define DUKTAPER_OPERATORS_SUMMARIZE_TWO_STATE_MOTION_H_

#include "../../dependencies/utilities/misc_eigen_wrappers.h"

#include "../operators_common.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class SummarizeTwoStateMotion : public scripting::OperatorBase<SummarizeTwoStateMotion>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string effect;
		scripting::SummaryOfAtoms atoms_summary;
		std::vector<std::string> object_names;

		Result() : effect("loaded")
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			scripting::VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.summaries_of_atoms[effect]=atoms_summary;
			std::vector<scripting::VariantValue>& object_names_array=heterostorage.variant_object.values_array("object_names");
			for(std::size_t i=0;i<object_names.size();i++)
			{
				object_names_array.push_back(scripting::VariantValue(object_names[i]));
			}
		}
	};

	std::string first_name;
	std::string second_name;
	std::string result_name;

	SummarizeTwoStateMotion()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		first_name=input.get_value_or_first_unused_unnamed_value("first");
		second_name=input.get_value_or_first_unused_unnamed_value("second");
		result_name=input.get_value_or_first_unused_unnamed_value("result");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("first", CDOD::DATATYPE_STRING, "name of first object"));
		doc.set_option_decription(CDOD("second", CDOD::DATATYPE_STRING, "name of second object"));
		doc.set_option_decription(CDOD("result", CDOD::DATATYPE_STRING, "name of result object"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(first_name.empty())
		{
			throw std::runtime_error(std::string("No first object name provided."));
		}

		if(second_name.empty())
		{
			throw std::runtime_error(std::string("No second object name provided."));
		}

		if(result_name.empty())
		{
			throw std::runtime_error(std::string("No result object name provided."));
		}

		if(first_name==second_name)
		{
			throw std::runtime_error(std::string("Equal first and second object names provided."));
		}

		congregation_of_data_managers.assert_object_availability(first_name);
		congregation_of_data_managers.assert_object_availability(second_name);

		scripting::DataManager* first_object=congregation_of_data_managers.get_object(first_name);
		scripting::DataManager* second_object=congregation_of_data_managers.get_object(second_name);

		std::vector<double> state_x=get_state_vector_from_object(*first_object);
		std::vector<double> state_y=get_state_vector_from_object(*second_object);

		std::vector<double> motion_state=misc_eigen_wrappers::summarize_two_state_motion_as_state(state_x, state_y);

		std::vector<scripting::Atom> atoms=first_object->atoms();
		for(std::size_t i=0;i<atoms.size();i++)
		{
			atoms[i].value.x=motion_state[i*3+0];
			atoms[i].value.y=motion_state[i*3+1];
			atoms[i].value.z=motion_state[i*3+2];
		}

		scripting:: DataManager* result_object=congregation_of_data_managers.add_object(scripting::DataManager(), result_name);
		result_object->reset_atoms_by_swapping(atoms);

		Result result;
		result.atoms_summary=scripting::SummaryOfAtoms(result_object->atoms());
		result.object_names.push_back(congregation_of_data_managers.get_object_attributes(result_object).name);

		return result;
	}

private:
	static std::vector<double> get_state_vector_from_object(const scripting::DataManager& data_manager)
	{
		std::vector<double> result(data_manager.atoms().size()*3, 0.0);
		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			result[i*3+0]=data_manager.atoms()[i].value.x;
			result[i*3+1]=data_manager.atoms()[i].value.y;
			result[i*3+2]=data_manager.atoms()[i].value.z;
		}
		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_SUMMARIZE_TWO_STATE_MOTION_H_ */
