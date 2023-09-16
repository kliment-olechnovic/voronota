#ifndef DUKTAPER_OPERATORS_EXPORT_ATOMS_TO_MMCIF_MULTIMODEL_H_
#define DUKTAPER_OPERATORS_EXPORT_ATOMS_TO_MMCIF_MULTIMODEL_H_

#include "export_atoms_to_mmcif.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ExportAtomsToMMCIFMultimodel : public scripting::OperatorBase<ExportAtomsToMMCIFMultimodel>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;
		scripting::SummaryOfAtoms atoms_summary;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			else
			{
				heterostorage.forwarding_strings["download"].push_back(file);
			}
			scripting::VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	scripting::CongregationOfDataManagers::ObjectQuery query;
	std::string file;
	bool model_numbers;
	std::string selection_expresion_for_atoms;
	std::string pdb_b_factor_name;

	ExportAtomsToMMCIFMultimodel() : model_numbers(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		query=scripting::OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		file=input.get_value<std::string>("file");
		model_numbers=input.get_flag("model-numbers");
		selection_expresion_for_atoms=input.get_value_or_default<std::string>("use", "[]");
		pdb_b_factor_name=input.get_value_or_default<std::string>("pdb-b-factor", "tf");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to output file"));
		doc.set_option_decription(CDOD("model-numbers", CDOD::DATATYPE_BOOL, "flag to set model numbers instead of using object names"));
		doc.set_option_decription(CDOD("use", CDOD::DATATYPE_STRING, "selection expression for atoms", "[]"));
		doc.set_option_decription(CDOD("pdb-b-factor", CDOD::DATATYPE_STRING, "name of adjunct values to write as b-factors in PDB output", "tf"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		scripting::assert_file_name_input(file, false);

		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects(query);

		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		scripting::OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		scripting::assert_io_stream(file, output);

		std::vector<gemmi_wrappers::ModelRecord> model_records;

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
			scripting::DataManager& data_manager=(*(objects[i]));
			const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(scripting::SelectionManager::Query(selection_expresion_for_atoms, false));
			if(!atom_ids.empty())
			{
				model_records.push_back(gemmi_wrappers::ModelRecord());
				gemmi_wrappers::ModelRecord& model_record=model_records.back();

				if(!model_numbers)
				{
					model_record.name=attributes.name;
				}
				else
				{
					std::ostringstream model_name_output;
					model_name_output << (i+1);
					model_record.name=model_name_output.str();
				}

				model_record.atom_records.reserve(atom_ids.size());

				for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
				{
					model_record.atom_records.push_back(gemmi_wrappers::AtomRecord());
					ExportAtomsToMMCIF::set_atom_record_for_mmcif(data_manager.atoms()[*it], pdb_b_factor_name, model_record.atom_records.back());
				}

				result.atoms_summary.feed(scripting::SummaryOfAtoms(data_manager.atoms(), atom_ids));
			}
		}

		if(model_records.empty())
		{
			throw std::runtime_error(std::string("No atoms selected in any relevant objects."));
		}

		gemmi_wrappers::write_to_stream(model_records, output);

		result.file=file;
		if(output_selector.location_type()==scripting::OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_EXPORT_ATOMS_TO_MMCIF_MULTIMODEL_H_ */
