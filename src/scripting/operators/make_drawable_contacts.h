#ifndef SCRIPTING_OPERATORS_MAKE_DRAWABLE_CONTACTS_H_
#define SCRIPTING_OPERATORS_MAKE_DRAWABLE_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

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

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts;
	SelectionManager::Query parameters_for_selecting;

	MakeDrawableContacts()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_to_draw_contacts=common::ConstructionOfContacts::ParametersToDrawContacts();
		parameters_to_draw_contacts.step=input.get_value_or_default<double>("step", parameters_to_draw_contacts.step);
		parameters_to_draw_contacts.projections=input.get_value_or_default<int>("projections", parameters_to_draw_contacts.projections);
		parameters_to_draw_contacts.simplify=input.get_flag("simplify");
		parameters_to_draw_contacts.sih_depth=input.get_value_or_default<int>("sih-depth", parameters_to_draw_contacts.sih_depth);
		parameters_to_draw_contacts.enable_alt=input.get_flag("enable-alt");
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
	}

	void document(CommandDocumentation& doc) const
	{
		common::ConstructionOfContacts::ParametersToDrawContacts params;
		doc.set_option_decription(CDOD("step", CDOD::DATATYPE_FLOAT, "edge step size", params.step));
		doc.set_option_decription(CDOD("projections", CDOD::DATATYPE_INT, "number of projections for edge calculation", params.projections));
		doc.set_option_decription(CDOD("simplify", CDOD::DATATYPE_BOOL, "flag to simplify graphics"));
		doc.set_option_decription(CDOD("sih-depth", CDOD::DATATYPE_FLOAT, "icosahedron subdivision depth for SAS calculation", params.sih_depth));
		doc.set_option_decription(CDOD("enable-alt", CDOD::DATATYPE_BOOL, "flag to enable alternative graphics"));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts_to_use=parameters_to_draw_contacts;
		parameters_to_draw_contacts_to_use.probe=data_manager.history_of_actions_on_contacts().probe();

		data_manager.reset_contacts_graphics_by_creating(parameters_to_draw_contacts_to_use, ids, false);

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_MAKE_DRAWABLE_CONTACTS_H_ */
