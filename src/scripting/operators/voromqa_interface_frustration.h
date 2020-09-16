#ifndef SCRIPTING_OPERATORS_VOROMQA_INTERFACE_FRUSTRATION_H_
#define SCRIPTING_OPERATORS_VOROMQA_INTERFACE_FRUSTRATION_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class VoroMQAInterfaceFrustration : public OperatorBase<VoroMQAInterfaceFrustration>
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

	VoroMQAInterfaceFrustration() : smoothing_iterations(2)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		adjunct_contact_energy=input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
		adjunct_contact_interface_frustration_energy_mean=input.get_value_or_default<std::string>("adj-contact-interface-frustration-energy-mean", "interface_frustration_energy_mean");
		smoothing_iterations=input.get_value_or_default<unsigned int>("smoothing-iterations", 2);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("adj-contact-energy", CDOD::DATATYPE_STRING, "name of input adjunct with raw energy values", "voromqa_energy"));
		doc.set_option_decription(CDOD("adj-contact-interface-frustration-energy-mean", CDOD::DATATYPE_STRING, "name of output adjunct for interface frustration energy mean values", "interface_frustration_energy_mean"));
		doc.set_option_decription(CDOD("smoothing-iterations", CDOD::DATATYPE_INT, "number of smoothing iterations", 2));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(adjunct_contact_energy, false);
		assert_adjunct_name_input(adjunct_contact_interface_frustration_energy_mean, false);

		data_manager.assert_contacts_adjunct_availability(adjunct_contact_energy);

		std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::vector< std::set<std::size_t> > atoms_graph(data_manager.atoms().size());
		for(std::size_t i=0;i<data_manager.contacts().size();i++)
		{
			const Contact& contact=data_manager.contacts()[i];
			if(!contact.solvent())
			{
				atoms_graph[contact.ids[0]].insert(contact.ids[1]);
				atoms_graph[contact.ids[1]].insert(contact.ids[0]);
			}
		}

		std::vector< std::set<std::size_t> > atoms_contact_ids(data_manager.atoms().size());
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			atoms_contact_ids[contact.ids[0]].insert(*it);
			atoms_contact_ids[contact.ids[1]].insert(*it);
		}

		std::map< std::size_t, std::set<std::size_t> > contacts_graph;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const Contact& central_contact=data_manager.contacts()[*it];
			for(int a=0;a<2;a++)
			{
				const std::size_t central_atom_id_1=central_contact.ids[a];
				const std::size_t central_atom_id_2=central_contact.ids[(a==0) ? 1 : 0];
				const std::set<std::size_t>& candidate_contact_ids=atoms_contact_ids[central_atom_id_1];
				for(std::set<std::size_t>::const_iterator jt=candidate_contact_ids.begin();jt!=candidate_contact_ids.end();++jt)
				{
					if((*jt)!=(*it))
					{
						const Contact& candidate_contact=data_manager.contacts()[*jt];
						const std::size_t candidate_atom_id_2=((candidate_contact.ids[0]==central_atom_id_1) ? candidate_contact.ids[1] : candidate_contact.ids[0]);
						if(atoms_graph[central_atom_id_2].count(candidate_atom_id_2)>0)
						{
							contacts_graph[*it].insert(*jt);
							contacts_graph[*jt].insert(*it);
						}
					}
				}
			}
		}

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
				std::set<std::size_t> neighbors=contacts_graph[central_id];
				neighbors.insert(central_id);
				double sum_of_areas=0.0;
				double sum_of_energies=0.0;
				for(std::set<std::size_t>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
				{
					const Contact& contact=data_manager.contacts()[*jt];
					sum_of_areas+=contact.value.area;
					sum_of_energies+=contact.value.area*contact_energy_means[*jt];
				}
				updated_contact_energy_means[central_id]=(sum_of_areas>0.0 ? (sum_of_energies/sum_of_areas) : 0.0);
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

#endif /* SCRIPTING_OPERATORS_VOROMQA_INTERFACE_FRUSTRATION_H_ */
