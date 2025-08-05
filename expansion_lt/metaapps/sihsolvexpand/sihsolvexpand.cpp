#include <iostream>
#include <fstream>

#include "voronotalt/parallelization_configuration.h"

#ifdef VORONOTALT_OPENMP
#include <omp.h>
#endif

#include "voronotalt/voronotalt.h"
#include "voronotalt/subdivided_icosahedron.h"

#include "voronotalt_cli/voronotalt_cli.h"

namespace
{

void print_help(std::ostream& output) noexcept
{
	output << R"(
SIhSolvExpand

'sihsolvexpand' executable generates volume-filling expanded set of balls for the chain of interest (e.g. a ligand)

Options:
    --chain-of-interest                              string  *  name of the chain to expand from
    --max-distance                                   number     maximum expansion distance, default is 7.0
    --probe                                          number     probe radius, default is 1.4
    --bound-by-spheres                               numbers    quadruples of numbers (x, y, z, r) that define bounding spheres
    --sih-depth                                      number     subdivided icosahedron depth, default is 2)";

	if(voronotalt::openmp_enabled())
	{
		output << R"(
    --processors                                     number     maximum number of OpenMP threads to use, default is 2)";
	}

	output << R"(
    --pdb-or-mmcif-heteroatoms                                  flag to include heteroatoms when reading input in PDB or mmCIF format, enabled by default
    --pdb-or-mmcif-hydrogens                                    flag to include hydrogen atoms when reading input in PDB or mmCIF format
    --pdb-or-mmcif-join-models                                  flag to join multiple models into an assembly when reading input in PDB or mmCIF format
    --help | -h                                                 flag to print help (for basic options) to stderr and exit

Standard input stream:
    Several input formats are supported:
      a) Space-separated or tab-separated header-less table of balls, one of the following line formats possible:
             chainID x y z radius
             chainID residueID x y z radius
             chainID residueID atomName x y z radius
      b) PDB file
      c) mmCIF file

Standard output stream:
    A tab-separeated table of receptor and expanded ligand balls, where every row is 'chainID x y z radius'

Standard error output stream:
    Log, error messages

Usage examples:

    cat ./2zsk.pdb | sihsolvexpand --chain-of-interest B > solvent.tsv

    cat ./2zsk.pdb | sihsolvexpand --chain-of-interest B --max-distance 5.0 > solvent.tsv
)";
}

class ApplicationParameters
{
public:
	unsigned int max_number_of_processors;
	voronotalt::Float max_expansion_distance;
	voronotalt::Float expansion_probe;
	unsigned int sih_depth;
	bool pdb_or_mmcif_heteroatoms;
	bool pdb_or_mmcif_hydrogens;
	bool pdb_or_mmcif_as_assembly;
	bool output_for_voronota_gl;
	bool read_successfuly;
	std::string chain_of_interest;
	std::vector<voronotalt::SimpleSphere> bounding_spheres_;
	std::ostringstream error_log_for_options_parsing;

	ApplicationParameters() noexcept :
		max_number_of_processors(voronotalt::openmp_enabled() ? 2 : 1),
		max_expansion_distance(7.0),
		expansion_probe(1.4),
		sih_depth(2),
		pdb_or_mmcif_heteroatoms(true),
		pdb_or_mmcif_hydrogens(false),
		pdb_or_mmcif_as_assembly(false),
		output_for_voronota_gl(false),
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
				if(opt.name=="chain-of-interest" && opt.args_strings.size()==1)
				{
					chain_of_interest=opt.args_strings.front();
				}
				else if(opt.name=="max-distance" && opt.args_doubles.size()==1)
				{
					max_expansion_distance=static_cast<voronotalt::Float>(opt.args_doubles.front());
					if(!(max_expansion_distance>=1.0 && max_expansion_distance<=30.0))
					{
						error_log_for_options_parsing << "Error: invalid command line argument for the maximum expansion distance, must be a value from 1.0 to 30.0.\n";
					}
				}
				else if(opt.name=="probe" && opt.args_doubles.size()==1)
				{
					expansion_probe=static_cast<voronotalt::Float>(opt.args_doubles.front());
					if(!(expansion_probe>=0.0 && expansion_probe<=30.0))
					{
						error_log_for_options_parsing << "Error: invalid command line argument for the rolling probe radius, must be a value from 0.0 to 30.0.\n";
					}
				}
				else if(opt.name=="bound-by-spheres" && opt.args_doubles.size()>=4 && opt.args_doubles.size()%4==0)
				{
					for(std::size_t j=0;j<opt.args_doubles.size();j+=4)
					{
						bounding_spheres_.push_back(voronotalt::SimpleSphere(opt.args_doubles[j+0], opt.args_doubles[j+1], opt.args_doubles[j+2], opt.args_doubles[j+3]));
					}
				}
				else if(opt.name=="sih-depth" && opt.args_ints.size()==1)
				{
					sih_depth=static_cast<unsigned int>(opt.args_ints.front());
					if(!(sih_depth>=0 && sih_depth<=4))
					{
						error_log_for_options_parsing << "Error: invalid command line argument for the subdivided icosahedron depth, must be an integer from 1 to 4.\n";
					}
				}
				else if(voronotalt::openmp_enabled() && opt.name=="processors" && opt.args_ints.size()==1)
				{
					max_number_of_processors=static_cast<unsigned int>(opt.args_ints.front());
					if(!(max_number_of_processors>=1 && max_number_of_processors<=1000))
					{
						error_log_for_options_parsing << "Error: invalid command line argument for the maximum number of processors, must be an integer from 1 to 1000.\n";
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
				else if(opt.name=="output-for-voronota-gl" && opt.is_flag())
				{
					output_for_voronota_gl=opt.is_flag_and_true();
				}
				else if(opt.name.empty())
				{
					error_log_for_options_parsing << "Error: unnamed command line arguments detected.\n";
				}
				else
				{
					error_log_for_options_parsing << "Error: invalid command line option '" << opt.name << "'.\n";
				}
			}
		}

