#include <filesystem>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "cadscore.h"

namespace
{

void print_help(std::ostream& output)
{
	output << "Voronota-LT-CAD-score version " << voronotalt::version() << "\n";
	output << R"(
'voronota-lt-cadscore' calculates CAD-score (Contact Area Difference score).

Options:
    --targets | -t                                   string     input file or directory paths for target (reference) structure files
    --models | -m                                    string     input file or directory paths for model structure files
    --recursive-directory-search                                flag to search directories recursively
    --include-heteroatoms                                       flag to include heteroatoms when reading input in PDB or mmCIF format
    --read-inputs-as-assemblies                                 flag to join multiple models into an assembly when reading a file in PDB or mmCIF format
    --radii-config-file                              string     input file path for reading atom radii assignment rules
    --processors                                     number     maximum number of OpenMP threads to use, default is 2 if OpenMP is enabled, 1 if disabled
    --probe                                          number     rolling probe radius, default is 1.4
    --restrict-input-atoms                           string     selection expression to restrict input balls
    --subselect-contacts                             string     selection expression to restrict contact area descriptors to score
    --subselect-atoms                                string     selection expression to restrict atom SAS and site area descriptors to score
    --score-atom-atom-contacts                                  flag to score contacts on atom level
    --score-residue-residue-contacts                            flag to score contacts on residue level
    --score-chain-chain-contacts                                flag to score contacts on chain level
    --score-atom-sas-areas                                      flag to score SAS areas on atom level
    --score-residue-sas-areas                                   flag to score SAS areas on residue level
    --score-chain-sas-areas                                     flag to score SAS areas on chain level
    --score-atom-sites                                          flag to score sites on atom level
    --score-residue-sites                                       flag to score sites on residue level
    --score-chain-sites                                         flag to score sites on chain level
    --score-everything                                          flag to score everything (contacts, sites, SAS areas) on all levels of detail
    --consider-residue-names                                    flag to include residue names in residue and atom identifiers, making mapping more strict
    --remap-chains                                              flag to automatically rename chains in models to maximize residue-residue contacts score
    --print-paths-in-output                                     flag to print file paths instead of file base names in output
    --output-level-of-detail                         number     integer level of detail for output, default is 0 (lowest level, to only print CAD-score) 
    --output-global-scores                           string     path to output table of global scores, default is '_stdout' to print to standard output 
    --output-local-scores                            string     path to output directory for files with tables of local scores
    --help | -h                                                 flag to print help info to stderr and exit

Standard output stream:
    Global scores info

Standard error output stream:
    Log (a name-value pair line), error messages

Usage examples:

    voronota-lt-cadscore -t ./target.pdb -m ./model1.pdb ./model2.pdb --score-residue-residue-contacts

    voronota-lt-cadscore -t ./target.pdb -m ./model1.pdb --restrict-input-atoms '[-chain A,B]' --subselect-contacts '[-inter-chain]' --score-everything

)";
}

class OpenMPUtilities
{
public:
	static bool openmp_enabled()
	{
#ifdef _OPENMP
		return true;
#else
		return false;
#endif
	}

	static unsigned int openmp_setup(const unsigned int max_number_of_processors)
	{
#ifdef _OPENMP
		omp_set_num_threads(max_number_of_processors);
		omp_set_max_active_levels(1);
		return max_number_of_processors;
#else
		return 1;
#endif
	}
};

class ApplicationParameters
{
public:
	double probe;
	unsigned int max_number_of_processors;
	bool recursive_directory_search;
	bool include_heteroatoms;
	bool read_inputs_as_assemblies;
	bool consider_residue_names;
	bool remap_chains;
	int max_chains_to_fully_permute;
	int output_level_of_detail;
	bool score_atom_atom_contacts;
	bool score_residue_residue_contacts;
	bool score_chain_chain_contacts;
	bool score_atom_sas_areas;
	bool score_residue_sas_areas;
	bool score_chain_sas_areas;
	bool score_atom_sites;
	bool score_residue_sites;
	bool score_chain_sites;
	bool print_paths_in_output;
	bool read_successfuly;
	std::vector<std::string> target_input_files;
	std::vector<std::string> model_input_files;
	std::string radii_config_file;
	std::string restrict_input_atoms;
	std::string restrict_contact_descriptors;
	std::string restrict_atom_descriptors;
	voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_input_balls;
	voronotalt::FilteringBySphereLabels::ExpressionForPair filtering_expression_for_restricting_contact_descriptors;
	voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_atom_descriptors;
	std::string output_global_scores;
	std::string output_local_scores;
	std::ostringstream error_log_for_options_parsing;

