#include <filesystem>

#define VORONOTALT_DISABLE_OPENMP

#include "cadscore.h"

namespace
{

void print_help(std::ostream& output) noexcept
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
    --remap-chains                                              flag to automatically rename chains in models to maximize residue-residue contacts score
    --print-paths-in-output                                     flag to print file paths instead of file base names in output
    --help | -h                                                 flag to print help info to stderr and exit

Standard output stream:
    Global scores

Standard error output stream:
    Log (a name-value pair line), error messages

Usage examples:

    voronota-lt-cadscore -t ./target.pdb -m ./model1.pdb ./model2.pdb --score-residue-residue-contacts

    voronota-lt-cadscore -t ./target.pdb -m ./model1.pdb --restrict-input-atoms '[-chain A,B]' --subselect-contacts '[-inter-chain]' --score-everything

)";
}

class ApplicationParameters
{
public:
	double probe;
	bool recursive_directory_search;
	bool include_heteroatoms;
	bool read_inputs_as_assemblies;
	bool remap_chains;
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
	std::ostringstream error_log_for_options_parsing;

	ApplicationParameters() noexcept :
		probe(1.4),
		recursive_directory_search(false),
		include_heteroatoms(false),
		read_inputs_as_assemblies(false),
		remap_chains(false),
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
		read_successfuly(false)
	{
	}

