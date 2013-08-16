#include <iostream>

#ifdef ENABLE_MPI
#include <mpi.h>
#include <cstring>
#endif

#include "apollota/triangulation.h"

#include "modes_commons.h"

namespace
{

inline bool number_is_power_of_two(const unsigned long x)
{
	return ( (x>0) && ((x & (x-1))==0) );
}

void calculate_triangulation_in_parallel_on_single_machine(
		const std::size_t parts,
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

	const std::vector< std::vector<std::size_t> > distributed_ids=apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);

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

	std::size_t sum_of_all_produced_quadruples_counts=0;
	apollota::Triangulation::QuadruplesMap result_quadruples_map;
	for(std::size_t i=0;i<distributed_quadruples_maps.size();i++)
	{
		sum_of_all_produced_quadruples_counts+=distributed_quadruples_maps[i].size();
		apollota::Triangulation::merge_quadruples_maps(distributed_quadruples_maps[i], result_quadruples_map);
	}
	const double parallel_results_relative_overlap=static_cast<double>(sum_of_all_produced_quadruples_counts)/static_cast<double>(result_quadruples_map.size()-1.0);

	if(!skip_output)
	{
		apollota::Triangulation::print_quadruples_map(result_quadruples_map, std::cout);
	}

	if(print_log)
	{
		std::clog << "balls " << spheres.size() << "\n";
		std::clog << "parts " << distributed_ids.size() << "\n";
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

void fill_plain_vector_from_spheres(const std::vector<apollota::SimpleSphere>& spheres, std::vector<double>& plain_vector)
{
	plain_vector.resize(spheres.size()*4);
	for(std::size_t i=0;i<spheres.size();i++)
	{
		plain_vector[i*4+0]=spheres[i].x;
		plain_vector[i*4+1]=spheres[i].y;
		plain_vector[i*4+2]=spheres[i].z;
		plain_vector[i*4+3]=spheres[i].r;
	}
}

void fill_spheres_from_plain_vector(const std::vector<double>& plain_vector, std::vector<apollota::SimpleSphere>& spheres)
{
	spheres.resize(plain_vector.size()/4);
	for(std::size_t i=0;i<spheres.size();i++)
	{
		spheres[i].x=plain_vector[i*4+0];
		spheres[i].y=plain_vector[i*4+1];
		spheres[i].z=plain_vector[i*4+2];
		spheres[i].r=plain_vector[i*4+3];
	}
}

void calculate_triangulation_in_parallel_on_multiple_machines(
		const std::vector<std::string>& argv,
		const std::size_t parts,
		const bool skip_output,
		const bool print_log,
		const double init_radius_for_BSH)
{
	MPIWrapper mpi_wrapper(argv);

	std::vector<apollota::SimpleSphere> spheres;
	{
		std::vector<double> spheres_plain_vector;
		int spheres_plain_vector_length=0;
		if(mpi_wrapper.rank==0)
		{
			auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
			fill_plain_vector_from_spheres(spheres, spheres_plain_vector);
			spheres_plain_vector_length=static_cast<int>(spheres_plain_vector.size());
		}
		MPI_Bcast(&spheres_plain_vector_length, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if(mpi_wrapper.rank!=0)
		{
			spheres_plain_vector.resize(static_cast<std::size_t>(spheres_plain_vector_length));
		}
		MPI_Bcast(spheres_plain_vector.data(), spheres_plain_vector_length, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		if(mpi_wrapper.rank!=0)
		{
			fill_spheres_from_plain_vector(spheres_plain_vector, spheres);
		}
	}

	std::cout << "MPI process " << mpi_wrapper.rank << " of " << mpi_wrapper.size << " initialized with " << spheres.size() << " spheres\n";
}

#endif

}

void calculate_triangulation_in_parallel(const auxiliaries::ProgramOptionsHandler& poh)
{
	std::set<std::string> available_processing_method_names;
	{
		available_processing_method_names.insert("sequential");
#ifdef _OPENMP
		available_processing_method_names.insert("openmp");
#endif
#ifdef ENABLE_MPI
		available_processing_method_names.insert("mpi");
#endif
	}

	std::string available_processing_method_names_string;
	{
		std::ostringstream output;
		for(std::set<std::string>::const_iterator it=available_processing_method_names.begin();it!=available_processing_method_names.end();++it)
		{
			output << " '" << (*it) << "'";
		}
		available_processing_method_names_string=output.str();
	}

	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--method"].init("string", "processing method name, variants are:"+available_processing_method_names_string, true);
		basic_map_of_option_descriptions["--parts"].init("number", "number of parts for splitting, must be power of 2", true);
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
		throw std::runtime_error("Invalid processing method name, acceptable values are:"+available_processing_method_names_string+".");
	}

	const std::size_t parts=poh.argument<std::size_t>("--parts");
	if(!number_is_power_of_two(parts))
	{
		throw std::runtime_error("Number of parts must be power of 2.");
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
		calculate_triangulation_in_parallel_on_single_machine(parts, skip_output, print_log, init_radius_for_BSH, false);
	}
#ifdef _OPENMP
	else if(method=="openmp")
	{
		calculate_triangulation_in_parallel_on_single_machine(parts, skip_output, print_log, init_radius_for_BSH, true);
	}
#endif
#ifdef ENABLE_MPI
	else if(method=="mpi")
	{
		calculate_triangulation_in_parallel_on_multiple_machines(poh.original_argv(), parts, skip_output, print_log, init_radius_for_BSH);
	}
#endif
	else
	{
		throw std::runtime_error("Processing method '"+method+"' is not available.");
	}
}
