#ifndef DUKTAPER_OPERATORS_VOROMQA_DARK_SPLIT_H_
#define DUKTAPER_OPERATORS_VOROMQA_DARK_SPLIT_H_

#include "../../../src/scripting/operators/voromqa_global.h"
#include "../../../src/scripting/operators/voromqa_local.h"

#include "voromqa_dark_global.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class VoroMQADarkSplit : public scripting::OperatorBase<VoroMQADarkSplit>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		struct SubResult
		{
			scripting::operators::VoroMQAGlobal::Result light_result;
			scripting::operators::VoroMQALocal::Result light_result_local_sas;
			VoroMQADarkGlobal::Result dark_result;

			void add(const SubResult& r)
			{
				light_result.add(r.light_result);
				light_result_local_sas.add(r.light_result_local_sas);
				dark_result.add(r.dark_result);
			}
		};

		std::map<std::string, SubResult> split_results;
		SubResult summarized_result;

		Result()
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			for(std::map<std::string, SubResult>::const_iterator it=split_results.begin();it!=split_results.end();++it)
			{
				scripting::VariantObject o;
				o.value("chain_group")=it->first;
				const SubResult& sub_result=it->second;
				sub_result.light_result.write_to_variant_object(o.object("light_scores"));
				sub_result.light_result_local_sas.write_to_variant_object(o.object("light_scores_local_sas"));
				sub_result.dark_result.write_to_variant_object(o.object("dark_scores"));
				heterostorage.variant_object.objects_array("split_results").push_back(o);
			}
			summarized_result.light_result.write_to_variant_object(heterostorage.variant_object.object("light_scores"));
			summarized_result.light_result_local_sas.write_to_variant_object(heterostorage.variant_object.object("light_scores_local_sas"));
			summarized_result.dark_result.write_to_variant_object(heterostorage.variant_object.object("dark_scores"));
		}

		void summarize()
		{
			summarized_result=SubResult();
			for(std::map<std::string, SubResult>::const_iterator it=split_results.begin();it!=split_results.end();++it)
			{
				summarized_result.add(it->second);
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
			scripting::operators::ConstructContacts().init().run(chain_group_data_manager);
			Result::SubResult sub_result;
			sub_result.light_result=scripting::operators::VoroMQAGlobal().init().run(chain_group_data_manager);
			sub_result.light_result_local_sas=scripting::operators::VoroMQALocal().init(CMDIN().set("contacts","[--solvent]")).run(chain_group_data_manager);
			sub_result.dark_result=VoroMQADarkGlobal().init().run(chain_group_data_manager);
			result.split_results[chain_group]=sub_result;
		}

		result.summarize();

		if(!global_adj_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_light_quality_score"]=result.summarized_result.light_result.quality_score;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_light_atoms_count"]=result.summarized_result.light_result.atoms_count;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_light_residues_count"]=result.summarized_result.light_result.residues_count;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_light_contacts_count"]=result.summarized_result.light_result.contacts_count;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_light_total_area"]=result.summarized_result.light_result.total_area;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_light_strange_area"]=result.summarized_result.light_result.strange_area;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_light_pseudo_energy"]=result.summarized_result.light_result.pseudo_energy;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_light_sas_area"]=result.summarized_result.light_result_local_sas.area;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_light_sas_pseudo_energy"]=result.summarized_result.light_result_local_sas.pseudo_energy;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_dark_quality_score"]=result.summarized_result.dark_result.global_score;
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_dark_residues_count"]=result.summarized_result.dark_result.number_of_residues;
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_VOROMQA_DARK_SPLIT_H_ */
