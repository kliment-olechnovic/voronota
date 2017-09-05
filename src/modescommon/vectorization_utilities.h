#ifndef MODESCOMMON_VECTORIZATION_UTILITIES_H_
#define MODESCOMMON_VECTORIZATION_UTILITIES_H_

#include <vector>
#include <list>
#include <map>

#include "filepath_utilities.h"

namespace modescommon
{

template<typename VectorNameType, typename ValueNameType, typename ValueType>
class VectorizationUtilities
{
public:
	typedef std::map<ValueNameType, ValueType> Map;
	typedef std::map<ValueNameType, std::size_t> MapKeysIDs;
	typedef std::map<VectorNameType, Map> MapOfMaps;
	typedef std::vector<ValueType> Vector;
	typedef std::map<std::string, Vector> MapOfVectors;
	typedef std::vector<typename MapOfVectors::const_iterator> IteratorsOfMapOfVectors;
	typedef std::vector< std::pair< std::size_t, std::list<std::size_t> > > Clusters;

	static MapOfMaps read_map_of_maps_from_multiple_files(const std::set<std::string>& input_files)
	{
		MapOfMaps map_of_maps;
		if(!input_files.empty())
		{
			const std::size_t common_path_start_length=modescommon::calc_common_path_start_length(input_files);
			for(std::set<std::string>::const_iterator it=input_files.begin();it!=input_files.end();++it)
			{
				const std::string& filename=(*it);
				const Map map=auxiliaries::IOUtilities().read_file_lines_to_map<Map>(filename);
				if(!map.empty())
				{
					map_of_maps[filename.substr(common_path_start_length)]=map;
				}
			}
		}
		return map_of_maps;
	}

	static const MapKeysIDs collect_map_keys_ids(const MapOfMaps& map_of_maps)
	{
		MapKeysIDs map_keys_ids;
		for(typename MapOfMaps::const_iterator it=map_of_maps.begin();it!=map_of_maps.end();++it)
		{
			const Map& map=it->second;
			for(typename Map::const_iterator jt=map.begin();jt!=map.end();++jt)
			{
				map_keys_ids[jt->first]=0;
			}
		}
		{
			std::size_t i=0;
			for(typename MapKeysIDs::iterator it=map_keys_ids.begin();it!=map_keys_ids.end();++it)
			{
				it->second=i;
				i++;
			}
		}
		return map_keys_ids;
	}

	static MapOfVectors collect_map_of_vectors(
			const MapOfMaps& map_of_maps,
			const MapKeysIDs& map_keys_ids)
	{
		MapOfVectors map_of_vectors;
		for(typename MapOfMaps::const_iterator it=map_of_maps.begin();it!=map_of_maps.end();++it)
		{
			const Map& map=it->second;
			Vector& vector=map_of_vectors[it->first];
			vector.resize(map_keys_ids.size());
			for(typename Map::const_iterator jt=map.begin();jt!=map.end();++jt)
			{
				typename MapKeysIDs::const_iterator map_keys_ids_it=map_keys_ids.find(jt->first);
				if(map_keys_ids_it!=map_keys_ids.end())
				{
					vector[map_keys_ids_it->second]=jt->second;
				}
			}
		}
		return map_of_vectors;
	}

	static void print_map_of_areas_vectors(
			const MapKeysIDs& map_keys_ids,
			const MapOfVectors& map_of_vectors)
	{
		std::cout << "title";
		for(typename MapKeysIDs::const_iterator it=map_keys_ids.begin();it!=map_keys_ids.end();++it)
		{
			std::cout << " " << it->first;
		}
		std::cout << "\n";
		for(typename MapOfVectors::const_iterator it=map_of_vectors.begin();it!=map_of_vectors.end();++it)
		{
			std::cout << it->first;
			const Vector& vector=it->second;
			for(std::size_t i=0;i<vector.size();i++)
			{
				std::cout << " " << vector[i];
			}
			std::cout << "\n";
		}
	}

