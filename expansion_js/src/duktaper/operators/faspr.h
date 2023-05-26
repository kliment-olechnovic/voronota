#ifndef DUKTAPER_OPERATORS_FASPR_H_
#define DUKTAPER_OPERATORS_FASPR_H_

#include "../../dependencies/faspr/faspr_wrapper.h"

#include "../../../../src/scripting/operators/import.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class FASPR : public scripting::OperatorBase<FASPR>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		scripting::operators::Import::Result import_result;

		Result()
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			import_result.store(heterostorage);
		}
	};

	FASPR()
	{
	}

	std::string lib_path;
	std::string lib_name;
	std::string lib_file;

	void initialize(scripting::CommandInput& input)
	{
		lib_path=input.get_value_or_default<std::string>("lib-path", ".");
		lib_name=input.get_value_or_default<std::string>("lib-name", FASPRConfig().ROTLIB2010);
		lib_file=input.get_value_or_default<std::string>("lib-file", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("lib-path", CDOD::DATATYPE_STRING, "path to FASPR rotamers library directory", "."));
		doc.set_option_decription(CDOD("lib-name", CDOD::DATATYPE_STRING, "name of FASPR rotamers library", FASPRConfig().ROTLIB2010));
		doc.set_option_decription(CDOD("lib-file", CDOD::DATATYPE_STRING, "file path to FASPR rotamers library, rewrites other arguments", ""));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		FASPRConfig faspr_config;
		if(!lib_file.empty())
		{
			const std::size_t slashpos=lib_file.find_last_of("/\\");
			if(slashpos<lib_file.size())
			{
				faspr_config.PROGRAM_PATH=lib_file.substr(0, slashpos+1);
				faspr_config.ROTLIB2010=lib_file.substr(slashpos+1);
			}
			else
			{
				faspr_config.PROGRAM_PATH=".";
				faspr_config.ROTLIB2010=lib_file;
			}
		}
		else
		{
			faspr_config.PROGRAM_PATH=lib_path;
			faspr_config.ROTLIB2010=lib_name;
		}

		if(!faspr_config.is_library_file_available())
		{
			throw std::runtime_error(std::string("No rotamer library '")+faspr_config.ROTLIB2010+"' in '"+faspr_config.PROGRAM_PATH+"'");
		}

		scripting::VirtualFileStorage::TemporaryFile backbone_pdb_file;
		scripting::VirtualFileStorage::TemporaryFile rebuilt_pdb_file;

		{
			std::vector<scripting::Atom> atoms_to_export;
			atoms_to_export.reserve(data_manager.atoms().size());

			for(std::size_t i=0;i<data_manager.primary_structure_info().residues.size();i++)
			{
				const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[i];
				bool has_CA=false;
				bool has_C=false;
				bool has_N=false;
				bool has_O=false;
				for(std::size_t j=0;j<residue.atom_ids.size() && !(has_CA && has_C && has_N && has_O);j++)
				{
					const scripting::Atom& atom=data_manager.atoms()[residue.atom_ids[j]];
					has_CA=(has_CA || atom.crad.name=="CA");
					has_C=(has_C || atom.crad.name=="C");
					has_N=(has_N || atom.crad.name=="N");
					has_O=(has_O || atom.crad.name=="O");
				}
				if(has_CA && has_C && has_N && has_O)
				{
					for(std::size_t j=0;j<residue.atom_ids.size();j++)
					{
						const scripting::Atom& atom=data_manager.atoms()[residue.atom_ids[j]];
						if(atom.crad.name=="CA" || atom.crad.name=="C" || atom.crad.name=="N" || atom.crad.name=="O")
						{
							if(allowed_residue_names().count(atom.crad.resName)>0)
							{
								atoms_to_export.push_back(atom);
							}
						}
					}
				}
			}

			for(std::size_t i=0;i<atoms_to_export.size();i++)
			{
				scripting::Atom& atom=atoms_to_export[i];
				if(atom.crad.resName=="MSE")
				{
					atom.crad.resName="MET";
				}
				else if(atom.crad.resName=="SEC")
				{
					atom.crad.resName="CYS";
				}
			}

			if(atoms_to_export.empty())
			{
				throw std::runtime_error(std::string("No residues with full set of backbone atoms found."));
			}

			scripting::OutputSelector output_selector(backbone_pdb_file.filename());
			std::ostream& output=output_selector.stream();
			scripting::assert_io_stream(backbone_pdb_file.filename(), output);
			common::WritingAtomicBallsInPDBFormat::write_atomic_balls(atoms_to_export, "tf", false, output);
		}

		const FASPRWrapper::ResultBundle faspr_result=FASPRWrapper::run_faspr(faspr_config, backbone_pdb_file.filename(), rebuilt_pdb_file.filename());

		Result result;
		result.import_result=scripting::operators::Import().init(CMDIN().set("file", rebuilt_pdb_file.filename()).set("format", "pdb")).run(data_manager);

		return result;
	}

private:
	static const std::set<std::string>& allowed_residue_names()
	{
		static std::set<std::string> names;
		if(names.empty())
		{
			names.insert("SEC");
			names.insert("MSE");
			names.insert("CYS");
			names.insert("TRP");
			names.insert("MET");
			names.insert("HIS");
			names.insert("TYR");
			names.insert("GLN");
			names.insert("PHE");
			names.insert("ASN");
			names.insert("PRO");
			names.insert("ARG");
			names.insert("THR");
			names.insert("ASP");
			names.insert("ILE");
			names.insert("LYS");
			names.insert("SER");
			names.insert("GLU");
			names.insert("VAL");
			names.insert("GLY");
			names.insert("ALA");
			names.insert("LEU");
		}
		return names;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_FASPR_H_ */

