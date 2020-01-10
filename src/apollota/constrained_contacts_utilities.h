#ifndef APOLLOTA_CONSTRAINED_CONTACT_UTILITIES_H_
#define APOLLOTA_CONSTRAINED_CONTACT_UTILITIES_H_

#include "constrained_contacts_construction.h"

namespace voronota
{

namespace apollota
{

template<typename OpenGLPrinter>
std::string draw_inter_atom_contact(
		const std::vector<SimpleSphere>& spheres,
		const Triangulation::VerticesVector& vertices_vector,
		const TriangulationQueries::PairsMap& pairs_vertices,
		const std::size_t a_id,
		const std::size_t b_id,
		const double probe,
		const double step,
		const int projections,
		const bool simplify)
{
	OpenGLPrinter opengl_printer;
	if(a_id<spheres.size() && b_id<spheres.size())
	{
		TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.find(Pair(a_id, b_id));
		if(pairs_vertices_it!=pairs_vertices.end())
		{
			const std::list<ConstrainedContactContour::Contour> contours=ConstrainedContactContour::construct_contact_contours(
					spheres, vertices_vector, pairs_vertices_it->second, a_id, b_id, probe, step, projections, simplify);
			for(std::list<ConstrainedContactContour::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
			{
				const ConstrainedContactContour::ContourAreaDescriptor d=ConstrainedContactContour::construct_contour_area_descriptor(*contours_it, spheres[a_id], spheres[b_id], false);
				const SimplePoint normal=sub_of_points<SimplePoint>(spheres[b_id], spheres[a_id]).unit();
				if(d.star_domain || d.simple_polygon_triangulation.empty())
				{
					opengl_printer.add_triangle_fan(d.center, d.outline, normal);
				}
				else
				{
					std::vector<SimplePoint> strip_vertices(3);
					std::vector<SimplePoint> strip_normals(3, normal);
					for(std::size_t i=0;i<d.simple_polygon_triangulation.size();i++)
					{
						const Triple& t=d.simple_polygon_triangulation[i];
						strip_vertices[0]=d.outline[t.get(0)];
						strip_vertices[1]=d.outline[t.get(1)];
						strip_vertices[2]=d.outline[t.get(2)];
						opengl_printer.add_triangle_strip(strip_vertices, strip_normals);
					}
				}
			}
		}
	}
	return opengl_printer.str();
}

template<typename OpenGLPrinter>
std::string draw_solvent_contact(
		const std::vector<SimpleSphere>& spheres,
		const Triangulation::VerticesVector& vertices_vector,
		const TriangulationQueries::IDsMap& ids_vertices,
		const std::size_t a_id,
		const double probe,
		const SubdividedIcosahedron& sih)
{
	OpenGLPrinter opengl_printer;
	if(a_id<spheres.size())
	{
		TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.find(a_id);
		if(ids_vertices_it!=ids_vertices.end())
		{
			const ConstrainedContactRemainder::Remainder remainder=ConstrainedContactRemainder::construct_contact_remainder(
					spheres, vertices_vector, ids_vertices_it->second, a_id, probe, sih);
			for(ConstrainedContactRemainder::Remainder::const_iterator remainder_it=remainder.begin();remainder_it!=remainder.end();++remainder_it)
			{
				std::vector<SimplePoint> ts(3);
				std::vector<SimplePoint> ns(3);
				for(int i=0;i<3;i++)
				{
					ts[i]=remainder_it->p[i];
					ns[i]=sub_of_points<SimplePoint>(ts[i], spheres[a_id]).unit();
				}
				opengl_printer.add_triangle_strip(ts, ns);
			}
		}
	}
	return opengl_printer.str();
}

inline SimplePoint calculate_direction_of_solvent_contact(
		const std::vector<SimpleSphere>& spheres,
		const Triangulation::VerticesVector& vertices_vector,
		const TriangulationQueries::IDsMap& ids_vertices,
		const std::size_t a_id,
		const double probe,
		const SubdividedIcosahedron& sih)
{
	if(a_id<spheres.size())
	{
		TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.find(a_id);
		if(ids_vertices_it!=ids_vertices.end())
		{
			const SimplePoint ball_center(spheres[a_id]);
			SimplePoint sum_of_weighted_directions(0.0, 0.0, 0.0);
			double sum_of_weights=0.0;

			const ConstrainedContactRemainder::Remainder remainder=ConstrainedContactRemainder::construct_contact_remainder(
					spheres, vertices_vector, ids_vertices_it->second, a_id, probe, sih);

			for(ConstrainedContactRemainder::Remainder::const_iterator remainder_it=remainder.begin();remainder_it!=remainder.end();++remainder_it)
			{
				const ConstrainedContactRemainder::TriangleRecord& tr=(*remainder_it);
				const double weight=triangle_area(tr.p[0], tr.p[1], tr.p[2]);
				for(int i=0;i<3;i++)
				{
					sum_of_weighted_directions=sum_of_weighted_directions+((tr.p[i]-ball_center)*(weight/3.0));
				}
				sum_of_weights+=weight;
			}

			return ((sum_of_weighted_directions*(1.0/sum_of_weights)).unit());
		}
	}
	return SimplePoint();
}

inline bool check_inter_atom_contact_centrality(
		const std::vector<SimpleSphere>& spheres,
		const TriangulationQueries::PairsMap& pairs_neighbors,
		const std::size_t a_id,
		const std::size_t b_id)
{
	if(a_id<spheres.size() && b_id<spheres.size())
	{
		TriangulationQueries::PairsMap::const_iterator pairs_neighbors_it=pairs_neighbors.find(Pair(a_id, b_id));
		if(pairs_neighbors_it!=pairs_neighbors.end())
		{
			const std::set<std::size_t>& neighbors=pairs_neighbors_it->second;
			if(!neighbors.empty())
			{
				const SimplePoint pa(spheres[a_id]);
				const SimplePoint pb(spheres[b_id]);
				const double ra=spheres[a_id].r;
				const double rb=spheres[b_id].r;
				const double distance_to_a_or_b=((distance_from_point_to_point(pa, pb)-ra-rb)*0.5);
				const SimplePoint p=(pa+((pb-pa).unit()*(ra+distance_to_a_or_b)));
				for(std::set<std::size_t>::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();++neighbors_it)
				{
					const std::size_t c_id=(*neighbors_it);
					if(c_id<spheres.size() && minimal_distance_from_point_to_sphere(p, spheres[c_id])<distance_to_a_or_b)
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

inline bool check_inter_atom_contact_peripherial(
		const std::vector<SimpleSphere>& spheres,
		const Triangulation::VerticesVector& vertices_vector,
		const TriangulationQueries::PairsMap& pairs_vertices,
		const std::size_t a_id,
		const std::size_t b_id,
		const double probe)
{
	if(a_id<spheres.size() && b_id<spheres.size())
	{
		TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.find(Pair(a_id, b_id));
		if(pairs_vertices_it!=pairs_vertices.end())
		{
			const std::set<std::size_t>& vertices_ids=pairs_vertices_it->second;
			for(std::set<std::size_t>::const_iterator vertices_ids_it=vertices_ids.begin();vertices_ids_it!=vertices_ids.end();++vertices_ids_it)
			{
				const std::size_t vertex_id=(*vertices_ids_it);
				if(vertex_id<vertices_vector.size() && vertices_vector[vertex_id].second.r>probe)
				{
					return true;
				}
			}
		}
	}
	return false;
}

}

}

#endif /* APOLLOTA_CONSTRAINED_CONTACT_UTILITIES_H_ */
