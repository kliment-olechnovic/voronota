#ifndef DUKTAPER_OPERATORS_PULCHRA_H_
#define DUKTAPER_OPERATORS_PULCHRA_H_

#include "../../dependencies/pulchra_bb/pulchra_wrapper.h"

#include "../../../../src/scripting/operators/import.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class PULCHRA : public scripting::OperatorBase<PULCHRA>
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

	PULCHRA()
	{
	}

	void initialize(scripting::CommandInput&)
	{
	}

	void document(scripting::CommandDocumentation&) const
	{
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		std::ostringstream calpha_string_output_stream;

		{
			std::vector<scripting::Atom> atoms_to_export;
			atoms_to_export.reserve(data_manager.atoms().size());

			for(std::size_t i=0;i<data_manager.primary_structure_info().residues.size();i++)
			{
				const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[i];
				bool has_CA=false;
				for(std::size_t j=0;j<residue.atom_ids.size() && !has_CA;j++)
				{
					const scripting::Atom& atom=data_manager.atoms()[residue.atom_ids[j]];
					has_CA=(has_CA || atom.crad.name=="CA");
				}
				if(has_CA)
				{
					for(std::size_t j=0;j<residue.atom_ids.size();j++)
					{
						const scripting::Atom& atom=data_manager.atoms()[residue.atom_ids[j]];
						if(atom.crad.name=="CA")
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
				throw std::runtime_error(std::string("No residues with CA atoms found."));
			}

			common::WritingAtomicBallsInPDBFormat::write_atomic_balls(atoms_to_export, "tf", false, calpha_string_output_stream);
		}

		scripting::VirtualFileStorage::TemporaryFile rebuilt_pdb_file;

		int pulchra_result_code=1;

		{
			scripting::OutputSelector output_selector(rebuilt_pdb_file.filename());
			std::ostream& output_stream=output_selector.stream();
			scripting::assert_io_stream(rebuilt_pdb_file.filename(), output_stream);

			pulchra_result_code=PULCHRAWrapper::run_pulchra(calpha_string_output_stream.str(), output_stream);
		}

		if(pulchra_result_code!=0)
		{
			std::ostringstream msg;
			msg << "Failed to run PULCHRA, return code = " << pulchra_result_code << ".";
			throw std::runtime_error(msg.str());
		}

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

#endif /* DUKTAPER_OPERATORS_PULCHRA_H_ */
