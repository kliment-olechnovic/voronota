#ifndef COMMON_CONSTRUCTION_OF_PATH_CENTRALITIES_H_
#define COMMON_CONSTRUCTION_OF_PATH_CENTRALITIES_H_

#include <stdexcept>
#include <vector>
#include <map>
#include <algorithm>

#include "chain_residue_atom_descriptor.h"

namespace voronota
{

namespace common
{

class ConstructionOfPathCentralities
{
public:
	typedef ChainResidueAtomDescriptor CRAD;
	typedef ChainResidueAtomDescriptorsPair CRADsPair;
	typedef double Weight;
	typedef std::size_t ID;

	static ID null_id()
	{
		return std::numeric_limits<ID>::max();
	}

	static Weight inf_weight()
	{
		return std::numeric_limits<Weight>::max();
	}

	static Weight sum_weights(const Weight w1, const Weight w2)
	{
		if(w1==inf_weight() || w2==inf_weight())
		{
			return inf_weight();
		}
		else
		{
			return (w1+w2);
		}
	}

	static std::pair<ID, ID> ordered_pair_of_ids(const ID a, const ID b)
	{
		return (a<b ? std::pair<ID, ID>(a, b) : std::pair<ID, ID>(b, a));
	}

	struct Vertex
	{
		CRAD crad;
		std::vector<std::size_t> edge_ids;

		Vertex()
		{
		}

		explicit Vertex(const CRAD& crad) : crad(crad)
		{
		}
	};

	struct Edge
	{
		Weight weight;
		ID vertex_ids[2];

		Edge() : weight(0.0)
		{
			vertex_ids[0]=0;
			vertex_ids[1]=0;
		}

		Edge(const ID a, const ID b, const Weight weight) : weight(weight)
		{
			vertex_ids[0]=a;
			vertex_ids[1]=b;
		}

		std::size_t neighbor(const ID a) const
		{
			return (a==vertex_ids[0] ? vertex_ids[1] : (a==vertex_ids[1] ? vertex_ids[0] : null_id()));
		}
	};

	struct Graph
	{
		std::vector<Vertex> vertices;
		std::vector<Edge> edges;

		bool valid() const
		{
			if(vertices.empty())
			{
				return false;
			}

			for(std::size_t i=0;i<vertices.size();i++)
			{
				const Vertex& vertex=vertices[i];
				for(std::size_t j=0;j<vertex.edge_ids.size();j++)
				{
					if(vertex.edge_ids[j]>=edges.size())
					{
						return false;
					}
					else
					{
						const Edge& edge=edges[vertex.edge_ids[j]];
						if(edge.vertex_ids[0]!=i && edge.vertex_ids[1]!=i)
						{
							return false;
						}
					}
				}
			}

			for(std::size_t i=0;i<edges.size();i++)
			{
				const Edge& edge=edges[i];
				if(edge.weight<0.0)
				{
					return false;
				}
				if(edge.vertex_ids[0]==edge.vertex_ids[1])
				{
					return false;
				}
				if(edge.vertex_ids[0]>=vertices.size() || edge.vertex_ids[1]>=vertices.size())
				{
					return false;
				}
			}

			return true;
		}
	};

	static Graph init_graph(const std::map<CRADsPair, double>& map_of_contacts)
	{
		Graph graph;

		std::map<CRAD, std::size_t> map_of_crads;
		for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			map_of_crads[it->first.a]=0;
			map_of_crads[it->first.b]=0;
		}

		graph.vertices.reserve(map_of_crads.size());
		for(std::map<CRAD, std::size_t>::iterator it=map_of_crads.begin();it!=map_of_crads.end();++it)
		{
			it->second=graph.vertices.size();
			graph.vertices.push_back(Vertex(it->first));
		}

