#include <iostream>

#ifdef ENABLE_MPI
#include <mpi.h>
#include <cstring>
#endif

#include "apollota/triangulation.h"

#include "modes_commons.h"

namespace
{

std::set<std::string> get_available_processing_method_names()
{
	std::set<std::string> names;
	names.insert("sequential");
#ifdef _OPENMP
	names.insert("openmp");
#endif
#ifdef ENABLE_MPI
	names.insert("mpi");
#endif
	return names;
}

std::string list_strings_from_set(const std::set<std::string>& names)
{
	std::ostringstream output;
	for(std::set<std::string>::const_iterator it=names.begin();it!=names.end();++it)
	{
		output << " '" << (*it) << "'";
	}
	return output.str();
}

inline bool number_is_power_of_two(const unsigned long x)
{
	return ( (x>0) && ((x & (x-1))==0) );
}

template<typename T>
std::vector<T> extract_vector_subset_by_selection(const std::vector<T>& input, const std::vector<std::size_t>& selection)
{
	if(selection.empty())
	{
		return input;
	}
	else
	{
		std::vector<T> result;
		result.reserve(selection.size());
		for(std::size_t i=0;i<selection.size();i++)
		{
			if(selection[i]<input.size())
			{
				result.push_back(input[selection[i]]);
			}
		}
		return result;
	}
}

long reduce_quadruples_maps(const std::vector<apollota::Triangulation::QuadruplesMap>& distributed_quadruples_maps, apollota::Triangulation::QuadruplesMap& result_quadruples_map)
{
	std::size_t sum_of_all_produced_quadruples_counts=0;
	for(std::size_t i=0;i<distributed_quadruples_maps.size();i++)
	{
		sum_of_all_produced_quadruples_counts+=distributed_quadruples_maps[i].size();
		apollota::Triangulation::merge_quadruples_maps(distributed_quadruples_maps[i], result_quadruples_map);
	}
	return (static_cast<long>(sum_of_all_produced_quadruples_counts)-static_cast<long>(result_quadruples_map.size()));
}

void calculate_triangulation_in_parallel_on_single_machine(
		const std::size_t parts,
		const std::vector<std::size_t>& selection,
		const bool skip_output,
		const bool print_log,
		const double init_radius_for_BSH,
		const bool use_openmp)
{
	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::vector< std::vector<std::size_t> > all_distributed_ids=apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);
	const std::vector< std::vector<std::size_t> > distributed_ids=extract_vector_subset_by_selection(all_distributed_ids, selection);
	if(distributed_ids.empty())
	{
		throw std::runtime_error("No requested parts available.");
	}

	const apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);

	std::vector<apollota::Triangulation::QuadruplesMap> distributed_quadruples_maps(distributed_ids.size());

	if(!use_openmp)
	{
		for(std::size_t i=0;i<distributed_ids.size();i++)
		{
			distributed_quadruples_maps[i]=apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i]).quadruples_map;
		}
	}
	else
	{
#ifdef _OPENMP
		int errors=0;
		{
#pragma omp parallel for reduction(+:errors)
			for(std::size_t i=0;i<distributed_ids.size();i++)
			{
				try
				{
					distributed_quadruples_maps[i]=apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i]).quadruples_map;
				}
				catch(...)
				{
					errors+=1;
				}
			}
		}
		if(errors>0)
		{
			throw std::runtime_error("Parallel processing failed because of exception.");
		}
#else
		throw std::runtime_error("OpenMP was not enabled during compilation.");
#endif
	}

	apollota::Triangulation::QuadruplesMap result_quadruples_map;
	const long parallel_results_absolute_overlap=reduce_quadruples_maps(distributed_quadruples_maps, result_quadruples_map);
	const double parallel_results_relative_overlap=static_cast<double>(parallel_results_absolute_overlap)/static_cast<double>(result_quadruples_map.size());

	if(!skip_output)
	{
		apollota::Triangulation::print_quadruples_map(result_quadruples_map, std::cout);
	}

	if(print_log)
	{
		std::clog << "balls " << spheres.size() << "\n";
		std::clog << "all_parts " << all_distributed_ids.size() << "\n";
		std::clog << "processed_parts " << distributed_ids.size() << "\n";
		std::clog << "quadruples " << result_quadruples_map.size() << "\n";
		std::clog << "tangent_spheres " << apollota::Triangulation::count_tangent_spheres_in_quadruples_map(result_quadruples_map) << "\n";
		std::clog << "parallel_results_overlap " << parallel_results_relative_overlap << "\n";
	}
}

