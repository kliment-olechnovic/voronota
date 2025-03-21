#include <iostream>
#include <functional>
#include <exception>
#include <limits>
#include <vector>
#include <algorithm>

#include "voronota_version.h"
#include "auxiliaries/program_options_handler.h"

void get_balls_from_atoms_file(const voronota::auxiliaries::ProgramOptionsHandler&);
void calculate_vertices(const voronota::auxiliaries::ProgramOptionsHandler&);
void calculate_vertices_in_parallel(const voronota::auxiliaries::ProgramOptionsHandler&);
void calculate_contacts(const voronota::auxiliaries::ProgramOptionsHandler&);
void query_balls(const voronota::auxiliaries::ProgramOptionsHandler&);
void query_contacts(const voronota::auxiliaries::ProgramOptionsHandler&);
void draw_contacts(const voronota::auxiliaries::ProgramOptionsHandler&);
void score_contacts_energy(const voronota::auxiliaries::ProgramOptionsHandler&);
void score_contacts_quality(const voronota::auxiliaries::ProgramOptionsHandler&);
void score_contacts_potential(const voronota::auxiliaries::ProgramOptionsHandler&);
void compare_contacts(const voronota::auxiliaries::ProgramOptionsHandler&);
void write_balls_to_atoms_file(const voronota::auxiliaries::ProgramOptionsHandler&);
void query_balls_clashes(const voronota::auxiliaries::ProgramOptionsHandler&);
void expand_descriptors(const voronota::auxiliaries::ProgramOptionsHandler&);

void query_balls_sequences_pairings_stats(const voronota::auxiliaries::ProgramOptionsHandler&);
void draw_balls(const voronota::auxiliaries::ProgramOptionsHandler&);
void query_contacts_depth_values(const voronota::auxiliaries::ProgramOptionsHandler&);
void plot_contacts(const voronota::auxiliaries::ProgramOptionsHandler&);
void score_contacts_potentials_stats(const voronota::auxiliaries::ProgramOptionsHandler&);
void score_contacts_energy_stats(const voronota::auxiliaries::ProgramOptionsHandler&);
void vectorize_contacts(const voronota::auxiliaries::ProgramOptionsHandler&);
void vectorize_points(const voronota::auxiliaries::ProgramOptionsHandler&);
void calculate_mock_solvent(const voronota::auxiliaries::ProgramOptionsHandler&);
void query_contacts_simulating_unfolding(const voronota::auxiliaries::ProgramOptionsHandler&);
void vectorize_contact_environments(const voronota::auxiliaries::ProgramOptionsHandler&);
void write_qa_scores_in_casp_format(const voronota::auxiliaries::ProgramOptionsHandler&);
void score_contacts_global_energy_by_cuts(const voronota::auxiliaries::ProgramOptionsHandler&);
void simulate_potential_for_membrane_proteins(const voronota::auxiliaries::ProgramOptionsHandler&);
void query_contacts_solvation_values(const voronota::auxiliaries::ProgramOptionsHandler&);
void query_contacts_interchain_exposure_values(const voronota::auxiliaries::ProgramOptionsHandler&);
void score_scores(const voronota::auxiliaries::ProgramOptionsHandler&);
void demo_ses(const voronota::auxiliaries::ProgramOptionsHandler&);
void rotational_optimization_demo(const voronota::auxiliaries::ProgramOptionsHandler&);
void place_membrane(const voronota::auxiliaries::ProgramOptionsHandler&);
void demo_polygon_triangulation(const voronota::auxiliaries::ProgramOptionsHandler&);
void demo_hypercut(const voronota::auxiliaries::ProgramOptionsHandler&);
void run_script(const voronota::auxiliaries::ProgramOptionsHandler&);
void draw_values_bitmap(const voronota::auxiliaries::ProgramOptionsHandler&);
void reprint_hbp(const voronota::auxiliaries::ProgramOptionsHandler&);
void compare_files_as_sets(const voronota::auxiliaries::ProgramOptionsHandler&);
void calculate_path_centralities(const voronota::auxiliaries::ProgramOptionsHandler&);
void query_balls_distances(const voronota::auxiliaries::ProgramOptionsHandler&);
void split_atoms_file(const voronota::auxiliaries::ProgramOptionsHandler&);
void demo_blank(const voronota::auxiliaries::ProgramOptionsHandler&);

struct ModeDescriptor
{
	typedef std::pointer_to_unary_function<const voronota::auxiliaries::ProgramOptionsHandler&, void> FunctionPtr;

	std::string name;
	FunctionPtr func_ptr;

	ModeDescriptor(const std::string& name, const FunctionPtr& func_ptr) : name(name), func_ptr(func_ptr)
	{
	}

	bool operator==(const std::string& check_name) const
	{
		return (check_name==name);
	}
};

