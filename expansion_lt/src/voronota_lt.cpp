#include "voronotalt/parallelization_configuration.h"

#include "voronotalt/voronotalt.h"

#include "voronotalt_cli/voronotalt_cli.h"

namespace
{

void print_help(std::ostream& output) noexcept
{
	output << R"(
Voronota-LT version 1.0.1

'voronota-lt' executable constructs a radical Voronoi tessellation (also known as a Laguerre-Voronoi diagram or a power diagram)
of atomic balls of van der Waals radii constrained inside a solvent-accessible surface defined by a rolling probe.
The software computes inter-atom contact areas, per-cell solvent accessible surface areas, per-cell constrained volumes.
'voronota-lt' is very fast when used on molecular data with a not large rolling probe radius (less than 2.0 angstroms, 1.4 is recommended)
and can be made even faster by running it using multiple processors.

Options:
    --probe                                          number     rolling probe radius, default is 1.4
    --processors                                     number     maximum number of OpenMP threads to use, default is 2 if OpenMP is enabled, 1 if disabled
    --compute-only-inter-residue-contacts                       flag to only compute inter-residue contacts, turns off per-cell summaries
    --compute-only-inter-chain-contacts                         flag to only compute inter-chain contacts, turns off per-cell summaries
    --run-in-aw-diagram-regime                                  flag to run construct a simplified additively weighted Voronoi diagram, turns off per-cell summaries
    --input | -i                                     string     input file path to use instead of standard input, or '_stdin' to still use standard input
    --periodic-box-directions                        numbers    coordinates of three vectors (x1 y1 z1 x2 y2 z2 x3 y3 z3) to define and use a periodic box
    --periodic-box-corners                           numbers    coordinates of two corners (x1 y1 z1 x2 y2 z2) to define and use a periodic box
    --pdb-or-mmcif-heteroatoms                                  flag to include heteroatoms when reading input in PDB or mmCIF format
    --pdb-or-mmcif-hydrogens                                    flag to include hydrogen atoms when reading input in PDB or mmCIF format
    --pdb-or-mmcif-join-models                                  flag to join multiple models into an assembly when reading input in PDB or mmCIF format
    --pdb-or-mmcif-radii-config-file                 string     input file path for reading atom radii assignment rules
    --grouping-directives                            string     string with grouping directives separated by ';'
    --grouping-directives-file                       string     input file path for grouping directives
    --restrict-input-balls                           string     selection expression to restrict input balls
    --restrict-contacts                              string     selection expression to restrict contacts before construction
    --restrict-contacts-for-output                   string     selection expression to restrict contacts for output
    --restrict-single-index-data-for-output          string     selection expression to restrict single-index data (balls, cells, sites) for output
    --print-contacts                                            flag to print table of contacts to stdout
    --print-contacts-residue-level                              flag to print table of residue-level grouped contacts to stdout
    --print-contacts-chain-level                                flag to print table of chain-level grouped contacts to stdout
    --print-cells                                               flag to print table of per-cell summaries to stdout
    --print-cells-residue-level                                 flag to print table of residue-level grouped per-cell summaries to stdout
    --print-cells-chain-level                                   flag to print table of chain-level grouped per-cell summaries to stdout
    --print-sites                                               flag to print table of binding site summaries to stdout
    --print-sites-residue-level                                 flag to print table of residue-level grouped binding site summaries to stdout
    --print-sites-chain-level                                   flag to print table of chain-level grouped binding site summaries to stdout
    --print-everything                                          flag to print everything to stdout, terminate if printing everything is not possible
    --write-input-balls-to-file                                 output file path to write input balls to file
    --write-contacts-to-file                         string     output file path to write table of contacts
    --write-contacts-residue-level-to-file           string     output file path to write table of residue-level grouped contacts
    --write-contacts-chain-level-to-file             string     output file path to write table of chain-level grouped contacts
    --write-cells-to-file                            string     output file path to write table of per-cell summaries
    --write-cells-residue-level-to-file              string     output file path to write table of residue-level grouped per-cell summaries
    --write-cells-chain-level-to-file                string     output file path to write table of chain-level grouped per-cell summaries
    --write-sites-to-file                            string     output file path to write table of binding site summaries
    --write-sites-residue-level-to-file              string     output file path to write table of residue-level grouped binding site summaries
    --write-sites-chain-level-to-file                string     output file path to write table of chain-level grouped binding site summaries
    --write-tessellation-edges-to-file               string     output file path to write generating IDs and lengths of SAS-constrained tessellation edges
    --write-tessellation-vertices-to-file            string     output file path to write generating IDs and positions of SAS-constrained tessellation vertices
    --write-raw-collisions-to-file                   string     output file path to write a table of both true (contact) and false (no contact) collisions
    --plot-contacts-to-file                          string     output file path to write SVG plot of contacts
    --plot-contacts-residue-level-to-file            string     output file path to write SVG plot of residue-level grouped contacts
    --plot-contacts-chain-level-to-file              string     output file path to write SVG plot of chain-level grouped contacts
    --plot-options-config                            strings    space-separated list of plotting flags, e.g. ylabeled xlabeled gradient dark
    --graphics-output-file-for-pymol                 string     output file path to write contacts drawing script for PyMol
    --graphics-output-file-for-chimera               string     output file path to write contacts drawing script for Chimera or ChimeraX
    --graphics-title                                 string     title to use for the graphics objects generated by the contacts drawing script
    --graphics-restrict-representations              strings    space-separated list of representations to output, e.g.: balls faces wireframe sas sasmesh lattice
    --graphics-coloring-config                       string     string with graphics coloring rules separated by ';'
    --graphics-coloring-config-file                  string     input file path for reading graphics coloring rules
    --sites-view-scrip-for-pymol                     string     output file path to write sites view script for PyMol
    --sites-view-scrip-for-chimera                   string     output file path to write sites view script for ChimeraX
    --mesh-output-obj-file                           string     output file path to write contacts surfaces mesh .obj file
    --mesh-print-topology-summary                               flag to print mesh topology summary
    --measure-running-time                                      flag to measure and output running times
    --write-log-to-file                              string     output file path to write global log, does not turn off printing log to stderr
    --minimum-columns | -m                                      flag to not print empty identifiers in summarized residue-level and chain-level tables
    --quiet | -q                                                flag to suppress printing non-error log messages to stderr
    --help | -h                                                 flag to print help info to stderr and exit

Standard input stream:
    Several input formats are supported:
      a) Space-separated or tab-separated header-less table of balls, one of the following line formats possible:
             x y z radius
             chainID x y z radius
             chainID residueID x y z radius
             chainID residueID atomName x y z radius
             chainID residueNum residueName atomName x y z radius
             chainID residueNum iCode residueName atomName x y z radius
      b) Output of 'voronota get-balls-from-atoms-file' is acceptable, where line format is:
             x y z radius # atomSerial chainID resSeq resName atomName altLoc iCode
      c) PDB file
      d) mmCIF file

Standard output stream:
    Requested tables with headers, with column values tab-separated

Standard error output stream:
    Log (a name-value pair line), error messages

Usage examples:

    cat ./2zsk.pdb | voronota-lt --print-contacts

    voronota-lt -i ./2zsk.pdb --print-contacts

    voronota-lt --input ./2zsk.pdb --print-contacts-residue-level --compute-only-inter-residue-contacts

    voronota-lt --input ./balls.xyzr --processors 8 --write-contacts-to-file ./contacts.tsv --write-cells-to-file ./cells.tsv

    voronota-lt -i ./balls.xyzr --probe 2 --periodic-box-corners 0 0 0 100 100 300 --processors 8 --write-cells-to-file ./cells.tsv
)";
}

class ApplicationParameters
{
public:
	struct RunningMode
	{
		enum ID
		{
			radical,
			simplified_aw,
		};
	};

	unsigned int max_number_of_processors;
	voronotalt::Float probe;
	bool compute_only_inter_residue_contacts;
	bool compute_only_inter_chain_contacts;
	bool pdb_or_mmcif_heteroatoms;
	bool pdb_or_mmcif_hydrogens;
	bool pdb_or_mmcif_as_assembly;
	bool measure_running_time;
	bool print_contacts;
	bool print_contacts_residue_level;
	bool print_contacts_chain_level;
	bool print_cells;
	bool print_cells_residue_level;
	bool print_cells_chain_level;
	bool print_sites;
	bool print_sites_residue_level;
	bool print_sites_chain_level;
	bool print_everything;
	bool plots_colorable;
	bool need_sites;
	bool need_summaries_on_residue_level;
	bool need_summaries_on_chain_level;
	RunningMode::ID running_mode;
	long mesh_extract_connected_component;
	bool mesh_print_topology_summary;
	bool exit_before_calculations;
	bool minimum_columns;
	bool quiet;
	bool read_successfuly;
	std::string input_from_file;
	std::string pdb_or_mmcif_radii_config_file;
	std::vector<voronotalt::SimplePoint> periodic_box_directions;
	std::vector<voronotalt::SimplePoint> periodic_box_corners;
	std::string grouping_directives;
	std::string grouping_directives_file;
	std::string write_input_balls_to_file;
	std::string write_contacts_to_file;
	std::string write_contacts_residue_level_to_file;
	std::string write_contacts_chain_level_to_file;
	std::string write_cells_to_file;
	std::string write_cells_residue_level_to_file;
	std::string write_cells_chain_level_to_file;
	std::string write_sites_to_file;
	std::string write_sites_residue_level_to_file;
	std::string write_sites_chain_level_to_file;
	std::string plot_contacts_to_file;
	std::string plot_contacts_residue_level_to_file;
	std::string plot_contacts_chain_level_to_file;
	std::set<std::string> plot_options_config;
	std::string graphics_output_file_for_pymol;
	std::string graphics_output_file_for_chimera;
	std::string graphics_title;
	std::set<std::string> graphics_restrict_representations;
	std::string graphics_coloring_config;
	std::string graphics_coloring_config_file;
	std::string sites_view_script_for_pymol;
	std::string sites_view_script_for_chimerax;
	std::string mesh_output_obj_file;
	std::string write_tessellation_edges_to_file;
	std::string write_tessellation_vertices_to_file;
	std::string write_raw_collisions_to_file;
	std::string write_log_to_file;
	std::string restrict_input_balls;
	std::string restrict_contacts;
	std::string restrict_contacts_for_output;
	std::string restrict_single_index_data_for_output;
	voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_input_balls;
	voronotalt::FilteringBySphereLabels::ExpressionForPair filtering_expression_for_restricting_collisions;
	voronotalt::FilteringBySphereLabels::ExpressionForPair filtering_expression_for_restricting_contacts_for_output;
	voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_balls_and_cells_for_output;
	voronotalt::ColorAssigner color_assigner;
	voronotalt::GroupChainsAssigner group_chains_assigner;
	std::ostringstream error_log_for_options_parsing;

