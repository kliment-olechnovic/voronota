#ifndef SCRIPTING_OPERATORS_SET_CHAIN_NAME_H_
#define SCRIPTING_OPERATORS_SET_CHAIN_NAME_H_

#include "../operators_common.h"

#if __cplusplus >= 201402L
#include <random>
#endif

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetChainName : public OperatorBase<SetChainName>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		std::string chain_name;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("chain_name")=chain_name;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string chain_name;

	SetChainName()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		chain_name=input.get_value_or_first_unused_unnamed_value("chain-name");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("chain-name", CDOD::DATATYPE_STRING, "chain name (e.g B) or chain renaming rule (e.g. A=B,B=A)"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(chain_name.empty())
		{
			throw std::runtime_error(std::string("No chain name or chain renaming rule provided."));
		}

		std::map<std::string, std::string> renaming_map;
		if(chain_name=="_shuffle")
		{
			std::set<std::string> set_of_chain_names;
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				set_of_chain_names.insert(data_manager.atoms()[i].crad.chainID);
			}
			std::vector<std::string> list_of_chain_names_original(set_of_chain_names.begin(), set_of_chain_names.end());
			std::vector<std::string> list_of_chain_names_shuffled=list_of_chain_names_original;

#if __cplusplus >= 201402L
			std::random_device rd;
			std::mt19937 rng(rd());
			std::shuffle(list_of_chain_names_shuffled.begin(), list_of_chain_names_shuffled.end(), rng);
#else
			std::random_shuffle(list_of_chain_names_shuffled.begin(), list_of_chain_names_shuffled.end());
#endif

			for(std::size_t i=0;i<list_of_chain_names_original.size();i++)
			{
				renaming_map[list_of_chain_names_original[i]]=list_of_chain_names_shuffled[i];
			}
		}
		else if(chain_name=="_invert_case")
		{
			std::set<std::string> set_of_chain_names;
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				set_of_chain_names.insert(data_manager.atoms()[i].crad.chainID);
			}
			for(std::set<std::string>::const_iterator it=set_of_chain_names.begin();it!=set_of_chain_names.end();++it)
			{
				std::string mod_str=(*it);
				for(std::size_t i=0;i<mod_str.size();++i)
				{
					char& c=mod_str[i];
					if(c>='A' && c<='Z')
					{
						c=(c-'A')+'a';
					}
					else if(c>='a' && c<='z')
					{
						c=(c-'a')+'A';
					}
				}
				renaming_map[*it]=mod_str;
			}
		}
		else
		{
			renaming_map=read_renaming_map(chain_name);
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::vector<Atom> atoms=data_manager.atoms();

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			if(renaming_map.empty())
			{
				atoms[*it].crad.chainID=chain_name;
			}
			else
			{
				std::string& chainID=atoms[*it].crad.chainID;
				std::map<std::string, std::string>::const_iterator renaming_map_it=renaming_map.find(chainID);
				if(renaming_map_it!=renaming_map.end())
				{
					chainID=renaming_map_it->second;
				}
			}
		}

		data_manager.reset_atoms_by_swapping(atoms);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);
		result.chain_name=chain_name;

		return result;
	}

private:
	static std::map<std::string, std::string> read_renaming_map(const std::string& rule_str)
	{
		std::map<std::string, std::string> renaming_map;

		if(rule_str.find('=')==std::string::npos)
		{
			return renaming_map;
		}

		{
			std::string rule_str_mod=rule_str;
			for(std::size_t i=0;i<rule_str.size();i++)
			{
				if(rule_str[i]==',')
				{
					rule_str_mod[i]=' ';
				}
			}
			std::istringstream list_input(rule_str_mod);
			while(list_input.good())
			{
				std::string pair_token;
				list_input >> pair_token >> std::ws;
				if(pair_token.find('=')==std::string::npos)
				{
					throw std::runtime_error(std::string("Invalid pair '")+pair_token+"' in chain renaming rule.");
				}
				else
				{
					pair_token[pair_token.find('=')]=' ';
					std::istringstream pair_input(pair_token);
					std::string name1;
					std::string name2;
					pair_input >> name1 >> name2;
					if(name1.empty() || name2.empty())
					{
						throw std::runtime_error(std::string("Incomplete pair '")+pair_token+"' in chain renaming rule.");
					}
					else
					{
						renaming_map[name1]=name2;
					}
				}
			}
		}

		return renaming_map;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_CHAIN_NAME_H_ */
