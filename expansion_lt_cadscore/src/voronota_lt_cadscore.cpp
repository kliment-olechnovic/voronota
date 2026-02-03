#include "cadscorelt/cadscorelt.h"
#include "envutil.h"

namespace app_cadscore
{

void print_help(std::ostream& output)
{
	output << "Voronota-LT-CAD-score version " << cadscorelt::version() << "\n";
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
    --reference-sequences-file                       string     input file path for reference sequences in FASTA format
    --stoichiometry-list                             numbers    list of stoichiometry values to apply when mapping chains to reference sequences
    --restrict-input-atoms                           string     selection expression to restrict input balls
    --subselect-contacts                             string     selection expression to restrict contact area descriptors to score, default is '[-min-sep 1]'
    --subselect-atoms                                string     selection expression to restrict atom SAS and site area descriptors to score, default is '[]'
    --scoring-types                                  strings    scoring types ('contacts', 'SAS', 'sites'), default is 'contacts'
    --scoring-levels                                 strings    scoring levels ('atom', 'residue', 'chain'), default is 'residue'
    --local-output-levels                            strings    list of local output levels (can include 'atom', 'residue', 'chain'), default is 'residue'
    --local-output-formats                           strings    list of local output formats (can include 'table', 'pdb', 'mmcif', 'contactmap')
    --consider-residue-names                                    flag to include residue names in residue and atom identifiers, making mapping more strict
    --remap-chains                                              flag to automatically rename chains in models to maximize residue-residue contacts score
    --table-depth                                    number     integer level of detail for table outputs, default is 0 (lowest level, to only print CAD-score) 
    --print-paths-in-output                                     flag to print file paths instead of file base names in output
    --output-global-scores                           string     path to output table of global scores, default is '_stdout' to print to standard output 
    --output-dir                                     string     path to output directory for all result files
    --help | -h                                                 flag to print help info to stderr and exit

Standard output stream:
    Global scores

Standard error output stream:
    Error messages

Usage examples:

    voronota-lt-cadscore -t ./target.pdb -m ./model1.pdb ./model2.pdb --scoring-types contacts --scoring-levels residue chain

    voronota-lt-cadscore -t ./target.pdb -m ./model1.pdb --restrict-input-atoms '[-chain A,B]' --subselect-contacts '[-inter-chain]' --scoring-types contacts

)";
}

class ApplicationParameters
{
public:
	double probe;
	unsigned int max_number_of_processors;
	int max_permutations_to_check_exhaustively;
	int table_depth;
	bool recursive_directory_search;
	bool include_heteroatoms;
	bool read_inputs_as_assemblies;
	bool consider_residue_names;
	bool remap_chains;
	bool scoring_type_contacts;
	bool scoring_type_sas;
	bool scoring_type_sites;
	bool scoring_level_atom;
	bool scoring_level_residue;
	bool scoring_level_chain;
	bool local_output_level_atom;
	bool local_output_level_residue;
	bool local_output_level_chain;
	bool local_output_format_table;
	bool local_output_format_pdb;
	bool local_output_format_mmcif;
	bool local_output_format_contactmap;
	bool print_paths_in_output;
	bool local_scores_requested;
	bool read_successfuly;
	std::vector<std::string> target_input_files;
	std::vector<std::string> model_input_files;
	std::vector<int> stoichiometry_list;
	std::string radii_config_file;
	std::string reference_sequences_file;
	std::string restrict_input_atoms;
	std::string restrict_contact_descriptors;
	std::string restrict_atom_descriptors;
	voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_input_balls;
	voronotalt::FilteringBySphereLabels::ExpressionForPair filtering_expression_for_restricting_contact_descriptors;
	voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_atom_descriptors;
	std::string output_global_scores;
	std::string output_dir;
	std::ostringstream error_log_for_options_parsing;

	ApplicationParameters() :
		probe(1.4),
		max_number_of_processors(envutil::OpenMPUtilities::openmp_enabled() ? 2 : 1),
		max_permutations_to_check_exhaustively(200),
		table_depth(0),
		recursive_directory_search(false),
		include_heteroatoms(false),
		read_inputs_as_assemblies(false),
		consider_residue_names(false),
		remap_chains(false),
		scoring_type_contacts(false),
		scoring_type_sas(false),
		scoring_type_sites(false),
		scoring_level_atom(false),
		scoring_level_residue(false),
		scoring_level_chain(false),
		local_output_level_atom(false),
		local_output_level_residue(false),
		local_output_level_chain(false),
		local_output_format_table(false),
		local_output_format_pdb(false),
		local_output_format_mmcif(false),
		local_output_format_contactmap(false),
		print_paths_in_output(false),
		local_scores_requested(false),
		read_successfuly(false),
		restrict_contact_descriptors("[-min-sep 1]"),
		output_global_scores("_stdout")
	{
	}

