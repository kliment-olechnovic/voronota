#ifndef SCRIPTING_LOADING_OF_DATA_H_
#define SCRIPTING_LOADING_OF_DATA_H_

#include "../apollota/basic_operations_on_spheres.h"

#include "../common/construction_of_atomic_balls.h"

#include "basic_types.h"
#include "io_selectors.h"

namespace voronota
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
		std::string format_fallback;

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

	static void construct_result(const Configuration& config, const Parameters& input_params, Result& result)
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
				if(!params.format_fallback.empty())
				{
					params.format=params.format_fallback;
				}
				else
				{
					throw std::runtime_error(std::string("Failed to deduce format from file name '")+params.file+"'.");
				}
			}
		}

		if(params.format!="pdb" && params.format!="mmcif" && params.format!="plain" && params.format!="xyzr" && params.format!="xyz")
		{
			throw std::runtime_error(std::string("Unrecognized format '")+params.format+"', allowed formats are 'pdb', 'mmcif', 'plain', 'xyzr' or 'xyz'.");
		}

		InputSelector finput_selector(params.file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+params.file+"'.");
		}

		if(params.format=="pdb" || params.format=="mmcif")
		{
			common::ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile parameters_to_collect_atoms;
			parameters_to_collect_atoms.include_heteroatoms=(params.forced_include_heteroatoms ? params.include_heteroatoms : config.include_heteroatoms);
			parameters_to_collect_atoms.include_hydrogens=(params.forced_include_hydrogens ? params.include_hydrogens : config.include_hydrogens);
			parameters_to_collect_atoms.multimodel_chains=(params.forced_multimodel_chains ? params.multimodel_chains : config.multimodel_chains);
			parameters_to_collect_atoms.mmcif=(params.format=="mmcif");

			if(!common::ConstructionOfAtomicBalls::collect_atomic_balls_from_file(config.atom_radius_assigner, parameters_to_collect_atoms, finput, result.atoms))
			{
				handle_reading_failure(params.file, params.format);
			}
		}
		else if(params.format=="plain")
		{
			auxiliaries::IOUtilities(true, '\n', ' ', "_end_atoms").read_lines_to_set(finput, result.atoms);

			if(result.atoms.empty())
			{
				handle_reading_failure(params.file, params.format);
			}

			auxiliaries::IOUtilities(true, '\n', ' ', "_end_contacts").read_lines_to_set(finput, result.contacts);
		}
		else if(params.format=="xyzr")
		{
			const std::vector<apollota::SimpleSphere> spheres=auxiliaries::IOUtilities().read_lines_to_set< std::vector<apollota::SimpleSphere> >(finput);

			result.atoms.reserve(spheres.size());
			for(std::size_t i=0;i<spheres.size();i++)
			{
				const apollota::SimpleSphere& ball=spheres[i];
				Atom atom;
				atom.value.x=ball.x;
				atom.value.y=ball.y;
				atom.value.z=ball.z;
				atom.value.r=ball.r;
				atom.crad.resSeq=i;
				result.atoms.push_back(atom);
			}

			if(result.atoms.empty())
			{
				handle_reading_failure(params.file, params.format);
			}
		}
		else if(params.format=="xyz")
		{
			const std::vector<apollota::SimplePoint> points=auxiliaries::IOUtilities().read_lines_to_set< std::vector<apollota::SimplePoint> >(finput);

			result.atoms.reserve(points.size());

			for(std::size_t i=0;i<points.size();i++)
			{
				const apollota::SimplePoint& point=points[i];
				Atom atom;
				atom.value.x=point.x;
				atom.value.y=point.y;
				atom.value.z=point.z;
				atom.value.r=1.0;
				atom.crad.resSeq=i;
				result.atoms.push_back(atom);
			}

			if(result.atoms.empty())
			{
				handle_reading_failure(params.file, params.format);
			}
		}
	}

	static void construct_result(const Parameters& input_params, Result& result)
	{
		construct_result(Configuration::get_default_configuration(), input_params, result);
	}

private:
	static std::string get_format_from_atoms_file_name(const std::string& filename)
	{
		static const std::multimap<std::string, std::string> map_of_format_extensions=generate_map_of_format_extensions();
		for(std::multimap<std::string, std::string>::const_iterator it=map_of_format_extensions.begin();it!=map_of_format_extensions.end();++it)
		{
			const std::string& format=it->first;
			const std::string& extension=it->second;
			const std::size_t pos=filename.rfind(extension);
			if(pos<filename.size() && (pos+extension.size())==filename.size())
			{
				return format;
			}
		}
		return std::string();
	}

	static std::multimap<std::string, std::string> generate_map_of_format_extensions()
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
		map_of_format_extensions.insert(std::pair<std::string, std::string>("xyzr", ".xyzr"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("xyzr", ".XYZR"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("xyz", ".xyz"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("xyz", ".XYZ"));
		return map_of_format_extensions;
	}

	static void handle_reading_failure(const std::string& file, const std::string& format)
	{
		throw std::runtime_error(std::string("Failed to read atoms from file '")+file+"' in '"+format+"' format.");
	}
};

}

}

#endif /* SCRIPTING_LOADING_OF_DATA_H_ */