		graph.edges.reserve(map_of_contacts.size());
		for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const Edge edge(map_of_crads[it->first.a], map_of_crads[it->first.b], it->second);
			graph.vertices[edge.vertex_ids[0]].edge_ids.push_back(graph.edges.size());
			graph.vertices[edge.vertex_ids[1]].edge_ids.push_back(graph.edges.size());
			graph.edges.push_back(edge);
		}

		return graph;
	}

	class PriorityQueue
	{
	public:
		typedef std::pair<Weight, ID> WeightedID;

		PriorityQueue()
		{
		}

		bool empty() const
		{
			return set_of_weighted_ids_.empty();
		}

		void set(const ID id, const Weight weight)
		{
			std::map<ID, Weight>::iterator it=map_of_ids_to_weights_.find(id);
			if(it!=map_of_ids_to_weights_.end())
			{
				set_of_weighted_ids_.erase(WeightedID(id, it->second));
				it->second=weight;
			}
			else
			{
				map_of_ids_to_weights_[id]=weight;
			}
			set_of_weighted_ids_.insert(WeightedID(weight, id));
		}

		void set(const WeightedID& weighted_id)
		{
			set(weighted_id.first, weighted_id.second);
		}

		WeightedID get_min() const
		{
			if(set_of_weighted_ids_.empty())
			{
				return WeightedID(inf_weight(), null_id());
			}
			return (*set_of_weighted_ids_.begin());
		}

		void pop_min()
		{
			if(set_of_weighted_ids_.empty())
			{
				return;
			}
			map_of_ids_to_weights_.erase(set_of_weighted_ids_.begin()->second);
			set_of_weighted_ids_.erase(set_of_weighted_ids_.begin());
		}

		WeightedID extract_min()
		{
			WeightedID weighted_id=get_min();
			pop_min();
			return weighted_id;
		}

	private:
		std::set<WeightedID> set_of_weighted_ids_;
		std::map<ID, Weight> map_of_ids_to_weights_;
	};

	struct ShortestPathsSearchResult
	{
		std::vector<Weight> dist;
		std::vector< std::vector<ID> > prev;

		void reset(const std::size_t size)
		{
			dist.clear();
			prev.clear();
			dist.resize(size, inf_weight());
			prev.resize(size);
		}
	};

	static void find_shortest_paths(const Graph& graph, const std::size_t source_id, const Weight tolerance, ShortestPathsSearchResult& result)
	{
		const std::size_t N=graph.vertices.size();

		result.reset(N);

		if(!graph.valid())
		{
			throw std::runtime_error("Invalid graph.");
		}

		if(source_id>=N)
		{
			throw std::runtime_error("Invalid source.");
		}

		result.dist[source_id]=0.0;

		PriorityQueue queue;

		for(ID id=0;id<N;id++)
		{
			queue.set(id, result.dist[id]);
		}

		while(!queue.empty())
		{
			const PriorityQueue::WeightedID weighted_id=queue.extract_min();
			const ID u=weighted_id.second;
			for(std::size_t i=0;i<graph.vertices[u].edge_ids.size();i++)
			{
				const Edge& edge=graph.edges[graph.vertices[u].edge_ids[i]];
				const ID v=edge.neighbor(u);
				const Weight alt=sum_weights(result.dist[u], edge.weight);
				if(alt<inf_weight())
				{
					if(alt+tolerance<result.dist[v])
					{
						result.dist[v]=alt;
						result.prev[v].clear();
						result.prev[v].push_back(u);
						queue.set(v, alt);
					}
					else if(fabs(alt-result.dist[v])<=tolerance)
					{
						if(std::find(result.prev[v].begin(), result.prev[v].end(), u)==result.prev[v].end())
						{
							result.prev[v].push_back(u);
						}
					}
				}
			}
		}
	}

	struct BetweennessCentralitiesResult
	{
		int max_number_of_paths;
		std::pair<double, double> range_of_vertex_centralities;
		std::pair<double, double> range_of_edge_centralities;
		std::map<ID, double> vertex_centralities;
		std::map<std::pair<ID, ID>, double> edge_centralities;

