#include <list>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

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
		const std::map<CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(filename);
		if(!map_of_contacts.empty())
		{
			maps_of_contacts[filename.substr(common_path_start_length)]=map_of_contacts;
		}
	}
	return maps_of_contacts;
}

const std::map<CRADsPair, std::size_t> collect_crads_ids(const std::map< std::string, std::map<CRADsPair, double> >& maps_of_contacts)
{
	std::map<CRADsPair, std::size_t> crads_ids;
	for(std::map< std::string, std::map<CRADsPair, double> >::const_iterator it=maps_of_contacts.begin();it!=maps_of_contacts.end();++it)
	{
		const std::map<CRADsPair, double>& map_of_contacts=it->second;
		for(std::map<CRADsPair, double>::const_iterator jt=map_of_contacts.begin();jt!=map_of_contacts.end();++jt)
		{
			crads_ids[jt->first]=0;
		}
	}
	{
		std::size_t i=0;
		for(std::map<CRADsPair, std::size_t>::iterator it=crads_ids.begin();it!=crads_ids.end();++it)
		{
			it->second=i;
			i++;
		}
	}
	return crads_ids;
}

std::map< std::string, std::vector<double> > collect_map_of_areas_vectors(
		const std::map< std::string, std::map<CRADsPair, double> >& maps_of_contacts,
		const std::map<CRADsPair, std::size_t>& crads_ids)
{
	std::map< std::string, std::vector<double> > map_of_areas_vectors;
	for(std::map< std::string, std::map<CRADsPair, double> >::const_iterator it=maps_of_contacts.begin();it!=maps_of_contacts.end();++it)
	{
		const std::map<CRADsPair, double>& map_of_contacts=it->second;
		std::vector<double>& areas_vector=map_of_areas_vectors[it->first];
		areas_vector.resize(crads_ids.size());
		for(std::map<CRADsPair, double>::const_iterator jt=map_of_contacts.begin();jt!=map_of_contacts.end();++jt)
		{
			std::map<CRADsPair, std::size_t>::const_iterator crads_ids_it=crads_ids.find(jt->first);
			if(crads_ids_it!=crads_ids.end())
			{
				areas_vector[crads_ids_it->second]=jt->second;
			}
		}
	}
	return map_of_areas_vectors;
}

void print_map_of_areas_vectors(
		const std::map<CRADsPair, std::size_t>& crads_ids,
		const std::map< std::string, std::vector<double> >& map_of_areas_vectors)
{
	std::cout << "title";
	for(std::map<CRADsPair, std::size_t>::const_iterator it=crads_ids.begin();it!=crads_ids.end();++it)
	{
		const CRADsPair& crads=it->first;
		std::cout << " " << crads.a.str() << "__" << crads.b.str();
	}
	std::cout << "\n";
	for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
	{
		std::cout << it->first;
		const std::vector<double>& areas_vector=it->second;
		for(std::size_t i=0;i<areas_vector.size();i++)
		{
			std::cout << " " << areas_vector[i];
		}
		std::cout << "\n";
	}
}

