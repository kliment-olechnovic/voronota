#include "apollota/constrained_contacts_construction.h"
#include "apollota/spheres_boundary_construction.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

//namespace
//{
//
//std::vector<apollota::SimplePoint> subdivide_triangles(const std::vector<apollota::SimplePoint>& points, const int times)
//{
//	if(points.size()%3!=0)
//	{
//		throw std::runtime_error("Invalid triangles points.");
//	}
//	std::vector<apollota::SimplePoint> result;
//	result.reserve(points.size()*4);
//	for(std::size_t i=0;i<points.size();i+=3)
//	{
//		const apollota::SimplePoint midpoints[3]={
//				(points[i]+points[i+1])*0.5,
//				(points[i]+points[i+2])*0.5,
//				(points[i+1]+points[i+2])*0.5};
//
//		result.push_back(points[i]);
//		result.push_back(midpoints[0]);
//		result.push_back(midpoints[1]);
//
//		result.push_back(points[i+1]);
//		result.push_back(midpoints[0]);
//		result.push_back(midpoints[2]);
//
//		result.push_back(points[i+2]);
//		result.push_back(midpoints[1]);
//		result.push_back(midpoints[2]);
//
//		result.push_back(midpoints[0]);
//		result.push_back(midpoints[1]);
//		result.push_back(midpoints[2]);
//	}
//	if(times>1)
//	{
//		return subdivide_triangles(result, times-1);
//	}
//	else
//	{
//		return result;
//	}
//}
//
//}
//
//void generate_demo_big(const auxiliaries::ProgramOptionsHandler& poh)
//{
//	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
//	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
//	pohw.describe_io("stdout", false, true, "drawing script fo PyMol");
//
//	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
//	const std::string representation=poh.argument<std::string>(pohw.describe_option("--representation", "string", "representation name"), "");
//
//	if(!pohw.assert_or_print_help(false))
//	{
//		return;
//	}
//
//	const double step=0.2;
//	const int projections=5;
//
//	std::vector<apollota::SimpleSphere> spheres;
//	auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);
//	if(spheres.size()<4)
//	{
//		throw std::runtime_error("Less than 4 balls provided to stdin.");
//	}
//
////	const std::size_t input_spheres_count=spheres.size();
//	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
//	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());
//
//	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
//	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);
//
//	std::vector< std::pair<apollota::Pair, apollota::ConstrainedContactContour::Contour> > surface_contours_vector;
//	std::vector<int> marks;
//	apollota::ConstrainedContactsConstruction::construct_surface_contours(spheres, vertices_vector, probe, step, projections, surface_contours_vector, marks);
//
//	std::vector< std::vector< std::pair<apollota::SimplePoint, apollota::SimplePoint> > > surface_toric_controls(surface_contours_vector.size());
//	for(std::size_t i=0;i<surface_contours_vector.size();i++)
//	{
//		const apollota::Pair& pair=surface_contours_vector[i].first;
//		const apollota::SimpleSphere& a=spheres[pair.get(0)];
//		const apollota::SimpleSphere& b=spheres[pair.get(1)];
//		const apollota::SimpleSphere c=apollota::intersection_circle_of_two_spheres<apollota::SimpleSphere>(apollota::SimpleSphere(a, a.r+probe), apollota::SimpleSphere(b, b.r+probe));
//		std::vector< std::pair<apollota::SimplePoint, apollota::SimplePoint> >& controls=surface_toric_controls[i];
//		if(c.r>probe)
//		{
//			controls.push_back(std::make_pair(apollota::SimplePoint(a), apollota::SimplePoint(b)));
//		}
//		else
//		{
//			const double dl=sqrt((probe*probe)-(c.r*c.r));
//			const apollota::SimplePoint ap(a);
//			const apollota::SimplePoint bp(b);
//			const apollota::SimplePoint cp(c);
//			controls.push_back(std::make_pair(ap, cp+((ap-cp).unit()*dl)));
//			controls.push_back(std::make_pair(bp, cp+((bp-cp).unit()*dl)));
//		}
//	}
//
//	std::vector< std::pair<apollota::Triple, apollota::SimplePoint> > surface_triples_vector;
//	surface_triples_vector.reserve(surface_contours_vector.size());
//	for(std::size_t i=0;i<surface_contours_vector.size();i++)
//	{
//		const apollota::Pair& pair=surface_contours_vector[i].first;
//		const apollota::ConstrainedContactContour::Contour& contour=surface_contours_vector[i].second;
//		if(contour.front().left_id<pair.get_min_max().first)
//		{
//			surface_triples_vector.push_back(std::make_pair(apollota::Triple(pair, contour.front().left_id), contour.front().p));
//		}
//		if(contour.back().right_id<pair.get_min_max().first)
//		{
//			surface_triples_vector.push_back(std::make_pair(apollota::Triple(pair, contour.back().right_id), contour.back().p));
//		}
//	}
//
//	typedef std::tr1::unordered_map<apollota::Pair, std::vector<std::size_t>, apollota::Pair::HashFunctor> PairsMap;
//	PairsMap pairs_map;
//	for(std::size_t i=0;i<surface_contours_vector.size();i++)
//	{
//		pairs_map[surface_contours_vector[i].first].push_back(i);
//	}
//
//	typedef std::tr1::unordered_map<apollota::Triple, std::vector<std::size_t>, apollota::Triple::HashFunctor> TriplesMap;
//	TriplesMap triples_map;
//	for(std::size_t i=0;i<surface_triples_vector.size();i++)
//	{
//		triples_map[surface_triples_vector[i].first].push_back(i);
//	}
//
//	std::vector< std::set<std::size_t> > surface_triples_conflicts(surface_triples_vector.size());
//	{
//		for(TriplesMap::const_iterator triples_map_it=triples_map.begin();triples_map_it!=triples_map.end();++triples_map_it)
//		{
//			const std::vector<std::size_t>& ids=triples_map_it->second;
//			if(ids.size()==2 && apollota::sphere_intersects_sphere(apollota::SimpleSphere(surface_triples_vector[ids[0]].second, probe), apollota::SimpleSphere(surface_triples_vector[ids[1]].second, probe)))
//			{
//				surface_triples_conflicts[ids[0]].insert(ids[1]);
//				surface_triples_conflicts[ids[1]].insert(ids[0]);
//			}
//		}
//
//		{
//			PairsMap difficult_pairs_to_triples_map;
//			for(std::size_t i=0;i<surface_triples_vector.size();i++)
//			{
//				const apollota::Triple& triple=surface_triples_vector[i].first;
//				for(unsigned int j=0;j<3;j++)
//				{
//					const apollota::Pair pair=triple.exclude(j);
//					const std::vector<std::size_t>& pair_ids=pairs_map[pair];
//					for(std::vector<std::size_t>::const_iterator pair_ids_it=pair_ids.begin();pair_ids_it!=pair_ids.end();++pair_ids_it)
//					{
//						if(surface_toric_controls[*pair_ids_it].size()>1)
//						{
//							difficult_pairs_to_triples_map[pair].push_back(i);
//						}
//					}
//				}
//			}
//			for(PairsMap::const_iterator it=difficult_pairs_to_triples_map.begin();it!=difficult_pairs_to_triples_map.end();++it)
//			{
//				const std::vector<std::size_t>& triples_ids=it->second;
//				for(std::size_t i=0;i<triples_ids.size();i++)
//				{
//					for(std::size_t j=0;j<triples_ids.size();j++)
//					{
//						if(i!=j)
//						{
//							surface_triples_conflicts[triples_ids[i]].insert(triples_ids[j]);
//						}
//					}
//				}
//			}
//		}
//
//		for(std::size_t i=0;i<surface_triples_conflicts.size();i++)
//		{
//			surface_triples_conflicts[i].erase(i);
//		}
//	}
//
//	{
//		for(std::size_t i=0;i<surface_contours_vector.size();i++)
//		{
//			const apollota::Pair& pair=surface_contours_vector[i].first;
//			apollota::ConstrainedContactContour::Contour& contour=surface_contours_vector[i].second;
//			for(int mode=0;mode<2;mode++)
//			{
//				const std::size_t mode_id=(mode==0 ? contour.front().left_id : contour.back().right_id);
//				apollota::SimplePoint& mode_p=(mode==0 ? contour.front().p : contour.back().p);
//				if(!pair.contains(mode_id))
//				{
//					const std::vector<std::size_t>& ids=triples_map[apollota::Triple(pair, mode_id)];
//					if(ids.size()==1)
//					{
//						mode_p=surface_triples_vector[ids[0]].second;
//					}
//					else if(ids.size()>1)
//					{
//						std::size_t closest_id=ids[0];
//						double closest_distance=apollota::distance_from_point_to_point(mode_p, surface_triples_vector[ids[0]].second);
//						for(std::size_t j=1;j<ids.size();j++)
//						{
//							const double distance=apollota::distance_from_point_to_point(mode_p, surface_triples_vector[ids[j]].second);
//							if(distance<closest_distance)
//							{
//								closest_id=ids[j];
//								closest_distance=distance;
//							}
//						}
//						mode_p=surface_triples_vector[closest_id].second;
//					}
//				}
//			}
//		}
//	}
//
//	if(representation=="")
//	{
//		auxiliaries::OpenGLPrinter opengl_printer;
//		opengl_printer.add_color(0xFFFF00);
//		for(std::size_t i=0;i<surface_contours_vector.size();i++)
//		{
//			const apollota::ConstrainedContactContour::Contour& contour=surface_contours_vector[i].second;
//			std::vector<apollota::SimplePoint> points;
//			points.reserve(contour.size());
//			for(apollota::ConstrainedContactContour::Contour::const_iterator contour_it=contour.begin();contour_it!=contour.end();++contour_it)
//			{
//				points.push_back(contour_it->p);
//			}
//			opengl_printer.add_color(((marks[i]+1)*123456789)%(0xFFFFFF));
//			opengl_printer.add_line_strip(points);
//		}
//		opengl_printer.add_color(0x00FFFF);
//		for(std::size_t i=0;i<surface_triples_vector.size();i++)
//		{
//			opengl_printer.add_sphere(apollota::SimpleSphere(surface_triples_vector[i].second, probe*0.05));
//		}
//		opengl_printer.print_pymol_script("surface_curves", true, std::cout);
//	}
//	else if(representation=="presurface")
//	{
//		auxiliaries::OpenGLPrinter opengl_printer;
//		opengl_printer.add_color(0xFFFF00);
//		for(std::size_t i=0;i<surface_contours_vector.size();i++)
//		{
//			const apollota::Pair& pair=surface_contours_vector[i].first;
//			const apollota::SimplePoint a(spheres[pair.get(0)]);
//			const apollota::SimplePoint b(spheres[pair.get(1)]);
//			const apollota::ConstrainedContactContour::Contour& contour=surface_contours_vector[i].second;
//			std::vector<apollota::SimplePoint> vertices;
//			std::vector<apollota::SimplePoint> normals;
//			for(apollota::ConstrainedContactContour::Contour::const_iterator contour_it=contour.begin();contour_it!=contour.end();++contour_it)
//			{
//				const apollota::SimplePoint& c=contour_it->p;
//				vertices.push_back(c+((a-c).unit()*probe));
//				vertices.push_back(c+((b-c).unit()*probe));
//				normals.push_back(((a-c).unit()*0.5)+((b-c).unit()*0.5));
//				normals.push_back(normals.back());
//			}
//			opengl_printer.add_triangle_strip(vertices, normals);
//		}
//		opengl_printer.add_color(0x00FFFF);
//		for(std::size_t i=0;i<surface_triples_vector.size();i++)
//		{
//			const apollota::Triple& triple=surface_triples_vector[i].first;
//			const apollota::SimplePoint& d=surface_triples_vector[i].second;
//			std::vector<apollota::SimplePoint> vertices;
//			for(int j=0;j<3;j++)
//			{
//				const apollota::SimplePoint& a(spheres[triple.get(j)]);
//				vertices.push_back(d+((a-d).unit()*probe));
//			}
//			std::vector<apollota::SimplePoint> normals(3, ((vertices[1]-vertices[0])&(vertices[2]-vertices[0])).unit());
//			opengl_printer.add_triangle_strip(vertices, normals);
//		}
//		opengl_printer.print_pymol_script("presurface", true, std::cout);
//	}
//	else if(representation=="surface")
//	{
//		auxiliaries::OpenGLPrinter opengl_printer;
//
//		opengl_printer.add_color(0xFFFF00);
//		for(std::size_t i=0;i<surface_contours_vector.size();i++)
//		{
//			const std::vector< std::pair<apollota::SimplePoint, apollota::SimplePoint> >& controls=surface_toric_controls[i];
//			for(std::vector< std::pair<apollota::SimplePoint, apollota::SimplePoint> >::const_iterator controls_it=controls.begin();controls_it!=controls.end();++controls_it)
//			{
//				const apollota::SimplePoint& a=controls_it->first;
//				const apollota::SimplePoint& b=controls_it->second;
//				const apollota::ConstrainedContactContour::Contour& contour=surface_contours_vector[i].second;
//				std::vector<apollota::SimplePoint> as;
//				std::vector<apollota::SimplePoint> bs;
//				std::vector<apollota::SimplePoint> cs;
//				for(apollota::ConstrainedContactContour::Contour::const_iterator contour_it=contour.begin();contour_it!=contour.end();++contour_it)
//				{
//					const apollota::SimplePoint& c=contour_it->p;
//					as.push_back(c+((a-c).unit()*probe));
//					bs.push_back(c+((b-c).unit()*probe));
//					cs.push_back(c);
//				}
//				std::vector<apollota::SimplePoint> normals;
//				for(std::size_t j=0;(j+1)<as.size();j++)
//				{
//					std::vector<apollota::SimplePoint> vertices;
//					std::vector<apollota::SimplePoint> normals;
//					const int steps=8;
//					for(int li=0;li<=steps;li++)
//					{
//						for(std::size_t e=0;e<2;e++)
//						{
//							const double l=static_cast<double>(li)/static_cast<double>(steps);
//							const apollota::SimplePoint p=((as[j+e]*(1.0-l))+(bs[j+e]*l));
//							const apollota::SimplePoint n=(cs[j+e]-p).unit();
//							vertices.push_back(cs[j+e]-(n*probe));
//							normals.push_back(n);
//						}
//					}
//					opengl_printer.add_triangle_strip(vertices, normals);
//				}
//			}
//		}
//
//		opengl_printer.add_color(0x00FFFF);
//		for(std::size_t i=0;i<surface_triples_vector.size();i++)
//		{
//			const apollota::Triple& triple=surface_triples_vector[i].first;
//			const apollota::SimplePoint& d=surface_triples_vector[i].second;
//			std::vector<apollota::SimplePoint> vertices;
//			for(int j=0;j<3;j++)
//			{
//				const apollota::SimplePoint& a(spheres[triple.get(j)]);
//				vertices.push_back(d+((a-d).unit()*probe));
//			}
//			vertices=subdivide_triangles(vertices, (surface_triples_conflicts[i].empty() ? 3 : 4));
//			std::vector<apollota::SimplePoint> normals(vertices.size());
//			for(std::size_t j=0;j<vertices.size();j++)
//			{
//				apollota::SimplePoint& a=vertices[j];
//				a=(d+((a-d).unit()*probe));
//				normals[j]=(d-a).unit();
//			}
//			if(surface_triples_conflicts[i].empty())
//			{
//				for(std::size_t j=0;j<vertices.size();j+=3)
//				{
//					opengl_printer.add_triangle_strip(std::vector<apollota::SimplePoint>(vertices.begin()+j, vertices.begin()+j+3), std::vector<apollota::SimplePoint>(normals.begin()+j, normals.begin()+j+3));
//				}
//			}
//			else
//			{
//				apollota::ConstrainedContactRemainder::Remainder remainder;
//				for(std::size_t j=0;j<vertices.size();j+=3)
//				{
//					remainder.push_back(apollota::ConstrainedContactRemainder::TriangleRecord(vertices[j], vertices[j+1], vertices[j+2], 0, 0, 0));
//				}
//				const std::set<std::size_t>& conflicts=surface_triples_conflicts[i];
//				for(std::set<std::size_t>::const_iterator conflicts_it=conflicts.begin();conflicts_it!=conflicts.end();++conflicts_it)
//				{
//					apollota::ConstrainedContactRemainder::cut_contact_remainder(apollota::SimpleSphere(surface_triples_vector[*conflicts_it].second, probe), std::make_pair(1000, apollota::SimpleSphere(d, probe)), remainder);
//				}
//				for(apollota::ConstrainedContactRemainder::Remainder::const_iterator remainder_it=remainder.begin();remainder_it!=remainder.end();++remainder_it)
//				{
//					std::vector<apollota::SimplePoint> cut_vertices(3);
//					std::vector<apollota::SimplePoint> cut_normals(3);
//					for(int j=0;j<3;j++)
//					{
//						cut_vertices[j]=remainder_it->p[j];
//						cut_normals[j]=(d-cut_vertices[j]).unit();
//					}
//					opengl_printer.add_triangle_strip(cut_vertices, cut_normals);
//				}
//			}
//		}
//
//		opengl_printer.print_pymol_script("surface", true, std::cout);
//	}
//}

