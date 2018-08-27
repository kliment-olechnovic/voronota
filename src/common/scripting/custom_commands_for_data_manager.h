#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_

#include "generic_command_for_data_manager.h"


namespace common
{

namespace scripting
{

class CustomCommandsForDataManager
{
public:
	class LoadAtoms : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile parameters_to_collect_atoms;
			parameters_to_collect_atoms.include_heteroatoms=cargs.input.get_flag("include-heteroatoms");
			parameters_to_collect_atoms.include_hydrogens=cargs.input.get_flag("include-hydrogens");
			parameters_to_collect_atoms.multimodel_chains=cargs.input.get_flag("as-assembly");
			const std::string atoms_file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			const std::string radii_file=cargs.input.get_value_or_default<std::string>("radii-file", "");
			const double default_radius=cargs.input.get_value_or_default<double>("default-radii", ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile::default_default_radius());
			const bool only_default_radius=cargs.input.get_flag("same-radius-for-all");
			std::string format=cargs.input.get_value_or_default<std::string>("format", "");

			cargs.input.assert_nothing_unusable();

			if(atoms_file.empty())
			{
				throw std::runtime_error(std::string("Empty input atoms file name."));
			}

			if(format.empty())
			{
				format=get_format_from_atoms_file_name(atoms_file);
				if(format.empty())
				{
					throw std::runtime_error(std::string("Could not deduce format from file name '")+atoms_file+"'.");
				}
			}

			if(format!="pdb" && format!="mmcif" && format!="plain")
			{
				throw std::runtime_error(std::string("Unrecognized format '")+format+"', allowed formats are 'pdb', 'mmcif' or 'plain'.");
			}

			if(!radii_file.empty() || only_default_radius || default_radius!=ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile::default_default_radius())
			{
				parameters_to_collect_atoms.set_atom_radius_assigner(default_radius, only_default_radius, radii_file);
			}

			std::vector<Atom> atoms;
			bool success=false;

			if(format=="pdb" || format=="mmcif")
			{
				if(format=="mmcif")
				{
					parameters_to_collect_atoms.mmcif=true;
				}
				success=ConstructionOfAtomicBalls::collect_atomic_balls_from_file(parameters_to_collect_atoms, atoms_file, atoms);
			}
			else if(format=="plain")
			{
				auxiliaries::IOUtilities(true, '\n', ' ', "_end_atoms").read_file_lines_to_set(atoms_file, atoms);
				if(!atoms.empty())
				{
					if(!radii_file.empty() || only_default_radius)
					{
						for(std::size_t i=0;i<atoms.size();i++)
						{
							Atom& atom=atoms[i];
							atom.value.r=parameters_to_collect_atoms.atom_radius_assigner.get_atom_radius(atom.crad.resName, atom.crad.name);
						}
					}
					success=true;
				}
			}

			if(success)
			{
				if(atoms.size()<4)
				{
					throw std::runtime_error(std::string("Less than 4 atoms read."));
				}
				else
				{
					cargs.data_manager.reset_atoms_by_swapping(atoms);
					cargs.changed_atoms=true;
					cargs.summary_of_atoms=SummaryOfAtoms::collect_summary(cargs.data_manager.atoms());
					cargs.output_for_log << "Read " << cargs.summary_of_atoms.number_total << " atoms from file '" << atoms_file << "'";
				}
			}
			else
			{
				throw std::runtime_error(std::string("Failed to read atoms from file '")+atoms_file+"' in '"+format+"' format.");
			}
		}

	private:
		static std::string get_format_from_atoms_file_name(const std::string& filename)
		{
			std::multimap<std::string, std::string> map_of_format_extensions;
			map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".pdb"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".PDB"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".ent"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".ENT"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".cif"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".CIF"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".mmcif"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".mmCIF"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".MMCIF"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("plain", ".atoms"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("plain", ".pa"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("plain", ".pac"));
			for(std::multimap<std::string, std::string>::const_iterator it=map_of_format_extensions.begin();it!=map_of_format_extensions.end();++it)
			{
				const std::string& format=it->first;
				const std::string& extension=it->second;
				const std::size_t pos=filename.find(extension);
				if(pos<filename.size() && (pos+extension.size())==filename.size())
				{
					return format;
				}
			}
			return std::string();
		}
	};
};

}

}

#endif /* COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_ */
