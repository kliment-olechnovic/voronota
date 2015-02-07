#include <iostream>
#include <stdexcept>

#include "apollota/constrained_contacts_construction.h"
#include "apollota/spheres_boundary_construction.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/opengl_printer.h"

#include "modescommon/ball_value.h"
#include "modescommon/contact_value.h"

namespace
{

std::string draw_inter_atom_contact(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::Triangulation::VerticesVector& vertices_vector,
		const apollota::TriangulationQueries::PairsMap& pairs_vertices,
		const std::size_t a_id,
		const std::size_t b_id,
		const double probe,
		const double step,
		const int projections)
{
	auxiliaries::OpenGLPrinter opengl_printer;
	if(a_id<spheres.size() && b_id<spheres.size())
	{
		apollota::TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.find(apollota::Pair(a_id, b_id));
		if(pairs_vertices_it!=pairs_vertices.end())
		{
			const std::list<apollota::ConstrainedContactContour::Contour> contours=apollota::ConstrainedContactContour::construct_contact_contours(
					spheres, vertices_vector, pairs_vertices_it->second, a_id, b_id, probe, step, projections);
			for(std::list<apollota::ConstrainedContactContour::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
			{
				const apollota::ConstrainedContactContour::Contour& contour=(*contours_it);
				const std::vector<apollota::SimplePoint> outline=apollota::ConstrainedContactContour::collect_points_from_contour(contour);
				opengl_printer.add_triangle_fan(
						apollota::HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(apollota::mass_center<apollota::SimplePoint>(outline.begin(), outline.end()), spheres[a_id], spheres[b_id]),
						outline,
						apollota::sub_of_points<apollota::SimplePoint>(spheres[b_id], spheres[a_id]).unit());
			}
		}
	}
	return opengl_printer.str();
}

std::string draw_solvent_contact(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::Triangulation::VerticesVector& vertices_vector,
		const apollota::TriangulationQueries::IDsMap& ids_vertices,
		const std::size_t a_id,
		const double probe,
		const apollota::SubdividedIcosahedron& sih)
{
	auxiliaries::OpenGLPrinter opengl_printer;
	if(a_id<spheres.size())
	{
		apollota::TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.find(a_id);
		if(ids_vertices_it!=ids_vertices.end())
		{
			const apollota::ConstrainedContactRemainder::Remainder remainder=apollota::ConstrainedContactRemainder::construct_contact_remainder(
					spheres, vertices_vector, ids_vertices_it->second, a_id, probe, sih);
			for(apollota::ConstrainedContactRemainder::Remainder::const_iterator remainder_it=remainder.begin();remainder_it!=remainder.end();++remainder_it)
			{
				std::vector<apollota::SimplePoint> ts(3);
				std::vector<apollota::SimplePoint> ns(3);
				for(int i=0;i<3;i++)
				{
					ts[i]=remainder_it->p[i];
					ns[i]=apollota::sub_of_points<apollota::SimplePoint>(ts[i], spheres[a_id]).unit();
				}
				opengl_printer.add_triangle_strip(ts, ns);
			}
		}
	}
	return opengl_printer.str();
}

}

void calculate_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--annotated", "", "flag to enable annotated mode"));
		ods.push_back(OD("--probe", "number", "probe radius"));
		ods.push_back(OD("--exclude-hidden-balls", "", "flag to exclude hidden input balls"));
		ods.push_back(OD("--step", "number", "curve step length"));
		ods.push_back(OD("--projections", "number", "curve optimization depth"));
		ods.push_back(OD("--sih-depth", "number", "spherical surface optimization depth"));
		ods.push_back(OD("--add-mirrored", "", "flag to add mirrored contacts to non-annnotated output"));
		ods.push_back(OD("--draw", "", "flag to output graphics for annotated contacts"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false,
					"list of balls\n(default mode line format: 'x y z r')\n(annotated mode line format: 'annotation x y z r tags adjuncts')");
			poh.print_io_description("stdout", false, true,
					"list of contacts\n(default mode line format: 'b1 b2 area')\n(annotated mode line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')");
			return;
		}
	}

	const bool annotated=poh.contains_option("--annotated");
	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");
	const double probe=std::max(0.01, std::min(14.0, poh.argument<double>("--probe", 1.4)));
	const double step=std::max(0.05, std::min(0.5, poh.argument<double>("--step", 0.2)));
	const int projections=std::max(1, std::min(10, poh.argument<int>("--projections", 5)));
	const int sih_depth=std::max(1, std::min(5, poh.argument<int>("--sih-depth", 3)));
	const bool add_mirrored=poh.contains_option("--add-mirrored");
	const bool draw=poh.contains_option("--draw");

	std::vector<apollota::SimpleSphere> spheres;
	std::vector< std::pair<auxiliaries::ChainResidueAtomDescriptor, BallValue> > input_ball_records;
	if(annotated)
	{
		auxiliaries::IOUtilities().read_lines_to_map(std::cin, input_ball_records);
		spheres.reserve(input_ball_records.size());
		for(std::size_t i=0;i<input_ball_records.size();i++)
		{
			spheres.push_back(apollota::SimpleSphere(input_ball_records[i].second));
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
	const apollota::TriangulationQueries::IDsMap ids_map=apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map);

	std::map<apollota::Pair, double> interactions_map;
	
	const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(spheres, vertices_vector, probe, step, projections);
	for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
	{
		if(it->first.get(0)<input_spheres_count && it->first.get(1)<input_spheres_count)
		{
			interactions_map[it->first]=it->second;
		}
	}

	const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(spheres, vertices_vector, probe, sih_depth);
	for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
	{
		if(it->first<input_spheres_count)
		{
			interactions_map[apollota::Pair(it->first, it->first)]=it->second;
		}
	}

	if(!input_ball_records.empty())
	{
		const apollota::TriangulationQueries::PairsMap pairs_vertices=(draw ? apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::PairsMap());
		const apollota::TriangulationQueries::IDsMap ids_vertices=(draw ? apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::IDsMap());
		const apollota::SubdividedIcosahedron sih(draw ? sih_depth : 0);

		std::map< auxiliaries::ChainResidueAtomDescriptorsPair, ContactValue > output_map_of_contacts;
		for(std::map<apollota::Pair, double>::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			const double area=it->second;
			if(area>0.0)
			{
				const std::size_t a_id=it->first.get(0);
				const std::size_t b_id=it->first.get(1);
				const auxiliaries::ChainResidueAtomDescriptorsPair crads(input_ball_records[a_id].first, (a_id==b_id ? auxiliaries::ChainResidueAtomDescriptor::solvent() : input_ball_records[b_id].first));
				ContactValue& value=output_map_of_contacts[crads];
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
							draw_solvent_contact(spheres, vertices_vector, ids_vertices, a_id, probe, sih) :
							draw_inter_atom_contact(spheres, vertices_vector, pairs_vertices, a_id, b_id, probe, step, projections));
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
}