#ifdef ENABLE_MPI

class MPIWrapper
{
public:
	MPIWrapper(const std::vector<std::string>& argv) : argc_(static_cast<int>(argv.size()))
	{
		argv_=new char*[argc_];
		for(int i=0;i<argc_;i++)
		{
			argv_[i]=new char[argv[i].size()+1];
			strcpy(argv_[i], argv[i].c_str());
		}
		MPI_Init(&argc_, &argv_);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	}

	~MPIWrapper()
	{
		MPI_Finalize();
		for(int i=0;i<argc_;i++)
		{
			delete[] argv_[i];
		}
		delete argv_;
	}

	int size;
	int rank;

private:
	int argc_;
	char** argv_;

};

void calculate_triangulation_in_parallel_on_multiple_machines(
		const std::vector<std::string>& argv,
		const std::size_t parts,
		const std::vector<std::size_t>& selection,
		const bool skip_output,
		const bool print_log,
		const double init_radius_for_BSH)
{
	MPIWrapper mpi_wrapper(argv);
	std::cout << "MPI process " << mpi_wrapper.rank << " of " << mpi_wrapper.size << " executed with options:\n";
	for(std::size_t i=0;i<argv.size();i++)
	{
		std::cout << " " << argv[i];
	}
	std::cout << "\n";
}

#endif

}

void calculate_triangulation_in_parallel(const auxiliaries::ProgramOptionsHandler& poh)
{
	const std::set<std::string> available_processing_method_names=get_available_processing_method_names();

	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--method"].init("string", "processing method name, variants are:"+list_strings_from_set(available_processing_method_names), true);
		basic_map_of_option_descriptions["--parts"].init("number", "number of parts for splitting, must be power of 2", true);
		basic_map_of_option_descriptions["--selection"].init("numbers", "numbers of selected parts - if not provided, all parts are selected");
		basic_map_of_option_descriptions["--skip-output"].init("", "flag to disable output of the resulting triangulation");
		basic_map_of_option_descriptions["--print-log"].init("", "flag to print log of calculations");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--init-radius-for-BSH"].init("number", "initial radius for bounding sphere hierarchy");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of balls (line format: 'x y z r')\n";
			std::cerr << "  stdout  ->  list of quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const std::string method=poh.argument<std::string>("--method");
	if(available_processing_method_names.count(method)==0)
	{
		throw std::runtime_error("Invalid processing method name, acceptable values are:"+list_strings_from_set(available_processing_method_names)+".");
	}

	const std::size_t parts=poh.argument<std::size_t>("--parts");
	if(!number_is_power_of_two(parts))
	{
		throw std::runtime_error("Number of parts must be power of 2.");
	}

	const std::vector<std::size_t> selection=poh.argument_vector<std::size_t>("--selection");
	for(std::size_t i=0;i<selection.size();i++)
	{
		if(selection[i]>=static_cast<std::size_t>(parts))
		{
			throw std::runtime_error("Every selection number should be less than number of parts.");
		}
	}

	const bool skip_output=poh.contains_option("--skip-output");

	const bool print_log=poh.contains_option("--print-log");

	const double init_radius_for_BSH=poh.argument<double>("--init-radius-for-BSH", 3.5);
	if(init_radius_for_BSH<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	if(method=="sequential")
	{
		calculate_triangulation_in_parallel_on_single_machine(parts, selection, skip_output, print_log, init_radius_for_BSH, false);
	}
#ifdef _OPENMP
	else if(method=="openmp")
	{
		calculate_triangulation_in_parallel_on_single_machine(parts, selection, skip_output, print_log, init_radius_for_BSH, true);
	}
#endif
#ifdef ENABLE_MPI
	else if(method=="mpi")
	{
		calculate_triangulation_in_parallel_on_multiple_machines(poh.original_argv(), parts, selection, skip_output, print_log, init_radius_for_BSH);
	}
#endif
	else
	{
		throw std::runtime_error("Processing method '"+method+"' is not available.");
	}
}
