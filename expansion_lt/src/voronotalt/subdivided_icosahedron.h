#ifndef VORONOTALT_SUBDIVIDED_ICOSAHEDRON_H_
#define VORONOTALT_SUBDIVIDED_ICOSAHEDRON_H_

#include <vector>
#include <map>

#include "basic_types_and_functions.h"

namespace voronotalt
{

struct SubdividedIcosahedron
{
	struct Pair
	{
		UnsignedInt ids[2];

		Pair(const UnsignedInt a, const UnsignedInt b) noexcept
		{
			ids[0]=a;
			ids[1]=b;
			if(ids[0]>ids[1])
			{
				std::swap(ids[0], ids[1]);
			}
		}

		bool operator==(const Pair& p) const noexcept
		{
			return (ids[0]==p.ids[0] && ids[1]==p.ids[1]);
		}

		bool operator<(const Pair& p) const noexcept
		{
			return (ids[0]<p.ids[0] || (ids[0]==p.ids[0] && ids[1]<p.ids[1]));
		}
	};

	struct Triple
	{
		UnsignedInt ids[3];

		Triple(const UnsignedInt a, const UnsignedInt b, const UnsignedInt c) noexcept
		{
			ids[0]=a;
			ids[1]=b;
			ids[2]=c;
			if(ids[0]>ids[2])
			{
				std::swap(ids[0], ids[2]);
			}
			if(ids[0]>ids[1])
			{
				std::swap(ids[0], ids[1]);
			}
			if(ids[1]>ids[2])
			{
				std::swap(ids[1], ids[2]);
			}
		}
	};

	std::vector<SimplePoint> vertices;
	std::vector<Triple> triples;
	Float max_edge_length;

	explicit SubdividedIcosahedron(const UnsignedInt depth) noexcept : max_edge_length(FLOATCONST(0.0))
	{
		const double t=(1+sqrt(5.0))/2.0;

		vertices.push_back(unit_point(SimplePoint( t, 1, 0)));
		vertices.push_back(unit_point(SimplePoint(-t, 1, 0)));
		vertices.push_back(unit_point(SimplePoint( t,-1, 0)));
		vertices.push_back(unit_point(SimplePoint(-t,-1, 0)));
		vertices.push_back(unit_point(SimplePoint( 1, 0, t)));
		vertices.push_back(unit_point(SimplePoint( 1, 0,-t)));
		vertices.push_back(unit_point(SimplePoint(-1, 0, t)));
		vertices.push_back(unit_point(SimplePoint(-1, 0,-t)));
		vertices.push_back(unit_point(SimplePoint( 0, t, 1)));
		vertices.push_back(unit_point(SimplePoint( 0,-t, 1)));
		vertices.push_back(unit_point(SimplePoint( 0, t,-1)));
		vertices.push_back(unit_point(SimplePoint( 0,-t,-1)));

		triples.push_back(Triple(0, 8, 4));
		triples.push_back(Triple(1, 10, 7));
		triples.push_back(Triple(2, 9, 11));
		triples.push_back(Triple(7, 3, 1));
		triples.push_back(Triple(0, 5, 10));
		triples.push_back(Triple(3, 9, 6));
		triples.push_back(Triple(3, 11, 9));
		triples.push_back(Triple(8, 6, 4));
		triples.push_back(Triple(2, 4, 9));
		triples.push_back(Triple(3, 7, 11));
		triples.push_back(Triple(4, 2, 0));
		triples.push_back(Triple(9, 4, 6));
		triples.push_back(Triple(2, 11, 5));
		triples.push_back(Triple(0, 10, 8));
		triples.push_back(Triple(5, 0, 2));
		triples.push_back(Triple(10, 5, 7));
		triples.push_back(Triple(1, 6, 8));
		triples.push_back(Triple(1, 8, 10));
		triples.push_back(Triple(6, 1, 3));
		triples.push_back(Triple(11, 7, 5));

		for(UnsignedInt l=0;l<depth;l++)
		{
			std::vector<Triple> next_triples;
			next_triples.reserve(triples.size()*4);
			std::map<Pair, UnsignedInt> pairs_to_vertices;
			for(UnsignedInt i=0;i<triples.size();i++)
			{
				const Triple& triple=triples[i];
				UnsignedInt middle_point_ids[3];
				for(UnsignedInt j=0;j<3;j++)
				{
					const Pair pair=(j==0 ? Pair(triple.ids[1], triple.ids[2]) : (j==1 ? Pair(triple.ids[0], triple.ids[2]) : Pair(triple.ids[0], triple.ids[1])));
					std::map<Pair, UnsignedInt>::const_iterator it=pairs_to_vertices.find(pair);
					if(it==pairs_to_vertices.end())
					{
						middle_point_ids[j]=vertices.size();
						pairs_to_vertices[pair]=middle_point_ids[j];
						vertices.push_back(unit_point(point_and_number_product(sum_of_points(vertices[pair.ids[0]], vertices[pair.ids[1]]), 0.5)));
					}
					else
					{
						middle_point_ids[j]=it->second;
					}
				}
				next_triples.push_back(Triple(triple.ids[0], middle_point_ids[1], middle_point_ids[2]));
				next_triples.push_back(Triple(triple.ids[1], middle_point_ids[0], middle_point_ids[2]));
				next_triples.push_back(Triple(triple.ids[2], middle_point_ids[0], middle_point_ids[1]));
				next_triples.push_back(Triple(middle_point_ids[0], middle_point_ids[1], middle_point_ids[2]));
			}
			triples.swap(next_triples);
		}

		for(std::size_t i=0;i<triples.size();i++)
		{
			const SubdividedIcosahedron::Triple& triple=triples[i];
			max_edge_length=std::max(max_edge_length, distance_from_point_to_point(vertices[triple.ids[0]], vertices[triple.ids[1]]));
			max_edge_length=std::max(max_edge_length, distance_from_point_to_point(vertices[triple.ids[0]], vertices[triple.ids[2]]));
			max_edge_length=std::max(max_edge_length, distance_from_point_to_point(vertices[triple.ids[1]], vertices[triple.ids[2]]));
		}
	}

	SimplePoint get_point_on_sphere(const UnsignedInt i, const SimpleSphere& sphere) const noexcept
	{
		SimplePoint result;
		if(i<vertices.size())
		{
			const SimplePoint& p=vertices[i];
			result=sum_of_points(sphere.p, point_and_number_product(p, sphere.r));
		}
		return result;
	}
};

}

#endif /* VORONOTALT_SUBDIVIDED_ICOSAHEDRON_H_ */
