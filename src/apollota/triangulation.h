#ifndef APOLLOTA_TRIANGULATION_H_
#define APOLLOTA_TRIANGULATION_H_

#include <limits>
#include <vector>
#include <map>
#include <set>

#include "../compatability/tr1_usage.h"

#if USE_TR1 > 0
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#else
#include <unordered_map>
#include <unordered_set>
#endif

#include "tuple.h"
#include "search_for_spherical_collisions.h"
#include "tangent_plane_of_three_spheres.h"
#include "tangent_sphere_of_four_spheres.h"
#include "tangent_sphere_of_three_spheres.h"

namespace voronota
{

namespace apollota
{

static const std::size_t npos=static_cast<std::size_t>(-1);

class Triangulation
{
public:

#if USE_TR1 > 0
typedef std::tr1::unordered_map<Quadruple, std::vector<SimpleSphere>, Quadruple::HashFunctor> QuadruplesMap;
#else
typedef std::unordered_map<Quadruple, std::vector<SimpleSphere>, Quadruple::HashFunctor> QuadruplesMap;
#endif

	typedef std::vector< std::pair<Quadruple, SimpleSphere> > VerticesVector;
	typedef std::vector< std::vector<std::size_t> > VerticesGraph;

	struct QuadruplesSearchLog
	{
		std::size_t added_quadruples;
		std::size_t added_tangent_spheres;
		std::size_t processed_faces;
		std::size_t encountered_difficult_faces;
		std::size_t produced_faces;
		std::size_t updated_faces;
		std::size_t encountered_triples_repetitions;
		std::size_t performed_iterations_for_finding_first_faces;
	};

	struct SurplusQuadruplesSearchLog
	{
		std::size_t surplus_quadruples;
		std::size_t surplus_tangent_spheres;
	};

	struct Result
	{
		QuadruplesMap quadruples_map;
		QuadruplesSearchLog quadruples_search_log;
		SurplusQuadruplesSearchLog surplus_quadruples_search_log;
		std::set<std::size_t> excluded_hidden_spheres_ids;
		std::set<std::size_t> ignored_spheres_ids;

		Result() : quadruples_search_log(), surplus_quadruples_search_log()
		{
		}
	};

	static Result construct_result(
			const std::vector<SimpleSphere>& spheres,
			const double initial_radius_for_spheres_bucketing,
			const bool exclude_hidden_spheres,
			const bool include_surplus_valid_quadruples)
	{
		Result result;

		{
			BoundingSpheresHierarchy bsh(spheres, initial_radius_for_spheres_bucketing, 1);

			std::vector<std::size_t> refined_spheres_backward_mapping;
			if(exclude_hidden_spheres)
			{
				result.excluded_hidden_spheres_ids=SearchForSphericalCollisions::find_all_hidden_spheres(bsh);
				if(!result.excluded_hidden_spheres_ids.empty())
				{
					std::vector<SimpleSphere> refined_spheres;
					const std::size_t refined_spheres_count=bsh.leaves_spheres().size()-result.excluded_hidden_spheres_ids.size();
					refined_spheres.reserve(refined_spheres_count);
					refined_spheres_backward_mapping.reserve(refined_spheres_count);
					for(std::size_t i=0;i<bsh.leaves_spheres().size();i++)
					{
						if(result.excluded_hidden_spheres_ids.count(i)==0)
						{
							refined_spheres.push_back(bsh.leaves_spheres()[i]);
							refined_spheres_backward_mapping.push_back(i);
						}
					}
					bsh=BoundingSpheresHierarchy(refined_spheres, initial_radius_for_spheres_bucketing, 1);
				}
			}

			result.quadruples_search_log=find_valid_quadruples(bsh, std::vector<int>(bsh.leaves_spheres().size(), 1), result.quadruples_map);
			if(include_surplus_valid_quadruples)
			{
				result.surplus_quadruples_search_log=find_surplus_valid_quadruples(bsh, result.quadruples_map);
			}

			if(!refined_spheres_backward_mapping.empty())
			{
				result.quadruples_map=renumber_quadruples_map(result.quadruples_map, refined_spheres_backward_mapping);
			}
		}

		result.ignored_spheres_ids=collect_ignored_spheres_ids(std::vector<int>(spheres.size(), 1), result.quadruples_map);

		return result;
	}

	template<typename AdmittanceSet>
	static Result construct_result_for_admittance_set(
			const BoundingSpheresHierarchy& bsh,
			const AdmittanceSet& admittance_set,
			const bool include_surplus_valid_quadruples)
	{
		Result result;
		std::vector<int> admittance(bsh.leaves_spheres().size(), 0);
		bool admittance_filled=false;
		for(typename AdmittanceSet::const_iterator it=admittance_set.begin();it!=admittance_set.end();++it)
		{
			const std::size_t id=(*it);
			if(id<admittance.size())
			{
				admittance[id]=1;
				admittance_filled=true;
			}
		}
		if(admittance_filled)
		{
			result.quadruples_search_log=find_valid_quadruples(bsh, admittance, result.quadruples_map);
			if(include_surplus_valid_quadruples)
			{
				result.surplus_quadruples_search_log=find_surplus_valid_quadruples(bsh, result.quadruples_map);
			}
			result.ignored_spheres_ids=collect_ignored_spheres_ids(admittance, result.quadruples_map);
		}
		return result;
	}

	static void merge_quadruples_maps(const QuadruplesMap& source_quadruples_map, QuadruplesMap& destination_quadruples_map)
	{
		for(QuadruplesMap::const_iterator it=source_quadruples_map.begin();it!=source_quadruples_map.end();++it)
		{
			const Quadruple& quadruple=it->first;
			const std::vector<SimpleSphere>& tangent_spheres=it->second;
			for(std::size_t i=0;i<tangent_spheres.size();i++)
			{
				augment_quadruples_map(quadruple, tangent_spheres[i], destination_quadruples_map);
			}
		}
	}