	ApplicationParameters() noexcept :
		max_number_of_processors(voronotalt::openmp_enabled() ? 2 : 1),
		probe(1.4),
		compute_only_inter_residue_contacts(false),
		compute_only_inter_chain_contacts(false),
		pdb_or_mmcif_heteroatoms(false),
		pdb_or_mmcif_hydrogens(false),
		pdb_or_mmcif_as_assembly(false),
		measure_running_time(false),
		print_contacts(false),
		print_contacts_residue_level(false),
		print_contacts_chain_level(false),
		print_cells(false),
		print_cells_residue_level(false),
		print_cells_chain_level(false),
		print_sites(false),
		print_sites_residue_level(false),
		print_sites_chain_level(false),
		print_everything(false),
		plots_colorable(false),
		need_sites(false),
		need_summaries_on_residue_level(false),
		need_summaries_on_chain_level(false),
		running_mode(RunningMode::radical),
		mesh_extract_connected_component(0),
		mesh_print_topology_summary(false),
		exit_before_calculations(false),
		minimum_columns(false),
		quiet(false),
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
				else if(opt.name=="processors" && opt.args_ints.size()==1)
				{
					max_number_of_processors=static_cast<unsigned int>(opt.args_ints.front());
					if(voronotalt::openmp_enabled())
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
				else if((opt.name=="input" || opt.name=="i") && opt.args_strings.size()==1)
				{
					input_from_file=opt.args_strings.front();
				}
				else if(opt.name=="periodic-box-directions" && opt.args_doubles.size()==9)
				{
					periodic_box_directions.resize(3);
					periodic_box_directions[0].x=opt.args_doubles[0];
					periodic_box_directions[0].y=opt.args_doubles[1];
					periodic_box_directions[0].z=opt.args_doubles[2];
					periodic_box_directions[1].x=opt.args_doubles[3];
					periodic_box_directions[1].y=opt.args_doubles[4];
					periodic_box_directions[1].z=opt.args_doubles[5];
					periodic_box_directions[2].x=opt.args_doubles[6];
					periodic_box_directions[2].y=opt.args_doubles[7];
					periodic_box_directions[2].z=opt.args_doubles[8];
				}
				else if(opt.name=="periodic-box-corners" && opt.args_doubles.size()==6)
				{
					periodic_box_corners.resize(2);
					periodic_box_corners[0].x=opt.args_doubles[0];
					periodic_box_corners[0].y=opt.args_doubles[1];
					periodic_box_corners[0].z=opt.args_doubles[2];
					periodic_box_corners[1].x=opt.args_doubles[3];
					periodic_box_corners[1].y=opt.args_doubles[4];
					periodic_box_corners[1].z=opt.args_doubles[5];
				}
				else if(opt.name=="compute-only-inter-residue-contacts" && opt.is_flag())
				{
					compute_only_inter_residue_contacts=opt.is_flag_and_true();
				}
				else if(opt.name=="compute-only-inter-chain-contacts" && opt.is_flag())
				{
					compute_only_inter_chain_contacts=opt.is_flag_and_true();
				}
				else if(opt.name=="run-in-aw-diagram-regime" && opt.is_flag())
				{
					if(opt.is_flag_and_true())
					{
						running_mode=RunningMode::simplified_aw;
					}
				}
				else if(opt.name=="pdb-or-mmcif-heteroatoms" && opt.is_flag())
				{
					pdb_or_mmcif_heteroatoms=opt.is_flag_and_true();
				}
				else if(opt.name=="pdb-or-mmcif-hydrogens" && opt.is_flag())
				{
					pdb_or_mmcif_hydrogens=opt.is_flag_and_true();
				}
				else if(opt.name=="pdb-or-mmcif-join-models" && opt.is_flag())
				{
					pdb_or_mmcif_as_assembly=opt.is_flag_and_true();
				}
				else if(opt.name=="pdb-or-mmcif-radii-config-file" && opt.args_strings.size()==1)
				{
					pdb_or_mmcif_radii_config_file=opt.args_strings.front();
				}
				else if(opt.name=="restrict-input-balls" && opt.args_strings.size()==1)
				{
					restrict_input_balls=opt.args_strings.front();
				}
				else if(opt.name=="restrict-contacts" && opt.args_strings.size()==1)
				{
					restrict_contacts=opt.args_strings.front();
				}
				else if(opt.name=="restrict-contacts-for-output" && opt.args_strings.size()==1)
				{
					restrict_contacts_for_output=opt.args_strings.front();
				}
				else if(opt.name=="restrict-single-index-data-for-output" && opt.args_strings.size()==1)
				{
					restrict_single_index_data_for_output=opt.args_strings.front();
				}
				else if(opt.name=="measure-running-time" && opt.is_flag())
				{
					measure_running_time=opt.is_flag_and_true();
				}
				else if(opt.name=="print-contacts" && opt.is_flag())
				{
					print_contacts=opt.is_flag_and_true();
				}
				else if(opt.name=="print-contacts-residue-level" && opt.is_flag())
				{
					print_contacts_residue_level=opt.is_flag_and_true();
				}
				else if(opt.name=="print-contacts-chain-level" && opt.is_flag())
				{
					print_contacts_chain_level=opt.is_flag_and_true();
				}
				else if(opt.name=="print-cells" && opt.is_flag())
				{
					print_cells=opt.is_flag_and_true();
				}
				else if(opt.name=="print-cells-residue-level" && opt.is_flag())
				{
					print_cells_residue_level=opt.is_flag_and_true();
				}
				else if(opt.name=="print-cells-chain-level" && opt.is_flag())
				{
					print_cells_chain_level=opt.is_flag_and_true();
				}
				else if(opt.name=="print-sites" && opt.is_flag())
				{
					print_sites=opt.is_flag_and_true();
				}
				else if(opt.name=="print-sites-residue-level" && opt.is_flag())
				{
					print_sites_residue_level=opt.is_flag_and_true();
				}
				else if(opt.name=="print-sites-chain-level" && opt.is_flag())
				{
					print_sites_chain_level=opt.is_flag_and_true();
				}
				else if(opt.name=="print-everything" && opt.is_flag())
				{
					print_everything=(opt.is_flag_and_true());
				}
				else if(opt.name=="grouping-directives" && opt.args_strings.size()==1)
				{
					grouping_directives=opt.args_strings.front();
				}
				else if(opt.name=="grouping-directives-file" && opt.args_strings.size()==1)
				{
					grouping_directives_file=opt.args_strings.front();
				}
				else if(opt.name=="write-input-balls-to-file" && opt.args_strings.size()==1)
				{
					write_input_balls_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-contacts-to-file" && opt.args_strings.size()==1)
				{
					write_contacts_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-contacts-residue-level-to-file" && opt.args_strings.size()==1)
				{
					write_contacts_residue_level_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-contacts-chain-level-to-file" && opt.args_strings.size()==1)
				{
					write_contacts_chain_level_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-cells-to-file" && opt.args_strings.size()==1)
				{
					write_cells_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-cells-residue-level-to-file" && opt.args_strings.size()==1)
				{
					write_cells_residue_level_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-cells-chain-level-to-file" && opt.args_strings.size()==1)
				{
					write_cells_chain_level_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-sites-to-file" && opt.args_strings.size()==1)
				{
					write_sites_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-sites-residue-level-to-file" && opt.args_strings.size()==1)
				{
					write_sites_residue_level_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-sites-chain-level-to-file" && opt.args_strings.size()==1)
				{
					write_sites_chain_level_to_file=opt.args_strings.front();
				}
				else if(opt.name=="sites-view-script-for-pymol" && opt.args_strings.size()==1)
				{
					sites_view_script_for_pymol=opt.args_strings.front();
				}
				else if(opt.name=="sites-view-script-for-chimerax" && opt.args_strings.size()==1)
				{
					sites_view_script_for_chimerax=opt.args_strings.front();
				}
				else if(opt.name=="plot-contacts-to-file" && opt.args_strings.size()==1)
				{
					plot_contacts_to_file=opt.args_strings.front();
				}
				else if(opt.name=="plot-contacts-residue-level-to-file" && opt.args_strings.size()==1)
				{
					plot_contacts_residue_level_to_file=opt.args_strings.front();
				}
				else if(opt.name=="plot-contacts-chain-level-to-file" && opt.args_strings.size()==1)
				{
					plot_contacts_chain_level_to_file=opt.args_strings.front();
				}
				else if(opt.name=="plot-options-config" && !opt.args_strings.empty())
				{
					plot_options_config=std::set<std::string>(opt.args_strings.begin(), opt.args_strings.end());
				}
				else if(opt.name=="graphics-output-file-for-pymol" && opt.args_strings.size()==1)
				{
					graphics_output_file_for_pymol=opt.args_strings.front();
				}
				else if(opt.name=="graphics-output-file-for-chimera" && opt.args_strings.size()==1)
				{
					graphics_output_file_for_chimera=opt.args_strings.front();
				}
				else if(opt.name=="graphics-title" && opt.args_strings.size()==1)
				{
					graphics_title=opt.args_strings.front();
				}
				else if(opt.name=="graphics-restrict-representations" && !opt.args_strings.empty())
				{
					graphics_restrict_representations=std::set<std::string>(opt.args_strings.begin(), opt.args_strings.end());
				}
				else if(opt.name=="graphics-coloring-config" && opt.args_strings.size()==1)
				{
					graphics_coloring_config=opt.args_strings.front();
				}
				else if(opt.name=="graphics-coloring-config-file" && opt.args_strings.size()==1)
				{
					graphics_coloring_config_file=opt.args_strings.front();
				}
				else if(opt.name=="mesh-output-obj-file" && opt.args_strings.size()==1)
				{
					mesh_output_obj_file=opt.args_strings.front();
				}
				else if(opt.name=="mesh-print-topology-summary" && opt.is_flag())
				{
					mesh_print_topology_summary=opt.is_flag_and_true();
				}
				else if(opt.name=="exit-before-calculations" && opt.is_flag())
				{
					exit_before_calculations=opt.is_flag_and_true();
				}
				else if((opt.name=="minimum-columns" || opt.name=="m") && opt.is_flag())
				{
					minimum_columns=opt.is_flag_and_true();
				}
				else if((opt.name=="quiet" || opt.name=="q") && opt.is_flag())
				{
					quiet=opt.is_flag_and_true();
				}
				else if(opt.name=="mesh-extract-connected-component" && opt.args_ints.size()==1)
				{
					mesh_extract_connected_component=static_cast<long>(opt.args_ints.front());
				}
				else if(opt.name=="write-tessellation-edges-to-file" && opt.args_strings.size()==1)
				{
					write_tessellation_edges_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-tessellation-vertices-to-file" && opt.args_strings.size()==1)
				{
					write_tessellation_vertices_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-raw-collisions-to-file" && opt.args_strings.size()==1)
				{
					write_raw_collisions_to_file=opt.args_strings.front();
				}
				else if(opt.name=="write-log-to-file" && opt.args_strings.size()==1)
				{
					write_log_to_file=opt.args_strings.front();
				}
				else if(opt.name.empty() && i==0 && opt.args_strings.size()==1)
				{
					input_from_file=opt.args_strings.front();
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

		if(print_everything)
		{
			print_contacts=true;
			print_contacts_residue_level=true;
			print_contacts_chain_level=true;
			if(running_mode==RunningMode::radical)
			{
				print_cells=true;
				print_cells_residue_level=true;
				print_cells_chain_level=true;
				print_sites=true;
				print_sites_residue_level=true;
				print_sites_chain_level=true;
			}
		}

		if((input_from_file.empty() || input_from_file=="_stdin") && voronotalt::is_stdin_from_terminal())
		{
			error_log_for_options_parsing << "Error: no input provided to stdin or from a file, please provide input or run with an -h or --help flag to see documentation and examples.\n";
		}

		if(running_mode==RunningMode::simplified_aw && !(periodic_box_directions.empty() && periodic_box_corners.empty()))
		{
			error_log_for_options_parsing << "Error: in this version a periodic box cannot be used in the simplified additively weighted Voronoi diagram regime.\n";
		}

		if(running_mode==RunningMode::simplified_aw && !(!print_cells && !print_cells_residue_level && !print_cells_chain_level && write_cells_to_file.empty() && write_cells_residue_level_to_file.empty() && write_cells_chain_level_to_file.empty()))
		{
			error_log_for_options_parsing << "Error: in this version cells output is disabled for the simplified additively weighted Voronoi diagram regime.\n";
		}

		if(running_mode==RunningMode::simplified_aw && !(!print_sites && !print_sites_residue_level && !print_sites_chain_level && write_sites_to_file.empty() && write_sites_residue_level_to_file.empty() && write_sites_chain_level_to_file.empty() && sites_view_script_for_pymol.empty() && sites_view_script_for_chimerax.empty()))
		{
			error_log_for_options_parsing << "Error: in this version sites output is disabled for the simplified additively weighted Voronoi diagram regime.\n";
		}

		if(running_mode==RunningMode::simplified_aw && (!mesh_output_obj_file.empty() || mesh_print_topology_summary))
		{
			error_log_for_options_parsing << "Error: in this version mesh output and analysis is disabled for the simplified additively weighted Voronoi diagram regime.\n";
		}

		if(running_mode==RunningMode::simplified_aw && !(write_tessellation_edges_to_file.empty() && write_tessellation_vertices_to_file.empty()))
		{
			error_log_for_options_parsing << "Error: in this version tessellation edges and vertices output is disabled for the simplified additively weighted Voronoi diagram regime.\n";
		}

		if(running_mode==RunningMode::simplified_aw && !write_raw_collisions_to_file.empty())
		{
			error_log_for_options_parsing << "Error: in this version raw collisions output is disabled for the simplified additively weighted Voronoi diagram regime.\n";
		}

		if(!periodic_box_directions.empty() && !periodic_box_corners.empty())
		{
			error_log_for_options_parsing << "Error: cannot use both the periodic box directions and the periodic box corners.\n";
		}

		if(!periodic_box_directions.empty() && periodic_box_directions.size()!=3)
		{
			error_log_for_options_parsing << "Error: not exactly three periodic box directions provided.\n";
		}

		if(!periodic_box_corners.empty() && periodic_box_corners.size()!=2)
		{
			error_log_for_options_parsing << "Error: not exactly two periodic box corners provided.\n";
		}

		if((!graphics_output_file_for_pymol.empty() || !graphics_output_file_for_chimera.empty()) && graphics_restrict_representations.empty())
		{
			graphics_restrict_representations.insert("faces");
			graphics_restrict_representations.insert("wireframe");
			if(!periodic_box_directions.empty() || !periodic_box_corners.empty())
			{
				graphics_restrict_representations.insert("lattice");
			}
		}

		plots_colorable=(!plot_contacts_to_file.empty() || !plot_contacts_residue_level_to_file.empty() || !plot_contacts_chain_level_to_file.empty()) && plot_options_config.count("colored")>0;

		if(!graphics_output_file_for_pymol.empty() || !graphics_output_file_for_chimera.empty() || plots_colorable)
		{
			color_assigner.add_rule("contact", "faces", 0xFFFF00);
			color_assigner.add_rule("contact", "wireframe", 0x808080);
			color_assigner.add_rule("ball", "balls", 0x00FFFF);
			color_assigner.add_rule("ball", "sas", 0x00FF00);
			color_assigner.add_rule("ball", "sasmesh", 0x808080);
			color_assigner.add_rule("other", "lattice", 0x00FF00);
			color_assigner.add_rule("other", "truecollisions", 0x3030FF);
			color_assigner.add_rule("other", "falsecollisions", 0xFFA0FF);
		}

		if(!restrict_input_balls.empty())
		{
			filtering_expression_for_restricting_input_balls=voronotalt::FilteringBySphereLabels::ExpressionForSingle(restrict_input_balls);
			if(!filtering_expression_for_restricting_input_balls.valid())
			{
				error_log_for_options_parsing << "Error: invalid input balls filtering expression.\n";
			}
		}

		if(!restrict_contacts.empty())
		{
			filtering_expression_for_restricting_collisions=voronotalt::FilteringBySphereLabels::ExpressionForPair(restrict_contacts);
			if(!filtering_expression_for_restricting_collisions.valid())
			{
				error_log_for_options_parsing << "Error: invalid contacts restriction filtering expression.\n";
			}
		}

		if(!restrict_contacts_for_output.empty())
		{
			filtering_expression_for_restricting_contacts_for_output=voronotalt::FilteringBySphereLabels::ExpressionForPair(restrict_contacts_for_output);
			if(!filtering_expression_for_restricting_contacts_for_output.valid())
			{
				error_log_for_options_parsing << "Error: invalid contacts restriction filtering expression for contacts output.\n";
			}
		}

		if(!restrict_single_index_data_for_output.empty())
		{
			filtering_expression_for_restricting_balls_and_cells_for_output=voronotalt::FilteringBySphereLabels::ExpressionForSingle(restrict_single_index_data_for_output);
			if(!filtering_expression_for_restricting_balls_and_cells_for_output.valid())
			{
				error_log_for_options_parsing << "Error: invalid cells restriction filtering expression for cells output.\n";
			}
		}

		read_successfuly=error_log_for_options_parsing.str().empty();

		if(read_successfuly)
		{
			need_sites=(print_sites || print_sites_residue_level || print_sites_chain_level || !write_sites_to_file.empty() || !write_sites_residue_level_to_file.empty() || !write_sites_chain_level_to_file.empty() || !sites_view_script_for_pymol.empty() || !sites_view_script_for_chimerax.empty());
			need_summaries_on_residue_level=(print_contacts_residue_level || print_cells_residue_level || print_sites_residue_level || !write_contacts_residue_level_to_file.empty() || !write_cells_residue_level_to_file.empty() || !write_sites_residue_level_to_file.empty() || !sites_view_script_for_pymol.empty() || !sites_view_script_for_chimerax.empty() || !plot_contacts_residue_level_to_file.empty());
			need_summaries_on_chain_level=(print_contacts_chain_level || print_cells_chain_level || print_sites_chain_level || !write_contacts_chain_level_to_file.empty() || !write_cells_chain_level_to_file.empty() || !write_sites_chain_level_to_file.empty() || !plot_contacts_chain_level_to_file.empty());
		}

		return read_successfuly;
	}

	bool contains_complex_filtering_expressions() const noexcept
	{
		return(!filtering_expression_for_restricting_input_balls.allow_all()
				|| !filtering_expression_for_restricting_collisions.allow_all()
				|| !filtering_expression_for_restricting_contacts_for_output.allow_all()
				|| !filtering_expression_for_restricting_balls_and_cells_for_output.allow_all());
	}
};

class ApplicationLogRecorders
{
public:
	voronotalt::TimeRecorderChrono time_recoder_for_all;
	voronotalt::TimeRecorderChrono time_recoder_for_input;
	voronotalt::TimeRecorderChrono time_recoder_for_tessellation;
	voronotalt::TimeRecorderChrono time_recoder_for_output;
	std::ostringstream log_output;

	explicit ApplicationLogRecorders(const ApplicationParameters& app_params) noexcept :
		time_recoder_for_all(app_params.measure_running_time),
		time_recoder_for_input(app_params.measure_running_time),
		time_recoder_for_tessellation(app_params.measure_running_time),
		time_recoder_for_output(app_params.measure_running_time)
	{
	}

	template<class Result, class GroupedResult>
	void print_tessellation_full_construction_result_log_basic(const Result& result, const GroupedResult& result_grouped_by_residue, const GroupedResult& result_grouped_by_chain) noexcept
	{
		log_output << "log_total_input_balls\t" << result.total_spheres << "\n";
		log_output << "log_total_collisions\t" << result.total_collisions << "\n";
		log_output << "log_total_relevant_collisions\t" << result.total_relevant_collisions << "\n";
		log_output << "log_total_contacts_count\t" << result.total_contacts_summary.count << "\n";
		log_output << "log_total_contacts_area\t" << result.total_contacts_summary.area << "\n";
		log_output << "log_total_residue_level_contacts_count\t" << result_grouped_by_residue.grouped_contacts_summaries.size() << "\n";
		log_output << "log_total_chain_level_contacts_count\t" << result_grouped_by_chain.grouped_contacts_summaries.size() << "\n";
	}

	template<class Result, class GroupedResult>
	void print_tessellation_full_construction_result_log_about_cells(const Result& result, const GroupedResult& result_grouped_by_residue, const GroupedResult& result_grouped_by_chain) noexcept
	{
		log_output << "log_total_cells_count\t" << result.total_cells_summary.count << "\n";
		log_output << "log_total_cells_sas_area\t" << result.total_cells_summary.sas_area << "\n";
		log_output << "log_total_cells_sas_inside_volume\t" << result.total_cells_summary.sas_inside_volume << "\n";
		log_output << "log_total_residue_level_cells_count\t" << result_grouped_by_residue.grouped_cells_summaries.size() << "\n";
		log_output << "log_total_chain_level_cells_count\t" << result_grouped_by_chain.grouped_cells_summaries.size() << "\n";
	}

	void finalize_and_output(const ApplicationParameters& app_params) noexcept
	{
		if(app_params.measure_running_time)
		{
			log_output << "log_threads\t" << voronotalt::openmp_get_max_threads() << "\n";
			time_recoder_for_input.print_recordings(log_output, "log time input stage", true);
			time_recoder_for_tessellation.print_recordings(log_output, "log time tessellation stage", true);
			time_recoder_for_output.print_recordings(log_output, "log time output stage", true);
			time_recoder_for_all.print_elapsed_time(log_output, "log time full program");
		}

		if(!app_params.quiet)
		{
			std::cerr << log_output.str();
		}

		if(!app_params.write_log_to_file.empty())
		{
			std::ofstream foutput(app_params.write_log_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				foutput << log_output.str();
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write log to file '" << app_params.write_log_to_file << "'\n";
			}
		}
	}
};

class ApplicationGraphicsRecorder
{
public:
	static inline bool allow_representation(const std::set<std::string>& restrict_representations, const std::string& representation) noexcept
	{
		return (restrict_representations.empty() || restrict_representations.count(representation)>0);
	}

	static inline std::string name_ball_group(const std::string& prefix, const voronotalt::SpheresInput::Result& spheres_input_result, const std::size_t index) noexcept
	{
		std::ostringstream output;
		output << prefix;
		if(!spheres_input_result.sphere_labels.empty())
		{
			const voronotalt::UnsignedInt N=spheres_input_result.sphere_labels.size();
			output << "_" << spheres_input_result.sphere_labels[index%N].chain_id;
		}
		return output.str();
	}

	static inline std::string name_contact_group(const std::string& prefix, const voronotalt::SpheresInput::Result& spheres_input_result, const std::size_t index1, const std::size_t index2) noexcept
	{
		std::ostringstream output;
		output << prefix;
		if(!spheres_input_result.sphere_labels.empty())
		{
			const voronotalt::UnsignedInt N=spheres_input_result.sphere_labels.size();
			const bool need_to_swap=(spheres_input_result.sphere_labels[index2%N].chain_id<spheres_input_result.sphere_labels[index1%N].chain_id);
			output << "_" << spheres_input_result.sphere_labels[(need_to_swap? index2 : index1)%N].chain_id;
			output << "_" << spheres_input_result.sphere_labels[(need_to_swap? index1 : index2)%N].chain_id;
		}
		return output.str();
	}

	voronotalt::GraphicsWriter graphics_writer;

	explicit ApplicationGraphicsRecorder(const ApplicationParameters& app_params) noexcept : graphics_writer(!app_params.graphics_output_file_for_pymol.empty() || !app_params.graphics_output_file_for_chimera.empty())
	{
	}

	void finalize_and_output(const ApplicationParameters& app_params, ApplicationLogRecorders& app_log_recorders) const noexcept
	{
		if(graphics_writer.enabled())
		{
			app_log_recorders.time_recoder_for_output.reset();
			if(!app_params.graphics_output_file_for_pymol.empty())
			{
				if(!graphics_writer.write_to_file_for_pymol(app_params.graphics_title, app_params.graphics_output_file_for_pymol))
				{
					std::cerr << "Error (non-terminating): failed to write graphics for PyMol to file '" << app_params.graphics_output_file_for_pymol << "'\n";
				}
				app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("write printed graphics for pymol to file");
			}
			if(!app_params.graphics_output_file_for_chimera.empty())
			{
				if(!graphics_writer.write_to_file_for_chimera(app_params.graphics_title, app_params.graphics_output_file_for_chimera))
				{
					std::cerr << "Error (non-terminating): failed to write graphics for Chimera to file '" << app_params.graphics_output_file_for_chimera << "'\n";
				}
				app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("write printed graphics for chimera to file");
			}
		}
	}
};

class ApplicationMeshRecorder
{
public:
	voronotalt::MeshWriter mesh_writer;

	explicit ApplicationMeshRecorder(const ApplicationParameters& app_params) noexcept : mesh_writer(!app_params.mesh_output_obj_file.empty() || app_params.mesh_print_topology_summary)
	{
	}

	void finalize_and_output(const ApplicationParameters& app_params, ApplicationLogRecorders& app_log_recorders) noexcept
	{
		if(mesh_writer.enabled())
		{
			app_log_recorders.time_recoder_for_output.reset();
			if(!app_params.mesh_output_obj_file.empty())
			{
				if(!mesh_writer.write_to_obj_file(app_params.mesh_output_obj_file))
				{
					std::cerr << "Error (non-terminating): failed to write mesh to file '" << app_params.mesh_output_obj_file << "'\n";
				}
			}
			if(app_params.mesh_print_topology_summary)
			{
				std::cout << "meshinfo_header\tgenus\tconnected_components\tboundary_components\teuler_characteristic\n"
						<< "meshinfo\t"
						<< mesh_writer.calculate_genus() << "\t"
						<< mesh_writer.get_number_of_connected_components() << "\t"
						<< mesh_writer.get_number_of_boundary_components() << "\t"
						<< mesh_writer.get_euler_characteristic() << "\n";
			}
			app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("write mesh obj to file");
		}
	}
};

void run_mode_radical(
		const ApplicationParameters& app_params,
		const voronotalt::SpheresInput::Result& spheres_input_result,
		ApplicationLogRecorders& app_log_recorders,
		ApplicationGraphicsRecorder& app_graphics_recorder,
		ApplicationMeshRecorder& app_mesh_recorder) noexcept
{
	app_log_recorders.time_recoder_for_tessellation.reset();

	const voronotalt::PeriodicBox periodic_box=voronotalt::PeriodicBox::create_periodic_box_from_shift_directions_or_from_corners(app_params.periodic_box_directions, app_params.periodic_box_corners);

	voronotalt::SpheresContainer::ResultOfPreparationForTessellation preparation_result;

	voronotalt::RadicalTessellation::Result result;
	voronotalt::RadicalTessellation::ResultGraphics result_graphics;

	{
		const std::vector<int> null_grouping;
		const std::vector<int>& grouping_for_filtering=(app_params.compute_only_inter_chain_contacts ? spheres_input_result.grouping_by_chain : (app_params.compute_only_inter_residue_contacts ? spheres_input_result.grouping_by_residue : null_grouping));

		voronotalt::SpheresContainer spheres_container;
		spheres_container.init(spheres_input_result.spheres, periodic_box, app_log_recorders.time_recoder_for_tessellation);

		spheres_container.prepare_for_tessellation(grouping_for_filtering, preparation_result, app_log_recorders.time_recoder_for_tessellation);

		if(!app_params.filtering_expression_for_restricting_collisions.allow_all())
		{
			const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=app_params.filtering_expression_for_restricting_collisions.filter_vector(spheres_input_result.sphere_labels, spheres_input_result.spheres, preparation_result.relevant_collision_ids);
			if(!ver.expression_matched() || !preparation_result.restrict_relevant_collision_ids(ver.expression_matched_all, ver.expression_matched_ids))
			{
				std::cerr << "Error: failed to restrict contacts for construction\n";
				return;
			}
			app_log_recorders.time_recoder_for_tessellation.record_elapsed_miliseconds_and_reset("restrict collisions for construction using filtering expression");
		}

		const bool summarize_sites=app_params.need_sites;
		const bool summarize_cells=!preparation_result.collision_ids_constrained;

		const bool with_tessellation_net=!(app_params.write_tessellation_edges_to_file.empty() && app_params.write_tessellation_vertices_to_file.empty());
		const bool with_graphics=(app_graphics_recorder.graphics_writer.enabled() || app_mesh_recorder.mesh_writer.enabled());
		const bool with_sas_graphics_if_possible=(app_graphics_recorder.graphics_writer.enabled() && summarize_cells && (ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "sas") || ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "sasmesh") || ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "vsticks")));

		voronotalt::RadicalTessellation::construct_full_tessellation(
				spheres_container,
				preparation_result,
				with_tessellation_net,
				voronotalt::RadicalTessellation::ParametersForGraphics(with_graphics, with_sas_graphics_if_possible),
				voronotalt::RadicalTessellation::ParametersForGeneratingSummaries(summarize_sites, summarize_cells),
				result,
				result_graphics,
				app_log_recorders.time_recoder_for_tessellation);

		if(!app_params.filtering_expression_for_restricting_contacts_for_output.allow_all())
		{
			const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=app_params.filtering_expression_for_restricting_contacts_for_output.filter_vector(spheres_input_result.sphere_labels, spheres_input_result.spheres, voronotalt::FilteringBySphereLabels::ExpressionForPair::adapt_indices_container(result.contacts_summaries));
			if(!ver.expression_matched() || !voronotalt::RadicalTessellation::restrict_result_contacts(ver.expression_matched_all, ver.expression_matched_ids, result, result_graphics))
			{
				std::cerr << "Error: failed to restrict contacts for output\n";
				return;
			}
			app_log_recorders.time_recoder_for_tessellation.record_elapsed_miliseconds_and_reset("restrict contacts for output using filtering expression");
		}

		if(!result.cells_summaries.empty() && !app_params.filtering_expression_for_restricting_balls_and_cells_for_output.allow_all())
		{
			const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=app_params.filtering_expression_for_restricting_balls_and_cells_for_output.filter_vector(spheres_input_result.sphere_labels);
			if(!ver.expression_matched() || !voronotalt::RadicalTessellation::restrict_result_cells(ver.expression_matched_all, ver.expression_matched_ids, result, result_graphics))
			{
				std::cerr << "Error: failed to restrict cells for output\n";
				return;
			}
			app_log_recorders.time_recoder_for_tessellation.record_elapsed_miliseconds_and_reset("restrict cells for output using filtering expression");
		}

		if(!result.sites_summaries.empty() && !app_params.filtering_expression_for_restricting_balls_and_cells_for_output.allow_all())
		{
			const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=app_params.filtering_expression_for_restricting_balls_and_cells_for_output.filter_vector(spheres_input_result.sphere_labels);
			if(!ver.expression_matched() || !voronotalt::RadicalTessellation::restrict_result_sites(ver.expression_matched_all, ver.expression_matched_ids, result))
			{
				std::cerr << "Error: failed to restrict sites for output\n";
				return;
			}
			app_log_recorders.time_recoder_for_tessellation.record_elapsed_miliseconds_and_reset("restrict sites for output using filtering expression");
		}
	}

	voronotalt::RadicalTessellation::GroupedResult result_grouped_by_residue;
	if(app_params.need_summaries_on_residue_level)
	{
		voronotalt::RadicalTessellation::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, app_log_recorders.time_recoder_for_tessellation);
	}

	voronotalt::RadicalTessellation::GroupedResult result_grouped_by_chain;
	if(app_params.need_summaries_on_chain_level)
	{
		voronotalt::RadicalTessellation::group_results(result, spheres_input_result.grouping_by_chain, result_grouped_by_chain, app_log_recorders.time_recoder_for_tessellation);
	}

	app_log_recorders.time_recoder_for_output.reset();

	std::map< std::pair<voronotalt::UnsignedInt, voronotalt::UnsignedInt>, std::pair<bool, voronotalt::UnsignedInt> > map_of_raw_collisions_to_contact_ids;

	if(!app_params.write_raw_collisions_to_file.empty() || ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "truecollisions") || ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "falsecollisions"))
	{
		for(voronotalt::UnsignedInt i=0;i<preparation_result.relevant_collision_ids.size();i++)
		{
			std::pair<voronotalt::UnsignedInt, voronotalt::UnsignedInt> collision_id=preparation_result.relevant_collision_ids[i];
			if(collision_id.first>collision_id.second)
			{
				std::swap(collision_id.first, collision_id.second);
			}
			std::pair<bool, voronotalt::UnsignedInt>& contact_id=map_of_raw_collisions_to_contact_ids[collision_id];
			contact_id.first=false;
			contact_id.second=0;
		}
		for(voronotalt::UnsignedInt i=0;i<result.contacts_summaries.size();i++)
		{
			const voronotalt::RadicalTessellation::ContactDescriptorSummary& cd=result.contacts_summaries[i];
			std::pair<voronotalt::UnsignedInt, voronotalt::UnsignedInt> collision_id(cd.id_a, cd.id_b);
			if(collision_id.first>collision_id.second)
			{
				std::swap(collision_id.first, collision_id.second);
			}
			std::pair<bool, voronotalt::UnsignedInt>& contact_id=map_of_raw_collisions_to_contact_ids[collision_id];
			contact_id.first=true;
			contact_id.second=i;
		}
		app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("collect true and false raw collisions");
	}

	app_log_recorders.print_tessellation_full_construction_result_log_basic(result, result_grouped_by_residue, result_grouped_by_chain);
	app_log_recorders.print_tessellation_full_construction_result_log_about_cells(result, result_grouped_by_residue, result_grouped_by_chain);

	app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("print total numbers");

	if(app_params.print_contacts || !app_params.write_contacts_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_contacts(result.contacts_summaries, spheres_input_result.sphere_labels, true, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_contacts)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_contacts_to_file.empty())
			{
				std::ofstream foutput(app_params.write_contacts_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write contacts to file '" << app_params.write_contacts_to_file << "'\n";
				}
			}
		}
	}

	if(app_params.print_contacts_residue_level || !app_params.write_contacts_residue_level_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_contacts_residue_level(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, app_params.minimum_columns, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_contacts_residue_level)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_contacts_residue_level_to_file.empty())
			{
				std::ofstream foutput(app_params.write_contacts_residue_level_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write contacts on residue level to file '" << app_params.write_contacts_residue_level_to_file << "'\n";
				}
			}
		}
	}

	if(app_params.print_contacts_chain_level || !app_params.write_contacts_chain_level_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_contacts_chain_level(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, app_params.minimum_columns, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_contacts_chain_level)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_contacts_chain_level_to_file.empty())
			{
				std::ofstream foutput(app_params.write_contacts_chain_level_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write contacts on chain level to file '" << app_params.write_contacts_chain_level_to_file << "'\n";
				}
			}
		}
	}

