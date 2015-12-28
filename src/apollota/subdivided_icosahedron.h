#ifndef APOLLOTA_SUBDIVIDED_ICOSAHEDRON_H_
#define APOLLOTA_SUBDIVIDED_ICOSAHEDRON_H_

#include "basic_operations_on_points.h"
#include "tuple.h"

namespace apollota
{

class SubdividedIcosahedron
{
public:
	explicit SubdividedIcosahedron(const int depth) : center_(0, 0, 0)
	{
		const double t=(1+sqrt(5.0))/2.0;

		vertices_.push_back(SimplePoint( t, 1, 0).unit());
		vertices_.push_back(SimplePoint(-t, 1, 0).unit());
		vertices_.push_back(SimplePoint( t,-1, 0).unit());
		vertices_.push_back(SimplePoint(-t,-1, 0).unit());
		vertices_.push_back(SimplePoint( 1, 0, t).unit());
		vertices_.push_back(SimplePoint( 1, 0,-t).unit());
		vertices_.push_back(SimplePoint(-1, 0, t).unit());
		vertices_.push_back(SimplePoint(-1, 0,-t).unit());
		vertices_.push_back(SimplePoint( 0, t, 1).unit());
		vertices_.push_back(SimplePoint( 0,-t, 1).unit());
		vertices_.push_back(SimplePoint( 0, t,-1).unit());
		vertices_.push_back(SimplePoint( 0,-t,-1).unit());

		triples_.push_back(Triple(0, 8, 4));
		triples_.push_back(Triple(1, 10, 7));
		triples_.push_back(Triple(2, 9, 11));
		triples_.push_back(Triple(7, 3, 1));
		triples_.push_back(Triple(0, 5, 10));
		triples_.push_back(Triple(3, 9, 6));
		triples_.push_back(Triple(3, 11, 9));
		triples_.push_back(Triple(8, 6, 4));
		triples_.push_back(Triple(2, 4, 9));
		triples_.push_back(Triple(3, 7, 11));
		triples_.push_back(Triple(4, 2, 0));
		triples_.push_back(Triple(9, 4, 6));
		triples_.push_back(Triple(2, 11, 5));
		triples_.push_back(Triple(0, 10, 8));
		triples_.push_back(Triple(5, 0, 2));
		triples_.push_back(Triple(10, 5, 7));
		triples_.push_back(Triple(1, 6, 8));
		triples_.push_back(Triple(1, 8, 10));
		triples_.push_back(Triple(6, 1, 3));
		triples_.push_back(Triple(11, 7, 5));

		for(int i=0;i<depth;i++)
		{
			grow();
		}

		max_edge_length_=calc_max_edge_length();
	}

	const SimplePoint center() const
	{
		return center_;
	}

	const std::vector<SimplePoint>& vertices() const
	{
		return vertices_;
	}

	const std::vector<Triple>& triples() const
	{
		return triples_;
	}

	template<typename PointType>
	void fit_into_sphere(const PointType& center, const double radius)
	{
		const SimplePoint new_center=custom_point_from_object<SimplePoint>(center);
		for(std::size_t i=0;i<vertices_.size();i++)
		{
			vertices_[i]=new_center+((vertices_[i]-center_).unit()*radius);
		}
		center_=new_center;
		max_edge_length_=calc_max_edge_length();
	}

	double max_edge_length() const
	{
		return max_edge_length_;
	}

private:
	void grow()
	{
		typedef std::tr1::unordered_map<Pair, std::size_t, Pair::HashFunctor> PairsMap;
		PairsMap pairs_vertices;
		std::vector<Triple> new_triples;
		new_triples.reserve(triples_.size()*4);
		std::size_t middle_point_ids[3]={0, 0, 0};
		for(std::size_t i=0;i<triples_.size();i++)
		{
			const Triple& triple=triples_[i];
			for(int j=0;j<3;j++)
			{
				const Pair pair=triple.exclude(j);
				PairsMap::const_iterator it=pairs_vertices.find(pair);
				if(it==pairs_vertices.end())
				{
					middle_point_ids[j]=vertices_.size();
					vertices_.push_back(((vertices_[pair.get(0)]+vertices_[pair.get(1)])*(0.5)).unit());
					pairs_vertices[pair]=middle_point_ids[j];
				}
				else
				{
					middle_point_ids[j]=it->second;
				}
			}
			new_triples.push_back(Triple(triple.get(0), middle_point_ids[1], middle_point_ids[2]));
			new_triples.push_back(Triple(triple.get(1), middle_point_ids[0], middle_point_ids[2]));
			new_triples.push_back(Triple(triple.get(2), middle_point_ids[0], middle_point_ids[1]));
			new_triples.push_back(Triple(middle_point_ids[0], middle_point_ids[1], middle_point_ids[2]));
		}
		triples_=new_triples;
	}

	double calc_max_edge_length() const
	{
		double max_length=0.0;
		for(std::size_t i=0;i<triples_.size();i++)
		{
			const Triple& t=triples_[i];
			max_length=std::max(max_length, distance_from_point_to_point(vertices_[t.get(0)], vertices_[t.get(1)]));
			max_length=std::max(max_length, distance_from_point_to_point(vertices_[t.get(0)], vertices_[t.get(2)]));
			max_length=std::max(max_length, distance_from_point_to_point(vertices_[t.get(1)], vertices_[t.get(2)]));
		}
		return max_length;
	}

	SimplePoint center_;
	std::vector<SimplePoint> vertices_;
	std::vector<Triple> triples_;
	double max_edge_length_;
};

}

#endif /* APOLLOTA_SUBDIVIDED_ICOSAHEDRON_H_ */
