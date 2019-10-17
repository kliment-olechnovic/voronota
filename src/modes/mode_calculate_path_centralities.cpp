#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/construction_of_path_centralities.h"

void calculate_path_centralities(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "graph (line format: 'annotation1 annotation2 weight')");
	pohw.describe_io("stdout", false, true, "path centralities (line format: 'annotation1 annotation2 value')");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	typedef common::ConstructionOfPathCentralities COPC;

	const std::map<COPC::CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<COPC::CRADsPair, double> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	const COPC::Graph graph=COPC::init_graph(map_of_contacts);

	COPC::BetweennessCentralitiesResult result=COPC::calculate_betweenness_centralities(graph, 0.0, true);

	std::map<COPC::CRADsPair, double> map_of_centralities;
	for(COPC::ID id=0;id<graph.vertices.size();id++)
	{
		const COPC::CRADsPair crads(graph.vertices[id].crad, COPC::CRAD::any());
		map_of_centralities[crads]=result.vertex_centralities[id];
	}
	for(std::size_t i=0;i<graph.edges.size();i++)
	{
		const COPC::Edge& edge=graph.edges[i];
		const COPC::ID id1=edge.vertex_ids[0];
		const COPC::ID id2=edge.vertex_ids[1];
		const COPC::CRADsPair crads(graph.vertices[id1].crad, graph.vertices[id2].crad);
		map_of_centralities[crads]=result.edge_centralities[COPC::ordered_pair_of_ids(id1, id2)];
	}

	auxiliaries::IOUtilities().write_map(map_of_centralities, std::cout);
}

