#ifndef CADSCORELT_CLUSTERING_H_
#define CADSCORELT_CLUSTERING_H_

#include <cstdint>
#include <cmath>
#include <vector>
#include <algorithm>

namespace cadscorelt
{

template<typename SimilarityValue>
class TaylorButinaInspiredClustering
{
public:
	static bool symmetrize_similarities_using_min(std::vector<SimilarityValue>& similarities) noexcept
	{
		const std::size_t N=static_cast<std::size_t>(std::sqrt(static_cast<double>(similarities.size())));
		if(similarities.size()!=(N*N))
		{
			return false;
		}

		for(std::size_t i=0;i<N;i++)
		{
			for(std::size_t j=i+1;j<N;j++)
			{
				const std::size_t a=(i*N+j);
				const std::size_t b=(j*N+i);
				const SimilarityValue min_value=std::min(similarities[a], similarities[b]);
				similarities[a]=min_value;
				similarities[b]=min_value;
			}
		}

		return true;
	}

	static bool cluster(const std::vector<SimilarityValue>& similarities, const SimilarityValue threshold, std::vector<int>& clusters) noexcept
	{
		clusters.clear();

		if(similarities.empty())
		{
			return false;
		}

		const std::size_t N=static_cast<std::size_t>(std::sqrt(static_cast<double>(similarities.size())));
		if(similarities.size()!=(N*N))
		{
			return false;
		}

		clusters.resize(N, 0);
		{
			int current_cluster_id=0;

			{
				std::vector< std::pair<int, std::size_t> > ids_with_neighbor_negative_counts;
				ids_with_neighbor_negative_counts.reserve(N);

				bool progressing=true;

				while(progressing)
				{
					ids_with_neighbor_negative_counts.clear();
					for(std::size_t i=0;i<N;i++)
					{
						if(clusters[i]==0)
						{
							ids_with_neighbor_negative_counts.push_back(std::pair<int, std::size_t>(0, i));
							for(std::size_t j=0;j<N;j++)
							{
								if(i!=j && clusters[j]==0 && similarities[i*N+j]>=threshold)
								{
									ids_with_neighbor_negative_counts.back().first--;
								}
							}
						}
					}
					std::vector< std::pair<int, std::size_t> >::const_iterator best_it=std::min_element(ids_with_neighbor_negative_counts.begin(), ids_with_neighbor_negative_counts.end());
					if(best_it!=ids_with_neighbor_negative_counts.end() && best_it->first<0)
					{
						const std::size_t i=best_it->second;
						current_cluster_id++;
						for(std::size_t j=0;j<N;j++)
						{
							if(clusters[j]==0 && (i==j || similarities[i*N+j]>=threshold))
							{
								clusters[j]=current_cluster_id;
							}
						}
					}
					else
					{
						progressing=false;
					}
				}
			}

			for(std::size_t i=0;i<N;i++)
			{
				if(clusters[i]==0)
				{
					std::pair<int, SimilarityValue> cluster_with_max_similarity;
					cluster_with_max_similarity.first=0;
					for(std::size_t j=0;j<N;j++)
					{
						const SimilarityValue similarity_value=similarities[i*N+j];
						if(clusters[j]!=0 && (cluster_with_max_similarity.first==0 || similarity_value>cluster_with_max_similarity.second))
						{
							cluster_with_max_similarity.first=clusters[j];
							cluster_with_max_similarity.second=similarity_value;
						}
					}
					if(cluster_with_max_similarity.first!=0 && cluster_with_max_similarity.second>=threshold)
					{
						clusters[i]=cluster_with_max_similarity.first;
					}
				}
			}

			for(std::size_t i=0;i<N;i++)
			{
				if(clusters[i]==0)
				{
					current_cluster_id++;
					clusters[i]=current_cluster_id;
				}
			}
		}

		return true;
	}
};

}

#endif /* CADSCORELT_CLUSTERING_H_ */
