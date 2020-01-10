#ifndef APOLLOTA_BOUNDING_SPHERES_HIERARCH_2_H_
#define APOLLOTA_BOUNDING_SPHERES_HIERARCH_2_H_

#include <vector>
#include <algorithm>
#include <limits>

#include "basic_operations_on_spheres.h"
#include "splitting_of_spheres.h"

namespace voronota
{

namespace apollota
{

class BoundingSpheresHierarchy
{
public:
	BoundingSpheresHierarchy()
	{
	}

	template<typename InputSphereType>
	BoundingSpheresHierarchy(const std::vector<InputSphereType>& input_spheres, const double initial_radius_for_spheres_bucketing, const std::size_t min_number_of_clusters) :
		leaves_spheres_(convert_input_spheres_to_simple_spheres(input_spheres)),
		input_radii_range_(calc_input_radii_range(leaves_spheres_)),
		clusters_layers_(cluster_spheres_in_layers(leaves_spheres_, initial_radius_for_spheres_bucketing, min_number_of_clusters))
	{
	}

	const std::vector<SimpleSphere>& leaves_spheres() const
	{
		return leaves_spheres_;
	}

	double min_input_radius() const
	{
		return input_radii_range_.first;
	}

	double max_input_radius() const
	{
		return input_radii_range_.second;
	}

	std::size_t levels() const
	{
		return clusters_layers_.size();
	}

	std::vector<SimpleSphere> collect_bounding_spheres(const std::size_t level) const
	{
		std::vector<SimpleSphere> result;
		if(level<clusters_layers_.size())
		{
			result.reserve(clusters_layers_[level].size());
			for(std::size_t i=0;i<clusters_layers_[level].size();i++)
			{
				result.push_back(SimpleSphere(clusters_layers_[level][i]));
			}
		}
		return result;
	}

	template<typename NodeChecker, typename LeafChecker>
	std::vector<std::size_t> search(NodeChecker& node_checker, LeafChecker& leaf_checker) const
	{
		std::vector<std::size_t> results;
		if(!clusters_layers_.empty())
		{
			std::vector<NodeCoordinates> stack;
			stack.reserve(clusters_layers_.back().size()+clusters_layers_.size()+1);
			{
				const std::size_t top_level=clusters_layers_.size()-1;
				for(std::size_t top_id=0;top_id<clusters_layers_[top_level].size();top_id++)
				{
					stack.push_back(NodeCoordinates(top_level, top_id, 0));
				}
			}
			while(!stack.empty())
			{
				const NodeCoordinates ncs=stack.back();
				if(
						ncs.level_id<clusters_layers_.size()
						&& ncs.cluster_id<clusters_layers_[ncs.level_id].size()
						&& ncs.child_id<clusters_layers_[ncs.level_id][ncs.cluster_id].children.size()
						&& (ncs.child_id>0 || node_checker(clusters_layers_[ncs.level_id][ncs.cluster_id]))
					)
				{
					const std::vector<std::size_t>& children=clusters_layers_[ncs.level_id][ncs.cluster_id].children;
					if(ncs.level_id==0)
					{
						for(std::size_t i=0;i<children.size();i++)
						{
							const std::size_t child=children[i];
							const std::pair<bool, bool> status=leaf_checker(child, leaves_spheres_[child]);
							if(status.first)
							{
								results.push_back(child);
								if(status.second)
								{
									return results;
								}
							}
						}
						stack.pop_back();
					}
					else
					{
						stack.back().child_id++;
						stack.push_back(NodeCoordinates(ncs.level_id-1, children[ncs.child_id], 0));
					}
				}
				else
				{
					stack.pop_back();
				}
			}
		}
		return results;
	}

