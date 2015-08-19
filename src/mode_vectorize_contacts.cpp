#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/vectorization_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

std::size_t calc_common_path_start_length(const std::set<std::string>& filenames)
{
	std::size_t common_path_start_length=0;
	if(filenames.size()>1)
	{
		const std::string& first_filename=(*filenames.begin());
		bool common_start=true;
		for(std::size_t pos=0;(common_start && pos<first_filename.size());pos++)
		{
			for(std::set<std::string>::const_iterator it=filenames.begin();(common_start && it!=filenames.end());++it)
			{
				const std::string& filename=(*it);
				if(!(pos<filename.size() && filename[pos]==first_filename[pos]))
				{
					common_start=false;
				}
			}
			if(common_start && (first_filename[pos]=='/' || first_filename[pos]=='\\'))
			{
				common_path_start_length=(pos+1);
			}
		}
	}
	return common_path_start_length;
}

std::map< std::string, std::map<CRADsPair, double> > read_maps_of_contacts()
{
	std::map< std::string, std::map<CRADsPair, double> > maps_of_contacts;
	const std::set<std::string> input_files=auxiliaries::IOUtilities().read_lines_to_set< std::set<std::string> >(std::cin);
	const std::size_t common_path_start_length=calc_common_path_start_length(input_files);
	for(std::set<std::string>::const_iterator it=input_files.begin();it!=input_files.end();++it)
	{
		const std::string& filename=(*it);
		const std::map<CRADsPair, double> raw_map_of_contacts=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(filename);
		if(!raw_map_of_contacts.empty())
		{
			maps_of_contacts[filename.substr(common_path_start_length)]=raw_map_of_contacts;
		}
	}
	return maps_of_contacts;
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

template<typename IteratorsOfMapOfVectors>
std::vector<double> calc_consensus_vector(const IteratorsOfMapOfVectors& iterators_of_map_of_vectors)
{
	std::vector<double> result;
	for(std::size_t i=0;i<iterators_of_map_of_vectors.size();i++)
	{
		const std::vector<double>& v=iterators_of_map_of_vectors[i]->second;
		if(result.size()<v.size())
		{
			result.resize(v.size(), 0.0);
		}
		for(std::size_t j=0;j<result.size();j++)
		{
			result[j]+=v[j];
		}
	}
	for(std::size_t j=0;j<result.size();j++)
	{
		result[j]/=static_cast<double>(iterators_of_map_of_vectors.size());
	}
	return result;
}

template<typename IteratorsOfMapOfVectors, typename Functor>
std::multimap<double, std::size_t> calc_consensus_similarities(
		const IteratorsOfMapOfVectors& iterators_of_map_of_vectors,
		Functor functor)
{
	std::multimap<double, std::size_t> similarities;
	const std::vector<double> consensus=calc_consensus_vector(iterators_of_map_of_vectors);
	for(std::size_t i=0;i<iterators_of_map_of_vectors.size();i++)
	{
		similarities.insert(std::make_pair(functor(consensus, iterators_of_map_of_vectors[i]->second), i));
	}
	return similarities;
}

template<typename IteratorsOfMapOfVectors>
void print_consensus_similarities(
		const IteratorsOfMapOfVectors& iterators_of_map_of_vectors,
		const std::multimap<double, std::size_t>& similarities,
		const std::string& output_file)
{
	std::ofstream output(output_file.c_str(), std::ios::out);
	if(!output.good())
	{
		return;
	}

	for(std::multimap<double, std::size_t>::const_reverse_iterator it=similarities.rbegin();it!=similarities.rend();++it)
	{
		if(it->second<iterators_of_map_of_vectors.size())
		{
			output << iterators_of_map_of_vectors[it->second]->first << " " << it->first << "\n";
		}
	}
}

}

void vectorize_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	typedef std::map< std::string, std::map<CRADsPair, double> > MapOfMaps;
	typedef std::map<CRADsPair, std::size_t> MapKeysIDs;
	typedef std::map< std::string, std::vector<double> > MapOfVectors;

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

	const MapOfMaps maps_of_contacts=read_maps_of_contacts();
	if(maps_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	const MapKeysIDs crads_ids=collect_map_keys_ids<MapKeysIDs>(maps_of_contacts);
	const MapOfVectors map_of_areas_vectors=collect_map_of_vectors<MapOfVectors>(maps_of_contacts, crads_ids);
	const std::vector<MapOfVectors::const_iterator> iterators_of_map_of_areas_vectors=collect_const_iterators_of_map(map_of_areas_vectors);

	print_similarity_matrix(map_of_areas_vectors, cadscore_matrix_file, calc_cadscore_of_two_vectors);
	print_similarity_matrix(map_of_areas_vectors, distance_matrix_file, calc_euclidean_distance_of_two_vectors);

	if(!consensus_list_file.empty())
	{
		print_consensus_similarities(
				iterators_of_map_of_areas_vectors,
				calc_consensus_similarities(iterators_of_map_of_areas_vectors, calc_cadscore_of_two_vectors),
				consensus_list_file);
	}

	if(!clustering_output_file.empty())
	{
		print_clusters(
				iterators_of_map_of_areas_vectors,
				calc_clusters(iterators_of_map_of_areas_vectors, calc_cadscore_of_two_vectors, clustering_threshold, true),
				clustering_output_file);
	}

	if(transpose)
	{
		print_map_of_areas_vectors_transposed(crads_ids, map_of_areas_vectors);
	}
	else
	{
		print_map_of_areas_vectors(crads_ids, map_of_areas_vectors);
	}
}