	static VerticesVector collect_vertices_vector_from_quadruples_map(const QuadruplesMap& quadruples_map)
	{
		typedef std::multimap<QuadruplesMap::key_type, QuadruplesMap::mapped_type> QuadruplesOrderedMultimap;
		QuadruplesOrderedMultimap quadruples_ordered_multimap;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			quadruples_ordered_multimap.insert(*it);
		}
		VerticesVector vertices_vector;
		vertices_vector.reserve(count_tangent_spheres_in_quadruples_map(quadruples_map));
		for(QuadruplesOrderedMultimap::const_iterator it=quadruples_ordered_multimap.begin();it!=quadruples_ordered_multimap.end();++it)
		{
			const std::vector<SimpleSphere>& tangent_spheres=it->second;
			if(tangent_spheres.size()==1)
			{
				vertices_vector.push_back(std::make_pair(it->first, tangent_spheres.front()));
			}
			else if(tangent_spheres.size()==2)
			{
				const SimpleSphere& a=tangent_spheres.front();
				const SimpleSphere& b=tangent_spheres.back();
				if(std::make_pair(a.r, std::make_pair(a.x, std::make_pair(a.y, a.z)))<std::make_pair(b.r, std::make_pair(b.x, std::make_pair(b.y, b.z))))
				{
					vertices_vector.push_back(std::make_pair(it->first, a));
					vertices_vector.push_back(std::make_pair(it->first, b));
				}
				else
				{
					vertices_vector.push_back(std::make_pair(it->first, b));
					vertices_vector.push_back(std::make_pair(it->first, a));
				}
			}
		}
		return vertices_vector;
	}

	static std::size_t count_tangent_spheres_in_quadruples_map(const QuadruplesMap& quadruples_map)
	{
		std::size_t sum=0;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			sum+=it->second.size();
		}
		return sum;
	}

	template<typename SphereType>
	static bool check_quadruples_map(const std::vector<SphereType>& spheres, const QuadruplesMap& quadruples_map)
	{
		for(typename QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const QuadruplesMap::key_type& q=it->first;
			const QuadruplesMap::mapped_type& ts=it->second;
			if(q.has_repetetions() || ts.empty() || ts.size()>2)
			{
				return false;
			}
			for(std::size_t i=0;i<ts.size();i++)
			{
				const SimpleSphere& t=ts[i];
				for(std::size_t j=0;j<spheres.size();j++)
				{
					if(sphere_intersects_sphere(t, spheres[j]))
					{
						return false;
					}
				}
			}
		}
		return true;
	}

	static VerticesGraph construct_vertices_graph(const std::vector<SimpleSphere>& spheres, const QuadruplesMap& quadruples_map)
	{
#if USE_TR1 > 0
typedef std::tr1::unordered_map<Triple, std::vector<std::size_t>, Triple::HashFunctor> TriplesVerticesMap;
#else
typedef std::unordered_map<Triple, std::vector<std::size_t>, Triple::HashFunctor> TriplesVerticesMap;
#endif

		const VerticesVector valid_vertices_vector=collect_vertices_vector_from_quadruples_map(quadruples_map);
		const VerticesVector invalid_vertices_vector=collect_vertices_vector_from_quadruples_map(collect_invalid_tangent_spheres_of_valid_quadruples(spheres, quadruples_map));
		VerticesVector all_vertices_vector=valid_vertices_vector;
		all_vertices_vector.insert(all_vertices_vector.end(), invalid_vertices_vector.begin(), invalid_vertices_vector.end());

		TriplesVerticesMap triples_vertices_map;
		for(std::size_t i=0;i<all_vertices_vector.size();i++)
		{
			const Quadruple& q=all_vertices_vector[i].first;
			for(int j=0;j<4;j++)
			{
				triples_vertices_map[q.exclude(j)].push_back(i);
			}
		}

		VerticesGraph vertices_graph(valid_vertices_vector.size(), std::vector<std::size_t>(4, npos));
		for(TriplesVerticesMap::const_iterator it=triples_vertices_map.begin();it!=triples_vertices_map.end();++it)
		{
			update_vertices_graph(spheres, all_vertices_vector, it->first, it->second, vertices_graph);
		}

		return vertices_graph;
	}

private:
	class Face
	{
	public:
		Face(const std::vector<SimpleSphere>& spheres, const Triple& abc_ids, const double min_sphere_radius) :
			spheres_(&spheres),
			abc_ids_(abc_ids),
			a_sphere_(&(spheres_->at(abc_ids_.get(0)))),
			b_sphere_(&(spheres_->at(abc_ids_.get(1)))),
			c_sphere_(&(spheres_->at(abc_ids_.get(2)))),
			tangent_planes_(TangentPlaneOfThreeSpheres::calculate((*a_sphere_), (*b_sphere_), (*c_sphere_))),
			can_have_d_(tangent_planes_.size()==2),
			can_have_e_(!can_have_d_ || greater(a_sphere_->r, min_sphere_radius) || greater(b_sphere_->r, min_sphere_radius) || greater(c_sphere_->r, min_sphere_radius))
		{
			if(can_have_d_)
			{
				d_ids_and_tangent_spheres_.resize(2, std::pair<std::size_t, SimpleSphere>(npos, SimpleSphere()));
				init_central_planes();
				init_middle_region_approximation();
			}
			else
			{
				tangent_planes_.clear();
			}
		}

		const Triple& abc_ids() const
		{
			return abc_ids_;
		}

		bool can_have_d() const
		{
			return can_have_d_;
		}

		template<typename InputSphereType>
		bool sphere_may_contain_candidate_for_d(const InputSphereType& input_sphere, const std::size_t d_number) const
		{
			return (
					can_have_d_
					&& (d_number<2)
					&& (halfspace_of_sphere(tangent_planes_[d_number].first, tangent_planes_[d_number].second, input_sphere)>=0)
					);
		}

		std::pair<bool, SimpleSphere> check_candidate_for_d(const std::size_t d_id, const std::size_t d_number) const
		{
			if(
					can_have_d_
					&& (d_id!=npos)
					&& (d_number<2)
					&& (d_id!=d_ids_and_tangent_spheres_[d_number].first)
					&& (!abc_ids_.contains(d_id))
					&& (halfspace_of_sphere(tangent_planes_[d_number].first, tangent_planes_[d_number].second, spheres_->at(d_id))>=0)
				)
			{
				const std::vector<SimpleSphere> tangent_spheres=TangentSphereOfFourSpheres::calculate((*a_sphere_), (*b_sphere_), (*c_sphere_), spheres_->at(d_id));
				if(!tangent_spheres.empty())
				{
					std::size_t i=0;
					if(tangent_spheres.size()==2)
					{
						const double hs0=halfspace_of_point(central_planes_[d_number].first, central_planes_[d_number].second, tangent_spheres[0]);
						const double hs1=halfspace_of_point(central_planes_[d_number].first, central_planes_[d_number].second, tangent_spheres[1]);
						if(hs0==1 && hs1==-1)
						{
							i=0;
						}
						else if(hs0==-1 && hs1==1)
						{
							i=1;
						}
						else if(hs0==-1 && hs1==-1)
						{
							i=(tangent_spheres[0].r<tangent_spheres[1].r ? 0 : 1);
						}
						else if(hs0==1 && hs1==1)
						{
							i=(tangent_spheres[0].r>tangent_spheres[1].r ? 0 : 1);
						}
					}
					const SimpleSphere& tangent_sphere=tangent_spheres[i];
					if(!sphere_intersects_recorded_sphere(d_ids_and_tangent_spheres_, tangent_sphere) && !sphere_intersects_recorded_sphere(e_ids_and_tangent_spheres_, tangent_sphere))
					{
						return std::make_pair(true, tangent_sphere);
					}
				}
			}
			return std::make_pair(false, SimpleSphere());
		}

