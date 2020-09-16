#ifndef SCRIPTING_OPERATORS_CALCULATE_BETWEENNESS_H_
#define SCRIPTING_OPERATORS_CALCULATE_BETWEENNESS_H_

#include "../operators_common.h"
#include "../../common/construction_of_path_centralities.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CalculateBetweenness : public OperatorBase<CalculateBetweenness>
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
	bool inter_residue;
	bool uniform;
	bool normalize;
	std::string adjunct_atoms;
	std::string adjunct_contacts;

	CalculateBetweenness() : inter_residue(false), uniform(false), normalize(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query("", "[--min-seq-sep 1]", input);
		inter_residue=!input.get_flag("not-inter-residue");
		uniform=input.get_flag("uniform");
		normalize=!input.get_flag("not-normalize");
		adjunct_atoms=input.get_value_or_default<std::string>("adj-atoms", "betweenness");
		adjunct_contacts=input.get_value_or_default<std::string>("adj-contacts", "betweenness");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query("", "[--min-seq-sep 1]", doc);
		doc.set_option_decription(CDOD("not-inter-residue", CDOD::DATATYPE_BOOL, "flag not consider inter-residue contacts"));
		doc.set_option_decription(CDOD("uniform", CDOD::DATATYPE_BOOL, "flag to not use contact areas"));
		doc.set_option_decription(CDOD("not-normalize", CDOD::DATATYPE_BOOL, "flag to not normalize centrality values"));
		doc.set_option_decription(CDOD("adj-atoms", CDOD::DATATYPE_STRING, "adjunct name to write centrality in atoms", "betweenness"));
		doc.set_option_decription(CDOD("adj-contacts", CDOD::DATATYPE_STRING, "adjunct name to write centrality in contacts", "betweenness"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(adjunct_atoms, true);
		assert_adjunct_name_input(adjunct_contacts, true);

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		typedef common::ConstructionOfPathCentralities COPC;

		std::map<COPC::CRADsPair, double> map_of_contacts;
		std::map< COPC::CRADsPair, std::set<std::size_t> > map_of_contacts_ids;
		std::map< COPC::CRAD, std::set<std::size_t> > map_of_atoms_ids;

		{
			const std::vector<Atom>& atoms=data_manager.atoms();
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Contact& contact=data_manager.contacts()[*it];
				if(!contact.solvent())
				{
					COPC::CRADsPair crads=
							COPC::CRADsPair(atoms[contact.ids[0]].crad, atoms[contact.ids[1]].crad).without_some_info(inter_residue, inter_residue, false, false);
					map_of_contacts[crads]+=contact.value.area;
					map_of_contacts_ids[crads].insert(*it);
					map_of_atoms_ids[atoms[contact.ids[0]].crad.without_some_info(inter_residue, inter_residue, false, false)].insert(contact.ids[0]);
					map_of_atoms_ids[atoms[contact.ids[1]].crad.without_some_info(inter_residue, inter_residue, false, false)].insert(contact.ids[1]);
				}
			}
		}

		for(std::map<COPC::CRADsPair, double>::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			it->second=(uniform ? 1.0 : (100.0/it->second));
		}

		const COPC::Graph graph=COPC::init_graph(map_of_contacts);

		COPC::BetweennessCentralitiesResult bc_result=COPC::calculate_betweenness_centralities(graph, 0.0, normalize);

		if(!adjunct_atoms.empty())
		{
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				data_manager.atom_adjuncts_mutable(i).erase(adjunct_atoms);
			}

			for(COPC::ID id=0;id<graph.vertices.size();id++)
			{
				const std::set<std::size_t>& atom_ids=map_of_atoms_ids[graph.vertices[id].crad];
				for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
				{
					data_manager.atom_adjuncts_mutable(*it)[adjunct_atoms]=bc_result.vertex_centralities[id];
				}
			}
		}

		if(!adjunct_contacts.empty())
		{
			for(std::size_t i=0;i<data_manager.contacts().size();i++)
			{
				data_manager.contact_adjuncts_mutable(i).erase(adjunct_contacts);
			}

			for(std::size_t i=0;i<graph.edges.size();i++)
			{
				const COPC::Edge& edge=graph.edges[i];
				const COPC::ID id1=edge.vertex_ids[0];
				const COPC::ID id2=edge.vertex_ids[1];
				const COPC::CRADsPair crads(graph.vertices[id1].crad, graph.vertices[id2].crad);
				const std::set<std::size_t>& contact_ids=map_of_contacts_ids[crads];
				for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
				{
					data_manager.contact_adjuncts_mutable(*it)[adjunct_contacts]=bc_result.edge_centralities[COPC::ordered_pair_of_ids(id1, id2)];
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

#endif /* SCRIPTING_OPERATORS_CALCULATE_BETWEENNESS_H_ */
