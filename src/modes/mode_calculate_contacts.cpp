#include <iostream>
#include <stdexcept>

#include "../apollota/constrained_contacts_construction.h"
#include "../apollota/constrained_contacts_utilities.h"
#include "../apollota/spheres_boundary_construction.h"
#include "../apollota/spherical_contacts_construction.h"

#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/opengl_printer.h"

#include "../common/chain_residue_atom_descriptor.h"
#include "../common/ball_value.h"
#include "../common/contact_value.h"

#include "modescommon/mock_solvent_utilities.h"

void calculate_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false,
			"list of balls\n(default mode line format: 'x y z r')\n(annotated mode line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true,
			"list of contacts\n(default mode line format: 'b1 b2 area')\n(annotated mode line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')");

	const bool annotated=poh.contains_option(pohw.describe_option("--annotated", "", "flag to enable annotated mode"));
	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
	const bool exclude_hidden_balls=poh.contains_option(pohw.describe_option("--exclude-hidden-balls", "", "flag to exclude hidden input balls"));
	const double step=poh.restrict_value_in_range(0.05, 0.5, poh.argument<double>(pohw.describe_option("--step", "number", "curve step length"), 0.2));
	const int projections=poh.restrict_value_in_range(1, 10, poh.argument<int>(pohw.describe_option("--projections", "number", "curve optimization depth"), 5));
	const int sih_depth=poh.restrict_value_in_range(1, 5, poh.argument<int>(pohw.describe_option("--sih-depth", "number", "spherical surface optimization depth"), 3));
	const bool add_mirrored=poh.contains_option(pohw.describe_option("--add-mirrored", "", "flag to add mirrored contacts to non-annnotated output"));
	const bool draw=poh.contains_option(pohw.describe_option("--draw", "", "flag to output graphics for annotated contacts"));
	const bool tag_centrality=poh.contains_option(pohw.describe_option("--tag-centrality", "", "flag to tag contacts centrality"));
	const bool tag_peripherial=poh.contains_option(pohw.describe_option("--tag-peripherial", "", "flag to tag peripherial contacts"));
	const std::string old_contacts_output=poh.argument<std::string>(pohw.describe_option("--old-contacts-output", "string", "file path to output spherical contacts"), "");
	const std::string volumes_output=poh.argument<std::string>(pohw.describe_option("--volumes-output", "string", "file path to output constrained cells volumes"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<apollota::SimpleSphere> spheres;
	std::vector< std::pair<common::ChainResidueAtomDescriptor, common::BallValue> > input_ball_records;
	std::set<std::size_t> mock_solvent_ids;
	if(annotated)
	{
		auxiliaries::IOUtilities().read_lines_to_map(std::cin, input_ball_records);
		spheres.reserve(input_ball_records.size());
		for(std::size_t i=0;i<input_ball_records.size();i++)
		{
			spheres.push_back(apollota::SimpleSphere(input_ball_records[i].second));
			if(modescommon::identify_mock_solvent(input_ball_records[i].first))
			{
				mock_solvent_ids.insert(i);
			}
		}
	}
	else
	{
		auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);
	}
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, exclude_hidden_balls, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);

	std::map<apollota::Pair, double> interactions_map;
	std::pair< bool, std::map<std::size_t, double> > volumes_map_bundle(!volumes_output.empty(), std::map<std::size_t, double>());
	
	{
		const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(spheres, vertices_vector, probe, step, projections, mock_solvent_ids, volumes_map_bundle);
		for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
		{
			if(it->first.get(0)<input_spheres_count && it->first.get(1)<input_spheres_count)
			{
				interactions_map[it->first]=it->second;
			}
		}
	}

	if(mock_solvent_ids.empty())
	{
		const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(spheres, vertices_vector, probe, sih_depth, volumes_map_bundle);
		for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
		{
			if(it->first<input_spheres_count)
			{
				interactions_map[apollota::Pair(it->first, it->first)]=it->second;
			}
		}
	}

	if(!input_ball_records.empty())
	{
		const apollota::TriangulationQueries::PairsMap pairs_vertices=((draw || tag_peripherial) ? apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::PairsMap());
		const apollota::TriangulationQueries::IDsMap ids_vertices=(draw ? apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::IDsMap());
		const apollota::SubdividedIcosahedron sih(draw ? sih_depth : 0);
		const apollota::TriangulationQueries::PairsMap pairs_neighbors=(tag_centrality ? apollota::TriangulationQueries::collect_pairs_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map) : apollota::TriangulationQueries::PairsMap());

		std::map< common::ChainResidueAtomDescriptorsPair, common::ContactValue > output_map_of_contacts;
		for(std::map<apollota::Pair, double>::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			const double area=it->second;
			if(area>0.0)
			{
				const std::size_t a_id=it->first.get(0);
				const std::size_t b_id=it->first.get(1);
				common::ChainResidueAtomDescriptor crad_a=input_ball_records[a_id].first;
				common::ChainResidueAtomDescriptor crad_b=input_ball_records[b_id].first;
				if(modescommon::identify_mock_solvent(crad_a))
				{
					crad_a=common::ChainResidueAtomDescriptor::solvent();
				}
				if(a_id==b_id || modescommon::identify_mock_solvent(crad_b))
				{
					crad_b=common::ChainResidueAtomDescriptor::solvent();
				}
				if(crad_a!=common::ChainResidueAtomDescriptor::solvent() || crad_b!=common::ChainResidueAtomDescriptor::solvent())
				{
					common::ContactValue value;
					value.area=area;
					if(a_id!=b_id)
					{
						value.dist=apollota::distance_from_point_to_point(spheres[a_id], spheres[b_id]);
					}
					else
					{
						value.dist=(spheres[a_id].r+(probe*3.0));
					}
					if(draw)
					{
						value.graphics=(a_id==b_id ?
								apollota::draw_solvent_contact<auxiliaries::OpenGLPrinter>(spheres, vertices_vector, ids_vertices, a_id, probe, sih) :
								apollota::draw_inter_atom_contact<auxiliaries::OpenGLPrinter>(spheres, vertices_vector, pairs_vertices, a_id, b_id, probe, step, projections, false));
					}
					if(tag_centrality && crad_a!=common::ChainResidueAtomDescriptor::solvent() && crad_b!=common::ChainResidueAtomDescriptor::solvent() && apollota::check_inter_atom_contact_centrality(spheres, pairs_neighbors, a_id, b_id))
					{
						value.props.tags.insert("central");
					}
					if(tag_peripherial && crad_a!=common::ChainResidueAtomDescriptor::solvent() && crad_b!=common::ChainResidueAtomDescriptor::solvent() && apollota::check_inter_atom_contact_peripherial(spheres, vertices_vector, pairs_vertices, a_id, b_id, probe))
					{
						value.props.tags.insert("peripherial");
					}
					output_map_of_contacts[common::ChainResidueAtomDescriptorsPair(crad_a, crad_b)].add(value);
				}
			}
		}
		auxiliaries::IOUtilities().write_map(output_map_of_contacts, std::cout);
	}
	else
	{
		if(add_mirrored)
		{
			std::map< std::pair<std::size_t, std::size_t>, double > mirrored_interactions_map;
			for(std::map<apollota::Pair, double>::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
			{
				mirrored_interactions_map[std::make_pair(it->first.get(0), it->first.get(1))]=it->second;
				if(it->first.get(0)!=it->first.get(1))
				{
					mirrored_interactions_map[std::make_pair(it->first.get(1), it->first.get(0))]=it->second;
				}
			}
			for(std::map< std::pair<std::size_t, std::size_t>, double >::const_iterator it=mirrored_interactions_map.begin();it!=mirrored_interactions_map.end();++it)
			{
				std::cout << it->first.first << " " << it->first.second << " " << it->second << "\n";
			}
		}
		else
		{
			for(std::map<apollota::Pair, double>::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
			{
				std::cout << it->first.get(0) << " " << it->first.get(1) << " " << it->second << "\n";
			}
		}
	}

	if(volumes_map_bundle.first && !volumes_map_bundle.second.empty() && !volumes_output.empty())
	{
		const std::map<std::size_t, double>& volumes_map=volumes_map_bundle.second;
		if(!input_ball_records.empty())
		{
			std::map<common::ChainResidueAtomDescriptor, double> output_volumes_map;
			for(std::map<std::size_t, double>::const_iterator it=volumes_map.begin();it!=volumes_map.end();++it)
			{
				const double volume=it->second;
				if(volume>0.0)
				{
					const std::size_t a_id=it->first;
					if(a_id<input_ball_records.size())
					{
						const common::ChainResidueAtomDescriptor& crad_a=input_ball_records[a_id].first;
						output_volumes_map[crad_a]=volume;
					}
				}
			}
			auxiliaries::IOUtilities().write_map_to_file(output_volumes_map, volumes_output);
		}
		else
		{
			std::map<std::size_t, double> output_volumes_map;
			for(std::map<std::size_t, double>::const_iterator it=volumes_map.begin();it!=volumes_map.end();++it)
			{
				const double volume=it->second;
				if(volume>0.0)
				{
					const std::size_t a_id=it->first;
					if(a_id<input_spheres_count)
					{
						output_volumes_map[a_id]=volume;
					}
				}
			}
			auxiliaries::IOUtilities().write_map_to_file(output_volumes_map, volumes_output);
		}
	}

	if(!old_contacts_output.empty() && mock_solvent_ids.empty())
	{
		std::ofstream foutput(old_contacts_output.c_str(), std::ios::out);
		if(foutput.good())
		{
			const std::vector< std::vector<std::size_t> > graph=apollota::TriangulationQueries::collect_ids_graph_from_ids_map(apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(vertices_vector), input_spheres_count);
			std::map<std::pair<std::size_t, std::size_t>, double> old_interactions_map;
			apollota::SphericalContactsConstruction sc_construction(sih_depth);
			for(std::size_t i=0;i<graph.size();i++)
			{
				const apollota::SphericalContactsConstruction::Result sc_result=sc_construction.construct_contacts(probe, spheres, i, graph[i]);
				for(std::map<std::size_t, double>::const_iterator it=sc_result.areas.begin();it!=sc_result.areas.end();++it)
				{
					old_interactions_map[std::pair<std::size_t, std::size_t>(i, it->first)]=it->second;
				}
			}
			if(!input_ball_records.empty())
			{
				std::map< std::pair<common::ChainResidueAtomDescriptor, common::ChainResidueAtomDescriptor>, common::ContactValue > output_map_of_contacts;
				for(std::map<std::pair<std::size_t, std::size_t>, double>::const_iterator it=old_interactions_map.begin();it!=old_interactions_map.end();++it)
				{
					const double area=it->second;
					if(area>0.0)
					{
						const std::size_t a_id=it->first.first;
						const std::size_t b_id=it->first.second;
						common::ChainResidueAtomDescriptor crad_a=input_ball_records[a_id].first;
						common::ChainResidueAtomDescriptor crad_b=input_ball_records[b_id].first;
						if(crad_a!=common::ChainResidueAtomDescriptor::solvent())
						{
							if(a_id==b_id)
							{
								crad_b=common::ChainResidueAtomDescriptor::solvent();
							}
							common::ContactValue value;
							value.area=area;
							if(a_id!=b_id)
							{
								value.dist=apollota::distance_from_point_to_point(spheres[a_id], spheres[b_id]);
							}
							else
							{
								value.dist=(spheres[a_id].r+(probe*3.0));
							}
							if(crad_b!=common::ChainResidueAtomDescriptor::solvent())
							{
								crad_b.altLoc="m";
							}
							output_map_of_contacts[std::make_pair(crad_a, crad_b)].add(value);
						}
					}
				}
				for(std::map< std::pair<common::ChainResidueAtomDescriptor, common::ChainResidueAtomDescriptor>, common::ContactValue >::const_iterator it=output_map_of_contacts.begin();it!=output_map_of_contacts.end();++it)
				{
					foutput << it->first.first << " " << it->first.second << " " << it->second << "\n";
				}
			}
			else
			{
				for(std::map<std::pair<std::size_t, std::size_t>, double>::const_iterator it=old_interactions_map.begin();it!=old_interactions_map.end();++it)
				{
					foutput << it->first.first << " " << it->first.second << " " << it->second << "\n";
				}
			}
		}
	}
}