		void set_d(const std::size_t d_id, const std::size_t d_number, const SimpleSphere& tangent_sphere)
		{
			if(can_have_d_ && d_number<2 && d_ids_and_tangent_spheres_[d_number].first!=d_id)
			{
				d_ids_and_tangent_spheres_[d_number]=std::make_pair(d_id, tangent_sphere);
			}
		}

		void set_d_with_d_number_selection(const std::size_t d_id, const SimpleSphere& tangent_sphere)
		{
			if(can_have_d_)
			{
				const int h0=halfspace_of_sphere(tangent_planes_[0].first, tangent_planes_[0].second, spheres_->at(d_id));
				const int h1=halfspace_of_sphere(tangent_planes_[1].first, tangent_planes_[1].second, spheres_->at(d_id));
				if(h0>=0 && h1==-1 && (halfspace_of_sphere(tangent_planes_[0].first, tangent_planes_[0].second, spheres_->at(d_id))>=0))
				{
					set_d(d_id, 0, tangent_sphere);
				}
				else if(h0==-1 && h1>=0 && (halfspace_of_sphere(tangent_planes_[1].first, tangent_planes_[1].second, spheres_->at(d_id))>=0))
				{
					set_d(d_id, 1, tangent_sphere);
				}
			}
		}

		void unset_d(const std::size_t d_number)
		{
			if(can_have_d_ && d_number<2)
			{
				d_ids_and_tangent_spheres_[d_number]=std::make_pair(npos, SimpleSphere());
			}
		}

		bool has_d(const std::size_t d_number) const
		{
			return (can_have_d_ && d_number<2 && d_ids_and_tangent_spheres_[d_number].first!=npos);
		}

		std::size_t get_d_id(const std::size_t d_number) const
		{
			return ((can_have_d_ && d_number<2) ? d_ids_and_tangent_spheres_[d_number].first : npos);
		}

		const SimpleSphere get_d_tangent_sphere(const std::size_t d_number) const
		{
			return ((can_have_d_ && d_number<2) ? d_ids_and_tangent_spheres_[d_number].second : SimpleSphere());
		}

		bool can_have_e() const
		{
			return can_have_e_;
		}

		void update_middle_region_approximation()
		{
			if(can_have_e_ && can_have_d_ && d_ids_and_tangent_spheres_[0].first!=npos && d_ids_and_tangent_spheres_[1].first!=npos)
			{
				const std::vector<SimpleSphere> disk0=TangentSphereOfThreeSpheres::calculate(
						SimpleSphere(SimplePoint(*a_sphere_)+(sub_of_points<SimplePoint>(d_ids_and_tangent_spheres_[0].second, *a_sphere_).unit()*a_sphere_->r), 0),
						SimpleSphere(SimplePoint(*b_sphere_)+(sub_of_points<SimplePoint>(d_ids_and_tangent_spheres_[0].second, *b_sphere_).unit()*b_sphere_->r), 0),
						SimpleSphere(SimplePoint(*c_sphere_)+(sub_of_points<SimplePoint>(d_ids_and_tangent_spheres_[0].second, *c_sphere_).unit()*c_sphere_->r), 0));
				if(disk0.size()==1)
				{
					const std::vector<SimpleSphere> disk1=TangentSphereOfThreeSpheres::calculate(
							SimpleSphere(SimplePoint(*a_sphere_)+(sub_of_points<SimplePoint>(d_ids_and_tangent_spheres_[1].second, *a_sphere_).unit()*a_sphere_->r), 0),
							SimpleSphere(SimplePoint(*b_sphere_)+(sub_of_points<SimplePoint>(d_ids_and_tangent_spheres_[1].second, *b_sphere_).unit()*b_sphere_->r), 0),
							SimpleSphere(SimplePoint(*c_sphere_)+(sub_of_points<SimplePoint>(d_ids_and_tangent_spheres_[1].second, *c_sphere_).unit()*c_sphere_->r), 0));
					if(disk1.size()==1)
					{
						const SimplePoint center(sum_of_points<SimplePoint>(disk0.front(), disk1.front())*0.5);
						middle_region_approximation_sphere_.first=true;
						middle_region_approximation_sphere_.second=SimpleSphere(center, std::max(distance_from_point_to_point(center, disk0.front())+disk0.front().r, distance_from_point_to_point(center, disk1.front())+disk1.front().r));
					}
				}
			}
		}

		template<typename InputSphereType>
		bool sphere_may_contain_candidate_for_e(const InputSphereType& input_sphere) const
		{
			return (
					can_have_e_
					&& (!can_have_d_ || !middle_region_approximation_sphere_.first || sphere_intersects_sphere(middle_region_approximation_sphere_.second, input_sphere))
					&& (!can_have_d_ || d_ids_and_tangent_spheres_[0].first==npos || d_ids_and_tangent_spheres_[1].first==npos
							|| (distance_from_point_to_line(input_sphere, d_ids_and_tangent_spheres_[0].second, d_ids_and_tangent_spheres_[1].second)<(input_sphere.r+std::max(d_ids_and_tangent_spheres_[0].second.r, d_ids_and_tangent_spheres_[1].second.r)))
							|| (can_have_negative_tangent_spheres_ && sphere_intersects_sphere(input_sphere, *a_sphere_) && sphere_intersects_sphere(input_sphere, *b_sphere_) && sphere_intersects_sphere(input_sphere, *c_sphere_)))
					&& (!can_have_d_ || (halfspace_of_sphere(tangent_planes_[0].first, tangent_planes_[0].second, input_sphere)<=0 && halfspace_of_sphere(tangent_planes_[1].first, tangent_planes_[1].second, input_sphere)<=0))
					);
		}