	static void print_map_of_areas_vectors_transposed(
			const MapKeysIDs& map_keys_ids,
			const MapOfVectors& map_of_vectors)
	{
		std::cout << "contact";
		for(typename MapOfVectors::const_iterator it=map_of_vectors.begin();it!=map_of_vectors.end();++it)
		{
			std::cout << " " << it->first;
		}
		std::cout << "\n";
		std::size_t i=0;
		for(typename MapKeysIDs::const_iterator it=map_keys_ids.begin();it!=map_keys_ids.end();++it)
		{
			std::cout << it->first;
			for(typename MapOfVectors::const_iterator jt=map_of_vectors.begin();jt!=map_of_vectors.end();++jt)
			{
				const Vector& vector=jt->second;
				std::cout << " " << (i<vector.size() ? vector[i] : 0.0);
			}
			std::cout << "\n";
			i++;
		}
	}

	template<typename Functor>
	static void print_similarity_matrix(const MapOfVectors& map_of_vectors, const std::string& output_file, Functor functor)
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

	static IteratorsOfMapOfVectors collect_iterators_of_map_of_vectors(const MapOfVectors& map)
	{
		IteratorsOfMapOfVectors iterators;
		iterators.reserve(map.size());
		for(typename MapOfVectors::const_iterator it=map.begin();it!=map.end();++it)
		{
			iterators.push_back(it);
		}
		return iterators;
	}

	template<typename Functor>
	static Clusters calc_clusters(
			const IteratorsOfMapOfVectors& iterators_of_map_of_vectors,
			Functor functor,
			const double threshold,
			const bool eliminate_false_singletons,
			const bool invert_similarity)
	{
		typedef std::list<std::size_t> Neighbors;
		typedef std::vector< std::pair<std::size_t, Neighbors> > ListOfNeighbors;
		typedef std::map< std::size_t, Neighbors > MapOfNeighbors;

		const double similarity_modifier=(invert_similarity ? -1.0 : 1.0);

		ListOfNeighbors big_clusters;
		ListOfNeighbors true_singleton_clusters;
		ListOfNeighbors false_singleton_clusters;

		MapOfNeighbors map_of_available_neighbors;
		for(std::size_t i=0;i<iterators_of_map_of_vectors.size();i++)
		{
			Neighbors& neighbors=map_of_available_neighbors[i];
			for(std::size_t j=0;j<iterators_of_map_of_vectors.size();j++)
			{
				const double similarity=functor(iterators_of_map_of_vectors[i]->second, iterators_of_map_of_vectors[j]->second)*similarity_modifier;
				if(similarity>=threshold*similarity_modifier)
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
							iterators_of_map_of_vectors[big_clusters_it->first]->second)*similarity_modifier;
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

	static void print_clusters(
			const IteratorsOfMapOfVectors& iterators_of_map_of_areas_vectors,
			const Clusters& clusters,
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

	static Vector calc_consensus_vector(const IteratorsOfMapOfVectors& iterators_of_map_of_vectors)
	{
		Vector result;
		for(std::size_t i=0;i<iterators_of_map_of_vectors.size();i++)
		{
			const Vector& v=iterators_of_map_of_vectors[i]->second;
			if(result.size()<v.size())
			{
				result.resize(v.size());
			}
			for(std::size_t j=0;j<result.size();j++)
			{
				result[j]=result[j]+v[j];
			}
		}
		for(std::size_t j=0;j<result.size();j++)
		{
			result[j]=result[j]*(1.0/static_cast<double>(iterators_of_map_of_vectors.size()));
		}
		return result;
	}

	template<typename Functor>
	static std::multimap<double, std::size_t> calc_consensus_similarities(
			const IteratorsOfMapOfVectors& iterators_of_map_of_vectors,
			Functor functor)
	{
		std::multimap<double, std::size_t> similarities;
		const Vector consensus=calc_consensus_vector(iterators_of_map_of_vectors);
		for(std::size_t i=0;i<iterators_of_map_of_vectors.size();i++)
		{
			similarities.insert(std::make_pair(functor(consensus, iterators_of_map_of_vectors[i]->second), i));
		}
		return similarities;
	}

	static void print_consensus_similarities(
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
};

}

#endif /* MODESCOMMON_VECTORIZATION_UTILITIES_H_ */