		BetweennessCentralitiesResult() :
			max_number_of_paths(0),
			range_of_vertex_centralities(std::numeric_limits<double>::max(), (-std::numeric_limits<double>::max())),
			range_of_edge_centralities(std::numeric_limits<double>::max(), (-std::numeric_limits<double>::max()))
		{
		}
	};

	static BetweennessCentralitiesResult calculate_betweenness_centralities(const Graph& graph, const Weight tolerance, const bool normalize)
	{
		BetweennessCentralitiesResult result;

		if(!graph.valid())
		{
			throw std::runtime_error("Invalid graph.");
		}

		for(ID source_id=0;source_id<graph.vertices.size();source_id++)
		{
			ShortestPathsSearchResult sps_result;
			find_shortest_paths(graph, source_id, tolerance, sps_result);
			for(ID target_id=0;target_id<graph.vertices.size();target_id++)
			{
				if(target_id!=source_id)
				{
					std::map<ID, int> local_vertex_centralities;
					std::map<std::pair<ID, ID>, int> local_edge_centralities;
					std::vector<ID> path;
					std::vector<ID> stack;
					int number_of_paths=0;
					stack.push_back(target_id);
					while(!stack.empty())
					{
						const ID u=stack.back();
						stack.pop_back();
						while(!path.empty() && sps_result.dist[path.back()]<=sps_result.dist[u])
						{
							path.pop_back();
						}
						path.push_back(u);
						if(u==source_id)
						{
							number_of_paths++;
							for(std::size_t i=0;(i+1)<path.size();i++)
							{
								if(i>0)
								{
									local_vertex_centralities[path[i]]++;
								}
								local_edge_centralities[ordered_pair_of_ids(path[i], path[i+1])]++;
							}
						}
						else
						{
							if(!sps_result.prev[u].empty())
							{
								stack.insert(stack.end(), sps_result.prev[u].begin(), sps_result.prev[u].end());
							}
						}
					}
					if(number_of_paths>0)
					{
						for(std::map<ID, int>::const_iterator it=local_vertex_centralities.begin();it!=local_vertex_centralities.end();++it)
						{
							result.vertex_centralities[it->first]+=static_cast<double>(it->second)/static_cast<double>(number_of_paths);
						}
						for(std::map<std::pair<ID, ID>, int>::const_iterator it=local_edge_centralities.begin();it!=local_edge_centralities.end();++it)
						{
							result.edge_centralities[it->first]+=static_cast<double>(it->second)/static_cast<double>(number_of_paths);
						}
					}
					result.max_number_of_paths=std::max(result.max_number_of_paths, number_of_paths);
				}
			}
		}

		for(std::map<ID, double>::const_iterator it=result.vertex_centralities.begin();it!=result.vertex_centralities.end();++it)
		{
			result.range_of_vertex_centralities.first=std::min(result.range_of_vertex_centralities.first, it->second);
			result.range_of_vertex_centralities.second=std::max(result.range_of_vertex_centralities.second, it->second);
		}
		for(std::map<std::pair<ID, ID>, double>::const_iterator it=result.edge_centralities.begin();it!=result.edge_centralities.end();++it)
		{
			result.range_of_edge_centralities.first=std::min(result.range_of_edge_centralities.first, it->second);
			result.range_of_edge_centralities.second=std::max(result.range_of_edge_centralities.second, it->second);
		}

		if(normalize)
		{
			for(std::map<ID, double>::iterator it=result.vertex_centralities.begin();it!=result.vertex_centralities.end();++it)
			{
				it->second=(it->second)/(result.range_of_vertex_centralities.second);
			}
			for(std::map<std::pair<ID, ID>, double>::iterator it=result.edge_centralities.begin();it!=result.edge_centralities.end();++it)
			{
				it->second=(it->second)/(result.range_of_edge_centralities.second);
			}
		}

		return result;
	}
};

}

}

#endif /* COMMON_CONSTRUCTION_OF_PATH_CENTRALITIES_H_ */
