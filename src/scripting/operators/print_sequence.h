#ifndef SCRIPTING_OPERATORS_PRINT_SEQUENCE_H_
#define SCRIPTING_OPERATORS_PRINT_SEQUENCE_H_

#include "../operators_common.h"
#include "../../auxiliaries/residue_letters_coding.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class PrintSequence : public OperatorBase<PrintSequence>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		std::vector<VariantObject> chains;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.objects_array("chains")=chains;
		}

	};

	SelectionManager::Query parameters_for_selecting;
	bool secondary_structure;

	PrintSequence() : secondary_structure(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		secondary_structure=input.get_flag("secondary-structure");
	}

	void document(CommandDocumentation& doc) const
	{
		Utilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("secondary-structure", CDOD::DATATYPE_BOOL, "flag to print secondary structure information"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::set<std::size_t> residue_ids;
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			residue_ids.insert(data_manager.primary_structure_info().map_of_atoms_to_residues[*it]);
		}

		std::vector< std::vector<std::size_t> > grouping;
		{
			std::map<common::ChainResidueAtomDescriptor, std::size_t> ordering;
			for(std::set<std::size_t>::const_iterator it=residue_ids.begin();it!=residue_ids.end();++it)
			{
				const common::ConstructionOfPrimaryStructure::Residue& r=data_manager.primary_structure_info().residues[*it];
				ordering[r.chain_residue_descriptor]=(*it);
			}

			std::map<common::ChainResidueAtomDescriptor, std::size_t>::const_iterator it=ordering.begin();
			while(it!=ordering.end())
			{
				if(it==ordering.begin())
				{
					grouping.push_back(std::vector<std::size_t>(1, it->second));
				}
				else
				{
					std::map<common::ChainResidueAtomDescriptor, std::size_t>::const_iterator it_prev=it;
					--it_prev;
					if(it->first.chainID==it_prev->first.chainID && (it->first.resSeq-it_prev->first.resSeq)<=1)
					{
						grouping.back().push_back(it->second);
					}
					else
					{
						grouping.push_back(std::vector<std::size_t>(1, it->second));
					}
				}
				++it;
			}
		}

		std::map< std::string, std::vector<std::size_t> > chaining;
		for(std::size_t i=0;i<grouping.size();i++)
		{
			const common::ConstructionOfPrimaryStructure::Residue& r=data_manager.primary_structure_info().residues[grouping[i].front()];
			chaining[r.chain_residue_descriptor.chainID].push_back(i);
		}

		Result result;

		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=chaining.begin();it!=chaining.end();++it)
		{
			VariantObject chain_info;
			chain_info.value("chain_name")=(it->first);
			const std::vector<std::size_t>& group_ids=it->second;
			for(std::size_t i=0;i<group_ids.size();i++)
			{
				std::ostringstream output_for_residue_sequence;
				std::ostringstream output_for_secondary_structure_sequence;
				VariantObject segment_info;
				const std::vector<std::size_t>& group=grouping[group_ids[i]];
				segment_info.value("length")=group.size();
				if(!group.empty())
				{
					for(std::size_t j=0;j<group.size();j++)
					{
						{
							const common::ConstructionOfPrimaryStructure::Residue& r=data_manager.primary_structure_info().residues[group[j]];
							output_for_residue_sequence << auxiliaries::ResidueLettersCoding::convert_residue_code_big_to_small(r.chain_residue_descriptor.resName);
							if(j==0)
							{
								segment_info.values_array("range").push_back(VariantValue(r.chain_residue_descriptor.resSeq));
							}
							if((j+1)==group.size())
							{
								segment_info.values_array("range").push_back(VariantValue(r.chain_residue_descriptor.resSeq));
							}
						}
						if(secondary_structure)
						{
							const common::ConstructionOfSecondaryStructure::ResidueDescriptor& r=data_manager.secondary_structure_info().residue_descriptors[group[j]];
							if(r.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX)
							{
								output_for_secondary_structure_sequence << "H";
							}
							else if(r.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND)
							{
								output_for_secondary_structure_sequence << "S";
							}
							else
							{
								output_for_secondary_structure_sequence << "-";
							}
						}
					}
					segment_info.value("residue_sequence")=output_for_residue_sequence.str();
					if(secondary_structure)
					{
						segment_info.value("secondary_structure")=output_for_secondary_structure_sequence.str();
					}
				}
				chain_info.objects_array("continuous_segments").push_back(segment_info);
			}
			result.chains.push_back(chain_info);
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_PRINT_SEQUENCE_H_ */
