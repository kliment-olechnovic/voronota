#ifndef APOLLOTA_SPHERICAL_CONTACTS_CONSTRUCTION_H_
#define APOLLOTA_SPHERICAL_CONTACTS_CONSTRUCTION_H_

#include "triangulation_queries.h"
#include "subdivided_icosahedron.h"

namespace voronota
{

namespace apollota
{

class SphericalContactsConstruction
{
public:
	struct Result
	{
		std::map<std::size_t, double> areas;

		void add(const std::size_t id, const SimplePoint& a, const SimplePoint& b, const SimplePoint& c)
		{
			areas[id]+=triangle_area(a, b, c);
		}
	};

	explicit SphericalContactsConstruction(const std::size_t subdivision_depth) : sih_(subdivision_depth)
	{
	}

	Result construct_contacts(
			const double probe_radius,
			const std::vector<SimpleSphere>& spheres,
			std::size_t self_id,
			const std::vector<size_t>& neighbours) const
	{
		Result result;
		if(self_id<spheres.size())
		{
			SubdividedIcosahedron sih=sih_;
			sih.fit_into_sphere(spheres[self_id], spheres[self_id].r+probe_radius);
			std::vector<std::size_t> influences(sih.vertices().size());
			for(std::size_t i=0;i<influences.size();i++)
			{
				double min_distance=minimal_distance_from_point_to_sphere(sih.vertices().at(i), spheres.at(self_id));
				influences[i]=self_id;
				for(std::size_t j=0;j<neighbours.size();j++)
				{
					double distance=minimal_distance_from_point_to_sphere(sih.vertices().at(i), spheres.at(neighbours[j]));
					if(distance<min_distance)
					{
						min_distance=distance;
						influences[i]=neighbours[j];
					}
				}
			}
			for(std::size_t e=0;e<sih.triples().size();e++)
			{
				const Triple& triple=sih.triples()[e];
				const std::size_t a=triple.get(0);
				const std::size_t b=triple.get(1);
				const std::size_t c=triple.get(2);
				if(influences[a]==influences[b] && influences[a]==influences[c])
				{
					result.add(influences[a], sih.vertices()[a], sih.vertices()[b], sih.vertices()[c]);
				}
				else if(influences[a]!=influences[b] && influences[a]!=influences[c] && influences[b]!=influences[c])
				{
					const SimplePoint& pa=sih.vertices()[a];
					const SimplePoint& pb=sih.vertices()[b];
					const SimplePoint& pc=sih.vertices()[c];

					const SimplePoint a_b_border=pa+((pb-pa).unit()*HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(pa, pb, spheres[influences[a]], spheres[influences[b]]));
					const SimplePoint a_c_border=pa+((pc-pa).unit()*HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(pa, pc, spheres[influences[a]], spheres[influences[c]]));
					const SimplePoint b_c_border=pb+((pc-pb).unit()*HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(pb, pc, spheres[influences[b]], spheres[influences[c]]));

					const SimplePoint middle=(a_b_border+a_c_border+b_c_border)*(1.0/3.0);

					result.add(influences[a], pa, a_b_border, a_c_border);
					result.add(influences[a], middle, a_b_border, a_c_border);

					result.add(influences[b], pb, a_b_border, b_c_border);
					result.add(influences[b], middle, a_b_border, b_c_border);

					result.add(influences[c], pc, a_c_border, b_c_border);
					result.add(influences[c], middle, a_c_border, b_c_border);
				}
				else
				{
					std::size_t s=a;
					std::size_t d1=b;
					std::size_t d2=c;
					if(influences[b]!=influences[a] && influences[b]!=influences[c])
					{
						s=b;
						d1=a;
						d2=c;
					}
					else if(influences[c]!=influences[a] && influences[c]!=influences[b])
					{
						s=c;
						d1=a;
						d2=b;
					}

					const SimplePoint& ps=sih.vertices()[s];
					const SimplePoint& pd1=sih.vertices()[d1];
					const SimplePoint& pd2=sih.vertices()[d2];

					const SimplePoint s_d1_border=ps+((pd1-ps).unit()*HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(ps, pd1, spheres[influences[s]], spheres[influences[d1]]));
					const SimplePoint s_d2_border=ps+((pd2-ps).unit()*HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(ps, pd2, spheres[influences[s]], spheres[influences[d2]]));

					result.add(influences[s], ps, s_d1_border, s_d2_border);
					result.add(influences[d1], pd1, s_d1_border, s_d2_border);
					result.add(influences[d2], pd2, pd1, s_d2_border);
				}
			}
		}
		return result;
	}

private:
	SubdividedIcosahedron sih_;
};

}

}

#endif /* APOLLOTA_SPHERICAL_CONTACTS_CONSTRUCTION_H_ */
