#ifndef SCRIPTING_OPERATORS_SET_ADJUNCTS_OF_ATOMS_BY_UFSR_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCTS_OF_ATOMS_BY_UFSR_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctsOfAtomsByTypeUFSR : public OperatorBase<SetAdjunctsOfAtomsByTypeUFSR>
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

	SelectionManager::Query parameters_for_selecting;
	std::string mode;
	int in_chain_sep_max;
	std::string name_prefix;

	SetAdjunctsOfAtomsByTypeUFSR() : in_chain_sep_max(30)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		mode=input.get_value_or_default<std::string>("mode", "simple");
		in_chain_sep_max=input.get_value_or_default<int>("in-chain-sep-max", 30);
		name_prefix=input.get_value<std::string>("name-prefix");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("mode", CDOD::DATATYPE_STRING, "distance calculation method, may be 'simple', 'out-chain', 'in-chain', 'in-chain-sep'"));
		doc.set_option_decription(CDOD("in-chain-sep-max", CDOD::DATATYPE_INT, "max sequence separation for 'in-chain-sep' mode"));
		doc.set_option_decription(CDOD("name-prefix", CDOD::DATATYPE_STRING, "adjuncts name prefix"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_adjunct_name_input(name_prefix+"a1", false);

		if(mode!="simple" && mode!="in-chain" && mode!="out-chain" && mode!="in-chain-sep")
		{
			throw std::runtime_error(std::string("Invalid mode, must be 'simple' or 'chained'."));
		}

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::vector<std::size_t> ids(atom_ids.begin(), atom_ids.end());

		std::vector< std::vector<double> > distances(ids.size(), std::vector<double>(ids.size(), 0.0));

		for(std::size_t i=0;(i+1)<ids.size();i++)
		{
			const Atom& ai=data_manager.atoms()[ids[i]];
			for(std::size_t j=(i+1);j<ids.size();j++)
			{
				const Atom& aj=data_manager.atoms()[ids[j]];
				if((mode=="simple") || (mode=="out-chain" && ai.crad.chainID!=aj.crad.chainID)|| (mode=="in-chain" && ai.crad.chainID==aj.crad.chainID) || (mode=="in-chain-sep" && ai.crad.chainID==aj.crad.chainID && std::abs(ai.crad.resSeq-aj.crad.resSeq)<=in_chain_sep_max))
				{
					const double d=apollota::distance_from_point_to_point(ai.value, aj.value);
					distances[i][j]=d;
					distances[j][i]=d;
				}
			}
		}

		std::vector<std::size_t> maxes(ids.size());
		{
			for(std::size_t i=0;i<ids.size();i++)
			{
				maxes[i]=i;
			}

			for(std::size_t i=0;(i+1)<ids.size();i++)
			{
				for(std::size_t j=(i+1);j<ids.size();j++)
				{
					const double d=distances[i][j];
					if(d>distances[i][maxes[i]])
					{
						maxes[i]=j;
					}
					if(d>distances[j][maxes[j]])
					{
						maxes[j]=i;
					}
				}
			}
		}

		std::vector< std::vector<double> > descriptors(ids.size(), std::vector<double>(3, 0.0));
		{
			for(std::size_t i=0;i<ids.size();i++)
			{
				const std::vector<double>& v=distances[i];
				double sum=0.0;
				for(std::size_t j=0;j<v.size();j++)
				{
					sum+=v[j];
				}
				const double mean=sum/static_cast<double>(v.size());
				double sum2=0.0;
				double sum3=0.0;
				for(std::size_t j=0;j<v.size();j++)
				{
					sum2+=(v[j]-mean)*(v[j]-mean);
					sum3+=(v[j]-mean)*(v[j]-mean)*(v[j]-mean);
				}
				descriptors[i][0]=mean;
				descriptors[i][1]=sum2/static_cast<double>(v.size());
				descriptors[i][2]=sum3/static_cast<double>(v.size());
			}
		}

		std::vector<std::string> names;
		names.push_back(name_prefix+"_a1");
		names.push_back(name_prefix+"_b1");
		names.push_back(name_prefix+"_c1");
		names.push_back(name_prefix+"_a2");
		names.push_back(name_prefix+"_b2");
		names.push_back(name_prefix+"_c2");
		names.push_back(name_prefix+"_a3");
		names.push_back(name_prefix+"_b3");
		names.push_back(name_prefix+"_c3");

		for(std::size_t i=0;i<ids.size();i++)
		{
			const std::size_t a=i;
			const std::size_t b=maxes[a];
			const std::size_t c=maxes[b];
			const std::size_t atom_id=ids[i];
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(atom_id);
			atom_adjuncts[names[0]]=descriptors[a][0];
			atom_adjuncts[names[1]]=descriptors[b][0];
			atom_adjuncts[names[2]]=descriptors[c][0];
			atom_adjuncts[names[3]]=descriptors[a][1];
			atom_adjuncts[names[4]]=descriptors[b][1];
			atom_adjuncts[names[5]]=descriptors[c][1];
			atom_adjuncts[names[6]]=descriptors[a][2];
			atom_adjuncts[names[7]]=descriptors[b][2];
			atom_adjuncts[names[8]]=descriptors[c][2];
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), atom_ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCTS_OF_ATOMS_BY_UFSR_H_ */
