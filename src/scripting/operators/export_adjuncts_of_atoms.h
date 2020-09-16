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
	std::vector<std::string> adjuncts;

	ExportAdjunctsOfAtoms() : no_serial(false), no_name(false), no_resSeq(false), no_resName(false), all(false)
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
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
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
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "adjunct names", ""));
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

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		output << "ID";
		for(std::size_t i=0;i<adjuncts_filled.size();i++)
		{
			output << " " << adjuncts_filled[i];
		}
		output << "\n";

		std::map< common::ChainResidueAtomDescriptor, std::vector<double> > map_of_output;
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const Atom& atom=data_manager.atoms()[*it];
			std::vector<double>& output_values=map_of_output[atom.crad.without_some_info(no_serial, no_name, no_resSeq, no_resName)];
			output_values.resize(adjuncts_filled.size(), std::numeric_limits<double>::max());
			for(std::size_t i=0;i<adjuncts_filled.size();i++)
			{
				std::map<std::string, double>::const_iterator jt=atom.value.props.adjuncts.find(adjuncts_filled[i]);
				if(jt!=atom.value.props.adjuncts.end())
				{
					output_values[i]=jt->second;
				}
			}
		}

		for(std::map< common::ChainResidueAtomDescriptor, std::vector<double> >::const_iterator it=map_of_output.begin();it!=map_of_output.end();++it)
		{
			output << it->first;
			const std::vector<double>& output_values=it->second;
			for(std::size_t i=0;i<output_values.size();i++)
			{
				output << " ";
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