	ApplicationParameters() :
		probe(1.4),
		max_number_of_processors(OpenMPUtilities::openmp_enabled() ? 2 : 1),
		recursive_directory_search(false),
		include_heteroatoms(false),
		read_inputs_as_assemblies(false),
		consider_residue_names(false),
		remap_chains(false),
		max_chains_to_fully_permute(5),
		output_level_of_detail(0),
		score_atom_atom_contacts(false),
		score_residue_residue_contacts(false),
		score_chain_chain_contacts(false),
		score_atom_sas_areas(false),
		score_residue_sas_areas(false),
		score_chain_sas_areas(false),
		score_atom_sites(false),
		score_residue_sites(false),
		score_chain_sites(false),
		print_paths_in_output(false),
		read_successfuly(false),
		output_global_scores("_stdout")
	{
	}

	bool read_from_command_line_args(const int argc, const char** argv)
	{
		read_successfuly=false;

		{
			const std::vector<voronotalt::CLOParser::Option> cloptions=voronotalt::CLOParser::read_options(argc, argv);

			for(std::size_t i=0;i<cloptions.size();i++)
			{
				const voronotalt::CLOParser::Option& opt=cloptions[i];
				if((opt.name=="help" || opt.name=="h") && opt.is_flag_and_true())
				{
					print_help(error_log_for_options_parsing);
					return false;
				}
			}

			for(std::size_t i=0;i<cloptions.size();i++)
			{
				const voronotalt::CLOParser::Option& opt=cloptions[i];
				if(opt.name=="probe" && opt.args_doubles.size()==1)
				{
					probe=static_cast<voronotalt::Float>(opt.args_doubles.front());
					if(!(probe>=0.0 && probe<=30.0))
					{
						error_log_for_options_parsing << "Error: invalid command line argument for the rolling probe radius, must be a value from 0.0 to 30.0.\n";
					}
				}
				else if(opt.name=="processors" && opt.args_ints.size()==1)
				{
					max_number_of_processors=static_cast<unsigned int>(opt.args_ints.front());
					if(OpenMPUtilities::openmp_enabled())
					{
						if(!(max_number_of_processors>=1 && max_number_of_processors<=1024))
						{
							error_log_for_options_parsing << "Error: invalid command line argument for the maximum number of processors, must be an integer from 1 to 1024.\n";
						}
					}
					else
					{
						if(max_number_of_processors!=1)
						{
							error_log_for_options_parsing << "Error: OpenMP is not enabled, therefore specifying the maximum number of processors other than 1 is not allowed.\n";
						}
					}
				}
				else if((opt.name=="targets" || opt.name=="t") && !opt.args_strings.empty())
				{
					target_input_files=opt.args_strings;
				}
				else if((opt.name=="model" || opt.name=="m") && !opt.args_strings.empty())
				{
					model_input_files=opt.args_strings;
				}
				else if(opt.name=="recursive-directory-search" && opt.is_flag())
				{
					recursive_directory_search=opt.is_flag_and_true();
				}
				else if(opt.name=="include-heteroatoms" && opt.is_flag())
				{
					include_heteroatoms=opt.is_flag_and_true();
				}
				else if(opt.name=="read-inputs-as-assemblies" && opt.is_flag())
				{
					read_inputs_as_assemblies=opt.is_flag_and_true();
				}
				else if(opt.name=="radii-config-file" && opt.args_strings.size()==1)
				{
					radii_config_file=opt.args_strings.front();
				}
				else if(opt.name=="restrict-input-atoms" && opt.args_strings.size()==1)
				{
					restrict_input_atoms=opt.args_strings.front();
				}
				else if(opt.name=="subselect-contacts" && opt.args_strings.size()==1)
				{
					restrict_contact_descriptors=opt.args_strings.front();
				}
				else if(opt.name=="subselect-atoms" && opt.args_strings.size()==1)
				{
					restrict_atom_descriptors=opt.args_strings.front();
				}
				else if(opt.name=="score-atom-atom-contacts" && opt.is_flag())
				{
					score_atom_atom_contacts=opt.is_flag_and_true();
				}
				else if(opt.name=="score-residue-residue-contacts" && opt.is_flag())
				{
					score_residue_residue_contacts=opt.is_flag_and_true();
				}
				else if(opt.name=="score-chain-chain-contacts" && opt.is_flag())
				{
					score_chain_chain_contacts=opt.is_flag_and_true();
				}
				else if(opt.name=="score-atom-sas-areas" && opt.is_flag())
				{
					score_atom_sas_areas=opt.is_flag_and_true();
				}
				else if(opt.name=="score-residue-sas-areas" && opt.is_flag())
				{
					score_residue_sas_areas=opt.is_flag_and_true();
				}
				else if(opt.name=="score-chain-sas-areas" && opt.is_flag())
				{
					score_chain_sas_areas=opt.is_flag_and_true();
				}
				else if(opt.name=="score-atom-sites" && opt.is_flag())
				{
					score_atom_sites=opt.is_flag_and_true();
				}
				else if(opt.name=="score-residue-sites" && opt.is_flag())
				{
					score_residue_sites=opt.is_flag_and_true();
				}
				else if(opt.name=="score-chain-sites" && opt.is_flag())
				{
					score_chain_sites=opt.is_flag_and_true();
				}
				else if(opt.name=="score-everything" && opt.is_flag())
				{
					if(opt.is_flag_and_true())
					{
						score_atom_atom_contacts=true;
						score_residue_residue_contacts=true;
						score_chain_chain_contacts=true;
						score_atom_sas_areas=true;
						score_residue_sas_areas=true;
						score_chain_sas_areas=true;
						score_atom_sites=true;
						score_residue_sites=true;
						score_chain_sites=true;
					}
				}
				else if(opt.name=="consider-residue-names" && opt.is_flag())
				{
					consider_residue_names=opt.is_flag_and_true();
				}
				else if(opt.name=="remap-chains" && opt.is_flag())
				{
					remap_chains=opt.is_flag_and_true();
				}
				else if(opt.name=="max-chains-to-fully-permute" && opt.args_ints.size()==1)
				{
					max_chains_to_fully_permute=static_cast<int>(opt.args_ints.front());
					if(max_chains_to_fully_permute>7)
					{
						error_log_for_options_parsing << "Error: invalid max number of chains to fully permute, must be not greater than 7.\n";
					}
				}
				else if(opt.name=="output-level-of-detail" && opt.args_ints.size()==1)
				{
					output_level_of_detail=static_cast<int>(opt.args_ints.front());
				}
				else if(opt.name=="print-paths-in-output" && opt.is_flag())
				{
					print_paths_in_output=opt.is_flag_and_true();
				}
				else if(opt.name=="output-global-scores" && opt.args_strings.size()==1)
				{
					output_global_scores=opt.args_strings.front();
				}
				else if(opt.name=="output-local-scores" && opt.args_strings.size()==1)
				{
					output_local_scores=opt.args_strings.front();
				}
				else if(opt.name.empty())
				{
					error_log_for_options_parsing << "Error: misplaced unnamed command line arguments detected.\n";
				}
				else
				{
					error_log_for_options_parsing << "Error: invalid command line option '" << opt.name << "'.\n";
				}
			}
		}

		if(!(score_atom_atom_contacts || score_residue_residue_contacts || score_chain_chain_contacts || score_atom_sas_areas || score_residue_sas_areas || score_chain_sas_areas || score_atom_sites || score_residue_sites || score_chain_sites))
		{
			error_log_for_options_parsing << "Error: no scoring mode specified.\n";
		}

		if(target_input_files.empty())
		{
			error_log_for_options_parsing << "Error: no input target files provided.\n";
		}

		if(model_input_files.empty())
		{
			error_log_for_options_parsing << "Error: no input model files provided.\n";
		}

		if(output_global_scores.empty() && output_local_scores.empty())
		{
			error_log_for_options_parsing << "Error: no outputs specified.\n";
		}

		if(!output_local_scores.empty() && model_input_files.size()!=1 && target_input_files.empty()!=1)
		{
			error_log_for_options_parsing << "Error: local scores output is only supported for one target and one model at a time.\n";
		}

		if(!restrict_input_atoms.empty())
		{
			filtering_expression_for_restricting_input_balls=voronotalt::FilteringBySphereLabels::ExpressionForSingle(restrict_input_atoms);
			if(!filtering_expression_for_restricting_input_balls.valid())
			{
				error_log_for_options_parsing << "Error: invalid input balls filtering expression.\n";
			}
		}

		if(!restrict_contact_descriptors.empty())
		{
			filtering_expression_for_restricting_contact_descriptors=voronotalt::FilteringBySphereLabels::ExpressionForPair(restrict_contact_descriptors);
			if(!filtering_expression_for_restricting_contact_descriptors.valid())
			{
				error_log_for_options_parsing << "Error: invalid contact descriptors restriction filtering expression.\n";
			}
		}

		if(!restrict_atom_descriptors.empty())
		{
			filtering_expression_for_restricting_atom_descriptors=voronotalt::FilteringBySphereLabels::ExpressionForSingle(restrict_atom_descriptors);
			if(!filtering_expression_for_restricting_atom_descriptors.valid())
			{
				error_log_for_options_parsing << "Error: invalid atom descriptors restriction filtering expression.\n";
			}
		}

		read_successfuly=error_log_for_options_parsing.str().empty();

		return read_successfuly;
	}
};