		std::vector<SimpleSphere> check_candidate_for_e(const std::size_t e_id) const
		{
			if(
					can_have_e_
					&& (e_id!=npos)
					&& (!abc_ids_.contains(e_id))
					&& (!can_have_d_ || (e_id!=d_ids_and_tangent_spheres_[0].first && e_id!=d_ids_and_tangent_spheres_[1].first))
					&& (!can_have_d_ || !middle_region_approximation_sphere_.first || sphere_intersects_sphere(middle_region_approximation_sphere_.second, spheres_->at(e_id)))
					&& (!can_have_d_ || (halfspace_of_sphere(tangent_planes_[0].first, tangent_planes_[0].second, spheres_->at(e_id))==-1 && halfspace_of_sphere(tangent_planes_[1].first, tangent_planes_[1].second, spheres_->at(e_id))==-1))
				)
			{
				const std::vector<SimpleSphere> tangent_spheres=TangentSphereOfFourSpheres::calculate((*a_sphere_), (*b_sphere_), (*c_sphere_), spheres_->at(e_id));
				std::vector<SimpleSphere> valid_tangent_spheres;
				for(std::size_t i=0;i<tangent_spheres.size();i++)
				{
					const SimpleSphere& tangent_sphere=tangent_spheres[i];
					if(!sphere_intersects_recorded_sphere(d_ids_and_tangent_spheres_, tangent_sphere) && !sphere_intersects_recorded_sphere(e_ids_and_tangent_spheres_, tangent_sphere))
					{
						valid_tangent_spheres.push_back(tangent_sphere);
					}
				}
				return valid_tangent_spheres;
			}
			return std::vector<SimpleSphere>();
		}

		void add_e(const std::size_t e_id, const SimpleSphere& tangent_sphere)
		{
			if(can_have_e_ && e_id!=npos)
			{
				e_ids_and_tangent_spheres_.push_back(std::make_pair(e_id, tangent_sphere));
			}
		}

		void unset_e()
		{
			e_ids_and_tangent_spheres_.clear();
		}

		bool has_e() const
		{
			return (!e_ids_and_tangent_spheres_.empty());
		}

		const std::vector< std::pair<std::size_t, SimpleSphere> >& get_e_ids_and_tangent_spheres() const
		{
			return e_ids_and_tangent_spheres_;
		}

		std::vector< std::pair<Quadruple, SimpleSphere> > produce_quadruples(const bool with_d0, const bool with_d1, const bool with_e) const
		{
			std::vector< std::pair<Quadruple, SimpleSphere> > quadruples_with_tangent_spheres;
			const std::vector< std::pair<std::size_t, SimpleSphere> > recorded_ids_and_tangent_spheres=collect_all_recorded_ids_and_tangent_spheres(with_d0, with_d1, with_e);
			for(std::size_t i=0;i<recorded_ids_and_tangent_spheres.size();i++)
			{
				quadruples_with_tangent_spheres.push_back(std::make_pair(Quadruple(abc_ids_, recorded_ids_and_tangent_spheres[i].first), recorded_ids_and_tangent_spheres[i].second));
			}
			return quadruples_with_tangent_spheres;
		}

		std::vector< std::pair<Triple, std::pair<std::size_t, SimpleSphere> > > produce_prefaces(const bool with_d0, const bool with_d1, const bool with_e) const
		{
			std::vector< std::pair<Triple, std::pair<std::size_t, SimpleSphere> > > produced_prefaces;
			const std::vector< std::pair<std::size_t, SimpleSphere> > recorded_ids_and_tangent_spheres=collect_all_recorded_ids_and_tangent_spheres(with_d0, with_d1, with_e);
			for(int j=0;j<3;j++)
			{
				for(std::size_t i=0;i<recorded_ids_and_tangent_spheres.size();i++)
				{
					produced_prefaces.push_back(std::make_pair(Triple(abc_ids_.exclude(j), recorded_ids_and_tangent_spheres[i].first), std::make_pair(abc_ids_.get(j), recorded_ids_and_tangent_spheres[i].second)));
				}
			}
			return produced_prefaces;
		}

	private:
		void init_central_planes()
		{
			central_planes_.clear();
			if(can_have_d_)
			{
				central_planes_.resize(2);
				const SimplePoint centeral_plane_normal=plane_normal_from_three_points<SimplePoint>((*a_sphere_), (*b_sphere_), (*c_sphere_));
				const bool consistent_orientation=(halfspace_of_point((*a_sphere_), centeral_plane_normal, (tangent_planes_[0].first+tangent_planes_[0].second))==1);
				central_planes_[0]=std::make_pair(SimplePoint(*a_sphere_), consistent_orientation ? centeral_plane_normal : centeral_plane_normal.inverted());
				central_planes_[1]=std::make_pair(SimplePoint(*a_sphere_), consistent_orientation ? centeral_plane_normal.inverted() : centeral_plane_normal);
			}
		}

		void init_middle_region_approximation()
		{
			can_have_negative_tangent_spheres_=(sphere_intersects_sphere(*a_sphere_, *b_sphere_) && sphere_intersects_sphere(*a_sphere_, *c_sphere_) && sphere_intersects_sphere(*b_sphere_, *c_sphere_));
			middle_region_approximation_sphere_.first=false;
			if(can_have_e_ && can_have_d_)
			{
				const std::vector<SimpleSphere> disk0=TangentSphereOfThreeSpheres::calculate(
						SimpleSphere(SimplePoint(*a_sphere_)+(tangent_planes_[0].second*(a_sphere_->r)), 0),
						SimpleSphere(SimplePoint(*b_sphere_)+(tangent_planes_[0].second*(b_sphere_->r)), 0),
						SimpleSphere(SimplePoint(*c_sphere_)+(tangent_planes_[0].second*(c_sphere_->r)), 0));
				if(disk0.size()==1)
				{
					const std::vector<SimpleSphere> disk1=TangentSphereOfThreeSpheres::calculate(
							SimpleSphere(SimplePoint(*a_sphere_)+(tangent_planes_[1].second*(a_sphere_->r)), 0),
							SimpleSphere(SimplePoint(*b_sphere_)+(tangent_planes_[1].second*(b_sphere_->r)), 0),
							SimpleSphere(SimplePoint(*c_sphere_)+(tangent_planes_[1].second*(c_sphere_->r)), 0));
					if(disk1.size()==1)
					{
						const SimplePoint center(sum_of_points<SimplePoint>(disk0.front(), disk1.front())*0.5);
						middle_region_approximation_sphere_.first=true;
						middle_region_approximation_sphere_.second=SimpleSphere(center, std::max(distance_from_point_to_point(center, disk0.front())+disk0.front().r, distance_from_point_to_point(center, disk1.front())+disk1.front().r));
					}
				}
			}
		}

