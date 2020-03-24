#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_EXPRESSION_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_EXPRESSION_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctOfAtomsByExpression : public OperatorBase<SetAdjunctOfAtomsByExpression>
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
	std::string expression;
	std::vector<std::string> input_adjuncts;
	std::vector<double> parameters;
	std::string output_adjunct;

	SetAdjunctOfAtomsByExpression()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		expression=input.get_value<std::string>("expression");
		input_adjuncts=input.get_value_vector_or_default<std::string>("input-adjuncts", std::vector<std::string>());
		parameters=input.get_value_vector_or_default<double>("parameters", std::vector<double>());
		output_adjunct=input.get_value<std::string>("output-adjunct");
	}

	void document(CommandDocumentation& doc) const
	{
		Utilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("expression", CDOD::DATATYPE_STRING, "expression"));
		doc.set_option_decription(CDOD("input-adjuncts", CDOD::DATATYPE_STRING_ARRAY, "input adjunct names", ""));
		doc.set_option_decription(CDOD("parameters", CDOD::DATATYPE_FLOAT_ARRAY, "expression parameters", ""));
		doc.set_option_decription(CDOD("output-adjunct", CDOD::DATATYPE_STRING, "output adjunct name"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(expression.empty())
		{
			throw std::runtime_error(std::string("No expression."));
		}

		if(expression=="_reverse_s")
		{
			if(input_adjuncts.empty())
			{
				throw std::runtime_error(std::string("Not 1 input adjunct name for the defined expression."));
			}
			if(parameters.size()!=5)
			{
				throw std::runtime_error(std::string("Not 5 parameters for the defined expression."));
			}
		}
		else
		{
			throw std::runtime_error(std::string("Unsupported expression."));
		}

		assert_adjunct_name_input(output_adjunct, false);

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			for(std::size_t i=0;i<input_adjuncts.size();i++)
			{
				if(atom_adjuncts.count(input_adjuncts[i])==0)
				{
					throw std::runtime_error(std::string("Input adjuncts not present everywhere in selection."));
				}
			}
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			if(expression=="_reverse_s")
			{
				atom_adjuncts[output_adjunct]=calc_reverse_s_transform(
						atom_adjuncts[input_adjuncts[0]],
						parameters[0], parameters[1], parameters[2], parameters[3], parameters[4]);
			}
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}

private:
	static double calc_reverse_s_transform(
			const double score, const double a_mean, const double a_sd, const double b_mean, const double b_sd, const double d0)
	{
		double s=(((score-a_mean)/a_sd)*b_sd+b_mean);
		s=std::max(s, 0.01);
		s=std::min(s, 1.0);
		return (d0*sqrt((1-s)/s));
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_EXPRESSION_H_ */
