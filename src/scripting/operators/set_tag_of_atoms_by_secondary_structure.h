#ifndef SCRIPTING_OPERATORS_SET_TAG_OF_ATOMS_BY_SECONDARY_STRUCTURE_H_
#define SCRIPTING_OPERATORS_SET_TAG_OF_ATOMS_BY_SECONDARY_STRUCTURE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetTagOfAtomsBySecondaryStructure : public OperatorBase<SetTagOfAtomsBySecondaryStructure>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	std::string tag_for_alpha;
	std::string tag_for_beta;

	SetTagOfAtomsBySecondaryStructure() : tag_for_alpha("ss=H"), tag_for_beta("ss=S")
	{
	}

	void initialize(CommandInput& input)
	{
		tag_for_alpha=input.get_value_or_default<std::string>("tag-for-alpha", "ss=H");
		tag_for_beta=input.get_value_or_default<std::string>("tag-for-beta", "ss=S");
		assert_tag_input(tag_for_alpha, false);
		assert_tag_input(tag_for_beta, false);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("tag-for-alpha", CDOD::DATATYPE_STRING, "tag name for alpha helices", "ss=H"));
		doc.set_option_decription(CDOD("tag-for-beta", CDOD::DATATYPE_STRING, "tag name for beta sheets", "ss=S"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_tag_input(tag_for_alpha, false);
		assert_tag_input(tag_for_beta, false);

		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			std::set<std::string>& atom_tags=data_manager.atom_tags_mutable(i);
			atom_tags.erase(tag_for_alpha);
			atom_tags.erase(tag_for_beta);
		}

		std::set<std::size_t> affected_ids;

		for(std::size_t residue_id=0;residue_id<data_manager.secondary_structure_info().residue_descriptors.size();residue_id++)
		{
			const common::ConstructionOfSecondaryStructure::ResidueDescriptor& residue_descriptor=data_manager.secondary_structure_info().residue_descriptors[residue_id];
			if(residue_descriptor.secondary_structure_type!=common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_NULL)
			{
				const std::vector<std::size_t>& atom_ids=data_manager.primary_structure_info().residues[residue_id].atom_ids;
				for(std::size_t i=0;i<atom_ids.size();i++)
				{
					std::set<std::string>& atom_tags=data_manager.atom_tags_mutable(atom_ids[i]);
					if(residue_descriptor.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX)
					{
						atom_tags.insert(tag_for_alpha);
					}
					else if(residue_descriptor.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND)
					{
						atom_tags.insert(tag_for_beta);
					}
				}
				affected_ids.insert(atom_ids.begin(), atom_ids.end());
			}
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), affected_ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_TAG_OF_ATOMS_BY_SECONDARY_STRUCTURE_H_ */
