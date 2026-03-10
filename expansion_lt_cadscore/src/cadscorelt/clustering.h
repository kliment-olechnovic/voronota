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
	static bool symmetrize_matrix_of_similarities_using_min(std::vector<SimilarityValue>& matrix_of_similarities) noexcept
	{
		const std::size_t N=static_cast<std::size_t>(std::sqrt(static_cast<double>(matrix_of_similarities.size())));
		if(matrix_of_similarities.size()!=(N*N))
		{
			return false;
		}

		for(std::size_t i=0;i<N;i++)
		{
			for(std::size_t j=i+1;j<N;j++)
			{
				const std::size_t a=(i*N+j);
				const std::size_t b=(j*N+i);
				const SimilarityValue min_value=std::min(matrix_of_similarities[a], matrix_of_similarities[b]);
				matrix_of_similarities[a]=min_value;
				matrix_of_similarities[b]=min_value;
			}
		}

		return true;
	}

	static bool construct_matrix_of_similarities_from_list_of_similarities(const std::size_t N, const SimilarityValue min_similarity, const SimilarityValue max_similarity, const std::vector< std::pair<std::size_t, std::size_t> >& list_of_pairs_of_indices, const std::vector<SimilarityValue>& list_of_similarities, std::vector<SimilarityValue>& matrix_of_similarities) noexcept
	{
		matrix_of_similarities.clear();
		if(list_of_similarities.size()!=list_of_pairs_of_indices.size())
		{
			return false;
		}
		for(std::size_t l=0;l<list_of_pairs_of_indices.size();l++)
		{
			const std::pair<std::size_t, std::size_t>& p=list_of_pairs_of_indices[l];
			if(!(p.first<N && p.second<N))
			{
				return false;
			}
		}
		matrix_of_similarities.resize(N*N, min_similarity);
		for(std::size_t l=0;l<list_of_pairs_of_indices.size();l++)
		{
			const std::pair<std::size_t, std::size_t>& p=list_of_pairs_of_indices[l];
			matrix_of_similarities[p.first*N+p.second]=list_of_similarities[l];
		}
		for(std::size_t i=0;i<N;i++)
		{
			matrix_of_similarities[i*N+i]=max_similarity;
		}
		return symmetrize_matrix_of_similarities_using_min(matrix_of_similarities);
	}

	static bool cluster_using_single_threshold(const std::vector<SimilarityValue>& matrix_of_similarities, const SimilarityValue threshold, std::vector<int>& clusters) noexcept
	{
		clusters.clear();

		if(matrix_of_similarities.empty())
		{
			return false;
		}

		const std::size_t N=static_cast<std::size_t>(std::sqrt(static_cast<double>(matrix_of_similarities.size())));
		if(matrix_of_similarities.size()!=(N*N))
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
								if(i!=j && clusters[j]==0 && matrix_of_similarities[i*N+j]>=threshold)
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
							if(clusters[j]==0 && (i==j || matrix_of_similarities[i*N+j]>=threshold))
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
						const SimilarityValue similarity_value=matrix_of_similarities[i*N+j];
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

	static bool cluster_using_multiple_thresholds(const std::vector<SimilarityValue>& matrix_of_similarities, const std::vector<SimilarityValue>& multiple_thresholds, std::vector< std::vector<int> >& multiple_clusterings) noexcept
	{
		multiple_clusterings.clear();
		if(multiple_thresholds.empty())
		{
			return false;
		}
		multiple_clusterings.resize(multiple_thresholds.size());
		for(std::size_t i=0;i<multiple_thresholds.size();i++)
		{
			if(!cluster_using_single_threshold(matrix_of_similarities, multiple_thresholds[i], multiple_clusterings[i]))
			{
				multiple_clusterings.clear();
				return false;
			}
		}
		return true;
	}
};

}

#endif /* CADSCORELT_CLUSTERING_H_ */
