#ifndef SCRIPTING_OPERATORS_VOROMQA_FRUSTRATION_H_
#define SCRIPTING_OPERATORS_VOROMQA_FRUSTRATION_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class VoroMQAFrustration : public OperatorBase<VoroMQAFrustration>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string adjunct_contact_energy;
	std::string adjunct_contact_frustration_energy_mean;
	std::string adjunct_atom_frustration_energy_mean;
	unsigned int smoothing_iterations;
	unsigned int smoothing_depth;

	VoroMQAFrustration() : smoothing_iterations(1), smoothing_depth(3)
	{
	}

	void initialize(CommandInput& input)
	{
		adjunct_contact_energy=input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
		adjunct_contact_frustration_energy_mean=input.get_value_or_default<std::string>("adj-contact-frustration-energy-mean", "frustration_energy_mean");
		adjunct_atom_frustration_energy_mean=input.get_value_or_default<std::string>("adj-atom-frustration-energy-mean", "frustration_energy_mean");
		smoothing_iterations=input.get_value_or_default<unsigned int>("smoothing-iterations", 1);
		smoothing_depth=input.get_value_or_default<unsigned int>("smoothing-depth", 3);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("adj-contact-energy", CDOD::DATATYPE_STRING, "name of input adjunct with raw energy values", "voromqa_energy"));
		doc.set_option_decription(CDOD("adj-contact-frustration-energy-mean", CDOD::DATATYPE_STRING, "name of output adjunct for frustration energy mean values", "frustration_energy_mean"));
		doc.set_option_decription(CDOD("adj-atom-frustration-energy-mean", CDOD::DATATYPE_STRING, "name of output adjunct for frustration energy mean values", "frustration_energy_mean"));
		doc.set_option_decription(CDOD("smoothing-iterations", CDOD::DATATYPE_INT, "number of smoothing iterations", 1));
		doc.set_option_decription(CDOD("smoothing-depth", CDOD::DATATYPE_INT, "smoothing depth", 3));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(adjunct_contact_energy, false);
		assert_adjunct_name_input(adjunct_contact_frustration_energy_mean, true);
		assert_adjunct_name_input(adjunct_atom_frustration_energy_mean, true);

		const std::set<std::size_t> solvent_contact_ids=data_manager.selection_manager().select_contacts(
				SelectionManager::Query(std::string("[--solvent --adjuncts ")+adjunct_contact_energy+"]", false));

		if(solvent_contact_ids.empty())
		{
			throw std::runtime_error(std::string("No solvent contacts with energy values."));
		}

		const std::set<std::size_t> exterior_atom_ids=data_manager.selection_manager().select_atoms_by_contacts(solvent_contact_ids, false);

		const std::set<std::size_t> exterior_contact_ids=data_manager.selection_manager().select_contacts(
				SelectionManager::Query("[--tags peripherial]", false));

		if(exterior_contact_ids.empty())
		{
			throw std::runtime_error(std::string("No peripherial contacts."));
		}

		std::vector<std::size_t> atom_solvent_contact_ids(data_manager.atoms().size(), 0);
		std::vector<double> atom_solvent_contact_areas(data_manager.atoms().size(), 0.0);
		std::vector<double> atom_solvent_contact_energy_means(data_manager.atoms().size(), 0.0);

		for(std::set<std::size_t>::const_iterator it=solvent_contact_ids.begin();it!=solvent_contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			atom_solvent_contact_ids[contact.ids[0]]=(*it);
			atom_solvent_contact_areas[contact.ids[0]]=contact.value.area;
			if(contact.value.area>0.0 && contact.value.props.adjuncts.count(adjunct_contact_energy)>0)
			{
				atom_solvent_contact_energy_means[contact.ids[0]]=(contact.value.props.adjuncts.find(adjunct_contact_energy)->second)/contact.value.area;
			}
		}

		std::vector< std::set<std::size_t> > graph(data_manager.atoms().size());
		for(std::set<std::size_t>::const_iterator it=exterior_contact_ids.begin();it!=exterior_contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			graph[contact.ids[0]].insert(contact.ids[1]);
			graph[contact.ids[1]].insert(contact.ids[0]);
		}

		for(unsigned int iteration=0;iteration<smoothing_iterations;iteration++)
		{
			std::vector<double> updated_atom_solvent_contact_energy_means=atom_solvent_contact_energy_means;

			for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
			{
				const std::size_t central_id=(*it);

				std::map<std::size_t, bool> neighbors;
				neighbors[central_id]=false;

				for(unsigned int level=0;level<smoothing_depth;level++)
				{
					std::map<std::size_t, bool> more_neighbors;
					for(std::map<std::size_t, bool>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
					{
						const std::size_t id=jt->first;
						if(!jt->second)
						{
							for(std::set<std::size_t>::const_iterator et=graph[id].begin();et!=graph[id].end();++et)
							{
								more_neighbors[*et]=(neighbors.count(*et)>0 && neighbors.find(*et)->second);
							}
						}
						more_neighbors[id]=true;
					}
					neighbors.swap(more_neighbors);
				}

				double sum_of_areas=0.0;
				double sum_of_energies=0.0;

				for(std::map<std::size_t, bool>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
				{
					sum_of_areas+=atom_solvent_contact_areas[jt->first];
					sum_of_energies+=atom_solvent_contact_energy_means[jt->first]*atom_solvent_contact_areas[jt->first];
				}

				updated_atom_solvent_contact_energy_means[central_id]=(sum_of_areas>0.0 ? (sum_of_energies/sum_of_areas) : 0.0);
			}

			atom_solvent_contact_energy_means.swap(updated_atom_solvent_contact_energy_means);
		}

		if(!adjunct_contact_frustration_energy_mean.empty())
		{
			for(std::size_t i=0;i<data_manager.contacts().size();i++)
			{
				data_manager.contact_adjuncts_mutable(i).erase(adjunct_contact_frustration_energy_mean);
			}
		}

		if(!adjunct_atom_frustration_energy_mean.empty())
		{
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				data_manager.atom_adjuncts_mutable(i).erase(adjunct_atom_frustration_energy_mean);
			}
		}

		if(!adjunct_contact_frustration_energy_mean.empty() || !adjunct_atom_frustration_energy_mean.empty())
		{
			for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
			{
				const std::size_t central_id=(*it);

				if(!adjunct_contact_frustration_energy_mean.empty())
				{
					data_manager.contact_adjuncts_mutable(atom_solvent_contact_ids[central_id])[adjunct_contact_frustration_energy_mean]=atom_solvent_contact_energy_means[central_id];
				}

				if(!adjunct_atom_frustration_energy_mean.empty())
				{
					data_manager.atom_adjuncts_mutable(central_id)[adjunct_atom_frustration_energy_mean]=atom_solvent_contact_energy_means[central_id];
				}
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_VOROMQA_FRUSTRATION_H_ */
