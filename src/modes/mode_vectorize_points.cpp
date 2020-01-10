#include "../apollota/basic_operations_on_points.h"

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "modescommon/vectorization_utilities.h"

namespace
{

typedef voronota::apollota::SimplePoint Point;

double calc_rmsd(const std::vector<Point>& a, const std::vector<Point>& b)
{
	if(a.size()!=b.size())
	{
		throw std::runtime_error("Vector sizes are not equal.");
	}
	double sum=0.0;
	if(!a.empty())
	{
		for(std::size_t i=0;i<a.size();i++)
		{
			sum+=voronota::apollota::squared_distance_from_point_to_point(a[i], b[i]);
		}
		sum/=static_cast<double>(a.size());
	}
	return sqrt(sum);
}

}

void vectorize_points(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of points files");
	pohw.describe_io("stdout", false, true, "nothing");

	const std::string rmsd_matrix_file=poh.argument<std::string>(pohw.describe_option("--RMSD-matrix", "string", "file path to output RMSD matrix"), "");
	const std::string consensus_list_file=poh.argument<std::string>(pohw.describe_option("--consensus-list", "string", "file path to output ordered similarities to consensus (average points)"), "");
	const std::string clustering_output_file=poh.argument<std::string>(pohw.describe_option("--clustering-output", "string", "file path to output clusters"), "");
	const double clustering_threshold=poh.argument<double>(pohw.describe_option("--clustering-threshold", "string", "clustering threshold value"), 0.0);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	typedef voronota::modescommon::VectorizationUtilities<std::string, std::string, Point> Vectorizer;

	const Vectorizer::MapOfMaps maps_of_maps=Vectorizer::read_map_of_maps_from_multiple_files(voronota::auxiliaries::IOUtilities().read_lines_to_set< std::set<std::string> >(std::cin));
	if(maps_of_maps.empty())
	{
		throw std::runtime_error("No input.");
	}

	const Vectorizer::MapKeysIDs map_keys_ids=Vectorizer::collect_map_keys_ids(maps_of_maps);
	const Vectorizer::MapOfVectors map_of_vectors=Vectorizer::collect_map_of_vectors(maps_of_maps, map_keys_ids);
	const Vectorizer::IteratorsOfMapOfVectors iterators_of_map_of_vectors=Vectorizer::collect_iterators_of_map_of_vectors(map_of_vectors);

	Vectorizer::print_similarity_matrix(map_of_vectors, rmsd_matrix_file, calc_rmsd);

	if(!consensus_list_file.empty())
	{
		Vectorizer::print_consensus_similarities(
				iterators_of_map_of_vectors,
				Vectorizer::calc_consensus_similarities(iterators_of_map_of_vectors, calc_rmsd),
				consensus_list_file);
	}

	if(!clustering_output_file.empty())
	{
		Vectorizer::print_clusters(
				iterators_of_map_of_vectors,
				Vectorizer::calc_clusters(iterators_of_map_of_vectors, calc_rmsd, clustering_threshold, true, true),
				clustering_output_file);
	}
}
