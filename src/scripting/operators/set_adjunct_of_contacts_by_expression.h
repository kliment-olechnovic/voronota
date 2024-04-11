#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_BY_EXPRESSION_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_BY_EXPRESSION_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctOfContactsByExpression : public OperatorBase<SetAdjunctOfContactsByExpression>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string expression;
	std::vector<std::string> input_adjuncts;
	std::vector<double> parameters;
	std::string output_adjunct;

	SetAdjunctOfContactsByExpression()
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

		if(expression=="_logistic")
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
		else if(expression=="_bound")
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
		else if(expression=="_min")
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
		else if(expression=="_max")
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
		else if(expression=="_to_goodness")
		{
			if(input_adjuncts.size()!=2)
			{
				throw std::runtime_error(std::string("Not 2 input adjunct names for the defined expression."));
			}
			if(parameters.size()!=2)
			{
				throw std::runtime_error(std::string("Not 2 parameters for the defined expression."));
			}
		}
		else
		{
			throw std::runtime_error(std::string("Unsupported expression."));
		}

		assert_adjunct_name_input(output_adjunct, false);

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(*it);
			for(std::size_t i=0;i<input_adjuncts.size();i++)
			{
				if(input_adjuncts[i]!="area" && input_adjuncts[i]!="dist" && contact_adjuncts.count(input_adjuncts[i])==0)
				{
					throw std::runtime_error(std::string("Input adjuncts not present everywhere in selection."));
				}
			}
		}

		{
			std::vector<double> input_adjunct_values(input_adjuncts.size(), 0.0);
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(*it);
				for(std::size_t i=0;i<input_adjuncts.size();i++)
				{
					if(input_adjuncts[i]=="area")
					{
						input_adjunct_values[i]=data_manager.contacts()[*it].value.area;
					}
					else if(input_adjuncts[i]=="dist")
					{
						input_adjunct_values[i]=data_manager.contacts()[*it].value.dist;
					}
					else
					{
						input_adjunct_values[i]=contact_adjuncts[input_adjuncts[i]];
					}
				}
				if(expression=="_logistic")
				{
					contact_adjuncts[output_adjunct]=OperatorsUtilities::calculate_logistic_transform(
							input_adjunct_values[0],
							parameters[0], parameters[1], parameters[2]);
				}
				else if(expression=="_linear_combo")
				{
					double v=parameters.back();
					for(std::size_t i=0;i<input_adjunct_values.size();i++)
					{
						v+=input_adjunct_values[i]*parameters[i];
					}
					contact_adjuncts[output_adjunct]=v;
				}
				else if(expression=="_multiply")
				{
					double v=1.0;
					for(std::size_t i=0;i<input_adjunct_values.size();i++)
					{
						v*=input_adjunct_values[i];
					}
					contact_adjuncts[output_adjunct]=v;
				}
				else if(expression=="_divide")
				{
					contact_adjuncts[output_adjunct]=input_adjunct_values[0]/input_adjunct_values[1];
				}
				else if(expression=="_bound")
				{
					contact_adjuncts[output_adjunct]=std::max(parameters[0], std::min(input_adjunct_values[0], parameters[1]));
				}
				else if(expression=="_min")
				{
					double v=input_adjunct_values[0];
					for(std::size_t i=1;i<input_adjunct_values.size();i++)
					{
						v=std::min(v, input_adjunct_values[i]);
					}
					contact_adjuncts[output_adjunct]=v;
				}
				else if(expression=="_max")
				{
					double v=input_adjunct_values[0];
					for(std::size_t i=1;i<input_adjunct_values.size();i++)
					{
						v=std::max(v, input_adjunct_values[i]);
					}
					contact_adjuncts[output_adjunct]=v;
				}
				else if(expression=="_to_goodness")
				{
					contact_adjuncts[output_adjunct]=OperatorsUtilities::calculate_to_goodness_transform(
							input_adjunct_values[0], input_adjunct_values[1],
							parameters[0], parameters[1]);
				}
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_CONTACTS_BY_EXPRESSION_H_ */