		std::vector< std::pair<std::size_t, SimpleSphere> > collect_all_recorded_ids_and_tangent_spheres(const bool with_d0, const bool with_d1, const bool with_e) const
		{
			std::vector< std::pair<std::size_t, SimpleSphere> > recorded_ids_and_tangent_spheres;
			recorded_ids_and_tangent_spheres.reserve(d_ids_and_tangent_spheres_.size()+e_ids_and_tangent_spheres_.size());
			if(can_have_d_ && with_d0 && d_ids_and_tangent_spheres_[0].first!=npos)
			{
				recorded_ids_and_tangent_spheres.push_back(d_ids_and_tangent_spheres_[0]);
			}
			if(can_have_d_ && with_d1 && d_ids_and_tangent_spheres_[1].first!=npos)
			{
				recorded_ids_and_tangent_spheres.push_back(d_ids_and_tangent_spheres_[1]);
			}
			if(can_have_e_ && with_e && !e_ids_and_tangent_spheres_.empty())
			{
				recorded_ids_and_tangent_spheres.insert(recorded_ids_and_tangent_spheres.end(), e_ids_and_tangent_spheres_.begin(), e_ids_and_tangent_spheres_.end());
			}
			return recorded_ids_and_tangent_spheres;
		}

		template<typename InputSphereType>
		bool sphere_intersects_recorded_sphere(const std::vector< std::pair<std::size_t, SimpleSphere> >& recorded_ids_and_tangent_spheres, const InputSphereType& input_sphere) const
		{
			for(std::size_t i=0;i<recorded_ids_and_tangent_spheres.size();i++)
			{
				if(recorded_ids_and_tangent_spheres[i].first!=npos && sphere_intersects_sphere(input_sphere, spheres_->at(recorded_ids_and_tangent_spheres[i].first)))
				{
					return true;
				}
			}
			return false;
		}

		const std::vector<SimpleSphere>* spheres_;
		Triple abc_ids_;
		const SimpleSphere* a_sphere_;
		const SimpleSphere* b_sphere_;
		const SimpleSphere* c_sphere_;
		std::vector< std::pair<SimplePoint, SimplePoint> > tangent_planes_;
		std::vector< std::pair<std::size_t, SimpleSphere> > d_ids_and_tangent_spheres_;
		std::vector< std::pair<std::size_t, SimpleSphere> > e_ids_and_tangent_spheres_;
		bool can_have_d_;
		bool can_have_e_;
		std::vector< std::pair<SimplePoint, SimplePoint> > central_planes_;
		bool can_have_negative_tangent_spheres_;
		std::pair<bool, SimpleSphere> middle_region_approximation_sphere_;
	};

	class SearchForAnyDOfFace
	{
	public:
		static bool find_any_d(const BoundingSpheresHierarchy& bsh, Face& face, const std::size_t d_number)
		{
			if(!face.has_d(d_number))
			{
				NodeCheckerForAnyD node_checker(face, d_number);
				LeafCheckerForAnyD leaf_checker(face, d_number);
				node_checker.constrain();
				bsh.search(node_checker, leaf_checker);
				if(node_checker.constrained && !face.has_d(d_number))
				{
					node_checker.unconstrain();
					bsh.search(node_checker, leaf_checker);
				}
				return face.has_d(d_number);
			}
			return false;
		}

	private:
		struct NodeCheckerForAnyD
		{
			const Face& face;
			const std::size_t d_number;
			bool constrained;
			SimpleSphere constraint_sphere;

			NodeCheckerForAnyD(const Face& target, const std::size_t d_number) : face(target), d_number(d_number), constrained(false)
			{
			}

			bool constrain()
			{
				if(face.has_d(d_number==0 ? 1 : 0))
				{
					constraint_sphere=face.get_d_tangent_sphere(d_number==0 ? 1 : 0);
					constrained=true;
				}
				else
				{
					constrained=false;
				}
				return constrained;
			}

			void unconstrain()
			{
				constrained=false;
			}

			bool operator()(const SimpleSphere& sphere) const
			{
				return (!constrained || sphere_intersects_sphere(constraint_sphere, sphere)) && face.sphere_may_contain_candidate_for_d(sphere, d_number);
			}
		};

		struct LeafCheckerForAnyD
		{
			Face& face;
			const std::size_t d_number;

			LeafCheckerForAnyD(Face& target, const std::size_t d_number) : face(target), d_number(d_number)
			{
			}

			std::pair<bool, bool> operator()(const std::size_t id, const SimpleSphere&)
			{
				const std::pair<bool, SimpleSphere> check_result=face.check_candidate_for_d(id, d_number);
				if(check_result.first)
				{
					face.set_d(id, d_number, check_result.second);
					return std::make_pair(true, true);
				}
				return std::make_pair(false, false);
			}
		};
	};

	class SearchForValidDOfFace
	{
	public:
		static bool find_valid_d(const BoundingSpheresHierarchy& bsh, Face& face, const std::size_t d_number)
		{
			if(face.has_d(d_number))
			{
				NodeCheckerForValidD node_checker(face, d_number);
				LeafCheckerForValidD leaf_checker(face, d_number);
				while(face.has_d(d_number))
				{
					const std::vector<std::size_t> results=bsh.search(node_checker, leaf_checker);
					if(results.empty())
					{
						return true;
					}
					else if(face.get_d_id(d_number)!=results.back())
					{
						face.unset_d(d_number);
					}
				}
			}
			return false;
		}

	private:
		struct NodeCheckerForValidD
		{
			const Face& face;
			const std::size_t d_number;

			NodeCheckerForValidD(const Face& target, const std::size_t d_number) : face(target), d_number(d_number)
			{
			}

			bool operator()(const SimpleSphere& sphere) const
			{
				return (face.has_d(d_number) && sphere_intersects_sphere(sphere, face.get_d_tangent_sphere(d_number)));
			}
		};

		struct LeafCheckerForValidD
		{
#if USE_TR1 > 0
typedef std::tr1::unordered_set<std::size_t> SafetyMonitor;
#else
typedef std::unordered_set<std::size_t> SafetyMonitor;
#endif

			Face& face;
			const std::size_t d_number;
			SafetyMonitor safety_monitor;

			LeafCheckerForValidD(Face& target, const std::size_t d_number) : face(target), d_number(d_number)
			{
			}

			std::pair<bool, bool> operator()(const std::size_t id, const SimpleSphere& sphere)
			{
				if(face.has_d(d_number) && sphere_intersects_sphere(sphere, face.get_d_tangent_sphere(d_number)))
				{
					const std::pair<bool, SimpleSphere> check_result=face.check_candidate_for_d(id, d_number);
					if(check_result.first && safety_monitor.count(id)==0)
					{
						face.set_d(id, d_number, check_result.second);
						safety_monitor.insert(id);
						return std::make_pair(true, true);
					}
					else
					{
						return std::make_pair(true, false);
					}
				}
				return std::make_pair(false, false);
			}
		};
	};

	class SearchForValidEOfFace
	{
	public:
		static bool find_valid_e(const BoundingSpheresHierarchy& bsh, Face& face)
		{
			face.update_middle_region_approximation();
			NodeCheckerForValidE node_checker(face);
			LeafCheckerForValidE leaf_checker(face, bsh);
			return !bsh.search(node_checker, leaf_checker).empty();
		}