	bool read_from_command_line_args(const int argc, const char** argv) noexcept
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
				else if(opt.name=="remap-chains" && opt.is_flag())
				{
					remap_chains=opt.is_flag_and_true();
				}
				else if(opt.name=="print-paths-in-output" && opt.is_flag())
				{
					print_paths_in_output=opt.is_flag_and_true();
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

		bool operator==(const FileInfo& v) const noexcept
		{
			return (path==v.path);
		}

		bool operator!=(const FileInfo& v) const noexcept
		{
			return (path!=v.path);
		}

		bool operator<(const FileInfo& v) const noexcept
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
};

bool run(const ApplicationParameters& app_params)
{
	cadscore::ScorableData::ConstructionParameters scorable_data_construction_parameters;
	scorable_data_construction_parameters.probe=app_params.probe;
	scorable_data_construction_parameters.record_atom_balls=false;
	scorable_data_construction_parameters.record_atom_atom_contact_summaries=app_params.score_atom_atom_contacts;
	scorable_data_construction_parameters.record_residue_residue_contact_summaries=app_params.score_residue_residue_contacts;
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

	const std::set<FileSystemUtilities::FileInfo> target_file_descriptors=FileSystemUtilities::collect_file_descriptors(app_params.target_input_files, app_params.recursive_directory_search);
	if(target_file_descriptors.empty())
	{
		std::cerr << "Error: no target input files found.\n";
		return false;
	}

	const std::set<FileSystemUtilities::FileInfo> model_file_descriptors=FileSystemUtilities::collect_file_descriptors(app_params.model_input_files, app_params.recursive_directory_search);
	if(model_file_descriptors.empty())
	{
		std::cerr << "Error: no model input files found.\n";
		return false;
	}

	std::set<FileSystemUtilities::FileInfo> all_unique_file_descriptors(model_file_descriptors.begin(), model_file_descriptors.end());
	all_unique_file_descriptors.insert(target_file_descriptors.begin(), target_file_descriptors.end());

	std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData> map_of_scorable_data;
	for(const FileSystemUtilities::FileInfo& fi : all_unique_file_descriptors)
	{
		std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::iterator it=map_of_scorable_data.emplace_hint(map_of_scorable_data.end(), fi.path, cadscore::ScorableData());
		cadscore::ScorableData& sd=it->second;
		sd.construct(scorable_data_construction_parameters, cadscore::MolecularFileInput(fi.path, app_params.include_heteroatoms, app_params.read_inputs_as_assemblies), std::cerr);
		if(!sd.valid())
		{
			std::cerr << "Skipping invalid input file '" << fi.path << "'.\n";
		}
	}

	std::vector< std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::iterator > target_sd_iterators;
	std::vector< std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::iterator > model_sd_iterators;
	target_sd_iterators.reserve(target_file_descriptors.size());
	model_sd_iterators.reserve(model_file_descriptors.size());
	for(std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::iterator it=map_of_scorable_data.begin();it!=map_of_scorable_data.end();++it)
	{
		if(target_file_descriptors.count(it->first)>0)
		{
			target_sd_iterators.push_back(it);
		}
		if(model_file_descriptors.count(it->first)>0)
		{
			model_sd_iterators.push_back(it);
		}
	}

	if(target_sd_iterators.empty())
	{
		std::cerr << "Error: no target inputs processed successfully.\n";
		return false;
	}

	if(model_sd_iterators.empty())
	{
		std::cerr << "Error: no model inputs processed successfully.\n";
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

	std::vector< std::pair<std::size_t, std::size_t> > pairs_of_target_model_ids;
	pairs_of_target_model_ids.reserve(target_sd_iterators.size()*model_sd_iterators.size());
	for(std::size_t ti=0;ti<target_sd_iterators.size();ti++)
	{
		for(std::size_t mi=0;mi<model_sd_iterators.size();mi++)
		{
			pairs_of_target_model_ids.emplace_back(std::pair<std::size_t, std::size_t>(ti, mi));
		}
	}

	std::vector< std::vector<cadscore::CADDescriptor> > output_cad_descriptors(pairs_of_target_model_ids.size(), std::vector<cadscore::CADDescriptor>(output_score_names.size()));
	std::vector<std::string> output_error_messages(pairs_of_target_model_ids.size());
	std::vector< std::pair<double, std::size_t> > output_sortable_indices(pairs_of_target_model_ids.size());

	cadscore::ScoringResult::ConstructionParameters scoring_result_construction_parameters;
	scoring_result_construction_parameters.remap_chains=app_params.remap_chains;

	for(std::size_t i=0;i<pairs_of_target_model_ids.size();i++)
	{
		const std::pair<std::size_t, std::size_t>& pair_ids=pairs_of_target_model_ids[i];
		std::vector<cadscore::CADDescriptor>& pair_cad_descriptors=output_cad_descriptors[i];
		std::string& pair_error_messages=output_error_messages[i];
		std::pair<double, std::size_t>& pair_sortable_index=output_sortable_indices[i];
		pair_sortable_index.first=-1.0;
		pair_sortable_index.second=i;
		std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::const_iterator target_sd_it=target_sd_iterators[pair_ids.first];
		const cadscore::ScorableData& target_sd=target_sd_it->second;
		std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::const_iterator model_sd_it=model_sd_iterators[pair_ids.second];
		const cadscore::ScorableData& model_sd=model_sd_it->second;
		cadscore::ScoringResult sr;
		std::ostringstream local_err_stream;
		sr.construct(scoring_result_construction_parameters, target_sd, model_sd, local_err_stream);
		pair_error_messages=local_err_stream.str();
		if(sr.valid() && pair_error_messages.empty())
		{
			std::size_t j=0;
			if(app_params.score_residue_residue_contacts){pair_cad_descriptors[j++]=sr.residue_residue_contact_cad_descriptor_global;}
			if(app_params.score_atom_atom_contacts){pair_cad_descriptors[j++]=sr.atom_atom_contact_cad_descriptor_global;}
			if(app_params.score_chain_chain_contacts){pair_cad_descriptors[j++]=sr.chain_chain_contact_cad_descriptor_global;}
			if(app_params.score_residue_sas_areas){pair_cad_descriptors[j++]=sr.residue_sas_cad_descriptor_global;}
			if(app_params.score_atom_sas_areas){pair_cad_descriptors[j++]=sr.atom_sas_cad_descriptor_global;}
			if(app_params.score_chain_sas_areas){pair_cad_descriptors[j++]=sr.chain_sas_cad_descriptor_global;}
			if(app_params.score_residue_sites){pair_cad_descriptors[j++]=sr.residue_site_cad_descriptor_global;}
			if(app_params.score_atom_sites){pair_cad_descriptors[j++]=sr.atom_site_cad_descriptor_global;}
			if(app_params.score_chain_sites){pair_cad_descriptors[j++]=sr.chain_site_cad_descriptor_global;}

			if(app_params.score_residue_residue_contacts){pair_sortable_index.first=sr.residue_residue_contact_cad_descriptor_global.score();}
			else if(app_params.score_atom_atom_contacts){pair_sortable_index.first=sr.atom_atom_contact_cad_descriptor_global.score();}
			else if(app_params.score_chain_chain_contacts){pair_sortable_index.first=sr.chain_chain_contact_cad_descriptor_global.score();}
			else if(app_params.score_residue_sas_areas){pair_sortable_index.first=sr.residue_sas_cad_descriptor_global.score();}
			else if(app_params.score_atom_sas_areas){pair_sortable_index.first=sr.atom_sas_cad_descriptor_global.score();}
			else if(app_params.score_chain_sas_areas){pair_sortable_index.first=sr.chain_sas_cad_descriptor_global.score();}
			else if(app_params.score_residue_sites){pair_sortable_index.first=sr.residue_site_cad_descriptor_global.score();}
			else if(app_params.score_atom_sites){pair_sortable_index.first=sr.atom_site_cad_descriptor_global.score();}
			else if(app_params.score_chain_sites){pair_sortable_index.first=sr.chain_site_cad_descriptor_global.score();}
		}
		else
		{
			if(pair_error_messages.empty())
			{
				pair_error_messages="unrecognized error";
			}
		}
		pair_sortable_index.first=(0.0-pair_sortable_index.first);
	}

	std::size_t count_of_failed_pairs=0;

	for(std::size_t i=0;i<pairs_of_target_model_ids.size();i++)
	{
		const std::string& pair_error_messages=output_error_messages[i];
		if(!pair_error_messages.empty())
		{
			count_of_failed_pairs++;
			const std::pair<std::size_t, std::size_t>& pair_ids=pairs_of_target_model_ids[i];
			std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::const_iterator target_sd_it=target_sd_iterators[pair_ids.first];
			std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::const_iterator model_sd_it=model_sd_iterators[pair_ids.second];
			const std::string& target_display_name=(app_params.print_paths_in_output ? target_sd_it->first.path : target_sd_it->first.name);
			const std::string& model_display_name=(app_params.print_paths_in_output ? model_sd_it->first.path : model_sd_it->first.name);
			std::cerr << "Error: failed to calculate score for target '" << target_display_name << "' and model '" << model_display_name << "' due to error: \n";
		}
	}

	if(count_of_failed_pairs>=pairs_of_target_model_ids.size())
	{
		std::cerr << "Error: failed to score any target-model pairs.\n";
		return false;
	}

	std::sort(output_sortable_indices.begin(), output_sortable_indices.end());

	{
		std::cout << "target" << "\t" << "model";
		for(const std::string& sname : output_score_names)
		{
			std::cout << "\t" << sname;
		}
		std::cout << "\n";
	}

	for(const std::pair<double, std::size_t>& pair_sortable_index : output_sortable_indices)
	{
		if(pair_sortable_index.first<=0.0)
		{
			const std::size_t i=pair_sortable_index.second;
			const std::pair<std::size_t, std::size_t>& pair_ids=pairs_of_target_model_ids[i];
			const std::vector<cadscore::CADDescriptor>& pair_cad_descriptors=output_cad_descriptors[i];
			std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::const_iterator target_sd_it=target_sd_iterators[pair_ids.first];
			std::map<FileSystemUtilities::FileInfo, cadscore::ScorableData>::const_iterator model_sd_it=model_sd_iterators[pair_ids.second];
			const std::string& target_display_name=(app_params.print_paths_in_output ? target_sd_it->first.path : target_sd_it->first.name);
			const std::string& model_display_name=(app_params.print_paths_in_output ? model_sd_it->first.path : model_sd_it->first.name);
			std::cout << target_display_name << "\t" << model_display_name;
			for(const cadscore::CADDescriptor& cadd : pair_cad_descriptors)
			{
				std::cout << "\t" << cadd.score();
			}
			std::cout << "\n";
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

