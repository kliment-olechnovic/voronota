#ifndef APOLLOTA_CONSTRAINED_CONTACT_UTILITIES_H_
#define APOLLOTA_CONSTRAINED_CONTACT_UTILITIES_H_

#include "constrained_contacts_construction.h"
#include "rolling_topology.h"

namespace voronota
{

namespace apollota
{

template<typename OpenGLPrinter>
bool draw_inter_atom_contact(
		const std::vector<SimpleSphere>& spheres,
		const Triangulation::VerticesVector& vertices_vector,
		const TriangulationQueries::PairsMap& pairs_vertices,
		const std::size_t a_id,
		const std::size_t b_id,
		const double probe,
		const double step,
		const int projections,
		const bool simplify,
		std::string& output_graphics)
{
	output_graphics.clear();
	if(a_id<spheres.size() && b_id<spheres.size())
	{
		TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.find(Pair(a_id, b_id));
		if(pairs_vertices_it!=pairs_vertices.end())
		{
			const std::list<ConstrainedContactContour::Contour> contours=ConstrainedContactContour::construct_contact_contours(
					spheres, vertices_vector, pairs_vertices_it->second, a_id, b_id, probe, step, projections, simplify);
			if(!contours.empty())
			{
				OpenGLPrinter opengl_printer;
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
				output_graphics=opengl_printer.str();
				return true;
			}
		}
	}
	return false;
}

template<typename OpenGLPrinter>
bool draw_solvent_contact(
		const std::vector<SimpleSphere>& spheres,
		const Triangulation::VerticesVector& vertices_vector,
		const TriangulationQueries::IDsMap& ids_vertices,
		const std::size_t a_id,
		const double probe,
		const SubdividedIcosahedron& sih,
		std::string& output_graphics)
{
	output_graphics.clear();
	if(a_id<spheres.size())
	{
		TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.find(a_id);
		if(ids_vertices_it!=ids_vertices.end())
		{
			const ConstrainedContactRemainder::Remainder remainder=ConstrainedContactRemainder::construct_contact_remainder(
					spheres, vertices_vector, ids_vertices_it->second, a_id, probe, sih);
			if(!remainder.empty())
			{
				OpenGLPrinter opengl_printer;
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
				output_graphics=opengl_printer.str();
				return true;
			}
		}
	}
	return false;
}

template<typename OpenGLPrinter>
bool draw_solvent_contact_without_tessellation(
		const std::vector<SimpleSphere>& spheres,
		const std::vector<std::size_t>& sorted_neighbor_ids,
		const std::size_t a_id,
		const double probe,
		const SubdividedIcosahedron& sih,
		std::string& output_graphics)
{
	output_graphics.clear();
	if(a_id<spheres.size())
	{
		const ConstrainedContactRemainder::Remainder remainder=ConstrainedContactRemainder::construct_contact_remainder_without_tessellation(spheres, sorted_neighbor_ids, a_id, probe, sih);
		if(!remainder.empty())
		{
			OpenGLPrinter opengl_printer;
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
			output_graphics=opengl_printer.str();
			return true;
		}
	}
	return false;
}

template<typename OpenGLPrinter>
bool draw_solvent_contact_in_two_scales(
		const std::vector<SimpleSphere>& spheres,
		const Triangulation::VerticesVector& vertices_vector,
		const TriangulationQueries::IDsMap& ids_vertices,
		const std::size_t a_id,
		const double probe,
		const SubdividedIcosahedron& sih,
		std::string& output_graphics1,
		std::string& output_graphics2)
{
	output_graphics1.clear();
	output_graphics2.clear();
	if(a_id<spheres.size())
	{
		TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.find(a_id);
		if(ids_vertices_it!=ids_vertices.end())
		{
			const ConstrainedContactRemainder::Remainder remainder=ConstrainedContactRemainder::construct_contact_remainder(
					spheres, vertices_vector, ids_vertices_it->second, a_id, probe, sih);
			if(!remainder.empty())
			{
				OpenGLPrinter opengl_printer1;
				OpenGLPrinter opengl_printer2;
				std::vector<SimplePoint> ts(3);
				std::vector<SimplePoint> ns(3);
				for(ConstrainedContactRemainder::Remainder::const_iterator remainder_it=remainder.begin();remainder_it!=remainder.end();++remainder_it)
				{
					for(int i=0;i<3;i++)
					{
						ts[i]=remainder_it->p[i];
						ns[i]=sub_of_points<SimplePoint>(ts[i], spheres[a_id]).unit();
					}
					opengl_printer1.add_triangle_strip(ts, ns);
					for(int i=0;i<3;i++)
					{
						ts[i]=ts[i]-(ns[i]*probe);
					}
					opengl_printer2.add_triangle_strip(ts, ns);
				}
				output_graphics1=opengl_printer1.str();
				output_graphics2=opengl_printer2.str();
				return true;
			}
		}
	}
	return false;
}

inline void displace_rolling_point(
		const SimplePoint& a,
		const SimplePoint& b,
		const SimplePoint& center,
		const double probe,
		const double displacement_scaling,
		SimplePoint& p)
{
	double displacement=std::max(probe-distance_from_point_to_point(p, center), 0.0);
	if(displacement>0.0)
	{
		const double p_angle=min_angle(p, center, p+(b-a));
		if(p_angle>0.0)
		{
			const double p_dist=distance_from_point_to_line(p, a, b);
			const double max_displacement=p_dist/sin(p_angle);
			displacement=std::min(displacement, max_displacement);
		}
		p=p+((p-center).unit()*displacement*displacement_scaling);
	}
}

template<typename OpenGLPrinter>
bool draw_solvent_alt_contact(
		const std::vector<SimpleSphere>& spheres,
		const std::vector<RollingTopology::RollingDescriptor>& rolling_descriptors,
		const std::size_t a_id,
		const std::vector<std::size_t>& ids_of_rolling_descriptors,
		const double probe,
		const double angle_step,
		const double offset_step,
		const double displacement_scaling,
		std::string& output_graphics)
{
	output_graphics.clear();
	if(a_id<spheres.size() && !ids_of_rolling_descriptors.empty())
	{
		OpenGLPrinter opengl_printer;
		std::vector<SimplePoint> vertices;
		std::vector<SimplePoint> normals;
		for(std::vector<std::size_t>::const_iterator ids_of_rolling_descriptors_it=ids_of_rolling_descriptors.begin();ids_of_rolling_descriptors_it!=ids_of_rolling_descriptors.end();++ids_of_rolling_descriptors_it)
		{
			const RollingTopology::RollingDescriptor& rolling_descriptor=rolling_descriptors[*ids_of_rolling_descriptors_it];
			if(rolling_descriptor.a_id==a_id || rolling_descriptor.b_id==a_id)
			{
				const SimplePoint a(rolling_descriptor.a_id==a_id ? spheres[rolling_descriptor.a_id] : spheres[rolling_descriptor.b_id]);
				const SimplePoint b(rolling_descriptor.a_id==a_id ? spheres[rolling_descriptor.b_id] : spheres[rolling_descriptor.a_id]);
				if(!rolling_descriptor.strips.empty())
				{
					for(std::list<RollingTopology::RollingStrip>::const_iterator strip_it=rolling_descriptor.strips.begin();strip_it!=rolling_descriptor.strips.end();++strip_it)
					{
						{
							const std::vector<SimplePoint> points=RollingTopology::construct_rolling_strip_approximation(rolling_descriptor, (*strip_it), angle_step);
							for(double offset=0.0;offset<0.51;offset+=offset_step)
							{
								vertices.clear();
								normals.clear();
								vertices.reserve(points.size()*2);
								normals.reserve(points.size()*2);
								for(std::size_t i=0;i<points.size();i++)
								{
									const SimplePoint& center=points[i];
									const SimplePoint ops[2]={(center+((a-center).unit()*probe)), (center+((b-center).unit()*probe))};
									const double prev_offset=(offset>0.0 ? (offset-offset_step) : (offset-0.01));
									SimplePoint ps[2]={ops[0]+((ops[1]-ops[0])*prev_offset), ops[0]+((ops[1]-ops[0])*offset)};
									for(int e=0;e<2;e++)
									{
										displace_rolling_point(a, b, center, probe, displacement_scaling, ps[e]);
										vertices.push_back(ps[e]);
										normals.push_back((center-ps[e]).unit());
									}
								}
								opengl_printer.add_triangle_strip(vertices, normals);
							}
						}

						for(int i=0;i<2;i++)
						{
							const SimplePoint center(i==0 ? strip_it->start.tangent : strip_it->end.tangent);
							const SimplePoint c(spheres[i==0 ? strip_it->start.generator : strip_it->end.generator]);
							const SimplePoint ops[3]={(center+((a-center).unit()*probe)), (center+((b-center).unit()*probe)), (center+((c-center).unit()*probe))};
							SimplePoint mps[3]={(ops[0]+ops[1])*0.5, (ops[0]+ops[2])*0.5, (ops[1]+ops[2])*0.5};
							double mps_angle_coefs[3]={min_angle(ops[0], ops[1], ops[2]), min_angle(ops[1], ops[0], ops[2]), min_angle(ops[2], ops[0], ops[1])};
							for(int e=0;e<3;e++)
							{
								mps_angle_coefs[e]*=mps_angle_coefs[e];
							}
							double mps_weights[3]={(mps_angle_coefs[0]+mps_angle_coefs[1]), (mps_angle_coefs[0]+mps_angle_coefs[2]), (mps_angle_coefs[1]+mps_angle_coefs[2])};
							for(int e=0;e<3;e++)
							{
								mps_weights[e]=(1.0-(mps_weights[e]/(mps_angle_coefs[0]+mps_angle_coefs[1]+mps_angle_coefs[2])));
							}
							for(int e=0;e<3;e++)
							{
								displace_rolling_point(((e==0 || e==1) ? a : b), ((e==0) ? b : c), center, probe, displacement_scaling, mps[e]);
							}
							const SimplePoint join_point=((mps[0]*mps_weights[0])+(mps[1]*mps_weights[1])+(mps[2]*mps_weights[2]))*(1.0/(mps_weights[0]+mps_weights[1]+mps_weights[2]));
							vertices.clear();
							normals.clear();
							vertices.push_back(join_point);
							vertices.push_back(ops[0]);
							for(double offset=offset_step;offset<0.51;offset+=offset_step)
							{
								SimplePoint p=(ops[0]+((ops[1]-ops[0])*offset));
								displace_rolling_point(a, b, center, probe, displacement_scaling, p);
								vertices.push_back(p);
							}
							for(std::size_t j=0;j<vertices.size();j++)
							{
								normals.push_back((center-vertices[j]).unit());
							}
							opengl_printer.add_triangle_fan(vertices, normals);
						}
					}
				}
				else if(rolling_descriptor.detached)
				{
					const std::vector<SimplePoint> points=RollingTopology::construct_rolling_circle_approximation(rolling_descriptor, angle_step);
					for(double offset=offset_step;offset<0.51;offset+=offset_step)
					{
						vertices.clear();
						normals.clear();
						vertices.reserve(points.size()*2);
						normals.reserve(points.size()*2);
						for(std::size_t i=0;i<points.size();i++)
						{
							const SimplePoint& center=points[i];
							const SimplePoint ops[2]={(center+((a-center).unit()*probe)), (center+((b-center).unit()*probe))};
							const double prev_offset=(offset>0.0 ? (offset-offset_step) : (offset-0.01));
							SimplePoint ps[2]={ops[0]+((ops[1]-ops[0])*prev_offset), ops[0]+((ops[1]-ops[0])*offset)};
							for(int e=0;e<2;e++)
							{
								displace_rolling_point(a, b, center, probe, displacement_scaling, ps[e]);
								vertices.push_back(ps[e]);
								normals.push_back((center-ps[e]).unit());
							}
						}
						opengl_printer.add_triangle_strip(vertices, normals);
					}
				}
			}
		}
		output_graphics=opengl_printer.str();
		return true;
	}
	return false;
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
