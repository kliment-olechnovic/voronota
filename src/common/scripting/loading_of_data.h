#ifndef COMMON_SCRIPTING_LOADING_OF_DATA_H_
#define COMMON_SCRIPTING_LOADING_OF_DATA_H_

#include "../construction_of_atomic_balls.h"

namespace common
{

namespace scripting
{

class LoadingOfData
{
public:
	class Configuration
	{
	public:
		bool include_heteroatoms;
		bool include_hydrogens;
		bool multimodel_chains;
		auxiliaries::AtomRadiusAssigner atom_radius_assigner;

		Configuration() :
			include_heteroatoms(false),
			include_hydrogens(false),
			multimodel_chains(false),
			atom_radius_assigner(generate_atom_radius_assigner(recommended_default_radius(), false, 0))
		{
		}

		static double recommended_default_radius()
		{
			return 1.7;
		}

		static auxiliaries::AtomRadiusAssigner generate_atom_radius_assigner(const double default_radius, const bool only_default_radius, std::istream* radii_file_stream)
		{
			auxiliaries::AtomRadiusAssigner atom_radius_assigner(default_radius);
			if(!only_default_radius)
			{
				if(radii_file_stream!=0 && radii_file_stream->good())
				{
					auxiliaries::IOUtilities().read_lines_to_container(
							*radii_file_stream,
							auxiliaries::AtomRadiusAssigner::add_descriptor_and_radius_from_stream_to_atom_radius_assigner,
							atom_radius_assigner);
				}
				else
				{
					atom_radius_assigner.add_radius_by_descriptor("*", "C*", 1.70);
					atom_radius_assigner.add_radius_by_descriptor("*", "N*", 1.55);
					atom_radius_assigner.add_radius_by_descriptor("*", "O*", 1.52);
					atom_radius_assigner.add_radius_by_descriptor("*", "S*", 1.80);
					atom_radius_assigner.add_radius_by_descriptor("*", "P*", 1.80);
					atom_radius_assigner.add_radius_by_descriptor("*", "H*", 1.20);
				}
			}
			return atom_radius_assigner;
		}

		static const Configuration& get_default_configuration()
		{
			return get_default_configuration_mutable();
		}

		static void setup_default_configuration(const Configuration& configuration)
		{
			get_default_configuration_mutable()=configuration;
		}

	private:
		static Configuration& get_default_configuration_mutable()
		{
			static Configuration configuration;
			return configuration;
		}
	};

	struct Parameters
	{
		bool forced_include_heteroatoms;
		bool forced_include_hydrogens;
		bool forced_multimodel_chains;
		bool include_heteroatoms;
		bool include_hydrogens;
		bool multimodel_chains;
		std::string file;
		std::string format;

		Parameters() :
			forced_include_heteroatoms(false),
			forced_include_hydrogens(false),
			forced_multimodel_chains(false),
			include_heteroatoms(false),
			include_hydrogens(false),
			multimodel_chains(false)
		{
		}
	};

	struct Result
	{
		std::vector<Atom> atoms;
		std::vector<Contact> contacts;
	};

	static void construct_result(const Parameters& input_params, Result& result)
	{
		result=Result();

		Parameters params=input_params;

		if(params.file.empty())
		{
			throw std::runtime_error(std::string("Empty input file name."));
		}

		if(params.format.empty())
		{
			params.format=get_format_from_atoms_file_name(params.file);
			if(params.format.empty())
			{
				throw std::runtime_error(std::string("Failed to deduce format from file name '")+params.file+"'.");
			}
		}

		if(params.format!="pdb" && params.format!="mmcif" && params.format!="plain")
		{
			throw std::runtime_error(std::string("Unrecognized format '")+params.format+"', allowed formats are 'pdb', 'mmcif' or 'plain'.");
		}

		std::ifstream finput(params.file.c_str(), std::ios::in);

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+params.file+"'.");
		}

		if(params.format=="pdb" || params.format=="mmcif")
		{
			const Configuration& config=Configuration::get_default_configuration();

			ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile parameters_to_collect_atoms;
			parameters_to_collect_atoms.include_heteroatoms=(params.forced_include_heteroatoms ? params.include_heteroatoms : config.include_heteroatoms);
			parameters_to_collect_atoms.include_hydrogens=(params.forced_include_hydrogens ? params.include_hydrogens : config.include_hydrogens);
			parameters_to_collect_atoms.multimodel_chains=(params.forced_multimodel_chains ? params.multimodel_chains : config.multimodel_chains);
			parameters_to_collect_atoms.mmcif=(params.format=="mmcif");

			if(!ConstructionOfAtomicBalls::collect_atomic_balls_from_file(config.atom_radius_assigner, parameters_to_collect_atoms, finput, result.atoms))
			{
				throw std::runtime_error(std::string("Failed to read atoms from file '")+params.file+"'.");
			}
		}
		else if(params.format=="plain")
		{
			auxiliaries::IOUtilities(true, '\n', ' ', "_end_atoms").read_lines_to_set(finput, result.atoms);

			if(result.atoms.empty())
			{
				throw std::runtime_error(std::string("Failed to read atoms from file '")+params.file+"'.");
			}

			auxiliaries::IOUtilities(true, '\n', ' ', "_end_contacts").read_lines_to_set(finput, result.contacts);
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

}

}

#endif /* COMMON_SCRIPTING_LOADING_OF_DATA_H_ */
