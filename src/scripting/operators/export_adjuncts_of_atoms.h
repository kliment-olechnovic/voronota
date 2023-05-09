#ifndef SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportAdjunctsOfAtoms : public OperatorBase<ExportAdjunctsOfAtoms>
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

	std::string file;
	SelectionManager::Query parameters_for_selecting;
	bool no_serial;
	bool no_name;
	bool no_resSeq;
	bool no_resName;
	bool all;
	bool expand_ids;
	std::vector<std::string> adjuncts;
	std::string sep;

	ExportAdjunctsOfAtoms() : no_serial(false), no_name(false), no_resSeq(false), no_resName(false), all(false), expand_ids(false), sep(" ")
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		no_serial=input.get_flag("no-serial");
		no_name=input.get_flag("no-name");
		no_resSeq=input.get_flag("no-resSeq");
		no_resName=input.get_flag("no-resName");
		all=input.get_flag("all");
		expand_ids=input.get_flag("expand-ids");
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
		sep=input.get_value_or_default<std::string>("sep", " ");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("no-serial", CDOD::DATATYPE_BOOL, "flag to exclude atom serials"));
		doc.set_option_decription(CDOD("no-name", CDOD::DATATYPE_BOOL, "flag to exclude atom names"));
		doc.set_option_decription(CDOD("no-resSeq", CDOD::DATATYPE_BOOL, "flag to exclude residue sequence numbers"));
		doc.set_option_decription(CDOD("no-resName", CDOD::DATATYPE_BOOL, "flag to exclude residue names"));
		doc.set_option_decription(CDOD("all", CDOD::DATATYPE_BOOL, "flag to export all adjuncts"));
		doc.set_option_decription(CDOD("expand-ids", CDOD::DATATYPE_BOOL, "flag to output expanded IDs"));
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "adjunct names", ""));
		doc.set_option_decription(CDOD("sep", CDOD::DATATYPE_STRING, "output separator string", " "));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_file_name_input(file, false);

		if(!all && adjuncts.empty())
		{
			throw std::runtime_error(std::string("No adjuncts specified."));
		}

		if(all && !adjuncts.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of adjuncts."));
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::vector<std::string> adjuncts_filled;

		if(all)
		{
			std::set<std::string> all_adjuncts;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Atom& atom=data_manager.atoms()[*it];
				for(std::map<std::string, double>::const_iterator jt=atom.value.props.adjuncts.begin();jt!=atom.value.props.adjuncts.end();++jt)
				{
					all_adjuncts.insert(jt->first);
				}
			}
			if(all_adjuncts.empty())
			{
				throw std::runtime_error(std::string("Selected atoms have no adjuncts."));
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
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t current_size=map_of_indices.size();
			map_of_indices[*it]=current_size;
		}

		std::map<common::ChainResidueAtomDescriptor, std::size_t> map_of_residue_indices;
		{
			std::set<common::ChainResidueAtomDescriptor> set_of_residue_crads;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				set_of_residue_crads.insert(data_manager.atoms()[*it].crad.without_atom());
			}
			for(std::set<common::ChainResidueAtomDescriptor>::const_iterator it=set_of_residue_crads.begin();it!=set_of_residue_crads.end();++it)
			{
				const std::size_t current_size=map_of_residue_indices.size();
				map_of_residue_indices[*it]=current_size;
			}
		}

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		output << common::ChainResidueAtomDescriptor::str_header("ID", expand_ids, "_", sep);
		for(std::size_t i=0;i<adjuncts_filled.size();i++)
		{
			output << sep << adjuncts_filled[i];
		}
		output << "\n";

		std::map< std::size_t, std::vector<double> > map_of_output;
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const Atom& atom=data_manager.atoms()[*it];
			std::vector<double>& output_values=map_of_output[*it];
			output_values.resize(adjuncts_filled.size(), std::numeric_limits<double>::max());
			for(std::size_t i=0;i<adjuncts_filled.size();i++)
			{
				std::map<std::string, double>::const_iterator jt=atom.value.props.adjuncts.find(adjuncts_filled[i]);
				if(jt!=atom.value.props.adjuncts.end())
				{
					output_values[i]=jt->second;
				}
				else if(adjuncts_filled[i]=="atom_index")
				{
					std::map<std::size_t, std::size_t>::const_iterator index_it=map_of_indices.find(*it);
					if(index_it!=map_of_indices.end())
					{
						output_values[i]=index_it->second;
					}
				}
				else if(adjuncts_filled[i]=="residue_index")
				{
					std::map<common::ChainResidueAtomDescriptor, std::size_t>::const_iterator index_it=map_of_residue_indices.find(atom.crad.without_atom());
					if(index_it!=map_of_residue_indices.end())
					{
						output_values[i]=index_it->second;
					}
				}
				else if(adjuncts_filled[i]=="center_x")
				{
					output_values[i]=atom.value.x;
				}
				else if(adjuncts_filled[i]=="center_y")
				{
					output_values[i]=atom.value.y;
				}
				else if(adjuncts_filled[i]=="center_z")
				{
					output_values[i]=atom.value.z;
				}
				else if(adjuncts_filled[i]=="radius")
				{
					output_values[i]=atom.value.r;
				}
			}
		}

		for(std::map< std::size_t, std::vector<double> >::const_iterator it=map_of_output.begin();it!=map_of_output.end();++it)
		{
			output << data_manager.atoms()[it->first].crad.without_some_info(no_serial, no_name, no_resSeq, no_resName).str(expand_ids, sep);
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
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_ATOMS_H_ */