	template<typename ListType, typename FunctorType>
	static std::vector<std::size_t> sort_objects_by_distance_to_one_of_them(const ListType& list, const std::size_t starting_id, const FunctorType& functor, const double max_distance=std::numeric_limits<double>::max())
	{
		std::vector<std::size_t> result;
		if(starting_id<list.size())
		{
			std::vector< std::pair<double, std::size_t> > distances;
			distances.reserve(list.size()-1);
			for(std::size_t i=0;i<list.size();i++)
			{
				if(i!=starting_id)
				{
					const double distance=functor(list[starting_id], list[i]);
					if(distance<=max_distance)
					{
						distances.push_back(std::make_pair(distance, i));
					}
				}
			}
			std::sort(distances.begin(), distances.end());
			result.reserve(distances.size()+1);
			result.push_back(starting_id);
			for(std::size_t i=0;i<distances.size();i++)
			{
				result.push_back(distances[i].second);
			}
		}
		return result;
	}

private:
	class Cluster : public SimpleSphere
	{
	public:
		std::vector<std::size_t> children;
		std::vector<std::size_t> leaves_ids;
	};

	struct NodeCoordinates
	{
		std::size_t level_id;
		std::size_t cluster_id;
		std::size_t child_id;

		NodeCoordinates(std::size_t level_id, std::size_t cluster_id, std::size_t child_id) :
			level_id(level_id), cluster_id(cluster_id), child_id(child_id)
		{
		}
	};

	template<typename InputSphereType>
	static std::vector<SimpleSphere> convert_input_spheres_to_simple_spheres(const std::vector<InputSphereType>& input_spheres)
	{
		std::vector<SimpleSphere> result;
		result.reserve(input_spheres.size());
		for(std::size_t i=0;i<input_spheres.size();i++)
		{
			result.push_back(SimpleSphere(input_spheres[i]));
		}
		return result;
	}

	template<typename SphereType>
	static std::pair<double, double> calc_input_radii_range(const std::vector<SphereType>& spheres)
	{
		std::pair<double, double> range(0, 0);
		if(!spheres.empty())
		{
			range.first=spheres.front().r;
			range.second=spheres.front().r;
			for(std::size_t i=1;i<spheres.size();i++)
			{
				range.first=std::min(range.first, spheres[i].r);
				range.second=std::max(range.second, spheres[i].r);
			}
		}
		return range;
	}

	template<typename SphereType>
	static std::vector<SimpleSphere> select_centers_for_clusters(const std::vector<SphereType>& spheres, const double expansion)
	{
		std::vector<SimpleSphere> centers;
		if(!spheres.empty())
		{
			std::vector<bool> allowed(spheres.size(), true);
			const std::vector<std::size_t> global_traversal=sort_objects_by_distance_to_one_of_them(spheres, 0, maximal_distance_from_point_to_sphere<SphereType, SphereType>);
			for(std::size_t k=0;k<spheres.size();k++)
			{
				const std::size_t i=global_traversal[k];
				if(allowed[i])
				{
					centers.push_back(spheres[i]);
					allowed[i]=false;
					for(std::size_t j=0;j<spheres.size();j++)
					{
						if(allowed[j] && sphere_intersects_sphere_with_expansion(spheres[i], spheres[j], expansion))
						{
							allowed[j]=false;
						}
					}
				}
			}
		}
		return centers;
	}

	template<typename SphereType>
	static std::vector<Cluster> cluster_spheres_using_centers(const std::vector<SphereType>& spheres, const std::vector<std::size_t>& selection, const std::vector<SimpleSphere>& centers)
	{
		std::vector<Cluster> clusters;
		clusters.reserve(centers.size());
		for(std::size_t i=0;i<centers.size();i++)
		{
			clusters.push_back(custom_sphere_from_object<Cluster>(centers[i]));
		}
		for(std::size_t i=0;i<selection.size();i++)
		{
			const SphereType& sphere=spheres[selection[i]];
			std::size_t min_dist_id=0;
			double min_dist_value=maximal_distance_from_point_to_sphere(clusters[min_dist_id], sphere);
			for(std::size_t j=1;j<clusters.size();j++)
			{
				const double dist_value=maximal_distance_from_point_to_sphere(clusters[j], sphere);
				if(dist_value<min_dist_value)
				{
					min_dist_id=j;
					min_dist_value=dist_value;
				}
			}
			Cluster& cluster=clusters[min_dist_id];
			cluster.r=std::max(cluster.r, min_dist_value);
			cluster.children.push_back(selection[i]);
		}
		std::vector<Cluster> nonempty_clusters;
		nonempty_clusters.reserve(clusters.size());
		for(std::size_t i=0;i<clusters.size();i++)
		{
			if(!clusters[i].children.empty())
			{
				nonempty_clusters.push_back(clusters[i]);
			}
		}
		return nonempty_clusters;
	}

