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
	bool expand_ids;
	std::vector<std::string> adjuncts;
	std::string sep;
	std::string adjacency_file;
	bool saturate_adjacency;

	ExportAdjunctsOfContacts() : no_serial(false), no_name(false), no_resSeq(false), no_resName(false), all(false), inter_residue(false), expand_ids(false), sep(" "), saturate_adjacency(false)
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
		expand_ids=input.get_flag("expand-ids");
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
		sep=input.get_value_or_default<std::string>("sep", " ");
		saturate_adjacency=input.get_flag("saturate-adjacency");
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
		doc.set_option_decription(CDOD("expand-ids", CDOD::DATATYPE_BOOL, "flag to output expanded IDs"));
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "adjunct names", ""));
		doc.set_option_decription(CDOD("sep", CDOD::DATATYPE_STRING, "output separator string", " "));
		doc.set_option_decription(CDOD("adjacency-file", CDOD::DATATYPE_STRING, "path to contact-contact adjacency output file", ""));
		doc.set_option_decription(CDOD("saturate-adjacency", CDOD::DATATYPE_BOOL, "flag to output enhanced adjacency links in inter-residue mode"));
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
			data_manager.assert_contacts_adjacencies_availability();
		}

		const std::set<std::size_t>& atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const std::set<std::size_t> contact_ids=(atom_ids.size()==data_manager.atoms().size()
				? data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts)
				: data_manager.selection_manager().select_contacts_by_atoms_and_atoms(data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts),	atom_ids, atom_ids, true, false, false));
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

		const std::set<std::string> adjuncts_filled_set(adjuncts_filled.begin(), adjuncts_filled.end());

		std::map<std::size_t, std::size_t> map_of_contact_indices;
		if(!adjacency_file.empty() || adjuncts_filled_set.count("contact_index")>0)
		{
			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				const std::size_t current_size=map_of_contact_indices.size();
				map_of_contact_indices[*it]=current_size;
			}
		}

		std::map<std::size_t, std::size_t> map_of_atom_indices;
		if(adjuncts_filled_set.count("atom_index1")>0 || adjuncts_filled_set.count("atom_index2")>0)
		{
			for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
			{
				const std::size_t current_size=map_of_atom_indices.size();
				map_of_atom_indices[*it]=current_size;
			}
		}

		std::map<std::size_t, std::size_t> map_of_inter_residue_contact_indices;
		if(inter_residue || adjuncts_filled_set.count("ir_contact_index")>0)
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
					if(index_it!=map_of_inter_residue_contact_indices.end())
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

			output << common::ChainResidueAtomDescriptor::str_header("ID1", expand_ids, "_", sep) << sep << common::ChainResidueAtomDescriptor::str_header("ID2", expand_ids, "_", sep);
			for(std::size_t i=0;i<adjuncts_filled.size();i++)
			{
				output << sep << adjuncts_filled[i];
			}
			output << "\n";

			if(!inter_residue)
			{
				for(std::map< std::size_t, std::vector<double> >::const_iterator it=map_of_output.begin();it!=map_of_output.end();++it)
				{
					{
						const Contact& contact=data_manager.contacts()[it->first];
						output << data_manager.atoms()[contact.ids[0]].crad.without_some_info(no_serial, no_name, no_resSeq, no_resName).str(expand_ids, sep) << sep;
						if(contact.solvent())
						{
							output << common::ChainResidueAtomDescriptor::solvent().str(expand_ids, sep);
						}
						else
						{
							output << data_manager.atoms()[contact.ids[1]].crad.without_some_info(no_serial, no_name, no_resSeq, no_resName).str(expand_ids, sep);
						}
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
			else
			{
				std::map< std::size_t, std::vector<double> > inter_residue_map_of_output;
				for(std::map< std::size_t, std::vector<double> >::const_iterator it=map_of_output.begin();it!=map_of_output.end();++it)
				{
					const std::vector<double>& input_values=it->second;
					std::vector<double>& output_values=inter_residue_map_of_output[map_of_inter_residue_contact_indices[it->first]];
					output_values.resize(adjuncts_filled.size(), std::numeric_limits<double>::max());
					for(std::size_t i=0;i<adjuncts_filled.size();i++)
					{
						const std::string& name=adjuncts_filled[i];
						const double input_value=input_values[i];
						double& output_value=output_values[i];
						const bool output_value_filled=(output_value!=std::numeric_limits<double>::max());
						if(name=="contact_index" || name=="ir_contact_index" || name=="atom_index1" || name=="atom_index2" || name=="distance")
						{
							output_value=(output_value_filled ? std::min(output_value, input_value) : input_value);
						}
						else
						{
							output_value=(output_value_filled ? (output_value+input_value) : input_value);
						}
					}
				}

				std::map<std::size_t, std::size_t> reverse_map_of_inter_residue_contact_indices;
				for(std::map<std::size_t, std::size_t>::const_iterator it=map_of_inter_residue_contact_indices.begin();it!=map_of_inter_residue_contact_indices.end();++it)
				{
					if(reverse_map_of_inter_residue_contact_indices.count(it->second)==0)
					{
						reverse_map_of_inter_residue_contact_indices[it->second]=it->first;
					}
				}

				for(std::map< std::size_t, std::vector<double> >::const_iterator it=inter_residue_map_of_output.begin();it!=inter_residue_map_of_output.end();++it)
				{
					{
						const Contact& contact=data_manager.contacts()[reverse_map_of_inter_residue_contact_indices[it->first]];
						output << data_manager.atoms()[contact.ids[0]].crad.without_some_info(true, true, no_resSeq, no_resName).str(expand_ids, sep) << sep;
						if(contact.solvent())
						{
							output << common::ChainResidueAtomDescriptor::solvent().str(expand_ids, sep);
						}
						else
						{
							output << data_manager.atoms()[contact.ids[1]].crad.without_some_info(true, true, no_resSeq, no_resName).str(expand_ids, sep);
						}
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
			const std::map< std::size_t, std::map<std::size_t, double> > contacts_graph=data_manager.extract_subset_of_contacts_adjacencies(contact_ids);
			OutputSelector output_selector(adjacency_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(adjacency_file, output);

			if(!inter_residue)
			{
				output << "contact_index1" << sep << "contact_index2" << sep << "edge_value" << "\n";
				for(std::map< std::size_t, std::map<std::size_t, double> >::const_iterator it=contacts_graph.begin();it!=contacts_graph.end();++it)
				{
					const std::size_t contact_index1=map_of_contact_indices[it->first];
					const std::map<std::size_t, double>& neighbors=it->second;
					for(std::map<std::size_t, double>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
					{
						const std::size_t contact_index2=map_of_contact_indices[jt->first];
						if(contact_index1<contact_index2)
						{
							output << contact_index1 << sep << contact_index2 << sep << jt->second << "\n";
						}
					}
				}
			}
			else
			{
				if(!saturate_adjacency)
				{
					std::map<apollota::Pair, double> inter_residue_contacts_graph;
					for(std::map< std::size_t, std::map<std::size_t, double> >::const_iterator it=contacts_graph.begin();it!=contacts_graph.end();++it)
					{
						const std::size_t contact_id1=it->first;
						const std::map<std::size_t, double>& neighbors=it->second;
						for(std::map<std::size_t, double>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
						{
							const std::size_t contact_id2=jt->first;
							if(contact_id1<contact_id2)
							{
								const apollota::Pair inter_residue_indices(map_of_inter_residue_contact_indices[contact_id1], map_of_inter_residue_contact_indices[contact_id2]);
								if(inter_residue_indices.get(0)!=inter_residue_indices.get(1))
								{
									inter_residue_contacts_graph[inter_residue_indices]+=jt->second;
								}
							}
						}
					}
					output << "ir_contact_index1" << sep << "ir_contact_index2" << sep << "edge_value" << "\n";
					for(std::map<apollota::Pair, double>::const_iterator it=inter_residue_contacts_graph.begin();it!=inter_residue_contacts_graph.end();++it)
					{
						output << it->first.get(0) << sep << it->first.get(1) << sep << it->second << "\n";
					}
				}
				else
				{
					std::map< apollota::Pair, std::vector<double> > inter_residue_contacts_graph;
					for(std::map< std::size_t, std::map<std::size_t, double> >::const_iterator it=contacts_graph.begin();it!=contacts_graph.end();++it)
					{
						const std::size_t contact_id1=it->first;
						const std::map<std::size_t, double>& neighbors=it->second;
						for(std::map<std::size_t, double>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
						{
							const std::size_t contact_id2=jt->first;
							if(contact_id1<contact_id2)
							{
								const apollota::Pair inter_residue_indices(map_of_inter_residue_contact_indices[contact_id1], map_of_inter_residue_contact_indices[contact_id2]);
								if(inter_residue_indices.get(0)!=inter_residue_indices.get(1))
								{
									std::vector<double>& v=inter_residue_contacts_graph[inter_residue_indices];
									if(v.size()!=3)
									{
										v.resize(3, 0.0);
									}
									v[0]+=jt->second;
									v[1]+=1.0;
									{
										const Contact& c1=data_manager.contacts()[contact_id1];
										const Contact& c2=data_manager.contacts()[contact_id2];
										apollota::SimplePoint p1a(data_manager.atoms()[c1.ids[0]].value);
										apollota::SimplePoint p1b(data_manager.atoms()[c1.ids[1]].value);
										apollota::SimplePoint p2a(data_manager.atoms()[c2.ids[0]].value);
										apollota::SimplePoint p2b(data_manager.atoms()[c2.ids[1]].value);
										v[2]+=apollota::min_angle(apollota::SimplePoint(), p1b-p1a, p2b-p2a);
									}
								}
							}
						}
					}
					for(std::map< apollota::Pair, std::vector<double> >::iterator it=inter_residue_contacts_graph.begin();it!=inter_residue_contacts_graph.end();++it)
					{
						if(it->second.size()==3)
						{
							for(int i=0;i<2;i++)
							{
								std::vector<double>& vii=inter_residue_contacts_graph[apollota::Pair(it->first.get(i), it->first.get(i))];
								if(vii.size()!=3)
								{
									vii.resize(3, 0.0);
								}
								vii[0]+=it->second[0];
								vii[1]+=it->second[1];
								vii[2]+=it->second[2];
							}
						}
					}
					for(std::map< apollota::Pair, std::vector<double> >::iterator it=inter_residue_contacts_graph.begin();it!=inter_residue_contacts_graph.end();++it)
					{
						if(it->second.size()==3)
						{
							if(it->first.get(0)==it->first.get(1))
							{
								it->second[0]=it->second[0]/(apollota::pi_value());
								it->second[2]=it->second[2]/(it->second[1]+1.0);
							}
							else
							{
								it->second[2]=it->second[2]/it->second[1];
							}
						}
					}
					output << "ir_contact_index1" << sep << "ir_contact_index2" << sep << "edge_value" << sep << "angle_value" << sep << "self_edge_value" << sep << "self_angle_value" << "\n";
					for(std::map< apollota::Pair, std::vector<double> >::const_iterator it=inter_residue_contacts_graph.begin();it!=inter_residue_contacts_graph.end();++it)
					{
						if(it->second.size()==3)
						{
							if(it->first.get(0)==it->first.get(1))
							{
								output << it->first.get(0) << sep << it->first.get(1) << sep << it->second[0] << sep << it->second[2] << sep << it->second[0] << sep << it->second[2] << "\n";
							}
							else
							{
								output << it->first.get(0) << sep << it->first.get(1) << sep << it->second[0] << sep << it->second[2] << sep << 0 << sep << 0 << "\n";
								output << it->first.get(1) << sep << it->first.get(0) << sep << it->second[0] << sep << it->second[2] << sep << 0 << sep << 0 << "\n";
							}
						}
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
