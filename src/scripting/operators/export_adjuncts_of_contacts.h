#ifndef SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_CONTACTS_H_
#define SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportAdjunctsOfContacts : public OperatorBase<ExportAdjunctsOfContacts>
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

	std::string file;
	SelectionManager::Query parameters_for_selecting_atoms;
	SelectionManager::Query parameters_for_selecting_contacts;
	bool no_serial;
	bool no_name;
	bool no_resSeq;
	bool no_resName;
	bool all;
	bool inter_residue;
	std::vector<std::string> adjuncts;
	std::string sep;
	std::string adjacency_file;

	ExportAdjunctsOfContacts() : no_serial(false), no_name(false), no_resSeq(false), no_resName(false), all(false), inter_residue(false), sep(" ")
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query("atoms-", "[]", input);
		parameters_for_selecting_contacts=OperatorsUtilities::read_generic_selecting_query("contacts-", "[]", input);
		no_serial=input.get_flag("no-serial");
		no_name=input.get_flag("no-name");
		no_resSeq=input.get_flag("no-resSeq");
		no_resName=input.get_flag("no-resName");
		all=input.get_flag("all");
		inter_residue=input.get_flag("inter-residue");
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
		sep=input.get_value_or_default<std::string>("sep", " ");
		adjacency_file=input.get_value_or_default<std::string>("adjacency-file", "");
		assert_file_name_input(adjacency_file, true);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		OperatorsUtilities::document_read_generic_selecting_query("atoms-", "[]", doc);
		OperatorsUtilities::document_read_generic_selecting_query("contacts-", "[]", doc);
		doc.set_option_decription(CDOD("no-serial", CDOD::DATATYPE_BOOL, "flag to exclude atom serials"));
		doc.set_option_decription(CDOD("no-name", CDOD::DATATYPE_BOOL, "flag to exclude atom names"));
		doc.set_option_decription(CDOD("no-resSeq", CDOD::DATATYPE_BOOL, "flag to exclude residue sequence numbers"));
		doc.set_option_decription(CDOD("no-resName", CDOD::DATATYPE_BOOL, "flag to exclude residue names"));
		doc.set_option_decription(CDOD("all", CDOD::DATATYPE_BOOL, "flag to export all adjuncts"));
		doc.set_option_decription(CDOD("inter-residue", CDOD::DATATYPE_BOOL, "flag for simplified inter-residue output"));
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "adjunct names", ""));
		doc.set_option_decription(CDOD("sep", CDOD::DATATYPE_STRING, "output separator string", " "));
		doc.set_option_decription(CDOD("adjacency-file", CDOD::DATATYPE_STRING, "path to contact-contact adjacency output file", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_file_name_input(file, false);

		if(!all && adjuncts.empty())
		{
			throw std::runtime_error(std::string("No adjuncts specified."));
		}

		if(all && !adjuncts.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of adjuncts."));
		}

		if(!adjacency_file.empty())
		{
			data_manager.assert_triangulation_info_availability();
		}

		const std::set<std::size_t>& atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts_by_atoms_and_atoms(
				data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts),
				atom_ids, atom_ids, false);
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::vector<std::string> adjuncts_filled;

		if(all)
		{
			std::set<std::string> all_adjuncts;
			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				const Contact& contact=data_manager.contacts()[*it];
				for(std::map<std::string, double>::const_iterator jt=contact.value.props.adjuncts.begin();jt!=contact.value.props.adjuncts.end();++jt)
				{
					all_adjuncts.insert(jt->first);
				}
			}
			if(all_adjuncts.empty())
			{
				throw std::runtime_error(std::string("Selected contacts have no adjuncts."));
			}
			adjuncts_filled=std::vector<std::string>(all_adjuncts.begin(), all_adjuncts.end());
		}
		else
		{
			adjuncts_filled=adjuncts;
		}

		if(adjuncts_filled.empty())
		{
			throw std::runtime_error(std::string("No adjuncts specified."));
		}

		std::map<std::size_t, std::size_t> map_of_contact_indices;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const std::size_t current_size=map_of_contact_indices.size();
			map_of_contact_indices[*it]=current_size;
		}

		std::map<std::size_t, std::size_t> map_of_atom_indices;
		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			const std::size_t current_size=map_of_atom_indices.size();
			map_of_atom_indices[*it]=current_size;
		}

		std::map<std::size_t, std::size_t> map_of_inter_residue_contact_indices;
		{
			std::set<common::ChainResidueAtomDescriptorsPair> set_of_inter_residue_contact_crads;
			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				set_of_inter_residue_contact_crads.insert(common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]).without_some_info(true, true, false, false));
			}
			std::map<common::ChainResidueAtomDescriptorsPair, std::size_t> map_of_inter_residue_contact_crads_indices;
			for(std::set<common::ChainResidueAtomDescriptorsPair>::const_iterator it=set_of_inter_residue_contact_crads.begin();it!=set_of_inter_residue_contact_crads.end();++it)
			{
				const std::size_t current_size=map_of_inter_residue_contact_crads_indices.size();
				map_of_inter_residue_contact_crads_indices[*it]=current_size;
			}
			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				map_of_inter_residue_contact_indices[*it]=map_of_inter_residue_contact_crads_indices[common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), data_manager.contacts()[*it]).without_some_info(true, true, false, false)];
			}
		}

		std::map< std::size_t, std::vector<double> > map_of_output;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			std::vector<double>& output_values=map_of_output[*it];
			output_values.resize(adjuncts_filled.size(), std::numeric_limits<double>::max());
			for(std::size_t i=0;i<adjuncts_filled.size();i++)
			{
				std::map<std::string, double>::const_iterator jt=contact.value.props.adjuncts.find(adjuncts_filled[i]);
				double output_value=std::numeric_limits<double>::max();
				if(jt!=contact.value.props.adjuncts.end())
				{
					output_value=jt->second;
				}
				else if(adjuncts_filled[i]=="area")
				{
					output_value=contact.value.area;
				}
				else if(adjuncts_filled[i]=="distance")
				{
					output_value=contact.value.dist;
				}
				else if(adjuncts_filled[i]=="contact_index")
				{
					std::map<std::size_t, std::size_t>::const_iterator index_it=map_of_contact_indices.find(*it);
					if(index_it!=map_of_contact_indices.end())
					{
						output_value=index_it->second;
					}
				}
				else if(adjuncts_filled[i]=="ir_contact_index")
				{
					std::map<std::size_t, std::size_t>::const_iterator index_it=map_of_inter_residue_contact_indices.find(*it);
					if(index_it!=map_of_contact_indices.end())
					{
						output_value=index_it->second;
					}
				}
				else if(adjuncts_filled[i]=="atom_index1")
				{
					std::map<std::size_t, std::size_t>::const_iterator index_it=map_of_atom_indices.find(contact.ids[0]);
					if(index_it!=map_of_atom_indices.end())
					{
						output_value=index_it->second;
					}
				}
				else if(adjuncts_filled[i]=="atom_index2")
				{
					std::map<std::size_t, std::size_t>::const_iterator index_it=map_of_atom_indices.find(contact.ids[1]);
					if(index_it!=map_of_atom_indices.end())
					{
						output_value=index_it->second;
					}
				}
				if(output_value<std::numeric_limits<double>::max())
				{
					output_values[i]=output_value;
				}
			}
		}

		{
			OutputSelector output_selector(file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			output << "ID1" << sep << "ID2";
			for(std::size_t i=0;i<adjuncts_filled.size();i++)
			{
				output << sep << adjuncts_filled[i];
			}
			output << "\n";

			std::set<std::size_t> outputed_inter_residue_ids;

			for(std::map< std::size_t, std::vector<double> >::const_iterator it=map_of_output.begin();it!=map_of_output.end();++it)
			{
				bool allowed=true;
				if(inter_residue)
				{
					const std::size_t inter_residue_id=map_of_inter_residue_contact_indices[it->first];
					if(outputed_inter_residue_ids.count(inter_residue_id)>0)
					{
						allowed=false;
					}
					else
					{
						outputed_inter_residue_ids.insert(inter_residue_id);
					}
				}
				if(allowed)
				{
					const Contact& contact=data_manager.contacts()[it->first];
					output << data_manager.atoms()[contact.ids[0]].crad.without_some_info(no_serial || inter_residue, no_name || inter_residue, no_resSeq, no_resName) << sep;
					if(contact.solvent())
					{
						output << common::ChainResidueAtomDescriptor::solvent();
					}
					else
					{
						output << data_manager.atoms()[contact.ids[1]].crad.without_some_info(no_serial || inter_residue, no_name || inter_residue, no_resSeq, no_resName);
					}
					const std::vector<double>& output_values=it->second;
					for(std::size_t i=0;i<output_values.size();i++)
					{
						output << sep;
						if(output_values[i]!=std::numeric_limits<double>::max())
						{
							output << output_values[i];
						}
						else
						{
							output << "NA";
						}
					}
					output << "\n";
				}
			}
		}

		if(!adjacency_file.empty())
		{
			std::map<apollota::Pair, std::size_t> map_of_pairs_to_contact_ids;
			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				const Contact& contact=data_manager.contacts()[*it];
				map_of_pairs_to_contact_ids[apollota::Pair(contact.ids[0], contact.ids[1])]=(*it);
			}

			std::set< std::pair<apollota::Pair, apollota::Pair> > pairs_adjacencies;

			for(apollota::Triangulation::QuadruplesMap::const_iterator it=data_manager.triangulation_info().quadruples_map.begin();it!=data_manager.triangulation_info().quadruples_map.end();++it)
			{
				const apollota::Quadruple& quadruple=it->first;
				for(int a=0;a<4;a++)
				{
					const apollota::Triple triple=quadruple.exclude(a);
					const apollota::Pair pair01(triple.get(0), triple.get(1));
					const apollota::Pair pair02(triple.get(0), triple.get(2));
					const apollota::Pair pair12(triple.get(1), triple.get(2));
					const bool allowed01=(map_of_pairs_to_contact_ids.count(pair01)>0);
					const bool allowed02=(map_of_pairs_to_contact_ids.count(pair02)>0);
					const bool allowed12=(map_of_pairs_to_contact_ids.count(pair12)>0);
					if(allowed01 && allowed02)
					{
						pairs_adjacencies.insert(std::pair<apollota::Pair, apollota::Pair>(pair01, pair02));
					}
					if(allowed01 && allowed12)
					{
						pairs_adjacencies.insert(std::pair<apollota::Pair, apollota::Pair>(pair01, pair12));
					}
					if(allowed02 && allowed12)
					{
						pairs_adjacencies.insert(std::pair<apollota::Pair, apollota::Pair>(pair02, pair12));
					}
				}
			}

			OutputSelector output_selector(adjacency_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(adjacency_file, output);

			if(!inter_residue)
			{
				output << "contact_index1" << sep << "contact_index2" << "\n";
				for(std::set< std::pair<apollota::Pair, apollota::Pair> >::const_iterator it=pairs_adjacencies.begin();it!=pairs_adjacencies.end();++it)
				{
					const apollota::Pair indices(map_of_pairs_to_contact_ids[it->first], map_of_pairs_to_contact_ids[it->second]);
					output << map_of_contact_indices[indices.get(0)] << sep << map_of_contact_indices[indices.get(1)] << "\n";
				}
			}
			else
			{
				output << "ir_contact_index1" << sep << "ir_contact_index2" << "\n";
				std::set<apollota::Pair> outputed_inter_residue_indices;
				for(std::set< std::pair<apollota::Pair, apollota::Pair> >::const_iterator it=pairs_adjacencies.begin();it!=pairs_adjacencies.end();++it)
				{
					const apollota::Pair indices(map_of_pairs_to_contact_ids[it->first], map_of_pairs_to_contact_ids[it->second]);
					const apollota::Pair inter_residue_indices(map_of_inter_residue_contact_indices[indices.get(0)], map_of_inter_residue_contact_indices[indices.get(1)]);
					if(inter_residue_indices.get(0)!=inter_residue_indices.get(1) && outputed_inter_residue_indices.count(inter_residue_indices)==0)
					{
						outputed_inter_residue_indices.insert(inter_residue_indices);
						output << inter_residue_indices.get(0) << sep << inter_residue_indices.get(1) << "\n";
					}
				}
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), contact_ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_CONTACTS_H_ */
