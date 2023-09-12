#ifndef DUKTAPER_OPERATORS_IMPORT_MMCIF_H_
#define DUKTAPER_OPERATORS_IMPORT_MMCIF_H_

#include "../operators_common.h"

#include "../dependencies/utilities/gemmi_wrappers.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ImportMMCIF : public scripting::OperatorBase<ImportMMCIF>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		scripting::SummaryOfAtoms atoms_summary;
		std::vector<std::string> object_names;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			scripting::VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.summaries_of_atoms["loaded"]=atoms_summary;
			std::vector<scripting::VariantValue>& object_names_array=heterostorage.variant_object.values_array("object_names");
			for(std::size_t i=0;i<object_names.size();i++)
			{
				object_names_array.push_back(scripting::VariantValue(object_names[i]));
			}
		}
	};

	bool forced_include_heteroatoms;
	bool forced_include_hydrogens;
	bool forced_same_radius_for_all;
	bool include_heteroatoms;
	bool include_hydrogens;
	bool use_label_ids;
	double same_radius_for_all;
	std::vector<std::string> files;
	std::string title;

	ImportMMCIF()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		files=input.get_value_vector_or_all_unused_unnamed_values("files");
		title=input.get_value_or_default<std::string>("title", "");
		forced_include_heteroatoms=input.is_option("include-heteroatoms");
		forced_include_hydrogens=input.is_option("include-hydrogens");
		forced_same_radius_for_all=input.is_option("same-radius-for-all");
		include_heteroatoms=input.get_flag("include-heteroatoms");
		include_hydrogens=input.get_flag("include-hydrogens");
		use_label_ids=input.get_flag("use-label-ids");
		same_radius_for_all=input.get_value_or_default<double>("same-radius-for-all", scripting::LoadingOfData::Configuration::recommended_default_radius());
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("files", CDOD::DATATYPE_STRING_ARRAY, "paths to files"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(files.empty())
		{
			throw std::runtime_error(std::string("Missing file paths."));
		}

		std::multimap< std::string, std::vector<scripting::Atom> > models_multimap;

		for(std::size_t i=0;i<files.size();i++)
		{
			const std::string& main_file=files[i];
			const std::string main_file_basename=scripting::OperatorsUtilities::get_basename_from_path(main_file);

			scripting::InputSelector finput_selector(main_file);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+main_file+"'.");
			}

			std::vector<gemmi_wrappers::ModelRecord> model_records=gemmi_wrappers::read(finput);

			if(model_records.empty())
			{
				throw std::runtime_error(std::string("Failed to read any structural models from file '")+main_file+"'.");
			}

			for(std::size_t j=0;j<model_records.size();j++)
			{
				gemmi_wrappers::ModelRecord& model_record=model_records[j];
				if(model_record.atom_records.empty())
				{
					throw std::runtime_error(std::string("Failed to read structural model atom records from file '")+main_file+"'.");
				}
				const std::vector<scripting::Atom> atoms=collect_atoms_from_atom_records(model_record.atom_records);
				if(atoms.empty())
				{
					throw std::runtime_error(std::string("Failed to process atom records from file '")+main_file+"'.");
				}

				std::string title_to_use=title;
				if(title_to_use.empty())
				{
					title_to_use=main_file_basename;
				}
				else
				{
					scripting::OperatorsUtilities::replace_all_marks_in_string(title_to_use, "file", main_file_basename);
					scripting::OperatorsUtilities::replace_all_marks_in_string(title_to_use, "model", model_record.name);
				}

				models_multimap.insert(std::make_pair(title_to_use, atoms));
			}
		}

		Result result;

		for(std::multimap< std::string, std::vector<scripting::Atom> >::iterator model_it=models_multimap.begin();model_it!=models_multimap.end();++model_it)
		{
			scripting::DataManager* object_new=congregation_of_data_managers.add_object(scripting::DataManager(), model_it->first);
			scripting::DataManager& data_manager=*object_new;
			data_manager.reset_atoms_by_swapping(model_it->second);
			result.atoms_summary.feed(scripting::SummaryOfAtoms(data_manager.atoms()));
			result.object_names.push_back(congregation_of_data_managers.get_object_attributes(object_new).name);
		}

		congregation_of_data_managers.set_all_objects_picked(false);
		for(std::size_t i=0;i<result.object_names.size();i++)
		{
			congregation_of_data_managers.set_object_picked(result.object_names[i], true);
		}

		return result;
	}

private:
	std::vector<scripting::Atom> collect_atoms_from_atom_records(const std::vector<gemmi_wrappers::AtomRecord>& atom_records) const
	{
		const scripting::LoadingOfData::Configuration& default_config=scripting::LoadingOfData::Configuration::get_default_configuration();

		std::vector<scripting::Atom> atoms;
		atoms.reserve(atom_records.size());

		for(std::size_t i=0;i<atom_records.size();i++)
		{
			const gemmi_wrappers::AtomRecord& atom_record=atom_records[i];
			if(
				atom_record.x_valid &&
				atom_record.y_valid &&
				atom_record.z_valid &&
				!atom_record.name.empty() &&
				(atom_record.altLoc.empty() || atom_record.altLoc=="A" || atom_record.altLoc=="1" || atom_record.altLoc==".") &&
				(atom_record.record_name=="ATOM" || ((include_heteroatoms || (!forced_include_heteroatoms && default_config.include_heteroatoms)) && atom_record.record_name=="HETATM")) &&
				((atom_record.name.find("H")!=0 && atom_record.element!="H" && atom_record.element!="D" && !atom_record.is_hydrogen) || (include_hydrogens || (!forced_include_hydrogens && default_config.include_hydrogens))) &&
				atom_record.resName!="HOH"
			)
			{
				const common::ChainResidueAtomDescriptor crad(
						(atom_record.serial_valid ? atom_record.serial : common::ChainResidueAtomDescriptor::null_num()),
						(use_label_ids ? atom_record.label_chainID : atom_record.auth_chainID),
						(use_label_ids ? atom_record.label_resSeq : atom_record.auth_resSeq),
						atom_record.resName,
						atom_record.name,
						atom_record.altLoc,
						atom_record.iCode);

				if(crad.valid())
				{
					common::BallValue value;
					value.x=atom_record.x;
					value.y=atom_record.y;
					value.z=atom_record.z;
					value.r=forced_same_radius_for_all ? same_radius_for_all : default_config.atom_radius_assigner.get_atom_radius(atom_record.resName, atom_record.name);
					if(atom_record.record_name=="HETATM")
					{
						value.props.tags.insert("het");
					}
					if(!atom_record.element.empty())
					{
						value.props.tags.insert(std::string("el=")+atom_record.element);
					}
					if(atom_record.occupancy_valid)
					{
						value.props.adjuncts["oc"]=atom_record.occupancy;
					}
					if(atom_record.tempFactor_valid)
					{
						value.props.adjuncts["tf"]=atom_record.tempFactor;
					}
					atoms.push_back(scripting::Atom(crad, value));
				}
			}
		}
		return atoms;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_IMPORT_MMCIF_H_ */