class FileSystemUtilities
{
public:
	struct FileInfo
	{
		std::string path;
		std::string name;

		explicit FileInfo(const std::filesystem::path& p) : path(p.lexically_normal().string()), name(p.filename().string())
		{
		}

		bool operator==(const FileInfo& v) const
		{
			return (path==v.path);
		}

		bool operator!=(const FileInfo& v) const
		{
			return (path!=v.path);
		}

		bool operator<(const FileInfo& v) const
		{
			return path<v.path;
		}
	};

	static std::set<FileInfo> collect_file_descriptors(const std::vector<std::string>& input_paths, bool recursive)
	{
		std::set<FileInfo> file_descriptors;
		for(const std::string& input_path : input_paths)
		{
			try
			{
				std::filesystem::path p(input_path);
				if(std::filesystem::is_regular_file(p))
				{
					file_descriptors.emplace(FileInfo(p));
				}
				else if(std::filesystem::is_directory(p))
				{
					const std::filesystem::directory_options options=std::filesystem::directory_options::skip_permission_denied;
					if(recursive)
					{
						for(const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(p, options))
						{
							if(entry.is_regular_file())
							{
								file_descriptors.emplace(FileInfo(entry.path()));
							}
						}
					}
					else
					{
						for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(p, options))
						{
							if(entry.is_regular_file())
							{
								file_descriptors.emplace(FileInfo(entry.path()));
							}
						}
					}
				}
			}
			catch(const std::filesystem::filesystem_error& e)
			{
				std::cerr << "Skipping path '" << input_path << "' due to filesystem error: " << e.what() << "\n";
			}
		}
		return file_descriptors;
	}

	static bool create_directory(const std::string& directory_path)
	{
		try
		{
			std::filesystem::create_directories(directory_path);
		}
		catch(const std::filesystem::filesystem_error& e)
		{
			std::cerr << "Failed to create directory '" << directory_path << "' due to filesystem error: " << e.what() << "\n";
			return false;
		}
		return true;
	}

	static bool create_parent_directory(const std::string& full_file_path)
	{
		std::filesystem::path file_path=full_file_path;
		std::filesystem::path dir_path=file_path.parent_path();
		if(!dir_path.empty())
		{
			try
			{
				std::filesystem::create_directories(dir_path);
			}
			catch(const std::filesystem::filesystem_error& e)
			{
				std::cerr << "Failed to create parent directory '" << dir_path << "' due to filesystem error: " << e.what() << "\n";
				return false;
			}
		}
		return true;
	}

	static bool write_file(const std::string& full_file_path, const std::string& output_string)
	{
		if(!create_parent_directory(full_file_path))
		{
			std::cerr << "Error: failed to create parent directory for file '" << full_file_path << "'.\n";
			return false;
		}
		std::ofstream output_file_stream(full_file_path.c_str(), std::ios::out);
		if(!output_file_stream.good())
		{
			std::cerr << "Error: failed to open file '" << full_file_path << "' to output table of global scores.\n";
			return false;
		}
		output_file_stream.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
		return true;
	}
};

