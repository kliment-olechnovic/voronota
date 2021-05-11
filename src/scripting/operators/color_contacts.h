#ifndef SCRIPTING_OPERATORS_COLOR_CONTACTS_H_
#define SCRIPTING_OPERATORS_COLOR_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ColorContacts : public OperatorBase<ColorContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> representation_names;
	bool next_random_color;
	auxiliaries::ColorUtilities::ColorInteger color_value;

	ColorContacts() : next_random_color(false), color_value(auxiliaries::ColorUtilities::null_color())
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		next_random_color=input.get_flag("next-random-color");
		color_value=(next_random_color ? OperatorsUtilities::get_next_random_color() : OperatorsUtilities::read_color(input));
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("rep", CDOD::DATATYPE_STRING_ARRAY, "representation names", ""));
		doc.set_option_decription(CDOD("next-random-color", CDOD::DATATYPE_BOOL, "flag to use next random color"));
		OperatorsUtilities::document_read_color(doc);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_representations_availability();

		const std::set<std::size_t> representation_ids=data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

		if(!auxiliaries::ColorUtilities::color_valid(color_value))
		{
			throw std::runtime_error(std::string("Contacts color not specified."));
		}

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_color(color_value), ids);

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_COLOR_CONTACTS_H_ */