std::vector<ModeDescriptor> get_list_of_modes()
{
	std::vector<ModeDescriptor> list_of_modes;
	list_of_modes.push_back(ModeDescriptor("get-balls-from-atoms-file", ModeDescriptor::FunctionPtr(get_balls_from_atoms_file)));
	list_of_modes.push_back(ModeDescriptor("calculate-vertices", ModeDescriptor::FunctionPtr(calculate_vertices)));
	list_of_modes.push_back(ModeDescriptor("calculate-vertices-in-parallel", ModeDescriptor::FunctionPtr(calculate_vertices_in_parallel)));
	list_of_modes.push_back(ModeDescriptor("calculate-contacts", ModeDescriptor::FunctionPtr(calculate_contacts)));
	list_of_modes.push_back(ModeDescriptor("query-balls", ModeDescriptor::FunctionPtr(query_balls)));
	list_of_modes.push_back(ModeDescriptor("query-contacts", ModeDescriptor::FunctionPtr(query_contacts)));
	list_of_modes.push_back(ModeDescriptor("draw-contacts", ModeDescriptor::FunctionPtr(draw_contacts)));
	list_of_modes.push_back(ModeDescriptor("score-contacts-energy", ModeDescriptor::FunctionPtr(score_contacts_energy)));
	list_of_modes.push_back(ModeDescriptor("score-contacts-quality", ModeDescriptor::FunctionPtr(score_contacts_quality)));
	list_of_modes.push_back(ModeDescriptor("score-contacts-potential", ModeDescriptor::FunctionPtr(score_contacts_potential)));
	list_of_modes.push_back(ModeDescriptor("compare-contacts", ModeDescriptor::FunctionPtr(compare_contacts)));
	list_of_modes.push_back(ModeDescriptor("write-balls-to-atoms-file", ModeDescriptor::FunctionPtr(write_balls_to_atoms_file)));
	list_of_modes.push_back(ModeDescriptor("query-balls-clashes", ModeDescriptor::FunctionPtr(query_balls_clashes)));
	list_of_modes.push_back(ModeDescriptor("run-script", ModeDescriptor::FunctionPtr(run_script)));
	list_of_modes.push_back(ModeDescriptor("expand-descriptors", ModeDescriptor::FunctionPtr(expand_descriptors)));
	return list_of_modes;
}

std::vector<ModeDescriptor> get_list_of_xmodes()
{
	std::vector<ModeDescriptor> list_of_modes;
	list_of_modes.push_back(ModeDescriptor("x-query-balls-sequences-pairings-stats", ModeDescriptor::FunctionPtr(query_balls_sequences_pairings_stats)));
	list_of_modes.push_back(ModeDescriptor("x-draw-balls", ModeDescriptor::FunctionPtr(draw_balls)));
	list_of_modes.push_back(ModeDescriptor("x-query-contacts-depth-values", ModeDescriptor::FunctionPtr(query_contacts_depth_values)));
	list_of_modes.push_back(ModeDescriptor("x-plot-contacts", ModeDescriptor::FunctionPtr(plot_contacts)));
	list_of_modes.push_back(ModeDescriptor("x-score-contacts-potentials-stats", ModeDescriptor::FunctionPtr(score_contacts_potentials_stats)));
	list_of_modes.push_back(ModeDescriptor("x-score-contacts-energy-stats", ModeDescriptor::FunctionPtr(score_contacts_energy_stats)));
	list_of_modes.push_back(ModeDescriptor("x-vectorize-contacts", ModeDescriptor::FunctionPtr(vectorize_contacts)));
	list_of_modes.push_back(ModeDescriptor("x-vectorize-points", ModeDescriptor::FunctionPtr(vectorize_points)));
	list_of_modes.push_back(ModeDescriptor("x-calculate-mock-solvent", ModeDescriptor::FunctionPtr(calculate_mock_solvent)));
	list_of_modes.push_back(ModeDescriptor("x-query-contacts-simulating-unfolding", ModeDescriptor::FunctionPtr(query_contacts_simulating_unfolding)));
	list_of_modes.push_back(ModeDescriptor("x-vectorize-contact-environments", ModeDescriptor::FunctionPtr(vectorize_contact_environments)));
	list_of_modes.push_back(ModeDescriptor("x-write-qa-scores-in-casp-format", ModeDescriptor::FunctionPtr(write_qa_scores_in_casp_format)));
	list_of_modes.push_back(ModeDescriptor("x-score-contacts-global-energy-by-cuts", ModeDescriptor::FunctionPtr(score_contacts_global_energy_by_cuts)));
	list_of_modes.push_back(ModeDescriptor("x-simulate-potential-for-membrane-proteins", ModeDescriptor::FunctionPtr(simulate_potential_for_membrane_proteins)));
	list_of_modes.push_back(ModeDescriptor("x-query-contacts-solvation-values", ModeDescriptor::FunctionPtr(query_contacts_solvation_values)));
	list_of_modes.push_back(ModeDescriptor("x-query-contacts-interchain-exposure-values", ModeDescriptor::FunctionPtr(query_contacts_interchain_exposure_values)));
	list_of_modes.push_back(ModeDescriptor("x-score-scores", ModeDescriptor::FunctionPtr(score_scores)));
	list_of_modes.push_back(ModeDescriptor("x-demo-ses", ModeDescriptor::FunctionPtr(demo_ses)));
	list_of_modes.push_back(ModeDescriptor("x-rotational-optimization-demo", ModeDescriptor::FunctionPtr(rotational_optimization_demo)));
	list_of_modes.push_back(ModeDescriptor("x-place-membrane", ModeDescriptor::FunctionPtr(place_membrane)));
	list_of_modes.push_back(ModeDescriptor("x-demo-polygon-triangulation", ModeDescriptor::FunctionPtr(demo_polygon_triangulation)));
	list_of_modes.push_back(ModeDescriptor("x-demo-hypercut", ModeDescriptor::FunctionPtr(demo_hypercut)));
	list_of_modes.push_back(ModeDescriptor("x-draw-values-bitmap", ModeDescriptor::FunctionPtr(draw_values_bitmap)));
	list_of_modes.push_back(ModeDescriptor("x-reprint-hbp", ModeDescriptor::FunctionPtr(reprint_hbp)));
	list_of_modes.push_back(ModeDescriptor("x-compare-files-as-sets", ModeDescriptor::FunctionPtr(compare_files_as_sets)));
	list_of_modes.push_back(ModeDescriptor("x-calculate-path-centralities", ModeDescriptor::FunctionPtr(calculate_path_centralities)));
	list_of_modes.push_back(ModeDescriptor("x-query-balls-distances", ModeDescriptor::FunctionPtr(query_balls_distances)));
	list_of_modes.push_back(ModeDescriptor("x-split-atoms-file", ModeDescriptor::FunctionPtr(split_atoms_file)));
	list_of_modes.push_back(ModeDescriptor("x-demo-blank", ModeDescriptor::FunctionPtr(demo_blank)));
	return list_of_modes;
}

