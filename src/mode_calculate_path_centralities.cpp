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

std::size_t null_id()
{
	return std::numeric_limits<std::size_t>::max();
}

//double inf_weight()
//{
//	return std::numeric_limits<double>::max();
//}

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
	double weight;
	std::size_t vertex_ids[2];

	Edge() : weight(0.0)
	{
		vertex_ids[0]=0;
		vertex_ids[1]=0;
	}

	Edge(const std::size_t a, const std::size_t b, const double weight) : weight(weight)
	{
		vertex_ids[0]=a;
		vertex_ids[1]=b;
	}

	std::size_t neighbor(const std::size_t a) const
	{
		return (a==vertex_ids[0] ? vertex_ids[1] : (a==vertex_ids[1] ? vertex_ids[0] : null_id()));
	}
};

struct Graph
{
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;
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

	std::cout << graph.vertices.size() << " " << graph.edges.size() << "\n";
}