	template<typename SphereType>
	static std::vector<Cluster> cluster_spheres_using_radius_expansion(const std::vector<SphereType>& spheres, const double radius_expansion)
	{
		const std::size_t max_part_size=10000;
		if(spheres.size()<=max_part_size)
		{
			std::vector<std::size_t> selection(spheres.size(), 0);
			for(std::size_t i=0;i<spheres.size();i++)
			{
				selection[i]=i;
			}
			return cluster_spheres_using_centers(spheres, selection, select_centers_for_clusters(spheres, radius_expansion));
		}
		else
		{
			std::vector<Cluster> result;
			const std::vector< std::vector<std::size_t> > selections=SplittingOfSpheres::split_for_size_of_part(spheres, max_part_size);
			for(std::size_t i=0;i<selections.size();i++)
			{
				const std::vector<std::size_t>& selection=selections[i];
				std::vector<SimpleSphere> selection_contents(selection.size());
				for(std::size_t j=0;j<selection.size();j++)
				{
					selection_contents[j]=SimpleSphere(spheres[selection[j]]);
				}
				const std::vector<Cluster> partial_result=cluster_spheres_using_centers(spheres, selection, select_centers_for_clusters(selection_contents, radius_expansion));
				result.insert(result.end(), partial_result.begin(), partial_result.end());
			}
			return result;
		}
	}

	template<typename SphereType>
	static std::vector< std::vector<Cluster> > cluster_spheres_in_layers(const std::vector<SphereType>& spheres, const double r, const std::size_t min_number_of_clusters)
	{
		std::vector< std::vector<Cluster> > clusters_layers;
		{
			std::vector<Cluster> clusters=cluster_spheres_using_radius_expansion(spheres, r);
			for(std::size_t i=0;i<clusters.size();i++)
			{
				clusters[i].leaves_ids=clusters[i].children;
			}
			clusters_layers.push_back(clusters);
		}
		bool need_more=clusters_layers.back().size()>min_number_of_clusters;
		while(need_more)
		{
			std::vector<Cluster> clusters=cluster_spheres_using_radius_expansion(clusters_layers.back(), 0.0);
			if(clusters.size()<clusters_layers.back().size() && clusters.size()>min_number_of_clusters)
			{
				for(std::size_t i=0;i<clusters.size();i++)
				{
					for(std::size_t j=0;j<clusters[i].children.size();j++)
					{
						clusters[i].leaves_ids.insert(clusters[i].leaves_ids.end(), clusters_layers.back()[clusters[i].children[j]].leaves_ids.begin(), clusters_layers.back()[clusters[i].children[j]].leaves_ids.end());
					}
					clusters[i].r=0.0;
					for(std::size_t j=0;j<clusters[i].leaves_ids.size();j++)
					{
						clusters[i].r=std::max(clusters[i].r, maximal_distance_from_point_to_sphere(clusters[i], spheres[clusters[i].leaves_ids[j]]));
					}
				}
				clusters_layers.push_back(clusters);
			}
			else
			{
				need_more=false;
			}
		}
		return clusters_layers;
	}

	std::vector<SimpleSphere> leaves_spheres_;
	std::pair<double, double> input_radii_range_;
	std::vector< std::vector<Cluster> > clusters_layers_;
};

}

}

#endif /* APOLLOTA_BOUNDING_SPHERES_HIERARCH_2_H_ */