	app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result contacts");

	if(app_params.print_cells || !app_params.write_cells_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_cells(result.cells_summaries, spheres_input_result.sphere_labels, true, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_cells)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_cells_to_file.empty())
			{
				std::ofstream foutput(app_params.write_cells_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write cells to file '" << app_params.write_cells_to_file << "'\n";
				}
			}
		}
	}

	if(app_params.print_cells_residue_level || !app_params.write_cells_residue_level_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_cells_residue_level(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_cells_representative_ids, result_grouped_by_residue.grouped_cells_summaries, app_params.minimum_columns, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_cells_residue_level)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_cells_residue_level_to_file.empty())
			{
				std::ofstream foutput(app_params.write_cells_residue_level_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write cells on residue level to file '" << app_params.write_cells_residue_level_to_file << "'\n";
				}
			}
		}
	}

	if(app_params.print_cells_chain_level || !app_params.write_cells_chain_level_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_cells_chain_level(result.cells_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_cells_representative_ids, result_grouped_by_chain.grouped_cells_summaries, app_params.minimum_columns, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_cells_chain_level)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_cells_chain_level_to_file.empty())
			{
				std::ofstream foutput(app_params.write_cells_chain_level_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write cells on chain level to file '" << app_params.write_cells_chain_level_to_file << "'\n";
				}
			}
		}
	}

	app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result cells sas and volumes");

	if(app_params.print_sites || !app_params.write_sites_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_sites(result.sites_summaries, spheres_input_result.sphere_labels, true, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_sites)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_sites_to_file.empty())
			{
				std::ofstream foutput(app_params.write_sites_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write bsites to file '" << app_params.write_sites_to_file << "'\n";
				}
			}
		}
	}

	if(app_params.print_sites_residue_level || !app_params.write_sites_residue_level_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_sites_residue_level(result.sites_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_sites_representative_ids, result_grouped_by_residue.grouped_sites_summaries, app_params.minimum_columns, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_sites_residue_level)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_sites_residue_level_to_file.empty())
			{
				std::ofstream foutput(app_params.write_sites_residue_level_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write bsites on residue level to file '" << app_params.write_sites_residue_level_to_file << "'\n";
				}
			}
		}
	}

	if(app_params.print_sites_chain_level || !app_params.write_sites_chain_level_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_sites_chain_level(result.sites_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_sites_representative_ids, result_grouped_by_chain.grouped_sites_summaries, app_params.minimum_columns, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_sites_chain_level)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_sites_chain_level_to_file.empty())
			{
				std::ofstream foutput(app_params.write_sites_chain_level_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write bsites on chain level to file '" << app_params.write_sites_chain_level_to_file << "'\n";
				}
			}
		}
	}

	if(!app_params.sites_view_script_for_pymol.empty())
	{
		std::ofstream foutput(app_params.sites_view_script_for_pymol.c_str(), std::ios::out);
		foutput << "select site_residues, none\n";
		for(std::size_t i=0;i<result_grouped_by_residue.grouped_sites_representative_ids.size();i++)
		{
			voronotalt::UnsignedInt id=result.sites_summaries[result_grouped_by_residue.grouped_sites_representative_ids[i]].id;
			if(id<spheres_input_result.sphere_labels.size())
			{
				const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[id];
				if(sl.expanded_residue_id.valid && !sl.chain_id.empty())
				{
					foutput << "select site_residues, site_residues or (chain " << sl.chain_id << " and resi " << sl.expanded_residue_id.rnum << ")\n";
				}
			}
		}
		foutput << "select site_atoms, none\n";
		for(std::size_t i=0;i<result.sites_summaries.size();i++)
		{
			voronotalt::UnsignedInt id=result.sites_summaries[i].id;
			if(id<spheres_input_result.sphere_labels.size())
			{
				const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[id];
				if(sl.expanded_residue_id.valid && !sl.chain_id.empty() && !sl.atom_name.empty())
				{
					foutput << "select site_atoms, site_atoms or (chain " << sl.chain_id << " and resi " << sl.expanded_residue_id.rnum << " and name \"" << sl.atom_name << "\")\n";
				}
			}
		}
		foutput << "show sticks, site_residues\n";
		foutput << "color cyan, site_residues\n";
		foutput << "color red, site_atoms\n";
	}

	if(!app_params.sites_view_script_for_chimerax.empty())
	{
		std::ofstream foutput(app_params.sites_view_script_for_chimerax.c_str(), std::ios::out);
		foutput << "select clear\n";
		bool started=false;
		for(std::size_t i=0;i<result_grouped_by_residue.grouped_sites_representative_ids.size();i++)
		{
			voronotalt::UnsignedInt id=result.sites_summaries[result_grouped_by_residue.grouped_sites_representative_ids[i]].id;
			if(id<spheres_input_result.sphere_labels.size())
			{
				const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[id];
				if(sl.expanded_residue_id.valid && !sl.chain_id.empty())
				{
					foutput << "select " << (started ? "add" : "") << " /" << sl.chain_id << ":" << sl.expanded_residue_id.rnum << "\n";
					started=true;
				}
			}
		}
		foutput << "name site_residues sel\n";
		foutput << "show site_residues atoms\n";
		foutput << "color site_residues cyan\n";
		foutput << "select clear\n";
		started=false;
		for(std::size_t i=0;i<result.sites_summaries.size();i++)
		{
			voronotalt::UnsignedInt id=result.sites_summaries[i].id;
			if(id<spheres_input_result.sphere_labels.size())
			{
				const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[id];
				if(sl.expanded_residue_id.valid && !sl.chain_id.empty() && !sl.atom_name.empty())
				{
					foutput << "select " << (started ? "add" : "") << " /" << sl.chain_id << ":" << sl.expanded_residue_id.rnum << "@" << sl.atom_name << "\n";
					started=true;
				}
			}
		}
		foutput << "name site_atoms sel\n";
		foutput << "show site_atoms atoms\n";
		foutput << "color site_atoms red\n";
	}

	app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result sites");

	if(!app_params.write_tessellation_edges_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_sequential_container_simply(result.tessellation_net.tes_edges, output_string);
		if(!output_string.empty())
		{
			std::ofstream foutput(app_params.write_tessellation_edges_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write tessellation edges to file '" << app_params.write_tessellation_edges_to_file << "'\n";
			}
		}
		app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("write tessellation edges");
	}

	if(!app_params.write_tessellation_vertices_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_sequential_container_simply(result.tessellation_net.tes_vertices, output_string);
		if(!output_string.empty())
		{
			std::ofstream foutput(app_params.write_tessellation_vertices_to_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			else
			{
				std::cerr << "Error (non-terminating): failed to write tessellation vertices to file '" << app_params.write_tessellation_vertices_to_file << "'\n";
			}
		}
		app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("write tessellation vertices");
	}

	if(!app_params.write_raw_collisions_to_file.empty())
	{
		std::ofstream foutput(app_params.write_raw_collisions_to_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			foutput << "rawc_header";
			if(!spheres_input_result.sphere_labels.empty())
			{
				foutput << "\tID1_chain\tID1_residue\tID1_atom\tID2_chain\tID2_residue\tID2_atom";
			}
			foutput << "\tID1_index\tID2_index\tdistance\tdistance_vdw\tarea\tarc_length\n";
			for(std::map< std::pair<voronotalt::UnsignedInt, voronotalt::UnsignedInt>, std::pair<bool, voronotalt::UnsignedInt> >::const_iterator it=map_of_raw_collisions_to_contact_ids.begin();it!=map_of_raw_collisions_to_contact_ids.end();++it)
			{
				const voronotalt::UnsignedInt a=it->first.first;
				const voronotalt::UnsignedInt b=it->first.second;
				const voronotalt::Float distance=voronotalt::distance_from_point_to_point(spheres_input_result.spheres[a].p, spheres_input_result.spheres[b].p);
				const voronotalt::Float distance_vdw=distance-(spheres_input_result.spheres[a].r-app_params.probe)-(spheres_input_result.spheres[b].r-app_params.probe);
				foutput << "rawc";
				if(!spheres_input_result.sphere_labels.empty())
				{
					const voronotalt::SphereLabeling::SphereLabel& sl1=spheres_input_result.sphere_labels[a];
					const voronotalt::SphereLabeling::SphereLabel& sl2=spheres_input_result.sphere_labels[b];
					foutput << "\t" << sl1.chain_id << "\t" << sl1.residue_id << "\t" << sl1.atom_name;
					foutput << "\t" << sl2.chain_id << "\t" << sl2.residue_id << "\t" << sl2.atom_name;
				}
				foutput << "\t" << a << "\t" << b << "\t" << distance << "\t" << distance_vdw;
				const std::pair<bool, voronotalt::UnsignedInt>& contact_id=it->second;
				if(contact_id.first)
				{
					const voronotalt::RadicalTessellation::ContactDescriptorSummary& cd=result.contacts_summaries[contact_id.second];
					foutput << "\t" << cd.area << "\t" << cd.arc_length;
				}
				else
				{
					foutput << "\t0\t0";
				}
				foutput << "\n";
			}
		}
		else
		{
			std::cerr << "Error (non-terminating): failed to write raw collisions to file '" << app_params.write_raw_collisions_to_file << "'\n";
		}
	}

	if(!app_params.plot_contacts_to_file.empty() || !app_params.plot_contacts_residue_level_to_file.empty() || !app_params.plot_contacts_chain_level_to_file.empty())
	{
		for(int j=0;j<3;j++)
		{
			const std::string& outfile=(j==0 ? app_params.plot_contacts_to_file : (j==1 ? app_params.plot_contacts_residue_level_to_file : app_params.plot_contacts_chain_level_to_file));
			if(!outfile.empty())
			{
				voronotalt::ContactPlotter plotter(j==0 ? voronotalt::ContactPlotter::LevelMode::inter_atom : (j==1 ? voronotalt::ContactPlotter::LevelMode::inter_residue : voronotalt::ContactPlotter::LevelMode::inter_chain));
				bool all_good=true;
				for(std::size_t i=0;all_good && i<result.contacts_summaries.size();i++)
				{
					const unsigned int current_color=(app_params.plots_colorable ? app_params.color_assigner.get_color("faces", spheres_input_result.sphere_labels, spheres_input_result.spheres, 0xFF00FF, result.contacts_summaries[i].id_a, result.contacts_summaries[i].id_b) : static_cast<unsigned int>(0));
					all_good=all_good && plotter.add_contact(i, result.contacts_summaries, spheres_input_result.sphere_labels, current_color);
				}
				if(!all_good)
				{
					std::cerr << "Error (non-terminating): failed to plot contacts\n";
				}
				else if(!plotter.write_to_file(outfile, app_params.plot_options_config))
				{
					std::cerr << "Error (non-terminating): failed to write plot of contacts to file '" << outfile << "'\n";
				}
			}
		}
		app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("write contact map plots");
	}

	if(app_graphics_recorder.graphics_writer.enabled())
	{
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "balls"))
		{
			const unsigned int base_color=app_params.color_assigner.get_color("balls");
			app_graphics_recorder.graphics_writer.add_color("balls", "", base_color);
			for(std::size_t i=0;i<spheres_input_result.spheres.size();i++)
			{
				if(app_params.filtering_expression_for_restricting_balls_and_cells_for_output.allow_all() || (i<spheres_input_result.sphere_labels.size() && app_params.filtering_expression_for_restricting_balls_and_cells_for_output.filter(spheres_input_result.sphere_labels[i]).expression_matched))
				{
					const std::string group_name=ApplicationGraphicsRecorder::name_ball_group("atoms", spheres_input_result, i);
					const unsigned int current_color=app_params.color_assigner.get_color("balls", spheres_input_result.sphere_labels, base_color, i);
					app_graphics_recorder.graphics_writer.add_color("balls", group_name, current_color);
					app_graphics_recorder.graphics_writer.add_sphere("balls", group_name, spheres_input_result.spheres[i], app_params.probe);
				}
			}
		}
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "faces"))
		{
			const unsigned int base_color=app_params.color_assigner.get_color("faces");
			app_graphics_recorder.graphics_writer.add_color("faces", "", base_color);
			for(std::size_t i=0;i<result_graphics.contacts_graphics.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& pair_summary=(i<result.contacts_summaries_with_redundancy_in_periodic_box.size() ? result.contacts_summaries_with_redundancy_in_periodic_box[i] : result.contacts_summaries[i]);
				{
					const std::string group_name=ApplicationGraphicsRecorder::name_contact_group("contacts", spheres_input_result, pair_summary.id_a, pair_summary.id_b);
					const unsigned int current_color=app_params.color_assigner.get_color("faces", spheres_input_result.sphere_labels, spheres_input_result.spheres, base_color, pair_summary.id_a, pair_summary.id_b);
					app_graphics_recorder.graphics_writer.add_color("faces", group_name, current_color);
					const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result_graphics.contacts_graphics[i];
					app_graphics_recorder.graphics_writer.add_triangle_fan("faces", group_name, pair_graphics.outer_points, pair_graphics.barycenter, pair_graphics.plane_normal);
				}
			}
		}
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "wireframe"))
		{
			const unsigned int base_color=app_params.color_assigner.get_color("wireframe");
			app_graphics_recorder.graphics_writer.add_color("wireframe", "", base_color);
			for(std::size_t i=0;i<result_graphics.contacts_graphics.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& pair_summary=(i<result.contacts_summaries_with_redundancy_in_periodic_box.size() ? result.contacts_summaries_with_redundancy_in_periodic_box[i] : result.contacts_summaries[i]);
				{
					const std::string group_name=ApplicationGraphicsRecorder::name_contact_group("contacts", spheres_input_result, pair_summary.id_a, pair_summary.id_b);
					const unsigned int current_color=app_params.color_assigner.get_color("wireframe", spheres_input_result.sphere_labels, spheres_input_result.spheres, base_color, pair_summary.id_a, pair_summary.id_b);
					app_graphics_recorder.graphics_writer.add_color("wireframe", group_name, current_color);
					const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result_graphics.contacts_graphics[i];
					app_graphics_recorder.graphics_writer.add_line_loop("wireframe", group_name, pair_graphics.outer_points);
				}
			}
		}
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "sas"))
		{
			if(!result_graphics.sas_graphics.empty())
			{
				const unsigned int base_color=app_params.color_assigner.get_color("sas");
				app_graphics_recorder.graphics_writer.add_color("sas", "", base_color);
				for(std::size_t i=0;i<result_graphics.sas_graphics.size();i++)
				{
					const voronotalt::SubdividedIcosahedronCut::GraphicsBundle& gb=result_graphics.sas_graphics[i];
					const std::size_t sphere_id=result.cells_summaries[i].id;
					const std::string group_name=ApplicationGraphicsRecorder::name_ball_group("atoms", spheres_input_result, sphere_id);
					const unsigned int current_color=app_params.color_assigner.get_color("sas", spheres_input_result.sphere_labels, base_color, sphere_id);
					app_graphics_recorder.graphics_writer.add_color("sas", group_name, current_color);
					for(std::size_t j=0;j<gb.triples.size();j++)
					{
						const voronotalt::SubdividedIcosahedron::Triple& t=gb.triples[j];
						app_graphics_recorder.graphics_writer.add_triangle_on_sphere("sas", group_name, spheres_input_result.spheres[sphere_id].p, gb.vertices[t.ids[0]], gb.vertices[t.ids[1]], gb.vertices[t.ids[2]]);
					}
				}
			}
		}
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "sasmesh"))
		{
			if(!result_graphics.sas_graphics.empty())
			{
				const unsigned int base_color=app_params.color_assigner.get_color("sasmesh");
				app_graphics_recorder.graphics_writer.add_color("sasmesh", "", base_color);
				for(std::size_t i=0;i<result_graphics.sas_graphics.size();i++)
				{
					const voronotalt::SubdividedIcosahedronCut::GraphicsBundle& gb=result_graphics.sas_graphics[i];
					const std::size_t sphere_id=result.cells_summaries[i].id;
					if(!gb.empty())
					{
						const std::string group_name=ApplicationGraphicsRecorder::name_ball_group("atoms", spheres_input_result, sphere_id);
						const unsigned int current_color=app_params.color_assigner.get_color("sasmesh", spheres_input_result.sphere_labels, base_color, sphere_id);
						app_graphics_recorder.graphics_writer.add_color("sasmesh", group_name, current_color);
						std::vector<voronotalt::SubdividedIcosahedron::Pair> pairs;
						gb.collect_pairs(pairs);
						for(std::size_t j=0;j<pairs.size();j++)
						{
							std::vector<voronotalt::SimplePoint> strip(2);
							strip[0]=gb.vertices[pairs[j].ids[0]];
							strip[1]=gb.vertices[pairs[j].ids[1]];
							app_graphics_recorder.graphics_writer.add_line_strip("sasmesh", group_name, strip);
						}
					}
				}
			}
		}
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "vsticks"))
		{
			const voronotalt::Float scaling=0.3;
			const voronotalt::Float narrowing=0.3;
			{
				app_graphics_recorder.graphics_writer.add_color("vstickheads", "", 0xFFFF00);
				app_graphics_recorder.graphics_writer.add_color("vsticklinks", "", 0x777777);
				for(std::size_t i=0;i<result_graphics.contacts_graphics.size();i++)
				{
					const voronotalt::RadicalTessellation::ContactDescriptorSummary& pair_summary=(i<result.contacts_summaries_with_redundancy_in_periodic_box.size() ? result.contacts_summaries_with_redundancy_in_periodic_box[i] : result.contacts_summaries[i]);
					{
						const std::string group_name=ApplicationGraphicsRecorder::name_contact_group("contacts", spheres_input_result, pair_summary.id_a, pair_summary.id_b);
						const std::string group_name_a=ApplicationGraphicsRecorder::name_ball_group("atoms", spheres_input_result, pair_summary.id_a);
						const std::string group_name_b=ApplicationGraphicsRecorder::name_ball_group("atoms", spheres_input_result, pair_summary.id_b);
						const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result_graphics.contacts_graphics[i];

						const voronotalt::SimpleSphere& sphere_a=spheres_input_result.spheres[pair_summary.id_a];
						const voronotalt::SimpleSphere& sphere_b=spheres_input_result.spheres[pair_summary.id_b];
						voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics pair_graphics_a=pair_graphics;
						voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics pair_graphics_b=pair_graphics;
						pair_graphics_a.barycenter=voronotalt::sum_of_points(sphere_a.p, voronotalt::point_and_number_product(voronotalt::sub_of_points(pair_graphics_a.barycenter, sphere_a.p), scaling));
						pair_graphics_b.barycenter=voronotalt::sum_of_points(sphere_b.p, voronotalt::point_and_number_product(voronotalt::sub_of_points(pair_graphics_b.barycenter, sphere_b.p), scaling));
						for(std::size_t j=0;j<pair_graphics.outer_points.size();j++)
						{
							pair_graphics_a.outer_points[j]=voronotalt::sum_of_points(sphere_a.p, voronotalt::point_and_number_product(voronotalt::sub_of_points(pair_graphics_a.outer_points[j], sphere_a.p), scaling));
							pair_graphics_b.outer_points[j]=voronotalt::sum_of_points(sphere_b.p, voronotalt::point_and_number_product(voronotalt::sub_of_points(pair_graphics_b.outer_points[j], sphere_b.p), scaling));
						}

						if(voronotalt::distance_from_point_to_point(sphere_a.p, sphere_b.p)>2.0)
						{
							app_graphics_recorder.graphics_writer.add_triangle_fan("vstickheads", group_name_a, pair_graphics_a.outer_points, pair_graphics_a.barycenter, pair_graphics_a.plane_normal);
							app_graphics_recorder.graphics_writer.add_triangle_fan("vstickheads", group_name_b, pair_graphics_b.outer_points, pair_graphics_b.barycenter, pair_graphics_b.plane_normal);
						}
						else
						{
							voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics pair_graphics_ma=pair_graphics;
							voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics pair_graphics_mb=pair_graphics;
							pair_graphics_ma.barycenter=voronotalt::sum_of_points(voronotalt::point_and_number_product(pair_graphics_a.barycenter, 0.99), voronotalt::point_and_number_product(pair_graphics_b.barycenter, 0.01));
							pair_graphics_mb.barycenter=voronotalt::sum_of_points(voronotalt::point_and_number_product(pair_graphics_a.barycenter, 0.01), voronotalt::point_and_number_product(pair_graphics_b.barycenter, 0.99));
							for(std::size_t j=0;j<pair_graphics.outer_points.size();j++)
							{
								pair_graphics_ma.outer_points[j]=voronotalt::sum_of_points(voronotalt::point_and_number_product(pair_graphics_a.outer_points[j], 0.99), voronotalt::point_and_number_product(pair_graphics_b.outer_points[j], 0.01));
								pair_graphics_mb.outer_points[j]=voronotalt::sum_of_points(voronotalt::point_and_number_product(pair_graphics_a.outer_points[j], 0.01), voronotalt::point_and_number_product(pair_graphics_b.outer_points[j], 0.99));
							}

							for(std::size_t j=0;j<pair_graphics.outer_points.size();j++)
							{
								pair_graphics_ma.outer_points[j]=voronotalt::sum_of_points(pair_graphics_ma.barycenter, voronotalt::point_and_number_product(voronotalt::sub_of_points(pair_graphics_ma.outer_points[j], pair_graphics_a.barycenter), narrowing));
								pair_graphics_mb.outer_points[j]=voronotalt::sum_of_points(pair_graphics_mb.barycenter, voronotalt::point_and_number_product(voronotalt::sub_of_points(pair_graphics_mb.outer_points[j], pair_graphics_b.barycenter), narrowing));
							}

							for(std::size_t j=0;j<pair_graphics.outer_points.size();j++)
							{
								std::size_t k=((j+1)<pair_graphics.outer_points.size() ? (j+1) : 0);
								std::vector<voronotalt::SimplePoint> sidepoints(4);
								{
									sidepoints[0]=pair_graphics_a.outer_points[j];
									sidepoints[1]=pair_graphics_a.outer_points[k];
									sidepoints[2]=pair_graphics_ma.outer_points[j];
									sidepoints[3]=pair_graphics_ma.outer_points[k];
									const voronotalt::SimplePoint sidenormal=voronotalt::unit_point(voronotalt::cross_product(voronotalt::sub_of_points(sidepoints[1], sidepoints[0]), voronotalt::sub_of_points(sidepoints[3], sidepoints[0])));
									app_graphics_recorder.graphics_writer.add_triangle_strip("vsticklinks", group_name, sidepoints, std::vector<voronotalt::SimplePoint>(sidepoints.size(), sidenormal));
								}
								{
									sidepoints[0]=pair_graphics_ma.outer_points[j];
									sidepoints[1]=pair_graphics_ma.outer_points[k];
									sidepoints[2]=pair_graphics_mb.outer_points[j];
									sidepoints[3]=pair_graphics_mb.outer_points[k];
									const voronotalt::SimplePoint sidenormal=voronotalt::unit_point(voronotalt::cross_product(voronotalt::sub_of_points(sidepoints[1], sidepoints[0]), voronotalt::sub_of_points(sidepoints[3], sidepoints[0])));
									app_graphics_recorder.graphics_writer.add_triangle_strip("vsticklinks", group_name, sidepoints, std::vector<voronotalt::SimplePoint>(sidepoints.size(), sidenormal));
								}
								{
									sidepoints[0]=pair_graphics_mb.outer_points[j];
									sidepoints[1]=pair_graphics_mb.outer_points[k];
									sidepoints[2]=pair_graphics_b.outer_points[j];
									sidepoints[3]=pair_graphics_b.outer_points[k];
									const voronotalt::SimplePoint sidenormal=voronotalt::unit_point(voronotalt::cross_product(voronotalt::sub_of_points(sidepoints[1], sidepoints[0]), voronotalt::sub_of_points(sidepoints[3], sidepoints[0])));
									app_graphics_recorder.graphics_writer.add_triangle_strip("vsticklinks", group_name, sidepoints, std::vector<voronotalt::SimplePoint>(sidepoints.size(), sidenormal));
								}
							}
						}
					}
				}
			}
			{
				if(!result_graphics.sas_graphics.empty())
				{
					app_graphics_recorder.graphics_writer.add_color("vsticksas", "", 0x00FFFF);
					for(std::size_t i=0;i<result_graphics.sas_graphics.size();i++)
					{
						const voronotalt::SubdividedIcosahedronCut::GraphicsBundle& gb=result_graphics.sas_graphics[i];
						const std::size_t sphere_id=result.cells_summaries[i].id;
						const std::string groupname=ApplicationGraphicsRecorder::name_ball_group("atoms", spheres_input_result, sphere_id);
						for(std::size_t j=0;j<gb.triples.size();j++)
						{
							const voronotalt::SubdividedIcosahedron::Triple& t=gb.triples[j];
							std::vector<voronotalt::SimplePoint> tvertices(3);
							for(std::size_t k=0;k<tvertices.size();k++)
							{
								tvertices[k]=voronotalt::sum_of_points(spheres_input_result.spheres[sphere_id].p, voronotalt::point_and_number_product(voronotalt::sub_of_points(gb.vertices[t.ids[k]], spheres_input_result.spheres[sphere_id].p), scaling));
							}
							app_graphics_recorder.graphics_writer.add_triangle_on_sphere("vsticksas", groupname, spheres_input_result.spheres[sphere_id].p, tvertices[0], tvertices[1], tvertices[2]);
						}
					}
				}
			}
		}
		if(periodic_box.enabled() && ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "lattice"))
		{
			voronotalt::SimplePoint origin;
			std::vector<voronotalt::SimplePoint> directions;
			if(app_params.periodic_box_directions.size()==3)
			{
				directions=app_params.periodic_box_directions;
			}
			else if(app_params.periodic_box_corners.size()==2)
			{
				const voronotalt::SimplePoint corner_min(
						std::min(app_params.periodic_box_corners[0].x, app_params.periodic_box_corners[1].x),
						std::min(app_params.periodic_box_corners[0].y, app_params.periodic_box_corners[1].y),
						std::min(app_params.periodic_box_corners[0].z, app_params.periodic_box_corners[1].z));
				const voronotalt::SimplePoint corner_max(
						std::max(app_params.periodic_box_corners[0].x, app_params.periodic_box_corners[1].x),
						std::max(app_params.periodic_box_corners[0].y, app_params.periodic_box_corners[1].y),
						std::max(app_params.periodic_box_corners[0].z, app_params.periodic_box_corners[1].z));
				origin=corner_min;
				directions.resize(3, voronotalt::SimplePoint());
				directions[0].x=corner_max.x-corner_min.x;
				directions[1].y=corner_max.y-corner_min.y;
				directions[2].z=corner_max.z-corner_min.z;
			}
			if(directions.size()==3)
			{
				app_graphics_recorder.graphics_writer.add_color("lattice", "", app_params.color_assigner.get_color("lattice"));
				for(int i=0;i<3;i++)
				{
					for(int wa=0;wa<=1;wa++)
					{
						for(int wb=0;wb<=1;wb++)
						{
							const int wx=(i==0 ? -1 : wa);
							const int wy=(i==1 ? -1 : (i==0 ? wa : wb));
							const int wz=(i==2 ? -1 : wb);
							voronotalt::SimplePoint o=origin;
							o=voronotalt::sum_of_points(o, voronotalt::point_and_number_product(directions[0], static_cast<voronotalt::Float>(wx)));
							o=voronotalt::sum_of_points(o, voronotalt::point_and_number_product(directions[1], static_cast<voronotalt::Float>(wy)));
							o=voronotalt::sum_of_points(o, voronotalt::point_and_number_product(directions[2], static_cast<voronotalt::Float>(wz)));
							app_graphics_recorder.graphics_writer.add_line("lattice", "borders", o, voronotalt::sum_of_points(o, voronotalt::point_and_number_product(directions[i], FLOATCONST(3.0))));
						}
					}
				}
			}
		}
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "truecollisions"))
		{
			app_graphics_recorder.graphics_writer.add_color("truecollisions", "", app_params.color_assigner.get_color("truecollisions"));
			for(std::map< std::pair<voronotalt::UnsignedInt, voronotalt::UnsignedInt>, std::pair<bool, voronotalt::UnsignedInt> >::const_iterator it=map_of_raw_collisions_to_contact_ids.begin();it!=map_of_raw_collisions_to_contact_ids.end();++it)
			{
				const voronotalt::UnsignedInt a=it->first.first;
				const voronotalt::UnsignedInt b=it->first.second;
				if(it->second.first)
				{
					app_graphics_recorder.graphics_writer.add_line("truecollisions", "all", spheres_input_result.spheres[a].p, spheres_input_result.spheres[b].p);
				}
			}
		}
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "falsecollisions"))
		{
			app_graphics_recorder.graphics_writer.add_color("falsecollisions", "", app_params.color_assigner.get_color("falsecollisions"));
			for(std::map< std::pair<voronotalt::UnsignedInt, voronotalt::UnsignedInt>, std::pair<bool, voronotalt::UnsignedInt> >::const_iterator it=map_of_raw_collisions_to_contact_ids.begin();it!=map_of_raw_collisions_to_contact_ids.end();++it)
			{
				const voronotalt::UnsignedInt a=it->first.first;
				const voronotalt::UnsignedInt b=it->first.second;
				if(!it->second.first)
				{
					app_graphics_recorder.graphics_writer.add_line("falsecollisions", "all", spheres_input_result.spheres[a].p, spheres_input_result.spheres[b].p);
				}
			}
		}
		app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("print graphics");
	}

	if(app_mesh_recorder.mesh_writer.enabled())
	{
		{
			voronotalt::MeshWriter::ChooserOfBestCoordinateID chooser;
			for(std::size_t i=0;i<result_graphics.contacts_graphics.size();i++)
			{
				chooser.feed(result_graphics.contacts_graphics[i].barycenter);
			}
			const unsigned int chosen_best_coordinate_id=chooser.choose_best_coordinate_id();
			if(chosen_best_coordinate_id>0)
			{
				app_mesh_recorder.mesh_writer=voronotalt::MeshWriter(true, chosen_best_coordinate_id);
			}
		}

		for(std::size_t i=0;i<result_graphics.contacts_graphics.size();i++)
		{
			const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result_graphics.contacts_graphics[i];
			app_mesh_recorder.mesh_writer.add_triangle_fan(pair_graphics.outer_points, pair_graphics.boundary_mask, pair_graphics.barycenter);
		}

		app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("collect mesh");

		if(app_params.mesh_extract_connected_component>0)
		{
			voronotalt::MeshWriter submesh_writer(false);
			if(app_params.mesh_extract_connected_component>app_mesh_recorder.mesh_writer.get_number_of_connected_components())
			{
				std::cerr << "Error (non-terminating): could not extract mesh connected component " << app_params.mesh_extract_connected_component << " because there are only " << app_mesh_recorder.mesh_writer.get_number_of_connected_components() << " connected components in total\n";
			}
			else
			{
				if(!app_mesh_recorder.mesh_writer.extract_connected_component(app_params.mesh_extract_connected_component, submesh_writer))
				{
					std::cerr << "Error (non-terminating): could not extract mesh connected component " << app_params.mesh_extract_connected_component << "\n";
				}
			}
			app_mesh_recorder.mesh_writer=submesh_writer;
		}

		if(app_mesh_recorder.mesh_writer.enabled())
		{
			app_log_recorders.log_output << "log_mesh_number_of_vertices\t" << app_mesh_recorder.mesh_writer.get_number_of_vertices() << "\n";
			app_log_recorders.log_output << "log_mesh_connected_components\t" << app_mesh_recorder.mesh_writer.get_number_of_connected_components() << "\n";
			app_log_recorders.log_output << "log_mesh_boundary_components\t" << app_mesh_recorder.mesh_writer.get_number_of_boundary_components() << "\n";
			app_log_recorders.log_output << "log_mesh_euler_characteristic\t" << app_mesh_recorder.mesh_writer.get_euler_characteristic() << "\n";
			app_log_recorders.log_output << "log_mesh_genus\t" << app_mesh_recorder.mesh_writer.calculate_genus() << "\n";
		}

		app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("analyze mesh");
	}
}

