#include <iostream>
#include <fstream>

#ifdef ENABLE_MPI
#include <mpi.h>
#include <cstring>
#endif

#include "../apollota/triangulation.h"
#include "../apollota/triangulation_output.h"

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

namespace
{

struct ParallelComputationResult
{
	ParallelComputationResult() : number_of_initialized_parts(0), number_of_produced_quadruples(0)
	{
	}

	std::vector<voronota::apollota::SimpleSphere> input_spheres;
	std::size_t number_of_initialized_parts;
	std::size_t number_of_produced_quadruples;
	voronota::apollota::Triangulation::QuadruplesMap merged_quadruples_map;
};

class ParallelComputationProcessingSimulated
{
public:
	static void process(
			const std::size_t parts,
			const double init_radius_for_BSH,
			const bool include_surplus_quadruples,
			ParallelComputationResult& result)
	{
		result.input_spheres.clear();
		std::vector<voronota::apollota::SimpleSphere>& spheres=result.input_spheres;
		voronota::auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);

		const std::vector< std::vector<std::size_t> > distributed_ids=voronota::apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);
		result.number_of_initialized_parts=distributed_ids.size();

		const voronota::apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);

		for(std::size_t i=0;i<distributed_ids.size();i++)
		{
			const voronota::apollota::Triangulation::QuadruplesMap partial_quadruples_map=voronota::apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i], include_surplus_quadruples).quadruples_map;
			result.number_of_produced_quadruples+=partial_quadruples_map.size();
			voronota::apollota::Triangulation::merge_quadruples_maps(partial_quadruples_map, result.merged_quadruples_map);
		}
	}
};

#ifdef _OPENMP
class ParallelComputationProcessingWithOpenMP
{
public:
	static void process(
			const std::size_t parts,
			const double init_radius_for_BSH,
			const bool include_surplus_quadruples,
			ParallelComputationResult& result)
	{
		result.input_spheres.clear();
		std::vector<voronota::apollota::SimpleSphere>& spheres=result.input_spheres;
		voronota::auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);

		const std::vector< std::vector<std::size_t> > distributed_ids=voronota::apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);
		result.number_of_initialized_parts=distributed_ids.size();

		const voronota::apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);

		std::vector<voronota::apollota::Triangulation::QuadruplesMap> distributed_quadruples_maps(distributed_ids.size());
		std::vector<int> distributed_errors(distributed_ids.size(), 0);

		const int distributed_ids_size=static_cast<int>(distributed_ids.size());
		{
	#pragma omp parallel for
			for(int i=0;i<distributed_ids_size;i++)
			{
				try
				{
					distributed_quadruples_maps[i]=voronota::apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i], include_surplus_quadruples).quadruples_map;
				}
				catch(...)
				{
					distributed_errors[i]=1;
				}
			}
		}

		{
			std::ostringstream errors_summary_stream;
			for(std::size_t i=0;i<distributed_errors.size();i++)
			{
				if(distributed_errors[i]!=0)
				{
					errors_summary_stream << " " << i;
				}
			}
			const std::string errors_summary=errors_summary_stream.str();
			if(!errors_summary.empty())
			{
				throw std::runtime_error("Parallel processing failed because of exceptions in parts:"+errors_summary+".");
			}
		}

		for(std::size_t i=0;i<distributed_quadruples_maps.size();i++)
		{
			result.number_of_produced_quadruples+=distributed_quadruples_maps[i].size();
			voronota::apollota::Triangulation::merge_quadruples_maps(distributed_quadruples_maps[i], result.merged_quadruples_map);
		}
	}
};
#endif

