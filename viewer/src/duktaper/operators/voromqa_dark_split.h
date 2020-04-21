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
				o.value("chain")=it->first;
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

	std::string global_adj_prefix;

	VoroMQADarkSplit()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "voromqa_dark_split");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("global-adj-prefix", scripting::CDOD::DATATYPE_STRING, "prefix for output global adjuncts", "voromqa_dark_split"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		if(data_manager.primary_structure_info().chains.size()<2)
		{
			throw std::runtime_error(std::string("Less than two chains available."));
		}

		Result result;

		for(std::size_t i=0;i<data_manager.primary_structure_info().chains.size();i++)
		{
			const std::string& chain_name=data_manager.primary_structure_info().chains[i].name;
			std::vector<scripting::Atom> chain_atoms;
			for(std::size_t j=0;j<data_manager.atoms().size();j++)
			{
				const scripting::Atom& atom=data_manager.atoms()[j];
				if(atom.crad.chainID==chain_name)
				{
					chain_atoms.push_back(atom);
				}
			}
			if(chain_atoms.empty())
			{
				throw std::runtime_error(std::string("No atoms in chain '")+chain_name+"'.");
			}
			scripting::DataManager chain_data_manager;
			chain_data_manager.reset_atoms_by_swapping(chain_atoms);
			chain_data_manager.add_atoms_representation("atoms", true);
			scripting::operators::ConstructContacts().init("").run(chain_data_manager);
			scripting::operators::VoroMQAGlobal().init("").run(chain_data_manager);
			VoroMQADarkGlobal::Result chain_result=VoroMQADarkGlobal().init("").run(chain_data_manager);
			result.split_results[chain_name]=chain_result;
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