void print_map_of_areas_vectors_transposed(
		const std::map<CRADsPair, std::size_t>& crads_ids,
		const std::map< std::string, std::vector<double> >& map_of_areas_vectors)
{
	std::cout << "contact";
	for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
	{
		std::cout << " " << it->first;
	}
	std::cout << "\n";
	std::size_t i=0;
	for(std::map<CRADsPair, std::size_t>::const_iterator it=crads_ids.begin();it!=crads_ids.end();++it)
	{
		const CRADsPair& crads=it->first;
		std::cout << crads.a.str() << "__" << crads.b.str();
		for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
		{
			const std::vector<double>& areas_vector=it->second;
			std::cout << " " << (i<areas_vector.size() ? areas_vector[i] : 0.0);
		}
		std::cout << "\n";
		i++;
	}
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

template<typename MapOfVectors, typename Functor>
void print_similarity_matrix(const MapOfVectors& map_of_vectors, const std::string& output_file, Functor functor)
{
	if(map_of_vectors.empty() || output_file.empty())
	{
		return;
	}
	std::ofstream output(output_file.c_str(), std::ios::out);
	if(!output.good())
	{
		return;
	}
	for(typename MapOfVectors::const_iterator it=map_of_vectors.begin();it!=map_of_vectors.end();++it)
	{
		output << (it==map_of_vectors.begin() ? "" : " ") << it->first;
	}
	output << "\n";
	for(typename MapOfVectors::const_iterator it=map_of_vectors.begin();it!=map_of_vectors.end();++it)
	{
		for(typename MapOfVectors::const_iterator jt=map_of_vectors.begin();jt!=map_of_vectors.end();++jt)
		{
			output << (jt==map_of_vectors.begin() ? "" : " ") << functor(it->second, jt->second);
		}
		output << "\n";
	}
}

template<typename Map>
std::vector< typename Map::const_iterator > collect_const_iterators_of_map(const Map& map)
{
	std::vector< typename Map::const_iterator > iterators;
	iterators.reserve(map.size());
	for(typename Map::const_iterator it=map.begin();it!=map.end();++it)
	{
		iterators.push_back(it);
	}
	return iterators;
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

template<typename IteratorsOfMapOfVectors, typename Functor>
std::vector< std::pair< std::size_t, std::list<std::size_t> > > calc_clusters(
		const IteratorsOfMapOfVectors& iterators_of_map_of_vectors,
		Functor functor,
		const double threshold,
		const bool eliminate_false_singletons)
{
	typedef std::list<std::size_t> Neighbors;
	typedef std::vector< std::pair<std::size_t, Neighbors> > ListOfNeighbors;
	typedef std::map< std::size_t, Neighbors > MapOfNeighbors;

	ListOfNeighbors big_clusters;
	ListOfNeighbors true_singleton_clusters;
	ListOfNeighbors false_singleton_clusters;

	MapOfNeighbors map_of_available_neighbors;
	for(std::size_t i=0;i<iterators_of_map_of_vectors.size();i++)
	{
		Neighbors& neighbors=map_of_available_neighbors[i];
		for(std::size_t j=0;j<iterators_of_map_of_vectors.size();j++)
		{
			const double similarity=functor(iterators_of_map_of_vectors[i]->second, iterators_of_map_of_vectors[j]->second);
			if(similarity>=threshold)
			{
				neighbors.push_back(j);
			}
		}
	}

	std::vector<int> deletion_flags(iterators_of_map_of_vectors.size(), 0);
	{
		MapOfNeighbors::iterator map_it=map_of_available_neighbors.begin();
		while(map_it!=map_of_available_neighbors.end())
		{
			if(map_it->second.size()<2)
			{
				true_singleton_clusters.push_back(std::make_pair(map_it->first, map_it->second));
				deletion_flags[map_it->first]=1;
				MapOfNeighbors::iterator deletion_it=map_it;
				++map_it;
				map_of_available_neighbors.erase(deletion_it);
			}
			else
			{
				++map_it;
			}
		}
	}

	while(!map_of_available_neighbors.empty())
	{
		MapOfNeighbors::const_iterator map_it_of_most_neighbors=map_of_available_neighbors.end();
		for(MapOfNeighbors::iterator map_it=map_of_available_neighbors.begin();map_it!=map_of_available_neighbors.end();++map_it)
		{
			Neighbors& neighbors=map_it->second;
			{
				Neighbors::iterator neighbors_it=neighbors.begin();
				while(neighbors_it!=neighbors.end())
				{
					if(deletion_flags[*neighbors_it]!=0)
					{
						Neighbors::iterator deletion_it=neighbors_it;
						++neighbors_it;
						neighbors.erase(deletion_it);
					}
					else
					{
						++neighbors_it;
					}
				}
			}
			if(map_it_of_most_neighbors==map_of_available_neighbors.end() || neighbors.size()>map_it_of_most_neighbors->second.size())
			{
				map_it_of_most_neighbors=map_it;
			}
		}
		if(map_it_of_most_neighbors!=map_of_available_neighbors.end())
		{
			const std::size_t center=map_it_of_most_neighbors->first;
			const Neighbors neighbors=map_it_of_most_neighbors->second;
			for(Neighbors::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();++neighbors_it)
			{
				deletion_flags[*neighbors_it]=1;
				map_of_available_neighbors.erase(*neighbors_it);
			}
			deletion_flags[center]=1;
			map_of_available_neighbors.erase(center);
			if(neighbors.size()>1)
			{
				big_clusters.push_back(std::make_pair(center, neighbors));
			}
			else
			{
				false_singleton_clusters.push_back(std::make_pair(center, neighbors));
			}
		}
		else
		{
			map_of_available_neighbors.clear();
		}
	}

	if(eliminate_false_singletons && !false_singleton_clusters.empty())
	{
		for(ListOfNeighbors::iterator singletons_it=false_singleton_clusters.begin();singletons_it!=false_singleton_clusters.end();++singletons_it)
		{
			double max_similarity=0.0;
			ListOfNeighbors::iterator big_clusters_it_of_closest=big_clusters.end();
			for(ListOfNeighbors::iterator big_clusters_it=big_clusters.begin();big_clusters_it!=big_clusters.end();++big_clusters_it)
			{
				const double similarity=functor(
						iterators_of_map_of_vectors[singletons_it->first]->second,
						iterators_of_map_of_vectors[big_clusters_it->first]->second);
				if(big_clusters_it_of_closest==big_clusters.end() || similarity>max_similarity)
				{
					max_similarity=similarity;
					big_clusters_it_of_closest=big_clusters_it;
				}
			}
			if(big_clusters_it_of_closest!=big_clusters.end())
			{
				big_clusters_it_of_closest->second.push_back(singletons_it->first);
			}
		}
		false_singleton_clusters.clear();
	}

	ListOfNeighbors clusters=big_clusters;
	if(!false_singleton_clusters.empty())
	{
		clusters.insert(clusters.end(), false_singleton_clusters.begin(), false_singleton_clusters.end());
	}
	if(!true_singleton_clusters.empty())
	{
		clusters.insert(clusters.end(), true_singleton_clusters.begin(), true_singleton_clusters.end());
	}

	return clusters;
}

template<typename IteratorsOfMapOfVectors>
void print_clusters(
		const IteratorsOfMapOfVectors& iterators_of_map_of_areas_vectors,
		const std::vector< std::pair< std::size_t, std::list<std::size_t> > >& clusters,
		const std::string& output_file)
{
	std::ofstream output(output_file.c_str(), std::ios::out);
	if(!output.good())
	{
		return;
	}

	for(std::vector< std::pair< std::size_t, std::list<std::size_t> > >::const_iterator clusters_it=clusters.begin();clusters_it!=clusters.end();++clusters_it)
	{
		output << iterators_of_map_of_areas_vectors[clusters_it->first]->first;
		const std::list<std::size_t>& neighbors=clusters_it->second;
		for(std::list<std::size_t>::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();++neighbors_it)
		{
			if((*neighbors_it)!=clusters_it->first)
			{
				output << " " << iterators_of_map_of_areas_vectors[*neighbors_it]->first;
			}
		}
		output << "\n";
	}
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

	const std::map< std::string, std::map<CRADsPair, double> > maps_of_contacts=read_maps_of_contacts();
	if(maps_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	const std::map<CRADsPair, std::size_t> crads_ids=collect_crads_ids(maps_of_contacts);
	const std::map< std::string, std::vector<double> > map_of_areas_vectors=collect_map_of_areas_vectors(maps_of_contacts, crads_ids);
	const std::vector< std::map< std::string, std::vector<double> >::const_iterator > iterators_of_map_of_areas_vectors=collect_const_iterators_of_map(map_of_areas_vectors);

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