		if(chain_of_interest.empty())
		{
			error_log_for_options_parsing << "Error: chain of interest not specified.\n";
		}

		if(voronotalt::is_stdin_from_terminal())
		{
			error_log_for_options_parsing << "Error: no input provided to stdin.\n";
		}

		read_successfuly=error_log_for_options_parsing.str().empty();

		return read_successfuly;
	}
};

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

#ifdef VORONOTALT_OPENMP
		omp_set_num_threads(app_params.max_number_of_processors);
#endif

	voronotalt::TimeRecorderChrono time_recorder(false);

	voronotalt::SpheresInput::Result spheres_input_result;

	{
		std::string input_data;

		{
			std::istreambuf_iterator<char> stdin_eos;
			std::string stdin_data(std::istreambuf_iterator<char>(std::cin), stdin_eos);
			input_data.swap(stdin_data);
		}

		if(input_data.empty())
		{
			std::cerr << "Error: empty input provided\n";
			return 1;
		}

		const voronotalt::MolecularFileReading::Parameters molecular_file_reading_parameters(app_params.pdb_or_mmcif_heteroatoms, app_params.pdb_or_mmcif_hydrogens, app_params.pdb_or_mmcif_as_assembly);

		if(!voronotalt::SpheresInput::read_labeled_or_unlabeled_spheres_from_string(input_data, molecular_file_reading_parameters, app_params.expansion_probe, spheres_input_result, std::cerr, time_recorder))
		{
			std::cerr << "Error: failed to read input without errors\n";
			return 1;
		}

		if(spheres_input_result.sphere_labels.size()!=spheres_input_result.spheres.size())
		{
			std::cerr << "Error: input has no chain labels\n";
			return 1;
		}
	}

	std::vector<voronotalt::SimpleSphere> all_input_spheres;
	all_input_spheres.reserve(spheres_input_result.spheres.size());

	for(std::size_t i=0;i<spheres_input_result.sphere_labels.size();i++)
	{
		if(spheres_input_result.sphere_labels[i].chain_id!=app_params.chain_of_interest)
		{
			all_input_spheres.push_back(spheres_input_result.spheres[i]);
		}
	}

	const std::size_t number_of_first_spheres=all_input_spheres.size();

	for(std::size_t i=0;i<spheres_input_result.sphere_labels.size();i++)
	{
		if(spheres_input_result.sphere_labels[i].chain_id==app_params.chain_of_interest)
		{
			all_input_spheres.push_back(spheres_input_result.spheres[i]);
		}
	}

	if(number_of_first_spheres==all_input_spheres.size())
	{
		std::cerr << "Error: no atoms from the chain of interest '" << app_params.chain_of_interest << "'\n";
		return 1;
	}

	const std::size_t initial_number_of_all_spheres=all_input_spheres.size();

	const int max_number_of_expansions=static_cast<int>(std::ceil(app_params.max_expansion_distance/(2.0*app_params.expansion_probe)));

	if(max_number_of_expansions<1)
	{
		std::cerr << "Error: estimated number of expansions is less than 1\n";
		return 1;
	}

	const voronotalt::SubdividedIcosahedron sih(app_params.sih_depth);

	int expansion_terminated_at_iteration=0;

	for(int expansion_iteration=1;expansion_iteration<=max_number_of_expansions && expansion_terminated_at_iteration==0;expansion_iteration++)
	{
		voronotalt::RadicalTessellation::Result result;

		voronotalt::RadicalTessellation::construct_full_tessellation(all_input_spheres, result);

		std::clog << "Stage " << expansion_iteration << " tessellation summary:" << std::endl;
		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_basic(result, voronotalt::RadicalTessellation::GroupedResult(), voronotalt::RadicalTessellation::GroupedResult(), std::clog);
		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_about_cells(result, voronotalt::RadicalTessellation::GroupedResult(), voronotalt::RadicalTessellation::GroupedResult(), std::clog);

		if(result.cells_summaries.size()!=all_input_spheres.size())
		{
			std::cerr << "Error: failed to summarize cells\n";
			return 1;
		}

		std::vector< std::set< std::pair<voronotalt::Float, voronotalt::UnsignedInt> > > graph(all_input_spheres.size()-number_of_first_spheres);

		for(std::size_t i=0;i<result.contacts_summaries.size();i++)
		{
			const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[i];
			if(cds.id_a>=number_of_first_spheres && cds.id_a<result.cells_summaries.size() && result.cells_summaries[cds.id_a].sas_area>0.0)
			{
				graph[cds.id_a-number_of_first_spheres].insert(std::pair<voronotalt::Float, voronotalt::UnsignedInt>(cds.distance, cds.id_b));
			}
			if(cds.id_b>=number_of_first_spheres && cds.id_b<result.cells_summaries.size() && result.cells_summaries[cds.id_b].sas_area>0.0)
			{
				graph[cds.id_b-number_of_first_spheres].insert(std::pair<voronotalt::Float, voronotalt::UnsignedInt>(cds.distance, cds.id_a));
			}
		}

		std::vector< std::vector<voronotalt::SimpleSphere> > per_input_pseudosolvent_spheres(all_input_spheres.size()-number_of_first_spheres);

#ifdef VORONOTALT_OPENMP
#pragma omp parallel
#endif
		{
#ifdef VORONOTALT_OPENMP
#pragma omp for
#endif
			for(std::size_t i=number_of_first_spheres;i<result.cells_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& cs=result.cells_summaries[i];
				if(cs.sas_area>0.0)
				{
					const voronotalt::SimpleSphere& sa=all_input_spheres[i];
					const std::set< std::pair<voronotalt::Float, voronotalt::UnsignedInt> >& neighbors=graph[i-number_of_first_spheres];
					for(std::size_t k=0;k<sih.vertices.size();k++)
					{
						const voronotalt::SimplePoint p=sih.get_point_on_sphere(k, sa);
						const voronotalt::Float dist_to_sa=(voronotalt::distance_from_point_to_point(p, sa.p)-sa.r);
						bool valid=true;
						for(std::size_t j=0;j<app_params.bounding_spheres_.size() && valid;j++)
						{
							valid=valid && (voronotalt::distance_from_point_to_point(p, app_params.bounding_spheres_[j].p)<=app_params.bounding_spheres_[j].r);
						}
						for(std::set< std::pair<voronotalt::Float, voronotalt::UnsignedInt> >::const_iterator jt=neighbors.begin();jt!=neighbors.end() && valid;++jt)
						{
							const voronotalt::SimpleSphere& sb=all_input_spheres[jt->second];
							valid=valid && (dist_to_sa<(voronotalt::distance_from_point_to_point(p, sb.p)-sb.r));
						}
						if(valid)
						{
							per_input_pseudosolvent_spheres[i-number_of_first_spheres].push_back(voronotalt::SimpleSphere(p, app_params.expansion_probe*2.0));
						}
					}
				}
			}
		}

		int number_of_added_spheres=0;
		for(std::size_t i=0;i<per_input_pseudosolvent_spheres.size();i++)
		{
			for(std::size_t j=0;j<per_input_pseudosolvent_spheres[i].size();j++)
			{
				all_input_spheres.push_back(per_input_pseudosolvent_spheres[i][j]);
				number_of_added_spheres++;
			}
		}

		if(number_of_added_spheres==0)
		{
			expansion_terminated_at_iteration=expansion_iteration;
		}
	}

	{
		for(std::size_t i=number_of_first_spheres;i<all_input_spheres.size();i++)
		{
			all_input_spheres[i].r+=(app_params.expansion_probe*0.5);
		}

		voronotalt::RadicalTessellation::Result result;

		voronotalt::RadicalTessellation::construct_full_tessellation(all_input_spheres, result);

		std::clog << "Final stage tessellation summary:" << std::endl;
		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_basic(result, voronotalt::RadicalTessellation::GroupedResult(), voronotalt::RadicalTessellation::GroupedResult(), std::clog);
		voronotalt::PrintingCustomTypes::print_tessellation_full_construction_result_log_about_cells(result, voronotalt::RadicalTessellation::GroupedResult(), voronotalt::RadicalTessellation::GroupedResult(), std::clog);

		if(result.cells_summaries.size()!=all_input_spheres.size())
		{
			std::cerr << "Error: failed to summarize cells in the final stage\n";
			return 1;
		}

		for(std::size_t i=0;i<all_input_spheres.size();i++)
		{
			all_input_spheres[i].r-=app_params.expansion_probe;
		}

		for(std::size_t i=0;i<all_input_spheres.size();i++)
		{
			const voronotalt::SimpleSphere& s=all_input_spheres[i];
			const std::string chain_name=(i<number_of_first_spheres ? "receptor" : "ligand") ;
			if(app_params.output_for_voronota_gl)
			{
				std::cout << "c<" << chain_name << ">r<" << i << ">R<XXX>A<XXX> " << s.p.x << " " << s.p.y << " " << s.p.z << " " << s.r << " . .\n";
			}
			else
			{
				std::cout << chain_name << "\t" << s.p.x << "\t" << s.p.y << "\t" << s.p.z << "\t" << s.r << "\t" << "\n";
			}
		}
	}

	return 0;
}

