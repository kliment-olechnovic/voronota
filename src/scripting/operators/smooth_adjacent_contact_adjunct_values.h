#ifndef SCRIPTING_OPERATORS_SMOOTH_ADJACENT_CONTACT_ADJUNCT_VALUES_H_
#define SCRIPTING_OPERATORS_SMOOTH_ADJACENT_CONTACT_ADJUNCT_VALUES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SmoothAdjacentContactAdjunctValues : public OperatorBase<SmoothAdjacentContactAdjunctValues>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string adjunct_input;
	std::string adjunct_output;
	unsigned int smoothing_iterations;
	double default_input_value;
	bool no_edge_weights;
	bool no_area_weights;

	SmoothAdjacentContactAdjunctValues() : smoothing_iterations(1), default_input_value(0.0), no_edge_weights(false), no_area_weights(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		adjunct_input=input.get_value<std::string>("adj-in");
		adjunct_output=input.get_value<std::string>("adj-out");
		smoothing_iterations=input.get_value_or_default<unsigned int>("iterations", 1);
		default_input_value=input.get_value_or_default<double>("default-value", 0.0);
		no_edge_weights=input.get_flag("no-edge-weights");
		no_area_weights=input.get_flag("no-area-weights");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("adj-in", CDOD::DATATYPE_STRING, "name of input adjunct"));
		doc.set_option_decription(CDOD("adj-out", CDOD::DATATYPE_STRING, "name of output adjunct"));
		doc.set_option_decription(CDOD("iterations", CDOD::DATATYPE_INT, "number of smoothing iterations", 1));
		doc.set_option_decription(CDOD("default-value", CDOD::DATATYPE_FLOAT, "default input value", 0.0));
		doc.set_option_decription(CDOD("no-edge-weights", CDOD::DATATYPE_BOOL, "flag to disable edge weights"));
		doc.set_option_decription(CDOD("no-area-weights", CDOD::DATATYPE_BOOL, "flag to disable area weights"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_adjacencies_availability();

		assert_adjunct_name_input(adjunct_input, false);
		assert_adjunct_name_input(adjunct_output, false);

		data_manager.assert_contacts_adjunct_availability(adjunct_input);

		std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		const std::map< std::size_t, std::map<std::size_t, double> > contacts_graph=data_manager.extract_subset_of_contacts_adjacencies(contact_ids);

		std::map<std::size_t, double> contact_values;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			const std::map<std::string, double>::const_iterator adjuncts_it=contact.value.props.adjuncts.find(adjunct_input);
			contact_values[*it]=(adjuncts_it!=contact.value.props.adjuncts.end() ? adjuncts_it->second : default_input_value);
		}

		for(unsigned int iteration=0;iteration<smoothing_iterations;iteration++)
		{
			std::map<std::size_t, double> updated_contact_values=contact_values;

			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				const std::size_t central_id=(*it);
				std::map< std::size_t, std::map<std::size_t, double> >::const_iterator contacts_graph_it=contacts_graph.find(central_id);
				if(contacts_graph_it!=contacts_graph.end())
				{
					const std::map<std::size_t, double>& neighbors=contacts_graph_it->second;
					if(!neighbors.empty())
					{
						double sum_of_weights=0.0;
						double sum_of_weighted_values=0.0;
						{
							double sum_of_edge_weights=0.0;
							if(no_edge_weights)
							{
								sum_of_edge_weights=1.0;
							}
							else
							{
								for(std::map<std::size_t, double>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
								{
									sum_of_edge_weights+=jt->second;
								}
							}
							const Contact& contact=data_manager.contacts()[central_id];
							const double weight=(no_area_weights ? 1.0 : contact.value.area)*sum_of_edge_weights;
							const double weighted_value=weight*contact_values[central_id];
							sum_of_weights+=weight;
							sum_of_weighted_values+=weighted_value;
						}
						for(std::map<std::size_t, double>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
						{
							const Contact& contact=data_manager.contacts()[jt->first];
							const double weight=(no_area_weights ? 1.0 : contact.value.area)*(no_edge_weights ? 1.0 : jt->second);
							const double weighted_value=weight*contact_values[jt->first];
							sum_of_weights+=weight;
							sum_of_weighted_values+=weighted_value;
						}
						updated_contact_values[central_id]=(sum_of_weights>0.0 ? (sum_of_weighted_values/sum_of_weights) : 0.0);
					}
				}
			}

			contact_values=updated_contact_values;
		}

		if(!adjunct_output.empty())
		{
			for(std::map<std::size_t, double>::const_iterator it=contact_values.begin();it!=contact_values.end();++it)
			{
				data_manager.contact_adjuncts_mutable(it->first)[adjunct_output]=it->second;
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SMOOTH_ADJACENT_CONTACT_ADJUNCT_VALUES_H_ */