	bool read_from_command_line_args(const int argc, const char** argv)
	{
		read_successfuly=false;

		std::set<std::string> set_of_scoring_types;
		std::set<std::string> set_of_scoring_levels;
		std::set<std::string> set_of_local_output_levels;
		std::set<std::string> set_of_local_output_formats;

		set_of_scoring_types.insert("contacts");
		set_of_scoring_levels.insert("residue");
		set_of_local_output_levels.insert("residue");

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
					if(envutil::OpenMPUtilities::openmp_enabled())
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
				else if((opt.name=="models" || opt.name=="m") && !opt.args_strings.empty())
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
				else if(opt.name=="reference-sequences-file" && opt.args_strings.size()==1)
				{
					reference_sequences_file=opt.args_strings.front();
				}
				else if(opt.name=="stoichiometry-list" && !opt.args_ints.empty())
				{
					stoichiometry_list=opt.args_ints;
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
				else if(opt.name=="scoring-types" && !opt.args_strings.empty())
				{
					set_of_scoring_types=std::set<std::string>(opt.args_strings.begin(), opt.args_strings.end());
				}
				else if(opt.name=="scoring-levels" && !opt.args_strings.empty())
				{
					set_of_scoring_levels=std::set<std::string>(opt.args_strings.begin(), opt.args_strings.end());
				}
				else if(opt.name=="local-output-levels" && !opt.args_strings.empty())
				{
					set_of_local_output_levels=std::set<std::string>(opt.args_strings.begin(), opt.args_strings.end());
				}
				else if(opt.name=="local-output-formats" && !opt.args_strings.empty())
				{
					set_of_local_output_formats=std::set<std::string>(opt.args_strings.begin(), opt.args_strings.end());
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
					max_permutations_to_check_exhaustively=static_cast<int>(opt.args_ints.front());
					if(max_permutations_to_check_exhaustively>1000)
					{
						error_log_for_options_parsing << "Error: invalid max number of chains permutations, must be not greater than 1000.\n";
					}
				}
				else if(opt.name=="table-depth" && opt.args_ints.size()==1)
				{
					table_depth=static_cast<int>(opt.args_ints.front());
				}
				else if(opt.name=="print-paths-in-output" && opt.is_flag())
				{
					print_paths_in_output=opt.is_flag_and_true();
				}
				else if(opt.name=="output-global-scores" && opt.args_strings.size()==1)
				{
					output_global_scores=opt.args_strings.front();
				}
				else if(opt.name=="output-dir" && opt.args_strings.size()==1)
				{
					output_dir=opt.args_strings.front();
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

		if(set_of_scoring_types.empty())
		{
			error_log_for_options_parsing << "Error: no scoring types provided.\n";
		}

		if(set_of_scoring_levels.empty())
		{
			error_log_for_options_parsing << "Error: no scoring levels provided.\n";
		}

		if(set_of_local_output_levels.empty())
		{
			error_log_for_options_parsing << "Error: no local output levels provided.\n";
		}

		for(const std::string& token : set_of_scoring_types)
		{
			if(token=="contacts")
			{
				scoring_type_contacts=true;
			}
			else if(token=="sas")
			{
				scoring_type_sas=true;
			}
			else if(token=="sites")
			{
				scoring_type_sites=true;
			}
			else
			{
				error_log_for_options_parsing << "Error: invalid scoring type '" << token << "'.\n";
			}
		}

		for(const std::string& token : set_of_scoring_levels)
		{
			if(token=="atom")
			{
				scoring_level_atom=true;
			}
			else if(token=="residue")
			{
				scoring_level_residue=true;
			}
			else if(token=="chain")
			{
				scoring_level_chain=true;
			}
			else
			{
				error_log_for_options_parsing << "Error: invalid scoring level '" << token << "'.\n";
			}
		}

		for(const std::string& token : set_of_local_output_levels)
		{
			if(token=="atom")
			{
				local_output_level_atom=true;
			}
			else if(token=="residue")
			{
				local_output_level_residue=true;
			}
			else if(token=="chain")
			{
				local_output_level_chain=true;
			}
			else
			{
				error_log_for_options_parsing << "Error: invalid local output level '" << token << "'.\n";
			}
		}

		for(const std::string& token : set_of_local_output_formats)
		{
			if(token=="table")
			{
				local_output_format_table=true;
			}
			else if(token=="pdb")
			{
				local_output_format_pdb=true;
			}
			else if(token=="mmcif")
			{
				local_output_format_mmcif=true;
			}
			else if(token=="contactmap")
			{
				local_output_format_contactmap=true;
			}
			else
			{
				error_log_for_options_parsing << "Error: invalid local output format '" << token << "'.\n";
			}
		}

		local_scores_requested=(local_output_format_table || local_output_format_pdb || local_output_format_mmcif || local_output_format_contactmap);

		if(target_input_files.empty())
		{
			error_log_for_options_parsing << "Error: no input target files provided.\n";
		}

		if(model_input_files.empty())
		{
			error_log_for_options_parsing << "Error: no input model files provided.\n";
		}

		if(output_dir.empty() && local_scores_requested)
		{
			error_log_for_options_parsing << "Error: no output directory provided to write local scores.\n";
		}

		if(output_global_scores.empty() && !local_scores_requested)
		{
			error_log_for_options_parsing << "Error: no outputs requested.\n";
		}

		if(local_scores_requested && model_input_files.size()!=1 && target_input_files.empty()!=1)
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

bool run(const ApplicationParameters& app_params)
{
	envutil::OpenMPUtilities::openmp_setup(app_params.max_number_of_processors);

	if(!app_params.radii_config_file.empty())
	{
		std::string input_data;

		if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(app_params.radii_config_file, input_data))
		{
			std::cerr << "Error: failed to open atom radii configuration file '" << app_params.radii_config_file << "' without errors\n";
			return false;
		}

		if(input_data.empty())
		{
			std::cerr << "Error: no data read from atom radii configuration file '" << app_params.radii_config_file << "'\n";
			return false;
		}

		if(!voronotalt::MolecularRadiiAssignment::set_radius_value_rules(input_data))
		{
			std::cerr << "Error: invalid atom radii configuration file.\n";
			return false;
		}
	}

	cadscorelt::IDResidue::consider_residue_names()=app_params.consider_residue_names;

	cadscorelt::ScorableData::ConstructionParameters scorable_data_construction_parameters;
	scorable_data_construction_parameters.probe=app_params.probe;
	scorable_data_construction_parameters.record_atom_balls=(app_params.local_scores_requested && (app_params.local_output_format_pdb || app_params.local_output_format_mmcif));
	scorable_data_construction_parameters.record_atom_atom_contact_summaries=(app_params.scoring_type_contacts && app_params.scoring_level_atom);
	scorable_data_construction_parameters.record_residue_residue_contact_summaries=(app_params.scoring_type_contacts && app_params.scoring_level_residue);
	scorable_data_construction_parameters.record_chain_chain_contact_summaries=(app_params.scoring_type_contacts && app_params.scoring_level_chain);
	scorable_data_construction_parameters.record_atom_cell_summaries=(app_params.scoring_type_sas && app_params.scoring_level_atom);
	scorable_data_construction_parameters.record_residue_cell_summaries=(app_params.scoring_type_sas && app_params.scoring_level_residue);
	scorable_data_construction_parameters.record_chain_cell_summaries=(app_params.scoring_type_sas && app_params.scoring_level_chain);
	scorable_data_construction_parameters.record_atom_site_summaries=(app_params.scoring_type_sites && app_params.scoring_level_atom);
	scorable_data_construction_parameters.record_residue_site_summaries=(app_params.scoring_type_sites && app_params.scoring_level_residue);
	scorable_data_construction_parameters.record_chain_site_summaries=(app_params.scoring_type_sites && app_params.scoring_level_chain);
	scorable_data_construction_parameters.filtering_expression_for_restricting_input_balls=app_params.filtering_expression_for_restricting_input_balls;
	scorable_data_construction_parameters.filtering_expression_for_restricting_contact_descriptors=app_params.filtering_expression_for_restricting_contact_descriptors;
	scorable_data_construction_parameters.filtering_expression_for_restricting_atom_descriptors=app_params.filtering_expression_for_restricting_atom_descriptors;

	if(!app_params.reference_sequences_file.empty())
	{
		scorable_data_construction_parameters.reference_sequences=sequtil::SequenceInputUtilities::read_sequences_from_file(app_params.reference_sequences_file);
		if(scorable_data_construction_parameters.reference_sequences.empty())
		{
			std::cerr << "Error: failed to read sequences from file '" << app_params.reference_sequences_file << "'\n";
			return false;
		}
	}

	if(!scorable_data_construction_parameters.reference_sequences.empty() && !app_params.stoichiometry_list.empty())
	{
		if(app_params.stoichiometry_list.size()!=scorable_data_construction_parameters.reference_sequences.size())
		{
			std::cerr << "Error: stoichiometry list length (" << app_params.stoichiometry_list.size() << ") does not equal the number of reference sequences (" << scorable_data_construction_parameters.reference_sequences.size() << ").\n";
			return false;
		}
		else
		{
			scorable_data_construction_parameters.reference_stoichiometry=app_params.stoichiometry_list;
		}
	}

	if(!scorable_data_construction_parameters.valid())
	{
		std::cerr << "Error: invalid data preparation parameters.\n";
		return false;
	}

	const std::set<envutil::FileSystemUtilities::FileInfo> set_of_target_file_descriptors=envutil::FileSystemUtilities::collect_file_descriptors(app_params.target_input_files, app_params.recursive_directory_search);
	if(set_of_target_file_descriptors.empty())
	{
		std::cerr << "Error: no target input files found.\n";
		return false;
	}

	const std::set<envutil::FileSystemUtilities::FileInfo> set_of_model_file_descriptors=envutil::FileSystemUtilities::collect_file_descriptors(app_params.model_input_files, app_params.recursive_directory_search);
	if(set_of_model_file_descriptors.empty())
	{
		std::cerr << "Error: no model input files found.\n";
		return false;
	}

	std::vector<envutil::FileSystemUtilities::FileInfo> list_of_unique_file_descriptors;
	{
		std::set<envutil::FileSystemUtilities::FileInfo> set_of_unique_file_descriptors=set_of_model_file_descriptors;
		set_of_unique_file_descriptors.insert(set_of_target_file_descriptors.begin(), set_of_target_file_descriptors.end());
		list_of_unique_file_descriptors.reserve(set_of_unique_file_descriptors.size());
		list_of_unique_file_descriptors.insert(list_of_unique_file_descriptors.end(), set_of_unique_file_descriptors.begin(), set_of_unique_file_descriptors.end());
	}

	if(app_params.local_scores_requested && list_of_unique_file_descriptors.size()!=2)
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

	std::vector<cadscorelt::ScorableData> list_of_unique_scorable_data(list_of_unique_file_descriptors.size());
	std::vector<std::string> list_of_error_messages_for_unique_scorable_data(list_of_unique_file_descriptors.size());

	const bool parallelize_on_root_level=(list_of_unique_file_descriptors.size()*2>app_params.max_number_of_processors);

#ifdef _OPENMP
#pragma omp parallel for if(parallelize_on_root_level)
#endif
	for(std::size_t i=0;i<list_of_unique_file_descriptors.size();i++)
	{
		const envutil::FileSystemUtilities::FileInfo& fi=list_of_unique_file_descriptors[i];
		cadscorelt::ScorableData& sd=list_of_unique_scorable_data[i];
		std::string& error_message=list_of_error_messages_for_unique_scorable_data[i];
		std::ostringstream local_error_stream;
		sd.construct(scorable_data_construction_parameters, cadscorelt::MolecularFileInput(fi.path, app_params.include_heteroatoms, app_params.read_inputs_as_assemblies), local_error_stream);
		error_message=local_error_stream.str();
		if(!sd.valid() && error_message.empty())
		{
			error_message="unrecognized error";
		}
	}

	for(std::size_t i=0;i<list_of_unique_file_descriptors.size();i++)
	{
		const envutil::FileSystemUtilities::FileInfo& fi=list_of_unique_file_descriptors[i];
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
			std::cerr << "Error (non-terminating): failed to process input file '" << fi.path << "' due to errors:\n";
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
		if(app_params.scoring_type_contacts && app_params.scoring_level_residue){output_score_names.push_back("residue_residue");}
		if(app_params.scoring_type_contacts && app_params.scoring_level_atom){output_score_names.push_back("atom_atom");}
		if(app_params.scoring_type_contacts && app_params.scoring_level_chain){output_score_names.push_back("chain_chain");}
		if(app_params.scoring_type_sas && app_params.scoring_level_residue){output_score_names.push_back("residue_sas");}
		if(app_params.scoring_type_sas && app_params.scoring_level_atom){output_score_names.push_back("atom_sas");}
		if(app_params.scoring_type_sas && app_params.scoring_level_chain){output_score_names.push_back("chain_sas");}
		if(app_params.scoring_type_sites && app_params.scoring_level_residue){output_score_names.push_back("residue_sites");}
		if(app_params.scoring_type_sites && app_params.scoring_level_atom){output_score_names.push_back("atom_sites");}
		if(app_params.scoring_type_sites && app_params.scoring_level_chain){output_score_names.push_back("chain_sites");}
	}

	std::vector< std::vector<cadscorelt::CADDescriptor> > list_of_output_cad_descriptors(list_of_pairs_of_target_model_indices.size(), std::vector<cadscorelt::CADDescriptor>(output_score_names.size()));
	std::vector<std::string> list_of_output_error_messages(list_of_pairs_of_target_model_indices.size());

	std::vector<std::string> list_of_chain_remapping_summaries((app_params.remap_chains || !scorable_data_construction_parameters.reference_sequences.empty()) ? list_of_pairs_of_target_model_indices.size() : static_cast<std::size_t>(0));

	bool success_writing_local_scores=true;

	{
		cadscorelt::ScoringResult::ConstructionParameters scoring_result_construction_parameters;
		scoring_result_construction_parameters.remap_chains=app_params.remap_chains;
		scoring_result_construction_parameters.max_permutations_to_check_exhaustively=app_params.max_permutations_to_check_exhaustively;
		scoring_result_construction_parameters.record_local_scores_on_atom_level=app_params.local_output_level_atom;
		scoring_result_construction_parameters.record_local_scores_on_residue_level=app_params.local_output_level_residue;
		scoring_result_construction_parameters.record_local_scores_on_chain_level=app_params.local_output_level_chain;
		scoring_result_construction_parameters.record_compatible_model_atom_balls=app_params.local_scores_requested && (app_params.local_output_format_pdb || app_params.local_output_format_mmcif);

#ifdef _OPENMP
#pragma omp parallel for
#endif
		for(std::size_t i=0;i<list_of_pairs_of_target_model_indices.size();i++)
		{
			const cadscorelt::ScorableData& target_sd=list_of_unique_scorable_data[list_of_pairs_of_target_model_indices[i].first];
			const cadscorelt::ScorableData& model_sd=list_of_unique_scorable_data[list_of_pairs_of_target_model_indices[i].second];
			std::vector<cadscorelt::CADDescriptor>& output_cad_descriptors=list_of_output_cad_descriptors[i];
			std::string& output_error_message=list_of_output_error_messages[i];
			cadscorelt::ScoringResult sr;
			std::ostringstream local_err_stream;
			sr.construct(scoring_result_construction_parameters, target_sd, model_sd, local_err_stream);
			output_error_message=local_err_stream.str();
			if(sr.valid() && output_error_message.empty())
			{
				{
					std::size_t j=0;
					if(app_params.scoring_type_contacts && app_params.scoring_level_residue){output_cad_descriptors[j++]=sr.cadscores_residue_residue_summarized_globally;}
					if(app_params.scoring_type_contacts && app_params.scoring_level_atom){output_cad_descriptors[j++]=sr.cadscores_atom_atom_summarized_globally;}
					if(app_params.scoring_type_contacts && app_params.scoring_level_chain){output_cad_descriptors[j++]=sr.cadscores_chain_chain_summarized_globally;}
					if(app_params.scoring_type_sas && app_params.scoring_level_residue){output_cad_descriptors[j++]=sr.cadscores_residue_sas_summarized_globally;}
					if(app_params.scoring_type_sas && app_params.scoring_level_atom){output_cad_descriptors[j++]=sr.cadscores_atom_sas_summarized_globally;}
					if(app_params.scoring_type_sas && app_params.scoring_level_chain){output_cad_descriptors[j++]=sr.cadscores_chain_sas_summarized_globally;}
					if(app_params.scoring_type_sites && app_params.scoring_level_residue){output_cad_descriptors[j++]=sr.cadscores_residue_site_summarized_globally;}
					if(app_params.scoring_type_sites && app_params.scoring_level_atom){output_cad_descriptors[j++]=sr.cadscores_atom_site_summarized_globally;}
					if(app_params.scoring_type_sites && app_params.scoring_level_chain){output_cad_descriptors[j++]=sr.cadscores_chain_site_summarized_globally;}
				}

				if(app_params.local_scores_requested && !app_params.output_dir.empty())
				{
					if(app_params.local_output_format_table)
					{
						if(success_writing_local_scores && !sr.cadscores_atom_atom.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_atom));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_residue_residue.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_residue_residue.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_atom_summarized_per_residue_residue));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_chain_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_chain_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_atom_summarized_per_chain_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_atom.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_atom.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_atom_summarized_per_atom));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_residue.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_residue.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_atom_summarized_per_residue));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_atom_summarized_per_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_residue.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_residue_residue));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_residue_summarized_per_chain_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue_summarized_per_chain_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_residue_residue_summarized_per_chain_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_residue_summarized_per_residue.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue_summarized_per_residue.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_residue_residue_summarized_per_residue));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_residue_summarized_per_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue_summarized_per_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_residue_residue_summarized_per_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_chain_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_chain_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_chain_chain_contact_summarized_per_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_chain_contact_summarized_per_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_chain_chain_contact_summarized_per_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_sas.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_sas.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_sas));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_sas_summarized_per_residue.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_sas_summarized_per_residue.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_sas_summarized_per_residue));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_sas_summarized_per_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_sas_summarized_per_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_sas_summarized_per_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_sas.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_sas.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_residue_sas));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_sas_summarized_per_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_sas_summarized_per_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_residue_sas_summarized_per_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_chain_sas.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_sas.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_chain_sas));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_site.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_site.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_site));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_site_summarized_per_residue.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_site_summarized_per_residue.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_site_summarized_per_residue));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_site_summarized_per_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_site_summarized_per_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_atom_site_summarized_per_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_site.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_site.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_residue_site));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_site_summarized_per_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_site_summarized_per_chain.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_residue_site_summarized_per_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_chain_site.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_site.tsv", cadscorelt::PrintingUtilites::print(app_params.table_depth, sr.cadscores_chain_site));
						}
					}

					if(app_params.local_output_format_pdb)
					{
						for(int t=0;t<2 && success_writing_local_scores;t++)
						{
							const std::vector<cadscorelt::AtomBall> relevant_atom_balls=(t==0 ? sr.compatible_model_atom_balls : target_sd.atom_balls);
							const std::string filename_end=(t==0 ? "_on_model.pdb" : "_on_target.pdb");
							if(cadscorelt::MolecularFileWritingUtilities::PDB::check_compatability_with_pdb_format(relevant_atom_balls))
							{
								if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_atom.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_atom"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_atom_atom_summarized_per_atom));
								}
								if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_residue.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_residue"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_atom_atom_summarized_per_residue));
								}
								if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_chain.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_atom_atom_summarized_per_chain));
								}
								if(success_writing_local_scores && !sr.cadscores_residue_residue_summarized_per_residue.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue_summarized_per_residue"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_residue_residue_summarized_per_residue));
								}
								if(success_writing_local_scores && !sr.cadscores_residue_residue_summarized_per_chain.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_residue_residue_summarized_per_chain));
								}
								if(success_writing_local_scores && !sr.cadscores_chain_chain_contact_summarized_per_chain.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_chain_contact_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_chain_chain_contact_summarized_per_chain));
								}
								if(success_writing_local_scores && !sr.cadscores_atom_sas.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_sas"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_atom_sas));
								}
								if(success_writing_local_scores && !sr.cadscores_atom_sas_summarized_per_residue.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_sas_summarized_per_residue"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_atom_sas_summarized_per_residue));
								}
								if(success_writing_local_scores && !sr.cadscores_atom_sas_summarized_per_chain.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_sas_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_atom_sas_summarized_per_chain));
								}
								if(success_writing_local_scores && !sr.cadscores_residue_sas.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_sas"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_residue_sas));
								}
								if(success_writing_local_scores && !sr.cadscores_residue_sas_summarized_per_chain.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_sas_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_residue_sas_summarized_per_chain));
								}
								if(success_writing_local_scores && !sr.cadscores_chain_sas.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_sas"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_chain_sas));
								}
								if(success_writing_local_scores && !sr.cadscores_atom_site.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_site"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_atom_site));
								}
								if(success_writing_local_scores && !sr.cadscores_atom_site_summarized_per_residue.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_site_summarized_per_residue"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_atom_site_summarized_per_residue));
								}
								if(success_writing_local_scores && !sr.cadscores_atom_site_summarized_per_chain.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_site_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_atom_site_summarized_per_chain));
								}
								if(success_writing_local_scores && !sr.cadscores_residue_site.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_site"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_residue_site));
								}
								if(success_writing_local_scores && !sr.cadscores_residue_site_summarized_per_chain.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_site_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_residue_site_summarized_per_chain));
								}
								if(success_writing_local_scores && !sr.cadscores_chain_site.empty())
								{
									success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_site"+filename_end, cadscorelt::MolecularFileWritingUtilities::PDB::print(relevant_atom_balls, sr.cadscores_chain_site));
								}
							}
							else
							{
								std::cerr << "Error (non-terminating): skipped writing PDB files with scores because the recorded atoms cannot not fit into a standard single-poodel PDB format file.\n";
							}
						}
					}

					if(app_params.local_output_format_mmcif)
					{
						for(int t=0;t<2 && success_writing_local_scores;t++)
						{
							const std::vector<cadscorelt::AtomBall> relevant_atom_balls=(t==0 ? sr.compatible_model_atom_balls : target_sd.atom_balls);
							const std::string filename_end=(t==0 ? "_on_model.cif" : "_on_target.cif");
							if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_atom.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_atom"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_atom_atom_summarized_per_atom));
							}
							if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_residue.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_residue"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_atom_atom_summarized_per_residue));
							}
							if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_chain.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_atom_atom_summarized_per_chain));
							}
							if(success_writing_local_scores && !sr.cadscores_residue_residue_summarized_per_residue.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue_summarized_per_residue"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_residue_residue_summarized_per_residue));
							}
							if(success_writing_local_scores && !sr.cadscores_residue_residue_summarized_per_chain.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_residue_residue_summarized_per_chain));
							}
							if(success_writing_local_scores && !sr.cadscores_chain_chain_contact_summarized_per_chain.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_chain_contact_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_chain_chain_contact_summarized_per_chain));
							}
							if(success_writing_local_scores && !sr.cadscores_atom_sas.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_sas"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_atom_sas));
							}
							if(success_writing_local_scores && !sr.cadscores_atom_sas_summarized_per_residue.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_sas_summarized_per_residue"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_atom_sas_summarized_per_residue));
							}
							if(success_writing_local_scores && !sr.cadscores_atom_sas_summarized_per_chain.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_sas_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_atom_sas_summarized_per_chain));
							}
							if(success_writing_local_scores && !sr.cadscores_residue_sas.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_sas"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_residue_sas));
							}
							if(success_writing_local_scores && !sr.cadscores_residue_sas_summarized_per_chain.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_sas_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_residue_sas_summarized_per_chain));
							}
							if(success_writing_local_scores && !sr.cadscores_chain_sas.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_sas"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_chain_sas));
							}
							if(success_writing_local_scores && !sr.cadscores_atom_site.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_site"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_atom_site));
							}
							if(success_writing_local_scores && !sr.cadscores_atom_site_summarized_per_residue.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_site_summarized_per_residue"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_atom_site_summarized_per_residue));
							}
							if(success_writing_local_scores && !sr.cadscores_atom_site_summarized_per_chain.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_site_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_atom_site_summarized_per_chain));
							}
							if(success_writing_local_scores && !sr.cadscores_residue_site.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_site"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_residue_site));
							}
							if(success_writing_local_scores && !sr.cadscores_residue_site_summarized_per_chain.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_site_summarized_per_chain"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_residue_site_summarized_per_chain));
							}
							if(success_writing_local_scores && !sr.cadscores_chain_site.empty())
							{
								success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_site"+filename_end, cadscorelt::MolecularFileWritingUtilities::MMCIF::print(relevant_atom_balls, sr.cadscores_chain_site));
							}
						}
					}

					if(app_params.local_output_format_contactmap)
					{
						if(success_writing_local_scores && !sr.cadscores_atom_atom.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom.svg", cadscorelt::ContactMapPlottingUtilities::print(sr.cadscores_atom_atom));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_residue_residue.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_residue_residue.svg", cadscorelt::ContactMapPlottingUtilities::print(sr.cadscores_atom_atom_summarized_per_residue_residue));
						}
						if(success_writing_local_scores && !sr.cadscores_atom_atom_summarized_per_chain_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_atom_atom_summarized_per_chain_chain.svg", cadscorelt::ContactMapPlottingUtilities::print(sr.cadscores_atom_atom_summarized_per_chain_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_residue.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue.svg", cadscorelt::ContactMapPlottingUtilities::print(sr.cadscores_residue_residue));
						}
						if(success_writing_local_scores && !sr.cadscores_residue_residue_summarized_per_chain_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_residue_residue_summarized_per_chain_chain.svg", cadscorelt::ContactMapPlottingUtilities::print(sr.cadscores_residue_residue_summarized_per_chain_chain));
						}
						if(success_writing_local_scores && !sr.cadscores_chain_chain.empty())
						{
							success_writing_local_scores=envutil::FileSystemUtilities::write_file(app_params.output_dir+"/cadscores_chain_chain.svg", cadscorelt::ContactMapPlottingUtilities::print(sr.cadscores_chain_chain));
						}
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
				if(!model_sd.chain_sequences_mapping_result.chain_renaming_label.empty())
				{
					summary+="m:";
					summary+=model_sd.chain_sequences_mapping_result.chain_renaming_label;
					summary+=";";
				}
				if(!target_sd.chain_sequences_mapping_result.chain_renaming_label.empty())
				{
					summary+="t:";
					summary+=target_sd.chain_sequences_mapping_result.chain_renaming_label;
					summary+=";";
				}
				if(!sr.params.chain_renaming_map.empty())
				{
					summary+="mt:(";
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
					summary+="];";
				}
				if(summary.empty())
				{
					summary=".";
				}
			}
		}
	}

	if(!success_writing_local_scores)
	{
		std::cerr << "Error: failed to write local score files to directory '" << app_params.output_dir << "'.\n";
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

	if(!app_params.output_global_scores.empty() || !app_params.output_dir.empty())
	{
		std::string output_string="target\tmodel";

		for(const std::string& sname : output_score_names)
		{
			output_string+="\t";
			cadscorelt::PrintingUtilites::print(app_params.table_depth, cadscorelt::CADDescriptor(), true, sname+std::string("_"), output_string);
		}
		if(!list_of_chain_remapping_summaries.empty())
		{
			output_string+="\trenaming_of_chains";
		}
		output_string+="\n";

		for(const std::size_t i : ordered_pair_ids_for_output)
		{
			const std::string& target_display_name=list_of_unique_file_display_names[list_of_pairs_of_target_model_indices[i].first];
			const std::string& model_display_name=list_of_unique_file_display_names[list_of_pairs_of_target_model_indices[i].second];
			const std::vector<cadscorelt::CADDescriptor>& output_cad_descriptors=list_of_output_cad_descriptors[i];
			output_string+=target_display_name;
			output_string+="\t";
			output_string+=model_display_name;
			for(const cadscorelt::CADDescriptor& cadd : output_cad_descriptors)
			{
				output_string+="\t";
				cadscorelt::PrintingUtilites::print(app_params.table_depth, cadd, false, std::string(), output_string);
			}
			if(!list_of_chain_remapping_summaries.empty())
			{
				output_string+="\t";
				output_string+=list_of_chain_remapping_summaries[i];
			}
			output_string+="\n";
		}

		if(app_params.output_global_scores=="_stdout")
		{
			std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
		}
		else
		{
			if(!envutil::FileSystemUtilities::write_file(app_params.output_global_scores, output_string))
			{
				std::cerr << "Error: failed to write table of global scores to file '" << app_params.output_global_scores << "'.\n";
				return false;
			}
		}

		if(!app_params.output_dir.empty())
		{
			if(!envutil::FileSystemUtilities::write_file(app_params.output_dir+"/global_scores.tsv", output_string))
			{
				std::cerr << "Error: failed to write table of global scores to file '" << app_params.output_global_scores << "'.\n";
				return false;
			}
		}
	}

	return true;
}

bool run(const int argc, const char** argv)
{
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
		return false;
	}

	return run(app_params);
}

}

int main(const int argc, const char** argv)
{
	std::ios_base::sync_with_stdio(false);

	return (app_cadscore::run(argc, argv) ? 0 : 1);
}

