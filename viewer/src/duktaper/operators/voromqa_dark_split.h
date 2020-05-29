#ifndef DUKTAPER_OPERATORS_VOROMQA_DARK_SPLIT_H_
#define DUKTAPER_OPERATORS_VOROMQA_DARK_SPLIT_H_

#include "voromqa_dark_global.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class VoroMQADarkSplit : public scripting::operators::OperatorBase<VoroMQADarkSplit>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		std::map<std::string, VoroMQADarkGlobal::Result> split_results;
		double global_score;
		int number_of_residues;

		Result() : global_score(0.0), number_of_residues(0)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			for(std::map<std::string, VoroMQADarkGlobal::Result>::const_iterator it=split_results.begin();it!=split_results.end();++it)
			{
				scripting::VariantObject o;
				o.value("chain_group")=it->first;
				o.value("global_score")=it->second.global_score;
				o.value("number_of_residues")=it->second.number_of_residues;
				heterostorage.variant_object.objects_array("split_results").push_back(o);
			}
			heterostorage.variant_object.value("global_score")=global_score;
			heterostorage.variant_object.value("number_of_residues")=number_of_residues;
		}

		void summarize()
		{
			global_score=0.0;
			number_of_residues=0;
			for(std::map<std::string, VoroMQADarkGlobal::Result>::const_iterator it=split_results.begin();it!=split_results.end();++it)
			{
				const VoroMQADarkGlobal::Result& r=it->second;
				global_score+=r.global_score*static_cast<double>(r.number_of_residues);
				number_of_residues+=r.number_of_residues;
			}
			if(number_of_residues>0)
			{
				global_score=global_score/static_cast<double>(number_of_residues);
			}
			else
			{
				global_score=0.0;
			}
		}
	};

	std::vector<std::string> chain_groups;
	std::string global_adj_prefix;

	VoroMQADarkSplit()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		chain_groups=input.get_value_vector_or_default<std::string>("chain-groups", std::vector<std::string>());
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "voromqa_dark_split");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "chain groups", ""));
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", "voromqa_dark_split"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		if(data_manager.primary_structure_info().chains.size()<2)
		{
			throw std::runtime_error(std::string("Less than two chains available."));
		}

		std::map< std::string, std::set<std::string> > map_of_chain_groups;
		if(!chain_groups.empty())
		{
			for(std::size_t i=0;i<chain_groups.size();i++)
			{
				const std::string& group=chain_groups[i];
				for(std::size_t j=0;j<group.size();j++)
				{
					map_of_chain_groups[group].insert(std::string(1, group[j]));
				}
			}
		}
		else
		{
			for(std::size_t i=0;i<data_manager.primary_structure_info().chains.size();i++)
			{
				const std::string& chain_name=data_manager.primary_structure_info().chains[i].name;
				map_of_chain_groups[chain_name].insert(chain_name);
			}
		}

		Result result;

		for(std::map< std::string, std::set<std::string> >::const_iterator chain_group_it=map_of_chain_groups.begin();chain_group_it!=map_of_chain_groups.end();++chain_group_it)
		{
			const std::string& chain_group=chain_group_it->first;
			const std::set<std::string>& chain_names=chain_group_it->second;
			std::vector<scripting::Atom> chain_group_atoms;
			for(std::size_t j=0;j<data_manager.atoms().size();j++)
			{
				const scripting::Atom& atom=data_manager.atoms()[j];
				if(chain_names.count(atom.crad.chainID)>0)
				{
					chain_group_atoms.push_back(atom);
				}
			}
			if(chain_group_atoms.empty())
			{
				throw std::runtime_error(std::string("No atoms for chain group '")+chain_group+"'.");
			}
			scripting::DataManager chain_group_data_manager;
			chain_group_data_manager.reset_atoms_by_swapping(chain_group_atoms);
			chain_group_data_manager.add_atoms_representation("atoms", true);
			scripting::operators::ConstructContacts().init("").run(chain_group_data_manager);
			scripting::operators::VoroMQAGlobal().init("").run(chain_group_data_manager);
			VoroMQADarkGlobal::Result chain_group_result=VoroMQADarkGlobal().init("").run(chain_group_data_manager);
			result.split_results[chain_group]=chain_group_result;
		}

		result.summarize();

		if(!global_adj_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_quality_score"]=result.global_score;
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_VOROMQA_DARK_SPLIT_H_ */
