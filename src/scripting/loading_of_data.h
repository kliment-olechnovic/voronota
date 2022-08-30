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
		bool only_default_radius;
		double default_radius;
		auxiliaries::AtomRadiusAssigner atom_radius_assigner;

		Configuration() :
			include_heteroatoms(false),
			include_hydrogens(false),
			multimodel_chains(false),
			only_default_radius(false),
			default_radius(recommended_default_radius()),
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
		bool forced_same_radius_for_all;
		bool include_heteroatoms;
		bool include_hydrogens;
		bool multimodel_chains;
		double same_radius_for_all;
		std::string file;
		std::string format;
		std::string format_fallback;

		Parameters() :
			forced_include_heteroatoms(false),
			forced_include_hydrogens(false),
			forced_multimodel_chains(false),
			forced_same_radius_for_all(false),
			include_heteroatoms(false),
			include_hydrogens(false),
			multimodel_chains(false),
			same_radius_for_all(1.0)
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

		if(params.format!="pdb" && params.format!="mmcif" && params.format!="plain" && params.format!="xyzr" && params.format!="xyz" && params.format!="mdl")
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
		else if(params.format=="xyz" || params.format=="xyzr")
		{
			const auxiliaries::AtomsIO::XYZRReader::Data xyzr_data=auxiliaries::AtomsIO::XYZRReader::read_data_from_file_stream(finput);

			result.atoms.reserve(xyzr_data.xyzr_records.size());
			for(std::size_t i=0;i<xyzr_data.xyzr_records.size();i++)
			{
				const auxiliaries::AtomsIO::XYZRReader::XYZRRecord& record=xyzr_data.xyzr_records[i];
				if(record.atom_type!="H" || (params.forced_include_hydrogens ? params.include_hydrogens : config.include_hydrogens))
				{
					Atom atom;
					atom.value.x=record.x;
					atom.value.y=record.y;
					atom.value.z=record.z;
					atom.value.r=(xyzr_data.with_r ? record.r : config.atom_radius_assigner.get_atom_radius("", record.atom_type));
					atom.crad.resSeq=i;
					atom.crad.name=record.atom_type;
					atom.value.props.tags.insert(std::string("el=")+record.atom_type);
					result.atoms.push_back(atom);
				}
			}

			if(result.atoms.empty())
			{
				handle_reading_failure(params.file, params.format);
			}
		}
		else if(params.format=="mdl")
		{
			const auxiliaries::AtomsIO::MDLReader::Data mdl_data=auxiliaries::AtomsIO::MDLReader::read_data_from_file_stream(finput);

			result.atoms.reserve(mdl_data.atom_records.size());
			for(std::size_t i=0;i<mdl_data.atom_records.size();i++)
			{
				const auxiliaries::AtomsIO::MDLReader::AtomRecord& record=mdl_data.atom_records[i];
				if(record.atom_type!="H" || (params.forced_include_hydrogens ? params.include_hydrogens : config.include_hydrogens))
				{
					Atom atom;
					atom.value.x=record.x;
					atom.value.y=record.y;
					atom.value.z=record.z;
					atom.value.r=config.atom_radius_assigner.get_atom_radius("", record.atom_type);
					atom.crad.chainID="";
					atom.crad.resSeq=1;
					{
						for(std::size_t j=0;j<mdl_data.title_line.size() && atom.crad.resName.size()<4;j++)
						{
							const char c=mdl_data.title_line[j];
							if((c>='A' && c<='Z') || (c>='a' && c<='z'))
							{
								atom.crad.resName.push_back(c);
							}
						}
						if(atom.crad.resName.empty())
						{
							atom.crad.resName="LIG";
						}
					}
					atom.crad.serial=static_cast<int>(i);
					{
						std::ostringstream name;
						name << record.atom_type << i;
						atom.crad.name=name.str();
					}
					atom.value.props.tags.insert(std::string("el=")+record.atom_type);
					atom.value.props.tags.insert("het");
					result.atoms.push_back(atom);
				}
			}

			if(result.atoms.empty())
			{
				handle_reading_failure(params.file, params.format);
			}
		}

		if(params.forced_same_radius_for_all)
		{
			if(!result.contacts.empty())
			{
				throw std::runtime_error(std::string("Not allowed to force radius when precomputed contacts are loaded."));
			}
			else
			{
				for(std::size_t i=0;i<result.atoms.size();i++)
				{
					result.atoms[i].value.r=params.same_radius_for_all;
				}
			}
		}
	}

	static void construct_result(const Parameters& input_params, Result& result)
	{
		construct_result(Configuration::get_default_configuration(), input_params, result);
	}

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

private:
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
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mdl", ".mdl"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mdl", ".MDL"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mdl", ".mol"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mdl", ".MOL"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mdl", ".sdf"));
		map_of_format_extensions.insert(std::pair<std::string, std::string>("mdl", ".SDF"));
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