	private:
		struct NodeCheckerForValidE
		{
			const Face& face;

			explicit NodeCheckerForValidE(const Face& target) : face(target)
			{
			}

			bool operator()(const SimpleSphere& sphere) const
			{
				return face.sphere_may_contain_candidate_for_e(sphere);
			}
		};

		struct LeafCheckerForValidE
		{
			Face& face;
			const BoundingSpheresHierarchy& bsh;

			LeafCheckerForValidE(Face& target, const BoundingSpheresHierarchy& bsh) : face(target), bsh(bsh)
			{
			}

			std::pair<bool, bool> operator()(const std::size_t id, const SimpleSphere&)
			{
				const std::vector<SimpleSphere> check_result=face.check_candidate_for_e(id);
				bool e_added=false;
				for(std::size_t i=0;i<check_result.size();i++)
				{
					if(SearchForSphericalCollisions::find_any_collision(bsh, check_result[i]).empty())
					{
						face.add_e(id, check_result[i]);
						e_added=true;
					}
				}
				return std::make_pair(e_added, false);
			}
		};
	};

	static std::size_t select_starting_sphere_for_finding_first_valid_faces(const BoundingSpheresHierarchy& bsh, const std::vector<int>& admittance)
	{
		const std::vector<SimpleSphere>& spheres=bsh.leaves_spheres();
		if(spheres.empty() || admittance.size()!=spheres.size())
		{
			return 0;
		}
		SimplePoint center(0.0, 0.0, 0.0);
		int count=0;
		for(std::size_t i=0;i<spheres.size();i++)
		{
			if(admittance[i]>0)
			{
				center=center+SimplePoint(spheres[i]);
				count++;
			}
		}
		if(count==0)
		{
			return 0;
		}
		center=center*(1/static_cast<double>(count));
		std::size_t result=0;
		double result_distance=std::numeric_limits<double>::max();
		for(std::size_t i=0;i<spheres.size();i++)
		{
			if(admittance[i]>0)
			{
				const double i_distance=distance_from_point_to_point(center, spheres[i]);
				if(i_distance<result_distance)
				{
					result=i;
					result_distance=i_distance;
				}
			}
		}
		return result;
	}

	static std::vector<Face> find_first_valid_faces(
			const BoundingSpheresHierarchy& bsh,
			const std::vector<int>& admittance,
			const std::size_t starting_sphere_id,
			std::size_t& iterations_count,
			const bool fix_starting_sphere_id,
			const bool allow_quadruples_with_two_tangent_spheres,
			const std::size_t max_size_of_traversal=std::numeric_limits<std::size_t>::max())
	{
		const std::vector<SimpleSphere>& spheres=bsh.leaves_spheres();
		std::vector<Face> result;
		if(spheres.size()>=4 && starting_sphere_id<spheres.size() && admittance.size()==spheres.size())
		{
			const std::vector<std::size_t> traversal=BoundingSpheresHierarchy::sort_objects_by_distance_to_one_of_them(spheres, starting_sphere_id, minimal_distance_from_sphere_to_sphere<SimpleSphere, SimpleSphere>);
			for(std::size_t d=3;d<std::min(traversal.size(), max_size_of_traversal);d++)
			{
				for(std::size_t a=0;a<(fix_starting_sphere_id ? 1 : d);a++)
				{
					for(std::size_t b=a+1;b<d;b++)
					{
						for(std::size_t c=b+1;c<d;c++)
						{
							iterations_count++;
							if(admittance[traversal[a]]>0 || admittance[traversal[b]]>0 || admittance[traversal[c]]>0)
							{
								const Triple triple(traversal[a], traversal[b], traversal[c]);
								const Quadruple quadruple(triple, traversal[d]);
								const std::vector<SimpleSphere> tangents=TangentSphereOfFourSpheres::calculate(spheres[quadruple.get(0)], spheres[quadruple.get(1)], spheres[quadruple.get(2)], spheres[quadruple.get(3)]);
								if(
										(tangents.size()==1 && SearchForSphericalCollisions::find_any_collision(bsh, tangents.front()).empty())
										|| (allow_quadruples_with_two_tangent_spheres && tangents.size()==2 && (SearchForSphericalCollisions::find_any_collision(bsh, tangents.front()).empty() || SearchForSphericalCollisions::find_any_collision(bsh, tangents.back()).empty()))
									)
								{
									result.push_back(Face(bsh.leaves_spheres(), triple, bsh.min_input_radius()));
									return result;
								}
							}
						}
					}
				}
			}
		}
		return result;
	}

	static std::pair<bool, bool> augment_quadruples_map(const Quadruple& quadruple, const SimpleSphere& quadruple_tangent_sphere, QuadruplesMap& quadruples_map)
	{
		bool quadruple_added=false;
		bool quadruple_tangent_sphere_added=false;
		QuadruplesMap::iterator qm_it=quadruples_map.find(quadruple);
		if(qm_it==quadruples_map.end())
		{
			quadruples_map[quadruple].push_back(quadruple_tangent_sphere);
			quadruple_added=true;
			quadruple_tangent_sphere_added=true;
		}
		else
		{
			std::vector<SimpleSphere>& quadruple_tangent_spheres_list=qm_it->second;
			if(quadruple_tangent_spheres_list.size()==1 && !spheres_equal(quadruple_tangent_spheres_list.front(), quadruple_tangent_sphere, tangent_spheres_equality_epsilon()))
			{
				quadruple_tangent_spheres_list.push_back(quadruple_tangent_sphere);
				quadruple_tangent_sphere_added=true;
			}
		}
		return std::make_pair(quadruple_added, quadruple_tangent_sphere_added);
	}