void run_mode_simplified_aw(
		const ApplicationParameters& app_params,
		const voronotalt::SpheresInput::Result& spheres_input_result,
		ApplicationLogRecorders& app_log_recorders,
		ApplicationGraphicsRecorder& app_graphics_recorder) noexcept
{
	app_log_recorders.time_recoder_for_tessellation.reset();

	voronotalt::SimplifiedAWTessellation::Result result;
	voronotalt::SimplifiedAWTessellation::ResultGraphics result_graphics;

	{
		const std::vector<int> null_grouping;
		const std::vector<int>& grouping_for_filtering=(app_params.compute_only_inter_chain_contacts ? spheres_input_result.grouping_by_chain : (app_params.compute_only_inter_residue_contacts ? spheres_input_result.grouping_by_residue : null_grouping));

		voronotalt::SpheresContainer spheres_container;
		spheres_container.init(spheres_input_result.spheres, app_log_recorders.time_recoder_for_tessellation);

		voronotalt::SpheresContainer::ResultOfPreparationForTessellation preparation_result;
		spheres_container.prepare_for_tessellation(grouping_for_filtering, preparation_result, app_log_recorders.time_recoder_for_tessellation);

		if(!app_params.filtering_expression_for_restricting_collisions.allow_all())
		{
			const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=app_params.filtering_expression_for_restricting_collisions.filter_vector(spheres_input_result.sphere_labels, spheres_input_result.spheres, preparation_result.relevant_collision_ids);
			if(ver.expression_matched())
			{
				preparation_result.restrict_relevant_collision_ids(ver.expression_matched_all, ver.expression_matched_ids);
			}
			else
			{
				preparation_result=voronotalt::SpheresContainer::ResultOfPreparationForTessellation();
			}
			app_log_recorders.time_recoder_for_tessellation.record_elapsed_miliseconds_and_reset("restrict collisions using filtering expression");
		}

		voronotalt::SimplifiedAWTessellation::construct_full_tessellation(
				spheres_container,
				preparation_result,
				app_graphics_recorder.graphics_writer.enabled(),
				result,
				result_graphics,
				app_log_recorders.time_recoder_for_tessellation);

		if(!app_params.filtering_expression_for_restricting_contacts_for_output.allow_all())
		{
			const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=app_params.filtering_expression_for_restricting_contacts_for_output.filter_vector(spheres_input_result.sphere_labels, spheres_input_result.spheres, voronotalt::FilteringBySphereLabels::ExpressionForPair::adapt_indices_container(result.contacts_summaries));
			if(!ver.expression_matched() || !voronotalt::SimplifiedAWTessellation::restrict_result_contacts(ver.expression_matched_all, ver.expression_matched_ids, result, result_graphics))
			{
				std::cerr << "Error: failed to restrict contacts for output\n";
				return;
			}
			app_log_recorders.time_recoder_for_tessellation.record_elapsed_miliseconds_and_reset("restrict contacts for output using filtering expression");
		}
	}

	voronotalt::SimplifiedAWTessellation::GroupedResult result_grouped_by_residue;
	if(app_params.need_summaries_on_residue_level)
	{
		voronotalt::SimplifiedAWTessellation::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, app_log_recorders.time_recoder_for_tessellation);
	}

	voronotalt::SimplifiedAWTessellation::GroupedResult result_grouped_by_chain;
	if(app_params.need_summaries_on_chain_level)
	{
		voronotalt::SimplifiedAWTessellation::group_results(result, spheres_input_result.grouping_by_chain, result_grouped_by_chain, app_log_recorders.time_recoder_for_tessellation);
	}

	app_log_recorders.time_recoder_for_output.reset();

	app_log_recorders.print_tessellation_full_construction_result_log_basic(result, result_grouped_by_residue, result_grouped_by_chain);

	app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("print total numbers");

	if(app_params.print_contacts || !app_params.write_contacts_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_contacts(result.contacts_summaries, spheres_input_result.sphere_labels, true, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_contacts)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_contacts_to_file.empty())
			{
				std::ofstream foutput(app_params.write_contacts_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write contacts to file '" << app_params.write_contacts_to_file << "'\n";
				}
			}
		}
	}

	if(app_params.print_contacts_residue_level || !app_params.write_contacts_residue_level_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_contacts_residue_level(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_residue.grouped_contacts_representative_ids, result_grouped_by_residue.grouped_contacts_summaries, app_params.minimum_columns, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_contacts_residue_level)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_contacts_residue_level_to_file.empty())
			{
				std::ofstream foutput(app_params.write_contacts_residue_level_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write contacts on residue level to file '" << app_params.write_contacts_residue_level_to_file << "'\n";
				}
			}
		}
	}

	if(app_params.print_contacts_chain_level || !app_params.write_contacts_chain_level_to_file.empty())
	{
		std::string output_string;
		voronotalt::PrintingCustomTypes::print_contacts_chain_level(result.contacts_summaries, spheres_input_result.sphere_labels, result_grouped_by_chain.grouped_contacts_representative_ids, result_grouped_by_chain.grouped_contacts_summaries, app_params.minimum_columns, output_string);
		if(!output_string.empty())
		{
			if(app_params.print_contacts_chain_level)
			{
				std::cout.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
			}
			if(!app_params.write_contacts_chain_level_to_file.empty())
			{
				std::ofstream foutput(app_params.write_contacts_chain_level_to_file.c_str(), std::ios::out);
				if(foutput.good())
				{
					foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
				}
				else
				{
					std::cerr << "Error (non-terminating): failed to write contacts on chain level to file '" << app_params.write_contacts_chain_level_to_file << "'\n";
				}
			}
		}
	}

	app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("print result contacts");

	if(!app_params.plot_contacts_to_file.empty() || !app_params.plot_contacts_residue_level_to_file.empty() || !app_params.plot_contacts_chain_level_to_file.empty())
	{
		for(int j=0;j<3;j++)
		{
			const std::string& outfile=(j==0 ? app_params.plot_contacts_to_file : (j==1 ? app_params.plot_contacts_residue_level_to_file : app_params.plot_contacts_chain_level_to_file));
			if(!outfile.empty())
			{
				voronotalt::ContactPlotter plotter(j==0 ? voronotalt::ContactPlotter::LevelMode::inter_atom : (j==1 ? voronotalt::ContactPlotter::LevelMode::inter_residue : voronotalt::ContactPlotter::LevelMode::inter_chain));
				bool all_good=true;
				for(std::size_t i=0;all_good && i<result.contacts_summaries.size();i++)
				{
					const unsigned int current_color=(app_params.plots_colorable ? app_params.color_assigner.get_color("faces", spheres_input_result.sphere_labels, spheres_input_result.spheres, 0xFF00FF, result.contacts_summaries[i].id_a, result.contacts_summaries[i].id_b) : static_cast<unsigned int>(0));
					all_good=all_good && plotter.add_contact(i, result.contacts_summaries, spheres_input_result.sphere_labels, current_color);
				}
				if(!all_good)
				{
					std::cerr << "Error (non-terminating): failed to plot contacts\n";
				}
				else if(!plotter.write_to_file(outfile, app_params.plot_options_config))
				{
					std::cerr << "Error (non-terminating): failed to write plot of contacts to file '" << outfile << "'\n";
				}
			}
		}
		app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("write contact map plots");
	}

	if(app_graphics_recorder.graphics_writer.enabled())
	{
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "balls"))
		{
			const unsigned int base_color=app_params.color_assigner.get_color("balls");
			app_graphics_recorder.graphics_writer.add_color("balls", "", base_color);
			for(std::size_t i=0;i<spheres_input_result.spheres.size();i++)
			{
				if(app_params.filtering_expression_for_restricting_balls_and_cells_for_output.allow_all() || (i<spheres_input_result.sphere_labels.size() && app_params.filtering_expression_for_restricting_balls_and_cells_for_output.filter(spheres_input_result.sphere_labels[i]).expression_matched))
				{
					const std::string group_name=ApplicationGraphicsRecorder::name_ball_group("atoms", spheres_input_result, i);
					const unsigned int current_color=app_params.color_assigner.get_color("balls", spheres_input_result.sphere_labels, base_color, i);
					app_graphics_recorder.graphics_writer.add_color("balls", group_name, current_color);
					app_graphics_recorder.graphics_writer.add_sphere("balls", group_name, spheres_input_result.spheres[i], app_params.probe);
				}
			}
		}
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "faces"))
		{
			const unsigned int base_color=app_params.color_assigner.get_color("faces");
			app_graphics_recorder.graphics_writer.add_color("faces", "", base_color);
			for(std::size_t i=0;i<result_graphics.contacts_graphics.size();i++)
			{
				const voronotalt::SimplifiedAWTessellation::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				{
					const std::string group_name=ApplicationGraphicsRecorder::name_contact_group("contacts", spheres_input_result, pair_summary.id_a, pair_summary.id_b);
					const unsigned int current_color=app_params.color_assigner.get_color("faces", spheres_input_result.sphere_labels, spheres_input_result.spheres, base_color, pair_summary.id_a, pair_summary.id_b);
					app_graphics_recorder.graphics_writer.add_color("faces", group_name, current_color);
					const voronotalt::SimplifiedAWTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result_graphics.contacts_graphics[i];
					for(std::size_t j=0;j<pair_graphics.contours_graphics.size();j++)
					{
						app_graphics_recorder.graphics_writer.add_triangle_fan("faces", group_name, pair_graphics.contours_graphics[j].outer_points, pair_graphics.contours_graphics[j].barycenter, spheres_input_result.spheres[pair_summary.id_a].p, spheres_input_result.spheres[pair_summary.id_b].p);
					}
				}
			}
		}
		if(ApplicationGraphicsRecorder::allow_representation(app_params.graphics_restrict_representations, "wireframe"))
		{
			const unsigned int base_color=app_params.color_assigner.get_color("wireframe");
			app_graphics_recorder.graphics_writer.add_color("wireframe", "", base_color);
			for(std::size_t i=0;i<result_graphics.contacts_graphics.size();i++)
			{
				const voronotalt::SimplifiedAWTessellation::ContactDescriptorSummary& pair_summary=result.contacts_summaries[i];
				{
					const std::string group_name=ApplicationGraphicsRecorder::name_contact_group("contacts", spheres_input_result, pair_summary.id_a, pair_summary.id_b);
					const unsigned int current_color=app_params.color_assigner.get_color("wireframe", spheres_input_result.sphere_labels, spheres_input_result.spheres, base_color, pair_summary.id_a, pair_summary.id_b);
					app_graphics_recorder.graphics_writer.add_color("wireframe", group_name, current_color);
					const voronotalt::SimplifiedAWTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=result_graphics.contacts_graphics[i];
					for(std::size_t j=0;j<pair_graphics.contours_graphics.size();j++)
					{
						app_graphics_recorder.graphics_writer.add_line_loop("wireframe", group_name, pair_graphics.contours_graphics[j].outer_points);
					}
				}
			}
		}
		app_log_recorders.time_recoder_for_output.record_elapsed_miliseconds_and_reset("print graphics");
	}
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

	voronotalt::openmp_set_num_threads_if_possible(app_params.max_number_of_processors);

	ApplicationLogRecorders app_log_recorders(app_params);

	app_log_recorders.time_recoder_for_input.reset();

	if(!app_params.pdb_or_mmcif_radii_config_file.empty())
	{
		std::string input_data;

		if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(app_params.pdb_or_mmcif_radii_config_file, input_data))
		{
			std::cerr << "Error: failed to open atom radii configuration file '" << app_params.pdb_or_mmcif_radii_config_file << "' without errors\n";
			return 1;
		}

		if(input_data.empty())
		{
			std::cerr << "Error: no data read from atom radii configuration file '" << app_params.pdb_or_mmcif_radii_config_file << "'\n";
			return 1;
		}

		if(!voronotalt::MolecularRadiiAssignment::set_radius_value_rules(input_data))
		{
			std::cerr << "Error: invalid atom radii configuration file.\n";
			return 1;
		}

		app_log_recorders.time_recoder_for_input.record_elapsed_miliseconds_and_reset("setting atom radii configuration from file");
	}

	if(!app_params.grouping_directives_file.empty())
	{
		std::string input_data;

		if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(app_params.grouping_directives_file, input_data))
		{
			std::cerr << "Error: failed to open group directives file '" << app_params.grouping_directives_file << "' without errors\n";
			return 1;
		}

		if(input_data.empty())
		{
			std::cerr << "Error: no data read from group directives file '" << app_params.grouping_directives_file << "'\n";
			return 1;
		}

		if(!app_params.group_chains_assigner.add_rules(input_data))
		{
			std::cerr << "Error: invalid group directives file.\n";
			return 1;
		}

		app_log_recorders.time_recoder_for_input.record_elapsed_miliseconds_and_reset("setting coloring configuration from file");
	}

	if(!app_params.grouping_directives.empty())
	{
		if(!app_params.group_chains_assigner.add_rules(app_params.grouping_directives))
		{
			std::cerr << "Error: invalid group directives string.\n";
			return 1;
		}

		app_log_recorders.time_recoder_for_input.record_elapsed_miliseconds_and_reset("setting coloring configuration from string");
	}

	if(!app_params.graphics_output_file_for_pymol.empty() || !app_params.graphics_output_file_for_chimera.empty() || app_params.plots_colorable)
	{
		if(!app_params.graphics_coloring_config_file.empty())
		{
			std::string input_data;

			if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(app_params.graphics_coloring_config_file, input_data))
			{
				std::cerr << "Error: failed to open coloring configuration file '" << app_params.graphics_coloring_config_file << "' without errors\n";
				return 1;
			}

			if(input_data.empty())
			{
				std::cerr << "Error: no data read from coloring configuration file '" << app_params.graphics_coloring_config_file << "'\n";
				return 1;
			}

			if(!app_params.color_assigner.add_rules(input_data))
			{
				std::cerr << "Error: invalid coloring configuration file.\n";
				return 1;
			}

			app_log_recorders.time_recoder_for_input.record_elapsed_miliseconds_and_reset("setting coloring configuration from file");
		}

		if(!app_params.graphics_coloring_config.empty())
		{
			if(!app_params.color_assigner.add_rules(app_params.graphics_coloring_config))
			{
				std::cerr << "Error: invalid coloring configuration string.\n";
				return 1;
			}

			app_log_recorders.time_recoder_for_input.record_elapsed_miliseconds_and_reset("setting coloring configuration from string");
		}
	}

	voronotalt::SpheresInput::Result spheres_input_result;

	{
		std::string input_data;

		if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(app_params.input_from_file, input_data))
		{
			std::cerr << "Error: failed to open input file '" << app_params.input_from_file << "' without errors\n";
			return 1;
		}

		if(input_data.empty())
		{
			std::cerr << "Error: empty input provided\n";
			return 1;
		}

		app_log_recorders.time_recoder_for_input.record_elapsed_miliseconds_and_reset("read input file data to memory");

		const voronotalt::MolecularFileReading::Parameters molecular_file_reading_parameters(app_params.pdb_or_mmcif_heteroatoms, app_params.pdb_or_mmcif_hydrogens, app_params.pdb_or_mmcif_as_assembly);

		if(!voronotalt::SpheresInput::read_labeled_or_unlabeled_spheres_from_string(input_data, molecular_file_reading_parameters, app_params.probe, spheres_input_result, std::cerr, app_log_recorders.time_recoder_for_input))
		{
			std::cerr << "Error: failed to read input without errors\n";
			return 1;
		}
	}

	if(app_params.contains_complex_filtering_expressions() && spheres_input_result.sphere_labels.size()!=spheres_input_result.spheres.size())
	{
		std::cerr << "Input has no labels for filtering\n";
		return 1;
	}

	if(!app_params.filtering_expression_for_restricting_input_balls.allow_all())
	{
		voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=app_params.filtering_expression_for_restricting_input_balls.filter_vector(spheres_input_result.sphere_labels);
		if(!ver.expression_valid)
		{
			std::cerr << "Restricting input by applying filtering expression failed\n";
			return 1;
		}
		if(!ver.expression_matched())
		{
			std::cerr << "No input satisfied restricting filtering expression\n";
			return 1;
		}
		if(!spheres_input_result.restrict_spheres(ver.expression_matched_all, ver.expression_matched_ids))
		{
			std::cerr << "Failed to restrict input\n";
			return 1;
		}
		app_log_recorders.time_recoder_for_input.record_elapsed_miliseconds_and_reset("restrict input using filtering expression");
	}

	if(!app_params.group_chains_assigner.empty())
	{
		if(!app_params.group_chains_assigner.assign_group_chains(spheres_input_result.sphere_labels))
		{
			std::cerr << "No grouping directives matched the input.\n";
			return 1;
		}
		else
		{
			voronotalt::SpheresInput::refresh_groupings_of_labeled_spheres(spheres_input_result);
		}
	}

	if(!app_params.write_input_balls_to_file.empty())
	{
		std::ofstream foutput(app_params.write_input_balls_to_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			std::string output_string;
			voronotalt::PrintingCustomTypes::print_balls(spheres_input_result.spheres, spheres_input_result.sphere_labels, app_params.probe, false, output_string);
		    if(!output_string.empty())
		    {
		    	foutput.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
		    }
		}
		else
		{
			std::cerr << "Error (non-terminating): failed to write input balls to file '" << app_params.write_input_balls_to_file << "'\n";
		}
		app_log_recorders.time_recoder_for_input.record_elapsed_miliseconds_and_reset("write input balls to file");
	}

	if(app_params.exit_before_calculations)
	{
		std::cerr << "As requested, exiting before any calculations\n";
		return 0;
	}

	if((app_params.compute_only_inter_chain_contacts || app_params.need_summaries_on_chain_level) && spheres_input_result.number_of_chain_groups<2)
	{
		std::cerr << "Error: inter-chain contact selection not possible - not enough distinct chains derived from labels\n";
		return 1;
	}

	if((app_params.compute_only_inter_residue_contacts || app_params.need_summaries_on_residue_level) && spheres_input_result.number_of_residue_groups<2)
	{
		std::cerr << "Error: inter-residue contact selection not possible - not enough distinct residues derived from labels\n";
		return 1;
	}

	ApplicationGraphicsRecorder app_graphics_recorder(app_params);
	ApplicationMeshRecorder app_mesh_recorder(app_params);

	if(app_params.running_mode==ApplicationParameters::RunningMode::radical)
	{
		run_mode_radical(app_params, spheres_input_result, app_log_recorders, app_graphics_recorder, app_mesh_recorder);
	}
	else if(app_params.running_mode==ApplicationParameters::RunningMode::simplified_aw)
	{
		run_mode_simplified_aw(app_params, spheres_input_result, app_log_recorders, app_graphics_recorder);
	}
	else
	{
		std::cerr << "Error: invalid running mode.\n";
		return 1;
	}

	app_graphics_recorder.finalize_and_output(app_params, app_log_recorders);

	app_mesh_recorder.finalize_and_output(app_params, app_log_recorders);

	app_log_recorders.finalize_and_output(app_params);

	return 0;
}

