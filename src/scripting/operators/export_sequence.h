#ifndef SCRIPTING_OPERATORS_EXPORT_SEQUENCE_H_
#define SCRIPTING_OPERATORS_EXPORT_SEQUENCE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportSequence : public OperatorBase<ExportSequence>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

    std::string file;
    SelectionManager::Query parameters_for_selecting;
    std::string gap_filler;
	bool not_fill_middle_gaps;
	bool not_fill_start_gaps;

	ExportSequence() : gap_filler("X"), not_fill_middle_gaps(false), not_fill_start_gaps(false)
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		gap_filler=input.get_value_or_default<std::string>("gap-filler", "X");
		not_fill_middle_gaps=input.get_flag("not-fill-middle-gaps");
		not_fill_start_gaps=input.get_flag("not-fill-start-gaps");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("gap-filler", CDOD::DATATYPE_STRING, "letter to use for filling gaps", "X"));
		doc.set_option_decription(CDOD("not-fill-middle-gaps", CDOD::DATATYPE_BOOL, "flag to not fill middle gaps"));
		doc.set_option_decription(CDOD("not-fill-start-gaps", CDOD::DATATYPE_BOOL, "flag to not fill start gaps"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_file_name_input(file, false);

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		const char gap_filler_letter=(gap_filler=="dash" ? '-' : (gap_filler.empty() ? 'X' : gap_filler[0]));

		const std::map<std::string, std::string> chain_sequences=common::ConstructionOfPrimaryStructure::collect_chain_sequences_from_atom_ids(data_manager.primary_structure_info(), ids, gap_filler_letter, !not_fill_middle_gaps, !not_fill_start_gaps);

		for(std::map<std::string, std::string>::const_iterator it=chain_sequences.begin();it!=chain_sequences.end();++it)
		{
			output << ">" << it->first << "\n";
			output << it->second << "\n";
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_SEQUENCE_H_ */
