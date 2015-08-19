#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/vectorization_utilities.h"

namespace
{

class CRADsPair : public auxiliaries::ChainResidueAtomDescriptorsPair
{
};

inline std::ostream& operator<<(std::ostream& output, const CRADsPair& crads_pair)
{
	if(crads_pair.reversed_display)
	{
		output << crads_pair.b << "__" << crads_pair.a;
	}
	else
	{
		output << crads_pair.a << "__" << crads_pair.b;
	}
	return output;
}

double calc_configurable_cadscore_of_two_vectors(const std::vector<double>& a, const std::vector<double>& b, const bool symmetric)
{
	if(a.size()!=b.size())
	{
		throw std::runtime_error("Vector sizes are not equal.");
	}
	double sum_diffs=0.0;
	double sum_all=0.0;
	for(std::size_t i=0;i<a.size();i++)
	{
		const double ref=(symmetric ? std::max(a[i], b[i]) : a[i]);
		sum_diffs+=std::min(fabs(a[i]-b[i]), ref);
		sum_all+=ref;
	}
	return (1.0-(sum_diffs/sum_all));
}

double calc_cadscore_of_two_vectors(const std::vector<double>& a, const std::vector<double>& b)
{
	return calc_configurable_cadscore_of_two_vectors(a, b, true);
}

double calc_euclidean_distance_of_two_vectors(const std::vector<double>& a, const std::vector<double>& b)
{
	if(a.size()!=b.size())
	{
		throw std::runtime_error("Vector sizes are not equal.");
	}
	double sum=0.0;
	for(std::size_t i=0;i<a.size();i++)
	{
		sum+=((a[i]-b[i])*(a[i]-b[i]));
	}
	return sqrt(sum);
}

}

void vectorize_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts files");
	pohw.describe_io("stdout", false, true, "table of contacts vectors");

	const bool transpose=poh.contains_option(pohw.describe_option("--transpose", "", "flag to transpose output table"));
	const std::string cadscore_matrix_file=poh.argument<std::string>(pohw.describe_option("--CAD-score-matrix", "string", "file path to output CAD-score matrix"), "");
	const std::string distance_matrix_file=poh.argument<std::string>(pohw.describe_option("--distance-matrix", "string", "file path to output euclidean distance matrix"), "");
	const std::string consensus_list_file=poh.argument<std::string>(pohw.describe_option("--consensus-list", "string", "file path to output ordered similarities to consensus (average contacts)"), "");
	const std::string clustering_output_file=poh.argument<std::string>(pohw.describe_option("--clustering-output", "string", "file path to output clusters"), "");
	const double clustering_threshold=poh.argument<double>(pohw.describe_option("--clustering-threshold", "string", "clustering threshold value"), 0.5);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	typedef VectorizationUtilities<std::string, CRADsPair, double> Vectorizer;

	const Vectorizer::MapOfMaps maps_of_maps=Vectorizer::read_map_of_maps_from_multiple_files(auxiliaries::IOUtilities().read_lines_to_set< std::set<std::string> >(std::cin));
	if(maps_of_maps.empty())
	{
		throw std::runtime_error("No input.");
	}

	const Vectorizer::MapKeysIDs map_keys_ids=Vectorizer::collect_map_keys_ids(maps_of_maps);
	const Vectorizer::MapOfVectors map_of_vectors=Vectorizer::collect_map_of_vectors(maps_of_maps, map_keys_ids);
	const Vectorizer::IteratorsOfMapOfVectors iterators_of_map_of_vectors=Vectorizer::collect_iterators_of_map_of_vectors(map_of_vectors);

	Vectorizer::print_similarity_matrix(map_of_vectors, cadscore_matrix_file, calc_cadscore_of_two_vectors);
	Vectorizer::print_similarity_matrix(map_of_vectors, distance_matrix_file, calc_euclidean_distance_of_two_vectors);

	if(!consensus_list_file.empty())
	{
		Vectorizer::print_consensus_similarities(
				iterators_of_map_of_vectors,
				Vectorizer::calc_consensus_similarities(iterators_of_map_of_vectors, calc_cadscore_of_two_vectors),
				consensus_list_file);
	}

	if(!clustering_output_file.empty())
	{
		Vectorizer::print_clusters(
				iterators_of_map_of_vectors,
				Vectorizer::calc_clusters(iterators_of_map_of_vectors, calc_cadscore_of_two_vectors, clustering_threshold, true, false),
				clustering_output_file);
	}

	if(transpose)
	{
		Vectorizer::print_map_of_areas_vectors_transposed(map_keys_ids, map_of_vectors);
	}
	else
	{
		Vectorizer::print_map_of_areas_vectors(map_keys_ids, map_of_vectors);
	}
}