#ifdef ENABLE_MPI
class ParallelComputationProcessingWithMPI
{
public:
	static bool process(
			const std::vector<std::string>& argv,
			const std::size_t parts,
			const double init_radius_for_BSH,
			const bool include_surplus_quadruples,
			ParallelComputationResult& result)
	{
		MPIHandle mpi_handle(argv);

		if(mpi_handle.size()<=2)
		{
			ParallelComputationProcessingSimulated::process(parts, init_radius_for_BSH, include_surplus_quadruples, result);
		}
		else
		{
			result.input_spheres.clear();
			std::vector<voronota::apollota::SimpleSphere>& spheres=result.input_spheres;
			{
				std::vector<double> spheres_plain_vector;
				int spheres_plain_vector_length=0;
				if(mpi_handle.rank()==0)
				{
					voronota::auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);
					fill_plain_vector_from_spheres(spheres, spheres_plain_vector);
					spheres_plain_vector_length=static_cast<int>(spheres_plain_vector.size());
				}
				MPI_Bcast(&spheres_plain_vector_length, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
				if(mpi_handle.rank()!=0)
				{
					spheres_plain_vector.resize(static_cast<std::size_t>(spheres_plain_vector_length));
				}
				MPI_Bcast(&spheres_plain_vector[0], spheres_plain_vector_length, MPI_DOUBLE, 0, MPI_COMM_WORLD);
				if(mpi_handle.rank()!=0)
				{
					fill_spheres_from_plain_vector(spheres_plain_vector, spheres);
				}
			}

			const std::vector< std::vector<std::size_t> > distributed_ids=voronota::apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);
			result.number_of_initialized_parts=distributed_ids.size();

			{
				const int QUADRUPLES_MAP_DATA_TAG=1;
				std::vector<double> plain_vector;
				if(mpi_handle.rank()==0)
				{
					for(std::size_t i=0;i<distributed_ids.size();i++)
					{
						MPI_Status status;
						MPI_Probe(MPI_ANY_SOURCE, QUADRUPLES_MAP_DATA_TAG, MPI_COMM_WORLD, &status);
						int plain_vector_size=0;
						MPI_Get_count(&status, MPI_DOUBLE, &plain_vector_size);
						if(plain_vector_size>0)
						{
							plain_vector.resize(static_cast<std::size_t>(plain_vector_size));
							MPI_Recv(&plain_vector[0], plain_vector_size, MPI_DOUBLE, status.MPI_SOURCE, QUADRUPLES_MAP_DATA_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
							voronota::apollota::Triangulation::QuadruplesMap partial_quadruples_map;
							fill_quadruples_map_from_plain_vector(plain_vector, partial_quadruples_map);
							result.number_of_produced_quadruples+=partial_quadruples_map.size();
							voronota::apollota::Triangulation::merge_quadruples_maps(partial_quadruples_map, result.merged_quadruples_map);
						}
					}
				}
				else
				{
					const voronota::apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);
					for(std::size_t i=0;i<distributed_ids.size();i++)
					{
						if(mpi_handle.rank()==(static_cast<int>(i)%(mpi_handle.size()-1)+1))
						{
							fill_plain_vector_from_quadruples_map(voronota::apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i], include_surplus_quadruples).quadruples_map, plain_vector);
							MPI_Send(&plain_vector[0], static_cast<int>(plain_vector.size()), MPI_DOUBLE, 0, QUADRUPLES_MAP_DATA_TAG, MPI_COMM_WORLD);
						}
					}
				}
			}
		}

		mpi_handle.set_abort_on_destruction(false);
		return (mpi_handle.rank()==0);
	}

private:
	class MPIHandle
	{
	public:
		explicit MPIHandle(const std::vector<std::string>& argv) :
			abort_on_destruction_(true),
			argc_(static_cast<int>(argv.size())),
			argv_(0),
			size_(0),
			rank_(0),
			start_time_(0.0)
		{
			argv_=new char*[argc_];
			for(int i=0;i<argc_;i++)
			{
				argv_[i]=new char[argv[i].size()+1];
				strcpy(argv_[i], argv[i].c_str());
			}
			MPI_Init(&argc_, &argv_);
	        MPI_Comm_size(MPI_COMM_WORLD, &size_);
	        MPI_Comm_rank(MPI_COMM_WORLD, &rank_);

	        start_time_=MPI_Wtime();
		}

		~MPIHandle()
		{
			if(abort_on_destruction_)
			{
				MPI_Abort(MPI_COMM_WORLD, 1);
			}
			else
			{
				const double elapsed_time=MPI_Wtime()-start_time_;
				std::cerr << "MPI process " << rank_ << " of " << size_ << " took " << elapsed_time << " seconds" << std::endl;
				MPI_Finalize();
			}

			for(int i=0;i<argc_;i++)
			{
				delete[] argv_[i];
			}
			delete argv_;
		}

		void set_abort_on_destruction(const bool abort_on_destruction)
		{
			abort_on_destruction_=abort_on_destruction;
		}

		const int size() const
		{
			return size_;
		}

		const int rank() const
		{
			return rank_;
		}

	private:
		MPIHandle(const MPIHandle&);
		MPIHandle& operator=(const MPIHandle&);

		bool abort_on_destruction_;
		int argc_;
		char** argv_;
		int size_;
		int rank_;
		double start_time_;
	};

	static void fill_plain_vector_from_spheres(const std::vector<voronota::apollota::SimpleSphere>& spheres, std::vector<double>& plain_vector)
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

	static void fill_spheres_from_plain_vector(const std::vector<double>& plain_vector, std::vector<voronota::apollota::SimpleSphere>& spheres)
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

	static void fill_plain_vector_from_quadruples_map(const voronota::apollota::Triangulation::QuadruplesMap& quadruples_map, std::vector<double>& plain_vector)
	{
		plain_vector.resize(voronota::apollota::Triangulation::count_tangent_spheres_in_quadruples_map(quadruples_map)*8);
		std::size_t i=0;
		for(voronota::apollota::Triangulation::QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const voronota::apollota::Quadruple& q=it->first;
			const std::vector<voronota::apollota::SimpleSphere>& ts=it->second;
			for(std::size_t j=0;j<ts.size();j++)
			{
				plain_vector[i*8+0]=q.get(0);
				plain_vector[i*8+1]=q.get(1);
				plain_vector[i*8+2]=q.get(2);
				plain_vector[i*8+3]=q.get(3);
				plain_vector[i*8+4]=ts[j].x;
				plain_vector[i*8+5]=ts[j].y;
				plain_vector[i*8+6]=ts[j].z;
				plain_vector[i*8+7]=ts[j].r;
				i++;
			}
		}
	}

	static void fill_quadruples_map_from_plain_vector(const std::vector<double>& plain_vector, voronota::apollota::Triangulation::QuadruplesMap& quadruples_map)
	{
		quadruples_map.clear();
		const std::size_t n=(plain_vector.size()/8);
		for(std::size_t i=0;i<n;i++)
		{
			quadruples_map[voronota::apollota::Quadruple(
					static_cast<std::size_t>(plain_vector[i*8+0]+0.5),
					static_cast<std::size_t>(plain_vector[i*8+1]+0.5),
					static_cast<std::size_t>(plain_vector[i*8+2]+0.5),
					static_cast<std::size_t>(plain_vector[i*8+3]+0.5))].push_back(voronota::apollota::SimpleSphere(
							plain_vector[i*8+4],
							plain_vector[i*8+5],
							plain_vector[i*8+6],
							plain_vector[i*8+7]));
		}
	}
};
#endif

