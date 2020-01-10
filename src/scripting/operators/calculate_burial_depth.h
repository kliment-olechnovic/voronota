#ifndef SCRIPTING_OPERATORS_CALCULATE_BURIAL_DEPTH_H_
#define SCRIPTING_OPERATORS_CALCULATE_BURIAL_DEPTH_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CalculateBurialDepth : public OperatorBase<CalculateBurialDepth>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string name;
	int min_seq_sep;

	CalculateBurialDepth() : min_seq_sep(1)
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value<std::string>("name");
		min_seq_sep=input.get_value_or_default<int>("min-seq-sep", 1);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "adjunct name to write value in atoms"));
		doc.set_option_decription(CDOD("min-seq-sep", CDOD::DATATYPE_INT, "minimal sequence separation restriction for contacts", 1));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(name, false);

		std::set<common::ChainResidueAtomDescriptorsPair> set_of_contacts;
		for(std::size_t i=0;i<data_manager.contacts().size();i++)
		{
			const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[i]);
			if(common::ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crads.a, crads.b, min_seq_sep, common::ChainResidueAtomDescriptor::null_num(), true))
			{
				set_of_contacts.insert(crads);
			}
		}

		const std::map<common::ChainResidueAtomDescriptor, int> map_crad_to_depth=common::ChainResidueAtomDescriptorsGraphOperations::calculate_burial_depth_values(set_of_contacts);

		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			const Atom& atom=data_manager.atoms()[i];
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(i);
			atom_adjuncts.erase(name);
			std::map<common::ChainResidueAtomDescriptor, int>::const_iterator it=map_crad_to_depth.find(atom.crad);
			if(it!=map_crad_to_depth.end())
			{
				atom_adjuncts[name]=it->second;
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CALCULATE_BURIAL_DEPTH_H_ */