	static QuadruplesSearchLog find_valid_quadruples(const BoundingSpheresHierarchy& bsh, const std::vector<int>& admittance, QuadruplesMap& quadruples_map)
	{
#if USE_TR1 > 0
typedef std::tr1::unordered_map<Triple, std::size_t, Triple::HashFunctor> TriplesMap;
typedef std::tr1::unordered_set<Triple, Triple::HashFunctor> TriplesSet;
#else
typedef std::unordered_map<Triple, std::size_t, Triple::HashFunctor> TriplesMap;
typedef std::unordered_set<Triple, Triple::HashFunctor> TriplesSet;
#endif

		QuadruplesSearchLog log=QuadruplesSearchLog();

		std::vector<Face> stack=find_first_valid_faces(bsh, admittance, select_starting_sphere_for_finding_first_valid_faces(bsh, admittance), log.performed_iterations_for_finding_first_faces, false, true);
		TriplesSet processed_triples_set;
		std::vector<int> spheres_usage_mapping(bsh.leaves_spheres().size(), 0);
		std::set<std::size_t> ignorable_spheres_ids;

		do
		{
			TriplesMap stack_map;
			for(std::size_t i=0;i<stack.size();i++)
			{
				stack_map[stack[i].abc_ids()]=i;
			}
			while(!stack.empty())
			{
				Face face=stack.back();
				stack.pop_back();
				stack_map.erase(face.abc_ids());
				processed_triples_set.insert(face.abc_ids());
				log.processed_faces++;
				if(!face.can_have_d())
				{
					log.encountered_difficult_faces++;
				}
				const bool found_d0=face.can_have_d() && !face.has_d(0) && SearchForAnyDOfFace::find_any_d(bsh, face, 0) && SearchForValidDOfFace::find_valid_d(bsh, face, 0);
				const bool found_d1=face.can_have_d() && !face.has_d(1) && SearchForAnyDOfFace::find_any_d(bsh, face, 1) && SearchForValidDOfFace::find_valid_d(bsh, face, 1);
				const bool found_e=face.can_have_e() && SearchForValidEOfFace::find_valid_e(bsh, face);
				if(found_d0 || found_d1 || found_e)
				{
					{
						const std::vector< std::pair<Quadruple, SimpleSphere> > additional_quadruples=face.produce_quadruples(found_d0, found_d1, found_e);
						for(std::size_t i=0;i<additional_quadruples.size();i++)
						{
							const std::pair<bool, bool> augmention_status=augment_quadruples_map(additional_quadruples[i].first, additional_quadruples[i].second, quadruples_map);
							log.added_quadruples+=(augmention_status.first ? 1 : 0);
							log.added_tangent_spheres+=(augmention_status.second ? 1 : 0);
						}
					}
					{
						const std::vector< std::pair<Triple, std::pair<std::size_t, SimpleSphere> > > produced_prefaces=face.produce_prefaces(found_d0, found_d1, found_e);
						for(std::size_t i=0;i<produced_prefaces.size();i++)
						{
							const std::pair<Triple, std::pair<std::size_t, SimpleSphere> >& produced_preface=produced_prefaces[i];
							if(admittance[produced_preface.first.get(0)]>0 || admittance[produced_preface.first.get(1)]>0 || admittance[produced_preface.first.get(2)]>0)
							{
								if(processed_triples_set.count(produced_preface.first)==0)
								{
									TriplesMap::const_iterator sm_it=stack_map.find(produced_preface.first);
									if(sm_it==stack_map.end())
									{
										stack_map[produced_preface.first]=stack.size();
										stack.push_back(Face(bsh.leaves_spheres(), produced_preface.first, bsh.min_input_radius()));
										stack.back().set_d_with_d_number_selection(produced_preface.second.first, produced_preface.second.second);
										log.produced_faces++;
									}
									else
									{
										stack.at(sm_it->second).set_d_with_d_number_selection(produced_preface.second.first, produced_preface.second.second);
										log.updated_faces++;
									}
								}
								else
								{
									log.encountered_triples_repetitions++;
								}
							}
						}
					}
				}
				if(face.has_d(0) || face.has_d(1) || face.has_e())
				{
					for(int j=0;j<3;j++)
					{
						if(face.abc_ids().get(j)<spheres_usage_mapping.size())
						{
							spheres_usage_mapping[face.abc_ids().get(j)]=1;
						}
					}
				}
			}
			for(std::size_t i=0;i<spheres_usage_mapping.size() && stack.empty();i++)
			{
				if(spheres_usage_mapping[i]==0 && admittance[i]>0 && ignorable_spheres_ids.count(i)==0)
				{
					stack=find_first_valid_faces(bsh, admittance, i, log.performed_iterations_for_finding_first_faces, true, true, 25);
					ignorable_spheres_ids.insert(i);
				}
			}
		}
		while(!stack.empty());

		return log;
	}