void print_error_message(const std::string& mode, const std::string& message)
{
	std::cerr << "Voronota version " << voronota::version();
	if(!mode.empty())
	{
		std::cerr << " command '" << mode << "'";
	}
	std::cerr << " exit error: " << message;
	std::cerr << std::endl;
}

int main(const int argc, const char** argv)
{
	const std::string mode=(argc>1 ? std::string(argv[1]) : std::string());

	const std::string xmode_prefix="x-";
	const bool xmode=(mode.rfind(xmode_prefix, 0)==0);

	std::cin.exceptions(std::istream::badbit);
	std::cout.exceptions(std::ostream::badbit);
	std::ios_base::sync_with_stdio(false);

	try
	{
		voronota::auxiliaries::ProgramOptionsHandler poh(argc, argv);

		const bool help=poh.contains_option("--help");

		{
			const std::string output_precision_option_name="--stdout-precision";
			if(poh.contains_option_with_argument(output_precision_option_name))
			{
				std::cout << std::fixed;
				std::cout.precision(poh.argument<int>(output_precision_option_name));
				poh.remove_option(output_precision_option_name);
			}
		}

		if(!xmode)
		{
			const std::vector<ModeDescriptor> list_of_modes=get_list_of_modes();
			if(!mode.empty() && std::count(list_of_modes.begin(), list_of_modes.end(), mode)>0)
			{
				std::find(list_of_modes.begin(), list_of_modes.end(), mode)->func_ptr(poh);
				return (help ? 1 : 0);
			}
			else
			{
				std::ostream& output=std::cout;
				output << "Voronota version " << voronota::version() << "\n\n";
				output << "Commands:\n\n";
				for(std::vector<ModeDescriptor>::const_iterator it=list_of_modes.begin();it!=list_of_modes.end();++it)
				{
					output << it->name << "\n";
				}
				output << "\n";
				if(help)
				{
					for(std::vector<ModeDescriptor>::const_iterator it=list_of_modes.begin();it!=list_of_modes.end();++it)
					{
						output << "Command '" << it->name << "' options:\n";
						it->func_ptr(poh);
						output << "\n";
					}
				}
			}
		}
		else
		{
			const std::vector<ModeDescriptor> list_of_modes=get_list_of_xmodes();
			if(!mode.empty() && std::count(list_of_modes.begin(), list_of_modes.end(), mode)>0)
			{
				std::find(list_of_modes.begin(), list_of_modes.end(), mode)->func_ptr(poh);
				return (help ? 1 : 0);
			}
			else
			{
				throw std::runtime_error("Invalid xmode.");
			}
		}

		return 1;
	}
	catch(const std::exception& e)
	{
		print_error_message(mode, e.what());
	}
	catch(...)
	{
		print_error_message(mode, "Unknown exception caught.");
	}

	return 2;
}
