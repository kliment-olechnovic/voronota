#ifndef DUKTAPER_OPERATORS_RUN_FASPR_H_
#define DUKTAPER_OPERATORS_RUN_FASPR_H_

#include "../../../../src/scripting/operators/import.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class RunFASPR : public scripting::OperatorBase<RunFASPR>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	RunFASPR()
	{
	}

	std::string faspr_binary;

	void initialize(scripting::CommandInput& input)
	{
		faspr_binary=input.get_value<std::string>("faspr-binary");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("faspr-binary", CDOD::DATATYPE_STRING, "path to FASPR binary executable"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(!CallShellUtilities::test_if_shell_command_available(faspr_binary))
		{
			throw std::runtime_error(std::string("FASPR application not available."));
		}

		CallShellUtilities::TemporaryDirectory tmpdir;

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

			const std::string backbone_file=(tmpdir.dir_path()+"/backbone.pdb");
			scripting::OutputSelector output_selector(backbone_file);
			std::ostream& output=output_selector.stream();
			scripting::assert_io_stream(backbone_file, output);
			common::WritingAtomicBallsInPDBFormat::write_atomic_balls(atoms_to_export, "tf", false, output);
		}

		{
			std::ostringstream command_output;
			command_output << faspr_binary << " -i '" << tmpdir.dir_path() << "/backbone.pdb'" << " -o '" << tmpdir.dir_path() << "/rebuilt.pdb'";
			operators::CallShell::Result faspr_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(faspr_result.exit_status!=0)
			{
				throw std::runtime_error(std::string("FASPR call failed."));
			}
		}

		if(!CallShellUtilities::test_if_file_not_empty(tmpdir.dir_path()+"/rebuilt.pdb"))
		{
			throw std::runtime_error(std::string("FASPR call did not produce output."));
		}

		scripting::operators::Import().init(CMDIN().set("file", tmpdir.dir_path()+"/rebuilt.pdb")).run(data_manager);

		Result result;

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_RUN_FASPR_H_ */