	static SurplusQuadruplesSearchLog find_surplus_valid_quadruples(const BoundingSpheresHierarchy& bsh, QuadruplesMap& quadruples_map)
	{
		SurplusQuadruplesSearchLog log=SurplusQuadruplesSearchLog();
		std::vector< std::pair<Quadruple, SimpleSphere> > surplus_candidates;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const std::vector<SimpleSphere>& tangent_spheres=it->second;
			for(std::size_t i=0;i<tangent_spheres.size();i++)
			{
				const SimpleSphere& tangent_sphere=tangent_spheres[i];
				const SimpleSphere expanded_tangent_sphere(tangent_sphere, tangent_sphere.r+tangent_spheres_equality_epsilon());
				const std::vector<std::size_t> expanded_collisions=SearchForSphericalCollisions::find_all_collisions(bsh, expanded_tangent_sphere);
				std::vector<std::size_t> refined_collisions;
				for(std::size_t j=0;j<expanded_collisions.size();j++)
				{
					if(sphere_touches_sphere(tangent_sphere, bsh.leaves_spheres().at(expanded_collisions[j])))
					{
						refined_collisions.push_back(expanded_collisions[j]);
					}
				}
				if(refined_collisions.size()>4)
				{
					for(std::size_t a=0;a<refined_collisions.size();a++)
					{
						for(std::size_t b=a+1;b<refined_collisions.size();b++)
						{
							for(std::size_t c=b+1;c<refined_collisions.size();c++)
							{
								for(std::size_t d=c+1;d<refined_collisions.size();d++)
								{
									surplus_candidates.push_back(std::make_pair(Quadruple(refined_collisions[a], refined_collisions[b], refined_collisions[c], refined_collisions[d]), tangent_sphere));
								}
							}
						}
					}
				}
			}
		}
		for(std::size_t i=0;i<surplus_candidates.size();i++)
		{
			const std::pair<bool, bool> augmention_status=augment_quadruples_map(surplus_candidates[i].first, surplus_candidates[i].second, quadruples_map);
			log.surplus_quadruples+=(augmention_status.first ? 1 : 0);
			log.surplus_tangent_spheres+=(augmention_status.second ? 1 : 0);
		}
		return log;
	}

	static QuadruplesMap renumber_quadruples_map(const QuadruplesMap& quadruples_map, const std::vector<std::size_t>& mapping)
	{
		QuadruplesMap renumbered_quadruples_map;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const Quadruple& q=it->first;
			if(q.get(3)<mapping.size())
			{
				const Quadruple mq(mapping[q.get(0)], mapping[q.get(1)], mapping[q.get(2)], mapping[q.get(3)]);
				if(mq.get(3)!=npos)
				{
					renumbered_quadruples_map[mq]=it->second;
				}
			}
		}
		return renumbered_quadruples_map;
	}

	static std::set<std::size_t> collect_ignored_spheres_ids(const std::vector<int>& admittance, const QuadruplesMap& quadruples_map)
	{
		std::set<std::size_t> ignored_spheres_ids;
		std::vector<int> spheres_inclusion_map(admittance.size(), 0);
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const Quadruple& q=it->first;
			for(int i=0;i<4;i++)
			{
				if(q.get(i)<spheres_inclusion_map.size())
				{
					spheres_inclusion_map[q.get(i)]=1;
				}
			}
		}
		for(std::size_t i=0;i<spheres_inclusion_map.size();i++)
		{
			if(spheres_inclusion_map[i]==0 && admittance[i]>0)
			{
				ignored_spheres_ids.insert(i);
			}
		}
		return ignored_spheres_ids;
	}

	inline static double tangent_spheres_equality_epsilon()
	{
		return std::max(default_comparison_epsilon(), 0.001);
	}

	static QuadruplesMap collect_invalid_tangent_spheres_of_valid_quadruples(const std::vector<SimpleSphere>& spheres, const QuadruplesMap& quadruples_map)
	{
		QuadruplesMap result;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const Quadruple& q=it->first;
			const std::vector<SimpleSphere>& valid_tangent_spheres=it->second;
			if(valid_tangent_spheres.size()==1 && q.get(0)<spheres.size() && q.get(1)<spheres.size() && q.get(2)<spheres.size() && q.get(3)<spheres.size())
			{
				const std::vector<SimpleSphere> all_tangent_spheres=TangentSphereOfFourSpheres::calculate(spheres[q.get(0)], spheres[q.get(1)], spheres[q.get(2)], spheres[q.get(3)]);
				if(all_tangent_spheres.size()==2)
				{
					if(!spheres_equal(valid_tangent_spheres.front(), all_tangent_spheres[0], tangent_spheres_equality_epsilon()))
					{
						result[q].push_back(all_tangent_spheres[0]);
					}
					else if(!spheres_equal(valid_tangent_spheres.front(), all_tangent_spheres[1], tangent_spheres_equality_epsilon()))
					{
						result[q].push_back(all_tangent_spheres[1]);
					}
				}
			}
		}
		return result;
	}

	static void update_vertices_graph(
			const std::vector<SimpleSphere>& spheres,
			const VerticesVector& all_vertices_vector,
			const Triple& triple,
			const std::size_t vi,
			const std::size_t vj,
			const bool lonely_pair,
			VerticesGraph& vertices_graph)
	{
		if(vi<vertices_graph.size() && vj<vertices_graph.size())
		{
			const Quadruple& qi=all_vertices_vector[vi].first;
			const Quadruple& qj=all_vertices_vector[vj].first;
			if(qi==qj)
			{
				const int number_of_subtriple=qi.number_of_subtuple(triple);
				if(number_of_subtriple>=0)
				{
					const std::size_t common_sphere_id=qi.get(number_of_subtriple);
					if(common_sphere_id<spheres.size())
					{
						const std::vector< std::pair<SimplePoint, SimplePoint> > tangent_planes=TangentPlaneOfThreeSpheres::calculate(spheres[triple.get(0)], spheres[triple.get(1)], spheres[triple.get(2)]);
						if(tangent_planes.size()!=2 ? lonely_pair : (halfspace_of_sphere(tangent_planes[0].first, tangent_planes[0].second, spheres[common_sphere_id])>=0 && halfspace_of_sphere(tangent_planes[1].first, tangent_planes[1].second, spheres[common_sphere_id])>=0))
						{
							vertices_graph[vi][number_of_subtriple]=vj;
							vertices_graph[vj][number_of_subtriple]=vi;
						}
					}
				}
			}
			else
			{
				const int number_of_subtriple_for_vi=qi.number_of_subtuple(triple);
				const int number_of_subtriple_for_vj=qj.number_of_subtuple(triple);
				if(number_of_subtriple_for_vi>=0 && number_of_subtriple_for_vj>=0)
				{
					vertices_graph[vi][number_of_subtriple_for_vi]=vj;
					vertices_graph[vj][number_of_subtriple_for_vj]=vi;
				}
			}
		}
	}


	static void update_vertices_graph(
			const std::vector<SimpleSphere>& spheres,
			const VerticesVector& all_vertices_vector,
			const Triple& triple,
			const std::vector<std::size_t>& triple_vertices_ids,
			VerticesGraph& vertices_graph)
	{
		const std::vector<std::size_t>& v=triple_vertices_ids;
		if(v.size()>1)
		{
			if(v.size()==2)
			{
				update_vertices_graph(spheres, all_vertices_vector, triple, v[0], v[1], true, vertices_graph);
			}
			else
			{
				const double r0=spheres[triple.get(0)].r;
				const double r1=spheres[triple.get(1)].r;
				const double r2=spheres[triple.get(2)].r;
				const SimpleSphere& o=spheres[triple.get((r0<=r1 && r0<=r2) ? 0 : ((r1<=r0 && r1<=r2) ? 1 : 2))];
				for(std::size_t i=0;i<v.size();i++)
				{
					const std::size_t vi=v[i];
					if(vi<vertices_graph.size())
					{
						std::vector< std::pair<double, std::size_t> > v_candidates;
						v_candidates.reserve(v.size()-1);
						for(std::size_t j=0;j<v.size();j++)
						{
							const std::size_t vj=v[j];
							if(vj!=vi)
							{
								const double angle=min_angle(o, all_vertices_vector[vi].second, all_vertices_vector[vj].second);
								v_candidates.push_back(std::make_pair(angle, vj));
							}
						}
						if(!v_candidates.empty())
						{
							std::sort(v_candidates.begin(), v_candidates.end());
							std::vector<std::size_t> v_sel(1, v_candidates[0].second);
							for(std::size_t j=1;j<v_candidates.size() && v_sel.size()==1;j++)
							{
								const std::size_t vj=v_candidates[j].second;
								const double angle_with_vi=min_angle(o, all_vertices_vector[vi].second, all_vertices_vector[vj].second);
								const double angle_with_v_sel0=min_angle(o, all_vertices_vector[v_sel[0]].second, all_vertices_vector[vj].second);
								if(angle_with_vi<angle_with_v_sel0)
								{
									v_sel.push_back(vj);
								}
							}
							for(std::size_t j=0;j<v_sel.size();j++)
							{
								update_vertices_graph(spheres, all_vertices_vector, triple, vi, v_sel[j], false, vertices_graph);
							}
						}
					}
				}
			}
		}
	}
};

}

}

#endif /* APOLLOTA_TRIANGULATION_H_ */
