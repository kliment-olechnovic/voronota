#include <list>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

std::map< std::string, std::map<CRADsPair, double> > read_maps_of_contacts()
{
	std::map< std::string, std::map<CRADsPair, double> > maps_of_contacts;
	const std::set<std::string> input_files=auxiliaries::IOUtilities().read_lines_to_set< std::set<std::string> >(std::cin);
	for(std::set<std::string>::const_iterator it=input_files.begin();it!=input_files.end();++it)
	{
		const std::string& filename=(*it);
		const std::map<CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(filename);
		if(!map_of_contacts.empty())
		{
			maps_of_contacts[filename]=map_of_contacts;
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

template<typename Functor>
void print_similarity_matrix(const std::map< std::string, std::vector<double> >& map_of_areas_vectors, const std::string& output_file, Functor functor)
{
	if(map_of_areas_vectors.empty() || output_file.empty())
	{
		return;
	}
	std::ofstream output(output_file.c_str(), std::ios::out);
	if(!output.good())
	{
		return;
	}
	for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
	{
		output << (it==map_of_areas_vectors.begin() ? "" : " ") << it->first;
	}
	output << "\n";
	for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
	{
		for(std::map< std::string, std::vector<double> >::const_iterator jt=map_of_areas_vectors.begin();jt!=map_of_areas_vectors.end();++jt)
		{
			output << (jt==map_of_areas_vectors.begin() ? "" : " ") << functor(it->second, jt->second);
		}
		output << "\n";
	}
}

template<typename Functor>
std::vector< std::pair< std::size_t, std::list<std::size_t> > > calc_clusters(
		const std::map< std::string, std::vector<double> >& map_of_areas_vectors,
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
	std::vector<int> deletion_flags(map_of_areas_vectors.size(), 0);

	{
		std::size_t i=0;
		for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
		{
			Neighbors& neighbors=map_of_available_neighbors[i];
			std::size_t j=0;
			for(std::map< std::string, std::vector<double> >::const_iterator jt=map_of_areas_vectors.begin();jt!=map_of_areas_vectors.end();++jt)
			{
				const double similarity=functor(it->second, jt->second);
				if(similarity>=threshold)
				{
					neighbors.push_back(j);
				}
				j++;
			}
			i++;
		}
	}

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
		std::vector< std::map< std::string, std::vector<double> >::const_iterator > iterators_of_map_of_areas_vectors(map_of_areas_vectors.size());
		{
			std::size_t i=0;
			for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
			{
				iterators_of_map_of_areas_vectors[i]=it;
				i++;
			}
		}
		for(ListOfNeighbors::iterator singletons_it=false_singleton_clusters.begin();singletons_it!=false_singleton_clusters.end();++singletons_it)
		{
			double max_similarity=0.0;
			ListOfNeighbors::iterator big_clusters_it_of_closest=big_clusters.end();
			for(ListOfNeighbors::iterator big_clusters_it=big_clusters.begin();big_clusters_it!=big_clusters.end();++big_clusters_it)
			{
				const double similarity=functor(
						iterators_of_map_of_areas_vectors[singletons_it->first]->second,
						iterators_of_map_of_areas_vectors[big_clusters_it->first]->second);
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

void print_clusters(
		const std::map< std::string, std::vector<double> >& map_of_areas_vectors,
		const std::vector< std::pair< std::size_t, std::list<std::size_t> > >& clusters,
		const std::string& output_file)
{
	std::ofstream output(output_file.c_str(), std::ios::out);
	if(!output.good())
	{
		return;
	}

	std::vector<std::string> titles(map_of_areas_vectors.size());
	{
		std::size_t i=0;
		for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
		{
			titles[i]=(it->first);
			i++;
		}
	}

	for(std::vector< std::pair< std::size_t, std::list<std::size_t> > >::const_iterator map_it=clusters.begin();map_it!=clusters.end();++map_it)
	{
		output << titles[map_it->first];
		const std::list<std::size_t>& neighbors=map_it->second;
		for(std::list<std::size_t>::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();++neighbors_it)
		{
			if((*neighbors_it)!=map_it->first)
			{
				output << " " << titles[*neighbors_it];
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

	const std::string cadscore_matrix_file=poh.argument<std::string>(pohw.describe_option("--CAD-score-matrix", "string", "file path to output CAD-score matrix"), "");
	const std::string distance_matrix_file=poh.argument<std::string>(pohw.describe_option("--distance-matrix", "string", "file path to output euclidean distance matrix"), "");
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

	print_similarity_matrix(map_of_areas_vectors, cadscore_matrix_file, calc_cadscore_of_two_vectors);
	print_similarity_matrix(map_of_areas_vectors, distance_matrix_file, calc_euclidean_distance_of_two_vectors);

	if(!clustering_output_file.empty())
	{
		print_clusters(
				map_of_areas_vectors,
				calc_clusters(map_of_areas_vectors, calc_cadscore_of_two_vectors, clustering_threshold, true),
				clustering_output_file);
	}

	print_map_of_areas_vectors(crads_ids, map_of_areas_vectors);
}
