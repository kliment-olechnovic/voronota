#ifndef SCRIPTING_OPERATORS_VOROMQA_INTERFACE_FRUSTRATION_USING_EDGES_H_
#define SCRIPTING_OPERATORS_VOROMQA_INTERFACE_FRUSTRATION_USING_EDGES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class VoroMQAInterfaceFrustrationUsingEdges : public OperatorBase<VoroMQAInterfaceFrustrationUsingEdges>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string adjunct_contact_energy;
	std::string adjunct_contact_interface_frustration_energy_mean;
	unsigned int smoothing_iterations;
	bool no_edge_weights;

	VoroMQAInterfaceFrustrationUsingEdges() : smoothing_iterations(2), no_edge_weights(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		adjunct_contact_energy=input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
		adjunct_contact_interface_frustration_energy_mean=input.get_value_or_default<std::string>("adj-contact-interface-frustration-energy-mean", "interface_frustration_energy_mean");
		smoothing_iterations=input.get_value_or_default<unsigned int>("smoothing-iterations", 2);
		no_edge_weights=input.get_flag("no-edge-weights");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("adj-contact-energy", CDOD::DATATYPE_STRING, "name of input adjunct with raw energy values", "voromqa_energy"));
		doc.set_option_decription(CDOD("adj-contact-interface-frustration-energy-mean", CDOD::DATATYPE_STRING, "name of output adjunct for interface frustration energy mean values", "interface_frustration_energy_mean"));
		doc.set_option_decription(CDOD("smoothing-iterations", CDOD::DATATYPE_INT, "number of smoothing iterations", 2));
		doc.set_option_decription(CDOD("no-edge-weights", CDOD::DATATYPE_BOOL, "flag to disable edge weights"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_adjacencies_availability();

		assert_adjunct_name_input(adjunct_contact_energy, false);
		assert_adjunct_name_input(adjunct_contact_interface_frustration_energy_mean, false);

		data_manager.assert_contacts_adjunct_availability(adjunct_contact_energy);

		std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		const std::map< std::size_t, std::map<std::size_t, double> > contacts_graph=data_manager.extract_subset_of_contacts_adjacencies(contact_ids);

		std::map<std::size_t, double> contact_energy_means;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			if(contact.value.area>0.0 && contact.value.props.adjuncts.count(adjunct_contact_energy)>0)
			{
				contact_energy_means[*it]=(contact.value.props.adjuncts.find(adjunct_contact_energy)->second)/contact.value.area;
			}
		}

		for(unsigned int iteration=0;iteration<smoothing_iterations;iteration++)
		{
			std::map<std::size_t, double> updated_contact_energy_means=contact_energy_means;

			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				const std::size_t central_id=(*it);
				std::map< std::size_t, std::map<std::size_t, double> >::const_iterator contacts_graph_it=contacts_graph.find(central_id);
				if(contacts_graph_it!=contacts_graph.end())
				{
					const std::map<std::size_t, double>& neighbors=contacts_graph_it->second;
					double sum_of_weights=0.0;
					double sum_of_energies=0.0;
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
						const double weight=contact.value.area*sum_of_edge_weights;
						const double energy=weight*contact_energy_means[central_id];
						sum_of_weights+=weight;
						sum_of_energies+=energy;
					}
					for(std::map<std::size_t, double>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
					{
						const Contact& contact=data_manager.contacts()[jt->first];
						const double weight=contact.value.area*(no_edge_weights ? 1.0 : jt->second);
						const double energy=weight*contact_energy_means[jt->first];
						sum_of_weights+=weight;
						sum_of_energies+=energy;
					}
					updated_contact_energy_means[central_id]=(sum_of_weights>0.0 ? (sum_of_energies/sum_of_weights) : 0.0);
				}
			}

			contact_energy_means=updated_contact_energy_means;
		}

		if(!adjunct_contact_interface_frustration_energy_mean.empty())
		{
			for(std::map<std::size_t, double>::const_iterator it=contact_energy_means.begin();it!=contact_energy_means.end();++it)
			{
				data_manager.contact_adjuncts_mutable(it->first)[adjunct_contact_interface_frustration_energy_mean]=it->second;
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_VOROMQA_INTERFACE_FRUSTRATION_USING_EDGES_H_ */
