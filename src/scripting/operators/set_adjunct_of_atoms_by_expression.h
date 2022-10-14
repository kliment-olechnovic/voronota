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
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		expression=input.get_value<std::string>("expression");
		input_adjuncts=input.get_value_vector_or_default<std::string>("input-adjuncts", std::vector<std::string>());
		parameters=input.get_value_vector_or_default<double>("parameters", std::vector<double>());
		output_adjunct=input.get_value<std::string>("output-adjunct");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
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
			if(input_adjuncts.size()!=1)
			{
				throw std::runtime_error(std::string("Not 1 input adjunct name for the defined expression."));
			}
			if(parameters.size()!=5)
			{
				throw std::runtime_error(std::string("Not 5 parameters for the defined expression."));
			}
		}
		else if(expression=="_logistic")
		{
			if(input_adjuncts.size()!=1)
			{
				throw std::runtime_error(std::string("Not 1 input adjunct name for the defined expression."));
			}
			if(parameters.size()!=3)
			{
				throw std::runtime_error(std::string("Not 3 parameters for the defined expression."));
			}
		}
		else if(expression=="_log")
		{
			if(input_adjuncts.size()!=1)
			{
				throw std::runtime_error(std::string("Not 1 input adjunct name for the defined expression."));
			}
			if(parameters.size()!=2)
			{
				throw std::runtime_error(std::string("Not 2 parameters for the defined expression."));
			}
		}
		else if(expression=="_linear_combo")
		{
			if(input_adjuncts.empty())
			{
				throw std::runtime_error(std::string("No input adjunct names for the defined expression."));
			}
			if(parameters.size()!=(input_adjuncts.size()+1))
			{
				throw std::runtime_error(std::string("Not correct number of parameters for the defined expression, must be equal to ((number of input adjuncts)+1)."));
			}
		}
		else if(expression=="_multiply")
		{
			if(input_adjuncts.size()<2)
			{
				throw std::runtime_error(std::string("Not 2 or more input adjunct names for the defined expression."));
			}
			if(parameters.size()!=0)
			{
				throw std::runtime_error(std::string("Not 0 parameters for the defined expression."));
			}
		}
		else if(expression=="_divide")
		{
			if(input_adjuncts.size()!=2)
			{
				throw std::runtime_error(std::string("Not 2 input adjunct names for the defined expression."));
			}
			if(parameters.size()!=0)
			{
				throw std::runtime_error(std::string("Not 0 parameters for the defined expression."));
			}
		}
		else if(expression=="_modulo")
		{
			if(input_adjuncts.size()!=1)
			{
				throw std::runtime_error(std::string("Not 1 input adjunct name for the defined expression."));
			}
			if(parameters.size()!=1)
			{
				throw std::runtime_error(std::string("Not 1 parameter for the defined expression."));
			}
		}
		else if(expression=="_sqrt")
		{
			if(input_adjuncts.size()!=1)
			{
				throw std::runtime_error(std::string("Not 1 input adjunct names for the defined expression."));
			}
			if(parameters.size()!=0)
			{
				throw std::runtime_error(std::string("Not 0 parameters for the defined expression."));
			}
		}
		else
		{
			throw std::runtime_error(std::string("Unsupported expression."));
		}

		assert_adjunct_name_input(output_adjunct, false);

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			for(std::size_t i=0;i<input_adjuncts.size();i++)
			{
				if(input_adjuncts[i]!="resSeq" && input_adjuncts[i]!="radius" && atom_adjuncts.count(input_adjuncts[i])==0)
				{
					throw std::runtime_error(std::string("Input adjuncts not present everywhere in selection."));
				}
			}
		}

		{
			std::vector<double> input_adjunct_values(input_adjuncts.size(), 0.0);
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
				for(std::size_t i=0;i<input_adjuncts.size();i++)
				{
					if(input_adjuncts[i]=="resSeq")
					{
						input_adjunct_values[i]=data_manager.atoms()[*it].crad.resSeq;
					}
					else if(input_adjuncts[i]=="radius")
					{
						input_adjunct_values[i]=data_manager.atoms()[*it].value.r;
					}
					else
					{
						input_adjunct_values[i]=atom_adjuncts[input_adjuncts[i]];
					}
				}
				if(expression=="_reverse_s")
				{
					atom_adjuncts[output_adjunct]=OperatorsUtilities::calculate_reverse_s_transform(
							input_adjunct_values[0],
							parameters[0], parameters[1], parameters[2], parameters[3], parameters[4]);
				}
				else if(expression=="_logistic")
				{
					atom_adjuncts[output_adjunct]=OperatorsUtilities::calculate_logistic_transform(
							input_adjunct_values[0],
							parameters[0], parameters[1], parameters[2]);
				}
				else if(expression=="_log")
				{
					atom_adjuncts[output_adjunct]=OperatorsUtilities::calculate_log_transform(
							input_adjunct_values[0],
							parameters[0], parameters[1]);
				}
				else if(expression=="_linear_combo")
				{
					double v=parameters.back();
					for(std::size_t i=0;i<input_adjunct_values.size();i++)
					{
						v+=input_adjunct_values[i]*parameters[i];
					}
					atom_adjuncts[output_adjunct]=v;
				}
				else if(expression=="_multiply")
				{
					double v=1.0;
					for(std::size_t i=0;i<input_adjunct_values.size();i++)
					{
						v*=input_adjunct_values[i];
					}
					atom_adjuncts[output_adjunct]=v;
				}
				else if(expression=="_divide")
				{
					atom_adjuncts[output_adjunct]=input_adjunct_values[0]/input_adjunct_values[1];
				}
				else if(expression=="_modulo")
				{
					const double v=input_adjunct_values.back();
					const double d=parameters.back();
					atom_adjuncts[output_adjunct]=(v-(std::floor(v/d)*d));
				}
				else if(expression=="_sqrt")
				{
					const double v=input_adjunct_values.back();
					atom_adjuncts[output_adjunct]=std::sqrt(v);
				}
			}
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_EXPRESSION_H_ */
