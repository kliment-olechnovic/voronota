#ifndef DUKTAPER_OPERATORS_EXPORT_ATOMS_TO_MMCIF_H_
#define DUKTAPER_OPERATORS_EXPORT_ATOMS_TO_MMCIF_H_

#include "../operators_common.h"

#include "../dependencies/utilities/gemmi_wrappers.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ExportAtomsToMMCIF : public scripting::OperatorBase<ExportAtomsToMMCIF>
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

	std::string file;
	std::string model_name;
	int model_number;
	std::string selection_expresion_for_atoms;
	std::string pdb_b_factor_name;

	ExportAtomsToMMCIF() : model_number(1)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		model_name=input.get_value_or_default<std::string>("model-name", "");
		model_number=input.get_value_or_default<int>("model-number", 1);
		selection_expresion_for_atoms=input.get_value_or_default<std::string>("use", "[]");
		pdb_b_factor_name=input.get_value_or_default<std::string>("pdb-b-factor", "tf");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to output file"));
		doc.set_option_decription(CDOD("model-name", CDOD::DATATYPE_STRING, "model name", ""));
		doc.set_option_decription(CDOD("model-number", CDOD::DATATYPE_INT, "model number", 1));
		doc.set_option_decription(CDOD("use", CDOD::DATATYPE_STRING, "selection expression for atoms", "[]"));
		doc.set_option_decription(CDOD("pdb-b-factor", CDOD::DATATYPE_STRING, "name of adjunct values to write as b-factors in PDB output", "tf"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		scripting::assert_file_name_input(file, false);

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(scripting::SelectionManager::Query(selection_expresion_for_atoms, false));

		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		scripting::OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		scripting::assert_io_stream(file, output);

		std::vector<gemmi_wrappers::ModelRecord> model_records(1);

		gemmi_wrappers::ModelRecord& model_record=model_records.back();

		if(!model_name.empty())
		{
			model_record.name=model_name;
		}
		else
		{
			std::ostringstream model_name_output;
			model_name_output << model_number;
			model_record.name=model_name_output.str();
		}

		model_record.atom_records.reserve(atom_ids.size());

		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			model_record.atom_records.push_back(gemmi_wrappers::AtomRecord());
			set_atom_record_for_mmcif(data_manager.atoms()[*it], pdb_b_factor_name, model_record.atom_records.back());
		}

		gemmi_wrappers::write_to_stream(model_records, output);

		Result result;
		result.file=file;
		if(output_selector.location_type()==scripting::OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.atoms_summary=scripting::SummaryOfAtoms(data_manager.atoms(), atom_ids);

		return result;
	}

	static bool set_atom_record_for_mmcif(const scripting::Atom& atom, const std::string& b_factor_name, gemmi_wrappers::AtomRecord& atom_record)
	{
		atom_record.auth_chainID=atom.crad.chainID;
		atom_record.auth_resSeq_valid=(atom.crad.resSeq!=common::ChainResidueAtomDescriptor::null_num());
		atom_record.auth_resSeq=(atom_record.auth_resSeq_valid ? atom.crad.resSeq : -1);
		atom_record.iCode=atom.crad.iCode;
		atom_record.resName=atom.crad.resName;
		atom_record.label_chainID=atom.crad.chainID;
		atom_record.label_resSeq_valid=(atom.crad.resSeq!=common::ChainResidueAtomDescriptor::null_num());
		atom_record.label_resSeq=(atom_record.label_resSeq_valid ? atom.crad.resSeq : -1);
		atom_record.name=atom.crad.name;
		atom_record.x_valid=true;
		atom_record.x=atom.value.x;
		atom_record.y_valid=true;
		atom_record.y=atom.value.y;
		atom_record.z_valid=true;
		atom_record.z=atom.value.z;
		{
			std::map<std::string, double>::const_iterator adjunct_it=atom.value.props.adjuncts.find("oc");
			atom_record.occupancy_valid=(adjunct_it!=atom.value.props.adjuncts.end());
			atom_record.occupancy=(atom_record.occupancy_valid ? adjunct_it->second : 1.0);
		}
		{
			std::map<std::string, double>::const_iterator adjunct_it=atom.value.props.adjuncts.find(b_factor_name);
			atom_record.tempFactor_valid=(adjunct_it!=atom.value.props.adjuncts.end());
			atom_record.tempFactor=(atom_record.tempFactor_valid ? adjunct_it->second : 0.0);
		}
		atom_record.record_name=(atom.value.props.tags.count("het")>0 ? "HETATM" : "ATOM");
		atom_record.element="";
		for(std::set<std::string>::const_iterator tag_it=atom.value.props.tags.begin();tag_it!=atom.value.props.tags.end() && atom_record.element.empty();++tag_it)
		{
			static const std::string el_prefix="el=";
			const std::string& tag=(*tag_it);
			if(tag.size()>el_prefix.size() && tag.compare(0, el_prefix.size(), el_prefix)==0)
			{
				atom_record.element=tag.substr(el_prefix.size());
			}
		}
		return true;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_EXPORT_ATOMS_TO_MMCIF_H_ */
