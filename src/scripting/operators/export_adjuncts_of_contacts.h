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
	std::vector<std::string> adjuncts;
	std::string sep;

	ExportAdjunctsOfContacts() : no_serial(false), no_name(false), no_resSeq(false), no_resName(false), all(false), sep(" ")
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
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
		sep=input.get_value_or_default<std::string>("sep", " ");
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
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "adjunct names", ""));
		doc.set_option_decription(CDOD("sep", CDOD::DATATYPE_STRING, "output separator string", " "));
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

		SelectionManager::Query parameters_for_selecting_contacts_considering_atoms=parameters_for_selecting_contacts;
		parameters_for_selecting_contacts_considering_atoms.from_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

		const std::set<std::size_t>& atom_ids=parameters_for_selecting_contacts_considering_atoms.from_ids;
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts_considering_atoms);
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

		std::map<std::size_t, std::size_t> map_of_indices;
		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			const std::size_t current_size=map_of_indices.size();
			map_of_indices[*it]=current_size;
		}

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		output << "ID1" << sep << "ID2";
		for(std::size_t i=0;i<adjuncts_filled.size();i++)
		{
			output << sep << adjuncts_filled[i];
		}
		output << "\n";

		std::map< std::size_t, std::vector<double> > map_of_output;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			std::vector<double>& output_values=map_of_output[*it];
			output_values.resize(adjuncts_filled.size(), std::numeric_limits<double>::max());
			for(std::size_t i=0;i<adjuncts_filled.size();i++)
			{
				std::map<std::string, double>::const_iterator jt=contact.value.props.adjuncts.find(adjuncts_filled[i]);
				if(jt!=contact.value.props.adjuncts.end())
				{
					output_values[i]=jt->second;
				}
				else if(adjuncts_filled[i]=="area")
				{
					output_values[i]=contact.value.area;
				}
				else if(adjuncts_filled[i]=="distance")
				{
					output_values[i]=contact.value.dist;
				}
				else if(adjuncts_filled[i]=="atom_index1")
				{
					std::map<std::size_t, std::size_t>::const_iterator index_it=map_of_indices.find(contact.ids[0]);
					if(index_it!=map_of_indices.end())
					{
						output_values[i]=index_it->second;
					}
				}
				else if(adjuncts_filled[i]=="atom_index2")
				{
					std::map<std::size_t, std::size_t>::const_iterator index_it=map_of_indices.find(contact.ids[1]);
					if(index_it!=map_of_indices.end())
					{
						output_values[i]=index_it->second;
					}
				}
			}
		}

		for(std::map< std::size_t, std::vector<double> >::const_iterator it=map_of_output.begin();it!=map_of_output.end();++it)
		{
			const Contact& contact=data_manager.contacts()[it->first];
			output << data_manager.atoms()[contact.ids[0]].crad.without_some_info(no_serial, no_name, no_resSeq, no_resName) << sep;
			if(contact.solvent())
			{
				output << common::ChainResidueAtomDescriptor::solvent();
			}
			else
			{
				output << data_manager.atoms()[contact.ids[1]].crad.without_some_info(no_serial, no_name, no_resSeq, no_resName);
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

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), contact_ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_CONTACTS_H_ */