void generate_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
	pohw.describe_io("stdout", false, true, "info");

	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
	const std::string representation=poh.argument<std::string>(pohw.describe_option("--representation", "string", "representation name"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

//	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);

	const apollota::TriangulationQueries::TriplesMap triples_map=apollota::TriangulationQueries::collect_triples_vertices_map_from_vertices_vector(vertices_vector);
	int failed_min_tangents=0;
	int failed_probe_tangents=0;

	for(apollota::TriangulationQueries::TriplesMap::const_iterator triples_map_it=triples_map.begin();triples_map_it!=triples_map.end();++triples_map_it)
	{
		const apollota::Triple& triple=triples_map_it->first;
		const apollota::SimpleSphere s[3]={spheres[triple.get(0)], spheres[triple.get(1)], spheres[triple.get(2)]};
		const std::vector<apollota::SimpleSphere> min_tangents=apollota::TangentSphereOfThreeSpheres::calculate(s[0], s[1], s[2]);
		if(min_tangents.empty())
		{
			failed_min_tangents++;
		}
		else
		{
			double min_tangent_radius=min_tangents[0].r;
			if(min_tangents.size()>1 && min_tangent_radius>min_tangents[1].r)
			{
				min_tangent_radius=min_tangents[1].r;
			}
			double max_tangent_radius=min_tangents[0].r;
			if(min_tangents.size()>1 && max_tangent_radius<min_tangents[1].r)
			{
				max_tangent_radius=min_tangents[1].r;
			}
			if(min_tangent_radius<probe && (min_tangent_radius==max_tangent_radius || max_tangent_radius>probe))
			{
				const std::vector<apollota::SimpleSphere> probe_tangents=apollota::TangentSphereOfThreeSpheres::calculate(s[0], s[1], s[2], probe);
				if(probe_tangents.size()!=2)
				{
					failed_probe_tangents++;
				}
			}
		}
	}

	std::cerr << triples_map.size() << " triples\n";
	std::cerr << failed_min_tangents << " failed_min_tangents\n";
	std::cerr << failed_probe_tangents << " failed_probe_tangents\n";
}
