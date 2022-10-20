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
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	FASPR()
	{
	}

	std::string lib_path;
	std::string lib_name;

	void initialize(scripting::CommandInput& input)
	{
		lib_path=input.get_value_or_default<std::string>("lib-path", ".");
		lib_name=input.get_value_or_default<std::string>("lib-name", FASPRConfig().ROTLIB2010);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("lib-path", CDOD::DATATYPE_STRING, "path to FASPR rotamers library directory", "."));
		doc.set_option_decription(CDOD("lib-name", CDOD::DATATYPE_STRING, "name of FASPR rotamers library", FASPRConfig().ROTLIB2010));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		FASPRConfig faspr_config;
		faspr_config.PROGRAM_PATH=lib_path;
		faspr_config.ROTLIB2010=lib_name;

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
							atoms_to_export.push_back(atom);
						}
					}
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

		scripting::operators::Import().init(CMDIN().set("file", rebuilt_pdb_file.filename()).set("format", "pdb")).run(data_manager);

		Result result;

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_FASPR_H_ */

