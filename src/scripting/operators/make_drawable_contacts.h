#ifndef SCRIPTING_OPERATORS_MAKE_DRAWABLE_CONTACTS_H_
#define SCRIPTING_OPERATORS_MAKE_DRAWABLE_CONTACTS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class MakeDrawableContacts : public OperatorBase<MakeDrawableContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			return (*this);
		}
	};

	common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts;
	SelectionManager::Query parameters_for_selecting;

	MakeDrawableContacts()
	{
	}

	MakeDrawableContacts& init(CommandInput& input)
	{
		parameters_to_draw_contacts=common::ConstructionOfContacts::ParametersToDrawContacts();
		parameters_to_draw_contacts.probe=input.get_value_or_default<double>("probe", parameters_to_draw_contacts.probe);
		parameters_to_draw_contacts.step=input.get_value_or_default<double>("step", parameters_to_draw_contacts.step);
		parameters_to_draw_contacts.projections=input.get_value_or_default<int>("projections", parameters_to_draw_contacts.projections);
		parameters_to_draw_contacts.simplify=input.get_flag("simplify");
		parameters_to_draw_contacts.sih_depth=input.get_value_or_default<int>("sih-depth", parameters_to_draw_contacts.sih_depth);
		parameters_for_selecting=Utilities::read_generic_selecting_query("", "[--min-seq-sep 1]", input);
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		data_manager.reset_contacts_graphics_by_creating(parameters_to_draw_contacts, ids);

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_MAKE_DRAWABLE_CONTACTS_H_ */