inline bool number_is_power_of_two(const unsigned long x)
{
	return ( (x>0) && ((x & (x-1))==0) );
}

}

void calculate_vertices_in_parallel(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	std::set<std::string> available_processing_method_names;
	{
		available_processing_method_names.insert("simulated");
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

	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
	pohw.describe_io("stdout", false, true, "list of Voronoi vertices, i.e. quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')");

	const std::string method=poh.argument<std::string>(pohw.describe_option("--method", "string", "parallelization method name, variants are:"+available_processing_method_names_string, true), "");
	const std::size_t parts=poh.argument<std::size_t>(pohw.describe_option("--parts", "number", "number of parts for splitting, must be power of 2", true), 0);
	const bool print_log=poh.contains_option(pohw.describe_option("--print-log", "", "flag to print log of calculations"));
	const bool include_surplus_quadruples=poh.contains_option(pohw.describe_option("--include-surplus-quadruples", "", "flag to include surplus quadruples"));
	const bool link=poh.contains_option(pohw.describe_option("--link", "", "flag to output links between vertices"));
	const double init_radius_for_BSH=poh.argument<double>(pohw.describe_option("--init-radius-for-BSH", "number", "initial radius for bounding sphere hierarchy"), 3.5);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	if(available_processing_method_names.count(method)==0)
	{
		throw std::runtime_error("Invalid processing method name, acceptable values are:"+available_processing_method_names_string+".");
	}

	if(!number_is_power_of_two(parts))
	{
		throw std::runtime_error("Number of parts must be power of 2.");
	}

	if(init_radius_for_BSH<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	ParallelComputationResult result;
	bool master_finished=true;

	if(method=="simulated")
	{
		ParallelComputationProcessingSimulated::process(parts, init_radius_for_BSH, include_surplus_quadruples, result);
	}
#ifdef _OPENMP
	else if(method=="openmp")
	{
		ParallelComputationProcessingWithOpenMP::process(parts, init_radius_for_BSH, include_surplus_quadruples, result);
	}
#endif
#ifdef ENABLE_MPI
	else if(method=="mpi")
	{
		master_finished=ParallelComputationProcessingWithMPI::process(poh.unused_argv(), parts, init_radius_for_BSH, include_surplus_quadruples, result);
	}
#endif
	else
	{
		throw std::runtime_error("Processing method '"+method+"' is not available.");
	}

	if(master_finished)
	{
		if(link)
		{
			voronota::apollota::TriangulationOutput::print_vertices_vector_with_vertices_graph(voronota::apollota::Triangulation::collect_vertices_vector_from_quadruples_map(result.merged_quadruples_map), voronota::apollota::Triangulation::construct_vertices_graph(result.input_spheres, result.merged_quadruples_map), std::cout);
		}
		else
		{
			voronota::apollota::TriangulationOutput::print_vertices_vector(voronota::apollota::Triangulation::collect_vertices_vector_from_quadruples_map(result.merged_quadruples_map), std::cout);
		}

		if(print_log)
		{
			std::clog << "balls " << result.input_spheres.size() << "\n";
			std::clog << "parts " << result.number_of_initialized_parts << "\n";
			std::clog << "produced_quadruples " << result.number_of_produced_quadruples << "\n";
			std::clog << "merged_quadruples " << result.merged_quadruples_map.size() << "\n";
			std::clog << "tangent_spheres " << voronota::apollota::Triangulation::count_tangent_spheres_in_quadruples_map(result.merged_quadruples_map) << "\n";
		}
	}
}
