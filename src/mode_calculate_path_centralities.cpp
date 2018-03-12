#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "common/chain_residue_atom_descriptor.h"

namespace
{

typedef common::ChainResidueAtomDescriptor CRAD;
typedef common::ChainResidueAtomDescriptorsPair CRADsPair;
typedef double Weight;
typedef std::size_t ID;

inline ID null_id()
{
	return std::numeric_limits<ID>::max();
}

inline Weight inf_weight()
{
	return std::numeric_limits<Weight>::max();
}

inline Weight sum_weights(const Weight w1, const Weight w2)
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

Graph init_graph(const std::map<CRADsPair, double>& map_of_contacts)
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
	std::vector<ID> prev;

	void reset(const std::size_t size)
	{
		dist.clear();
		prev.clear();
		dist.resize(size, inf_weight());
		prev.resize(size, null_id());
	}
};

void find_shortest_paths(const Graph& graph, const std::size_t source_id, ShortestPathsSearchResult& result)
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
			if(alt<result.dist[v])
			{
				result.dist[v]=alt;
				result.prev[v]=u;
				queue.set(v, alt);
			}
		}
	}
}

}

void calculate_path_centralities(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "graph (line format: 'annotation1 annotation2 weight')");
	pohw.describe_io("stdout", false, true, "path centralities (line format: 'annotation1 annotation2 value')");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::map<CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRADsPair, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	const Graph graph=init_graph(map_of_contacts);

	std::cout << graph.vertices.size() << " " << graph.edges.size() << " " << graph.valid() << "\n";

	ShortestPathsSearchResult spsr;
	find_shortest_paths(graph, 0, spsr);
	for(std::size_t i=0;i<spsr.dist.size();i++)
	{
		std::cout << spsr.dist[i] << ((i+1)%10==0 ? "\n" : " ");
	}
}