bool run(const ApplicationParameters& app_params)
{
	cadscore::IDResidue::consider_residue_names()=app_params.consider_residue_names;

	cadscore::ScorableData::ConstructionParameters scorable_data_construction_parameters;
	scorable_data_construction_parameters.probe=app_params.probe;
	scorable_data_construction_parameters.record_atom_balls=false;
	scorable_data_construction_parameters.record_atom_atom_contact_summaries=app_params.score_atom_atom_contacts;
	scorable_data_construction_parameters.record_residue_residue_contact_summaries=app_params.score_residue_residue_contacts || app_params.remap_chains;
	scorable_data_construction_parameters.record_chain_chain_contact_summaries=app_params.score_chain_chain_contacts;
	scorable_data_construction_parameters.record_atom_cell_summaries=app_params.score_atom_sas_areas;
	scorable_data_construction_parameters.record_residue_cell_summaries=app_params.score_residue_sas_areas;
	scorable_data_construction_parameters.record_chain_cell_summaries=app_params.score_chain_sas_areas;
	scorable_data_construction_parameters.record_atom_site_summaries=app_params.score_atom_sites;
	scorable_data_construction_parameters.record_residue_site_summaries=app_params.score_residue_sites;
	scorable_data_construction_parameters.record_chain_site_summaries=app_params.score_chain_sites;
	scorable_data_construction_parameters.filtering_expression_for_restricting_input_balls=app_params.filtering_expression_for_restricting_input_balls;
	scorable_data_construction_parameters.filtering_expression_for_restricting_contact_descriptors=app_params.filtering_expression_for_restricting_contact_descriptors;
	scorable_data_construction_parameters.filtering_expression_for_restricting_atom_descriptors=app_params.filtering_expression_for_restricting_atom_descriptors;

	if(!scorable_data_construction_parameters.valid())
	{
		std::cerr << "Error: invalid data preparation parameters.\n";
		return false;
	}

	const std::set<FileSystemUtilities::FileInfo> set_of_target_file_descriptors=FileSystemUtilities::collect_file_descriptors(app_params.target_input_files, app_params.recursive_directory_search);
	if(set_of_target_file_descriptors.empty())
	{
		std::cerr << "Error: no target input files found.\n";
		return false;
	}

	const std::set<FileSystemUtilities::FileInfo> set_of_model_file_descriptors=FileSystemUtilities::collect_file_descriptors(app_params.model_input_files, app_params.recursive_directory_search);
	if(set_of_model_file_descriptors.empty())
	{
		std::cerr << "Error: no model input files found.\n";
		return false;
	}

	std::vector<FileSystemUtilities::FileInfo> list_of_unique_file_descriptors;
	{
		std::set<FileSystemUtilities::FileInfo> set_of_unique_file_descriptors=set_of_model_file_descriptors;
		set_of_unique_file_descriptors.insert(set_of_target_file_descriptors.begin(), set_of_target_file_descriptors.end());
		list_of_unique_file_descriptors.reserve(set_of_unique_file_descriptors.size());
		list_of_unique_file_descriptors.insert(list_of_unique_file_descriptors.end(), set_of_unique_file_descriptors.begin(), set_of_unique_file_descriptors.end());
	}

	if(!app_params.output_local_scores.empty() && list_of_unique_file_descriptors.size()!=2)
	{
		std::cerr << "Error: multiple input files found, but local scores output is only supported for one target and one model at a time.\n";
		return false;
	}

	std::vector<std::string> list_of_unique_file_display_names(list_of_unique_file_descriptors.size());
	{
		if(app_params.print_paths_in_output)
		{
			for(std::size_t i=0;i<list_of_unique_file_descriptors.size();i++)
			{
				list_of_unique_file_display_names[i]=list_of_unique_file_descriptors[i].path;
			}
		}
		else
		{
			std::map<std::string, int> map_of_basename_counts;
			for(std::size_t i=0;i<list_of_unique_file_descriptors.size();i++)
			{
				int& counter=map_of_basename_counts[list_of_unique_file_descriptors[i].name];
				counter++;
				list_of_unique_file_display_names[i]=list_of_unique_file_descriptors[i].name;
				if(counter>1)
				{
					list_of_unique_file_display_names[i]+=std::string("_v")+std::to_string(counter);
				}
			}
		}
	}

	std::vector<std::size_t> target_sd_indices;
	target_sd_indices.reserve(set_of_target_file_descriptors.size());
	std::vector<std::size_t> model_sd_indices;
	model_sd_indices.reserve(set_of_model_file_descriptors.size());

	std::vector<cadscore::ScorableData> list_of_unique_scorable_data(list_of_unique_file_descriptors.size());
	std::vector<std::string> list_of_error_messages_for_unique_scorable_data(list_of_unique_file_descriptors.size());

	const bool parallelize_on_root_level=(list_of_unique_file_descriptors.size()*2>app_params.max_number_of_processors);

#ifdef _OPENMP
#pragma omp parallel for if(parallelize_on_root_level)
#endif
	for(std::size_t i=0;i<list_of_unique_file_descriptors.size();i++)
	{
		const FileSystemUtilities::FileInfo& fi=list_of_unique_file_descriptors[i];
		cadscore::ScorableData& sd=list_of_unique_scorable_data[i];
		std::string& error_message=list_of_error_messages_for_unique_scorable_data[i];
		std::ostringstream local_error_stream;
		sd.construct(scorable_data_construction_parameters, cadscore::MolecularFileInput(fi.path, app_params.include_heteroatoms, app_params.read_inputs_as_assemblies), local_error_stream);
		error_message=local_error_stream.str();
		if(!sd.valid() && error_message.empty())
		{
			error_message="unrecognized error";
		}
	}

	for(std::size_t i=0;i<list_of_unique_file_descriptors.size();i++)
	{
		const FileSystemUtilities::FileInfo& fi=list_of_unique_file_descriptors[i];
		const std::string& error_message=list_of_error_messages_for_unique_scorable_data[i];
		if(error_message.empty())
		{
			if(set_of_target_file_descriptors.count(fi)>0)
			{
				target_sd_indices.push_back(i);
			}
			if(set_of_model_file_descriptors.count(fi)>0)
			{
				model_sd_indices.push_back(i);
			}
		}
		else
		{
			std::cerr << "Error (non-terminating): failed to process input file '" << fi.path<< "' due to errors:\n";
			std::cerr << error_message << "\n";
		}
	}

	if(target_sd_indices.empty())
	{
		std::cerr << "Error: no target inputs processed successfully.\n";
		return false;
	}

	if(model_sd_indices.empty())
	{
		std::cerr << "Error: no model inputs processed successfully.\n";
		return false;
	}

	std::vector< std::pair<std::size_t, std::size_t> > list_of_pairs_of_target_model_indices;
	list_of_pairs_of_target_model_indices.reserve(target_sd_indices.size()*model_sd_indices.size());
	for(const std::size_t ti : target_sd_indices)
	{
		for(const std::size_t mi : model_sd_indices)
		{
			if(ti!=mi)
			{
				list_of_pairs_of_target_model_indices.emplace_back(std::pair<std::size_t, std::size_t>(ti, mi));
			}
		}
	}

	if(list_of_pairs_of_target_model_indices.empty())
	{
		std::cerr << "Error: no target-model pairs collected with target and model coming not from the same input file.\n";
		return false;
	}

	std::vector<std::string> output_score_names;
	{
		if(app_params.score_residue_residue_contacts){output_score_names.push_back("residue_residue");}
		if(app_params.score_atom_atom_contacts){output_score_names.push_back("atom_atom");}
		if(app_params.score_chain_chain_contacts){output_score_names.push_back("chain_chain");}
		if(app_params.score_residue_sas_areas){output_score_names.push_back("residue_sas");}
		if(app_params.score_atom_sas_areas){output_score_names.push_back("atom_sas");}
		if(app_params.score_chain_sas_areas){output_score_names.push_back("chain_sas");}
		if(app_params.score_residue_sites){output_score_names.push_back("residue_sites");}
		if(app_params.score_atom_sites){output_score_names.push_back("atom_sites");}
		if(app_params.score_chain_sites){output_score_names.push_back("chain_sites");}
	}

	std::vector< std::vector<cadscore::CADDescriptor> > list_of_output_cad_descriptors(list_of_pairs_of_target_model_indices.size(), std::vector<cadscore::CADDescriptor>(output_score_names.size()));
	std::vector<std::string> list_of_output_error_messages(list_of_pairs_of_target_model_indices.size());

	std::vector<std::string> list_of_chain_remapping_summaries(app_params.remap_chains ? list_of_pairs_of_target_model_indices.size() : static_cast<std::size_t>(0));

	bool success_writing_local_scores=true;

	{
		cadscore::ScoringResult::ConstructionParameters scoring_result_construction_parameters;
		scoring_result_construction_parameters.remap_chains=app_params.remap_chains;
		scoring_result_construction_parameters.max_chains_to_fully_permute=app_params.max_chains_to_fully_permute;
		scoring_result_construction_parameters.record_local_scores=!app_params.output_local_scores.empty();

#ifdef _OPENMP
#pragma omp parallel for
#endif
		for(std::size_t i=0;i<list_of_pairs_of_target_model_indices.size();i++)
		{
			const cadscore::ScorableData& target_sd=list_of_unique_scorable_data[list_of_pairs_of_target_model_indices[i].first];
			const cadscore::ScorableData& model_sd=list_of_unique_scorable_data[list_of_pairs_of_target_model_indices[i].second];
			std::vector<cadscore::CADDescriptor>& output_cad_descriptors=list_of_output_cad_descriptors[i];
			std::string& output_error_message=list_of_output_error_messages[i];
			cadscore::ScoringResult sr;
			std::ostringstream local_err_stream;
			sr.construct(scoring_result_construction_parameters, target_sd, model_sd, local_err_stream);
			output_error_message=local_err_stream.str();
			if(sr.valid() && output_error_message.empty())
			{
				{
					std::size_t j=0;
					if(app_params.score_residue_residue_contacts){output_cad_descriptors[j++]=sr.residue_residue_contact_cad_descriptor_global;}
					if(app_params.score_atom_atom_contacts){output_cad_descriptors[j++]=sr.atom_atom_contact_cad_descriptor_global;}
					if(app_params.score_chain_chain_contacts){output_cad_descriptors[j++]=sr.chain_chain_contact_cad_descriptor_global;}
					if(app_params.score_residue_sas_areas){output_cad_descriptors[j++]=sr.residue_sas_cad_descriptor_global;}
					if(app_params.score_atom_sas_areas){output_cad_descriptors[j++]=sr.atom_sas_cad_descriptor_global;}
					if(app_params.score_chain_sas_areas){output_cad_descriptors[j++]=sr.chain_sas_cad_descriptor_global;}
					if(app_params.score_residue_sites){output_cad_descriptors[j++]=sr.residue_site_cad_descriptor_global;}
					if(app_params.score_atom_sites){output_cad_descriptors[j++]=sr.atom_site_cad_descriptor_global;}
					if(app_params.score_chain_sites){output_cad_descriptors[j++]=sr.chain_site_cad_descriptor_global;}
				}

				if(!app_params.output_local_scores.empty())
				{
					if(success_writing_local_scores && !sr.atom_atom_contact_cad_descriptors.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/atom_atom_contact_cad_descriptors.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.atom_atom_contact_cad_descriptors));
					}
					if(success_writing_local_scores && !sr.atom_atom_contact_cad_descriptors_per_atom.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/atom_atom_contact_cad_descriptors_per_atom.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.atom_atom_contact_cad_descriptors_per_atom));
					}
					if(success_writing_local_scores && !sr.residue_residue_contact_cad_descriptors.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/residue_residue_contact_cad_descriptors.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.residue_residue_contact_cad_descriptors));
					}
					if(success_writing_local_scores && !sr.residue_residue_contact_cad_descriptors_per_residue.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/residue_residue_contact_cad_descriptors_per_residue.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.residue_residue_contact_cad_descriptors_per_residue));
					}
					if(success_writing_local_scores && !sr.chain_chain_contact_cad_descriptors.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/chain_chain_contact_cad_descriptors.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.chain_chain_contact_cad_descriptors));
					}
					if(success_writing_local_scores && !sr.chain_chain_contact_cad_descriptors_per_chain.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/chain_chain_contact_cad_descriptors_per_chain.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.chain_chain_contact_cad_descriptors_per_chain));
					}
					if(success_writing_local_scores && !sr.atom_sas_cad_descriptors.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/atom_sas_cad_descriptors.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.atom_sas_cad_descriptors));
					}
					if(success_writing_local_scores && !sr.residue_sas_cad_descriptors.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/residue_sas_cad_descriptors.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.residue_sas_cad_descriptors));
					}
					if(success_writing_local_scores && !sr.chain_sas_cad_descriptors.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/chain_sas_cad_descriptors.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.chain_sas_cad_descriptors));
					}
					if(success_writing_local_scores && !sr.atom_site_cad_descriptors.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/atom_site_cad_descriptors.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.atom_site_cad_descriptors));
					}
					if(success_writing_local_scores && !sr.residue_site_cad_descriptors.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/residue_site_cad_descriptors.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.residue_site_cad_descriptors));
					}
					if(success_writing_local_scores && !sr.chain_site_cad_descriptors.empty())
					{
						success_writing_local_scores=FileSystemUtilities::write_file(app_params.output_local_scores+"/chain_site_cad_descriptors.tsv", cadscore::PrintingUtilites::print(app_params.output_level_of_detail, sr.chain_site_cad_descriptors));
					}
				}
			}
			else
			{
				if(output_error_message.empty())
				{
					output_error_message="unrecognized error";
				}
			}
			if(!list_of_chain_remapping_summaries.empty())
			{
				std::string& summary=list_of_chain_remapping_summaries[i];
				summary="(";
				for(std::map<std::string, std::string>::const_iterator mit=sr.params.chain_renaming_map.begin();mit!=sr.params.chain_renaming_map.end();++mit)
				{
					if(mit!=sr.params.chain_renaming_map.begin())
					{
						summary+=",";
					}
					summary+=mit->first;
				}
				summary+=")[";
				for(std::map<std::string, std::string>::const_iterator mit=sr.params.chain_renaming_map.begin();mit!=sr.params.chain_renaming_map.end();++mit)
				{
					if(mit!=sr.params.chain_renaming_map.begin())
					{
						summary+=",";
					}
					summary+=mit->second;
				}
				summary+="]";
			}
		}
	}

	if(!success_writing_local_scores)
	{
		std::cerr << "Error: failed to write local score tables to files in directory '" << app_params.output_local_scores << "'.\n";
		return false;
	}

	std::vector<std::size_t> failed_pair_ids;
	{
		for(std::size_t i=0;i<list_of_output_error_messages.size();i++)
		{
			if(!list_of_output_error_messages[i].empty())
			{
				failed_pair_ids.push_back(i);
			}
		}
	}

	{
		for(const std::size_t i : failed_pair_ids)
		{
			const std::string& target_display_name=list_of_unique_file_display_names[list_of_pairs_of_target_model_indices[i].first];
			const std::string& model_display_name=list_of_unique_file_display_names[list_of_pairs_of_target_model_indices[i].second];
			std::cerr << "Error (non-terminating): failed to calculate score for target '" << target_display_name << "' and model '" << model_display_name << "' due to errors:\n";
			std::cerr << list_of_output_error_messages[i] << "\n";
		}
	}

	if(failed_pair_ids.size()==list_of_pairs_of_target_model_indices.size())
	{
		std::cerr << "Error: failed to score any target-model pairs.\n";
		return false;
	}

	std::vector<std::size_t> ordered_pair_ids_for_output;
	{
		std::vector< std::pair<double, std::size_t> > sortable_ids;
		sortable_ids.reserve(list_of_pairs_of_target_model_indices.size()-failed_pair_ids.size());
		for(std::size_t i=0;i<list_of_pairs_of_target_model_indices.size();i++)
		{
			if(list_of_output_error_messages[i].empty())
			{
				sortable_ids.push_back(std::pair<double, std::size_t>(0.0-list_of_output_cad_descriptors[i].front().score(), i));
			}
		}
		std::sort(sortable_ids.begin(), sortable_ids.end());
		ordered_pair_ids_for_output.resize(sortable_ids.size());
		for(std::size_t i=0;i<sortable_ids.size();i++)
		{
			ordered_pair_ids_for_output[i]=sortable_ids[i].second;
		}
	}

	if(!app_params.output_global_scores.empty())
	{
		std::string output_string="target\tmodel";

		for(const std::string& sname : output_score_names)
		{
			output_string+="\t";
			cadscore::PrintingUtilites::print(app_params.output_level_of_detail, cadscore::CADDescriptor(), true, sname+std::string("_"), output_string);
		}
		if(!list_of_chain_remapping_summaries.empty())
		{
			output_string+="\tremapping_of_chains";
		}
		output_string+="\n";

		for(const std::size_t i : ordered_pair_ids_for_output)
		{
			const std::string& target_display_name=list_of_unique_file_display_names[list_of_pairs_of_target_model_indices[i].first];
			const std::string& model_display_name=list_of_unique_file_display_names[list_of_pairs_of_target_model_indices[i].second];
			const std::vector<cadscore::CADDescriptor>& output_cad_descriptors=list_of_output_cad_descriptors[i];
			output_string+=target_display_name;
			output_string+="\t";
			output_string+=model_display_name;
			for(const cadscore::CADDescriptor& cadd : output_cad_descriptors)
			{
				output_string+="\t";
				cadscore::PrintingUtilites::print(app_params.output_level_of_detail, cadd, false, std::string(), output_string);
			}
			if(!list_of_chain_remapping_summaries.empty())
			{
				output_string+="\t";
				output_string+=list_of_chain_remapping_summaries[i];
			}
			output_string+="\n";
		}

		if(app_params.output_global_scores!="_stdout")
		{
			if(!FileSystemUtilities::write_file(app_params.output_global_scores, output_string))
			{
				std::cerr << "Error: failed to write table of global scores to file '" << app_params.output_global_scores << "'.\n";
				return false;
			}
		}
		else
		{
			std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
		}
	}

	return true;
}

}

int main(const int argc, const char** argv)
{
	std::ios_base::sync_with_stdio(false);

	ApplicationParameters app_params;

	if(!app_params.read_from_command_line_args(argc, argv))
	{
		if(!app_params.error_log_for_options_parsing.str().empty())
		{
			std::cerr << app_params.error_log_for_options_parsing.str() << "\n";
		}
		else
		{
			std::cerr << "Error: invalid command line arguments.\n";
		}
		return 1;
	}

	OpenMPUtilities::openmp_setup(app_params.max_number_of_processors);

	if(!app_params.radii_config_file.empty())
	{
		std::string input_data;

		if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(app_params.radii_config_file, input_data))
		{
			std::cerr << "Error: failed to open atom radii configuration file '" << app_params.radii_config_file << "' without errors\n";
			return 1;
		}

		if(input_data.empty())
		{
			std::cerr << "Error: no data read from atom radii configuration file '" << app_params.radii_config_file << "'\n";
			return 1;
		}

		if(!voronotalt::MolecularRadiiAssignment::set_radius_value_rules(input_data))
		{
			std::cerr << "Error: invalid atom radii configuration file.\n";
			return 1;
		}
	}

	if(run(app_params))
	{
		return 0;
	}

	return 1;
}

