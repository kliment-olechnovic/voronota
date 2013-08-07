#ifndef APOLLOTA_TRIANGULATION_H_
#define APOLLOTA_TRIANGULATION_H_

#include <iostream>
#include <sstream>
#include <limits>
#include <vector>
#include <map>
#include <memory>

#include <tr1/unordered_map>
#include <tr1/unordered_set>

#include "tuple.h"
#include "bounding_spheres_hierarchy.h"
#include "search_for_spherical_collisions.h"
#include "tangent_plane_of_three_spheres.h"
#include "tangent_sphere_of_four_spheres.h"
#include "tangent_sphere_of_three_spheres.h"

namespace apollota
{

static const std::size_t npos=static_cast<std::size_t>(-1);

class Triangulation
{
public:
	typedef std::tr1::unordered_map<Quadruple, std::vector<SimpleSphere>, Quadruple::HashFunctor> QuadruplesMap;
	typedef std::map<QuadruplesMap::key_type, QuadruplesMap::mapped_type> QuadruplesOrderedMap;
	typedef std::tr1::unordered_map<std::size_t, std::tr1::unordered_set<std::size_t> > NeighborsMap;
	typedef std::vector< std::vector<std::size_t> > NeighborsGraph;
	typedef std::tr1::unordered_map<Pair, std::tr1::unordered_set<std::size_t>, Pair::HashFunctor> PairsNeighborsMap;
	typedef std::tr1::unordered_map<Triple, std::tr1::unordered_set<std::size_t>, Triple::HashFunctor> TriplesNeighborsMap;

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
		std::set<std::size_t> hidden_spheres_ids;
		std::set<std::size_t> ignored_spheres_ids;

		Result() : quadruples_search_log(), surplus_quadruples_search_log()
		{
		}

		void print_status(std::ostream& output) const
		{
			output << "quadruples " << quadruples_map.size() << "\n";
			output << "tangent_spheres " << count_tangent_spheres_in_quadruples_map(quadruples_map) << "\n";
			output << "processed_faces " << quadruples_search_log.processed_faces << "\n";
			output << "difficult_faces " << quadruples_search_log.encountered_difficult_faces << "\n";
			output << "first_iterations " << quadruples_search_log.performed_iterations_for_finding_first_faces << "\n";
			output << "surplus_tangent_spheres " << surplus_quadruples_search_log.surplus_tangent_spheres << "\n";
			output << "hidden_balls " << hidden_spheres_ids.size() << "\n";
			output << "ignored_balls " << ignored_spheres_ids.size() << "\n";
			output << "epsilon " << comparison_epsilon() << "\n";
		}
	};

	template<typename SphereType>
	static Result construct_result(
			const std::vector<SphereType>& spheres,
			const double initial_radius_for_spheres_bucketing,
			const bool exclude_hidden_spheres,
			const bool include_surplus_valid_quadruples)
	{
		Result result;

		{
			std::auto_ptr< BoundingSpheresHierarchy<SphereType> > bsh(new BoundingSpheresHierarchy<SphereType>(spheres, initial_radius_for_spheres_bucketing, 1));

			std::vector<std::size_t> refined_spheres_forward_mapping;
			std::vector<SphereType> refined_spheres;
			std::vector<std::size_t> refined_spheres_backward_mapping;
			if(exclude_hidden_spheres)
			{
				result.hidden_spheres_ids=SearchForSphericalCollisions::find_all_hidden_spheres(*bsh);
				if(!result.hidden_spheres_ids.empty())
				{
					refined_spheres_forward_mapping.resize(spheres.size(), npos);
					const std::size_t refined_spheres_count=spheres.size()-result.hidden_spheres_ids.size();
					refined_spheres.reserve(refined_spheres_count);
					refined_spheres_backward_mapping.reserve(refined_spheres_count);
					for(std::size_t i=0;i<spheres.size();i++)
					{
						if(result.hidden_spheres_ids.count(i)==0)
						{
							refined_spheres_forward_mapping[i]=refined_spheres.size();
							refined_spheres.push_back(spheres[i]);
							refined_spheres_backward_mapping.push_back(i);
						}
					}
					bsh.reset(new BoundingSpheresHierarchy<SphereType>(refined_spheres, initial_radius_for_spheres_bucketing, 1));
				}
			}

			result.quadruples_search_log=find_valid_quadruples(*bsh, result.quadruples_map);
			if(include_surplus_valid_quadruples)
			{
				result.surplus_quadruples_search_log=find_surplus_valid_quadruples(*bsh, result.quadruples_map);
			}

			if(!refined_spheres_backward_mapping.empty())
			{
				result.quadruples_map=renumber_quadruples_map(result.quadruples_map, refined_spheres_backward_mapping);
			}
		}

		{
			std::vector<int> spheres_inclusion_map(spheres.size(), 0);
			for(QuadruplesMap::const_iterator it=result.quadruples_map.begin();it!=result.quadruples_map.end();++it)
			{
				const Quadruple& q=it->first;
				for(int i=0;i<4;i++)
				{
					spheres_inclusion_map[q.get(i)]=1;
				}
			}
			for(std::size_t i=0;i<spheres_inclusion_map.size();i++)
			{
				if(spheres_inclusion_map[i]==0 && result.hidden_spheres_ids.count(i)==0)
				{
					result.ignored_spheres_ids.insert(i);
				}
			}
		}

		return result;
	}

	static QuadruplesOrderedMap collect_ordered_map_of_quadruples(const QuadruplesMap& quadruples_map)
	{
		QuadruplesOrderedMap quadruples_ordered_map;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			quadruples_ordered_map.insert(*it);
		}
		return quadruples_ordered_map;
	}

	static NeighborsMap collect_neighbors_map_from_quadruples_map(const QuadruplesMap& quadruples_map)
	{
		NeighborsMap neighbors_map;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const Quadruple& quadruple=it->first;
			for(int a=0;a<4;a++)
			{
				for(int b=a+1;b<4;b++)
				{
					neighbors_map[quadruple.get(a)].insert(quadruple.get(b));
					neighbors_map[quadruple.get(b)].insert(quadruple.get(a));
				}
			}
		}
		return neighbors_map;
	}

	static NeighborsGraph collect_neighbors_graph_from_neighbors_map(const NeighborsMap& neighbors_map, const std::size_t number_of_vertices)
	{
		NeighborsGraph neighbors_graph(number_of_vertices);
		for(NeighborsMap::const_iterator it=neighbors_map.begin();it!=neighbors_map.end();++it)
		{
			if((it->first)<neighbors_graph.size())
			{
				neighbors_graph[it->first].insert(neighbors_graph[it->first].end(), it->second.begin(), it->second.end());
			}
		}
		return neighbors_graph;
	}

	static PairsNeighborsMap collect_pairs_neighbors_map_from_quadruples_map(const QuadruplesMap& quadruples_map)
	{
		PairsNeighborsMap pairs_neighbors_map;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const Quadruple& quadruple=it->first;
			for(int a=0;a<4;a++)
			{
				for(int b=a+1;b<4;b++)
				{
					const Pair pair(quadruple.get(a), quadruple.get(b));
					for(int c=0;c<4;c++)
					{
						if(c!=a && c!=b)
						{
							pairs_neighbors_map[pair].insert(quadruple.get(c));
						}
					}
				}
			}
		}
		return pairs_neighbors_map;
	}

	static TriplesNeighborsMap collect_triples_neighbors_map_from_quadruples_map(const QuadruplesMap& quadruples_map)
	{
		TriplesNeighborsMap triples_neighbors_map;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const Quadruple& quadruple=it->first;
			for(int a=0;a<4;a++)
			{
				triples_neighbors_map[quadruple.exclude(a)].insert(quadruple.get(a));
			}
		}
		return triples_neighbors_map;
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

	static void print_quadruples_map(const QuadruplesMap& quadruples_map, std::ostream& output)
	{
		output.precision(std::numeric_limits<double>::digits10);
		output << std::fixed;
		const QuadruplesOrderedMap quadruples_ordered_map=collect_ordered_map_of_quadruples(quadruples_map);
		for(QuadruplesOrderedMap::const_iterator it=quadruples_ordered_map.begin();it!=quadruples_ordered_map.end();++it)
		{
			const Quadruple& quadruple=it->first;
			const std::vector<SimpleSphere>& tangent_spheres=it->second;
			for(std::size_t i=0;i<tangent_spheres.size();i++)
			{
				const SimpleSphere& tangent_sphere=tangent_spheres[i];
				output << quadruple.get(0) << " " << quadruple.get(1) << " " << quadruple.get(2) << " " << quadruple.get(3) << " ";
				output << tangent_sphere.x << " " << tangent_sphere.y << " " << tangent_sphere.z << " " << tangent_sphere.r << "\n";
			}
		}
	}

	static QuadruplesMap read_quadruples_map(std::istream& input)
	{
		QuadruplesMap quadruples_map;
		bool valid=true;
		while(input.good() && valid)
		{
			std::string line;
			std::getline(input, line);
			line=line.substr(0, line.find("#", 0));
			if(!line.empty())
			{
				std::istringstream line_stream(line);
				std::size_t q[4]={0, 0, 0, 0};
				double s[4]={0, 0, 0, 0};
				for(int i=0;i<4 && valid;i++)
				{
					line_stream >> q[i];
					valid=!line_stream.fail();
				}
				for(int i=0;i<4 && valid;i++)
				{
					line_stream >> s[i];
					valid=!line_stream.fail();
				}
				if(valid)
				{
					quadruples_map[Quadruple(q[0], q[1], q[2], q[3])].push_back(SimpleSphere(s[0], s[1], s[2], s[3]));
				}
			}
		}
		return quadruples_map;
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

	template<typename SphereType>
	static bool check_quadruples_map(const std::vector<SphereType>& spheres, const QuadruplesMap& quadruples_map)
	{
		for(typename QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const QuadruplesMap::key_type& q=it->first;
			const QuadruplesMap::mapped_type& ts=it->second;
			if(q.has_repetetions() || ts.empty() || ts.size()>2 || (ts.size()==2 && spheres_equal(ts.front(), ts.back())))
			{
				return false;
			}
			for(std::size_t i=0;i<ts.size();i++)
			{
				const SimpleSphere& t=ts[i];
				for(int j=0;j<4;j++)
				{
					if(!sphere_touches_sphere(t, spheres[q.get(j)]))
					{
						return false;
					}
				}
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

private:
	template<typename SphereType>
	class Face
	{
	public:
		typedef SphereType Sphere;

		Face(const std::vector<Sphere>& spheres, const Triple& abc_ids, const double min_sphere_radius) :
			spheres_(&spheres),
			abc_ids_(abc_ids),
			a_sphere_(&(spheres_->at(abc_ids_.get(0)))),
			b_sphere_(&(spheres_->at(abc_ids_.get(1)))),
			c_sphere_(&(spheres_->at(abc_ids_.get(2)))),
			tangent_planes_(TangentPlaneOfThreeSpheres::calculate((*a_sphere_), (*b_sphere_), (*c_sphere_))),
			can_have_d_(tangent_planes_.size()==2),
			can_have_e_(!can_have_d_ || greater(a_sphere_->r, min_sphere_radius) || greater(b_sphere_->r, min_sphere_radius) || greater(c_sphere_->r, min_sphere_radius)),
			threshold_distance_for_e_checking(std::max(0.0, std::max(a_sphere_->r, std::max(b_sphere_->r, c_sphere_->r)))*2-2*min_sphere_radius)
		{
			if(can_have_d_)
			{
				d_ids_and_tangent_spheres_.resize(2, std::pair<std::size_t, SimpleSphere>(npos, SimpleSphere()));
				init_central_planes();
				init_middle_region_approximation_sphere();
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

		template<typename InputSphereType>
		bool sphere_may_contain_candidate_for_e(const InputSphereType& input_sphere) const
		{
			return (
					can_have_e_
					&& (!can_have_d_ || !middle_region_approximation_sphere_.first || sphere_intersects_sphere(middle_region_approximation_sphere_.second, input_sphere))
					&& (!can_have_d_ || d_ids_and_tangent_spheres_[0].first==npos || d_ids_and_tangent_spheres_[1].first==npos || sphere_intersects_sphere_with_expansion(input_sphere, d_ids_and_tangent_spheres_[0].second, threshold_distance_for_e_checking) || sphere_intersects_sphere_with_expansion(input_sphere, d_ids_and_tangent_spheres_[1].second, threshold_distance_for_e_checking))
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
					&& (!can_have_d_ || d_ids_and_tangent_spheres_[0].first==npos || d_ids_and_tangent_spheres_[1].first==npos || sphere_intersects_sphere_with_expansion(spheres_->at(e_id), d_ids_and_tangent_spheres_[0].second, threshold_distance_for_e_checking) || sphere_intersects_sphere_with_expansion(spheres_->at(e_id), d_ids_and_tangent_spheres_[1].second, threshold_distance_for_e_checking))
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

		void init_middle_region_approximation_sphere()
		{
			middle_region_approximation_sphere_.first=false;
			if(can_have_d_)
			{
				const std::vector<SimpleSphere> minimal_tangent_sphere=TangentSphereOfThreeSpheres::calculate((*a_sphere_), (*b_sphere_), (*c_sphere_));
				if(minimal_tangent_sphere.size()==1)
				{
					const std::vector<SimpleSphere> disk=TangentSphereOfThreeSpheres::calculate(
							SimpleSphere(SimplePoint(*a_sphere_)+(tangent_planes_[0].second*(a_sphere_->r)), 0),
							SimpleSphere(SimplePoint(*b_sphere_)+(tangent_planes_[0].second*(b_sphere_->r)), 0),
							SimpleSphere(SimplePoint(*c_sphere_)+(tangent_planes_[0].second*(c_sphere_->r)), 0));
					if(disk.size()==1)
					{
						middle_region_approximation_sphere_.first=true;
						middle_region_approximation_sphere_.second=minimal_tangent_sphere.front();
						middle_region_approximation_sphere_.second.r=distance_from_point_to_point(minimal_tangent_sphere.front(), disk.front())+disk.front().r;
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

		const std::vector<Sphere>* spheres_;
		Triple abc_ids_;
		const Sphere* a_sphere_;
		const Sphere* b_sphere_;
		const Sphere* c_sphere_;
		std::vector< std::pair<SimplePoint, SimplePoint> > tangent_planes_;
		std::vector< std::pair<SimplePoint, SimplePoint> > central_planes_;
		std::vector< std::pair<std::size_t, SimpleSphere> > d_ids_and_tangent_spheres_;
		std::vector< std::pair<std::size_t, SimpleSphere> > e_ids_and_tangent_spheres_;
		bool can_have_d_;
		bool can_have_e_;
		double threshold_distance_for_e_checking;
		std::pair<bool, SimpleSphere> middle_region_approximation_sphere_;
	};

	class SearchForAnyDOfFace
	{
	public:
		template<typename SphereType>
		static bool find_any_d(const BoundingSpheresHierarchy<SphereType>& bsh, Face<SphereType>& face, const std::size_t d_number)
		{
			if(!face.has_d(d_number))
			{
				NodeCheckerForAnyD<SphereType> node_checker(face, d_number);
				LeafCheckerForAnyD<SphereType> leaf_checker(face, d_number);
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
		template<typename SphereType>
		struct NodeCheckerForAnyD
		{
			const Face<SphereType>& face;
			const std::size_t d_number;
			bool constrained;
			SimpleSphere constraint_sphere;

			NodeCheckerForAnyD(const Face<SphereType>& target, const std::size_t d_number) : face(target), d_number(d_number), constrained(false)
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

		template<typename SphereType>
		struct LeafCheckerForAnyD
		{
			Face<SphereType>& face;
			const std::size_t d_number;

			LeafCheckerForAnyD(Face<SphereType>& target, const std::size_t d_number) : face(target), d_number(d_number)
			{
			}

			std::pair<bool, bool> operator()(const std::size_t id, const SphereType&)
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
		template<typename SphereType>
		static bool find_valid_d(const BoundingSpheresHierarchy<SphereType>& bsh, Face<SphereType>& face, const std::size_t d_number)
		{
			if(face.has_d(d_number))
			{
				NodeCheckerForValidD<SphereType> node_checker(face, d_number);
				LeafCheckerForValidD<SphereType> leaf_checker(face, d_number);
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
		template<typename SphereType>
		struct NodeCheckerForValidD
		{
			const Face<SphereType>& face;
			const std::size_t d_number;

			NodeCheckerForValidD(const Face<SphereType>& target, const std::size_t d_number) : face(target), d_number(d_number)
			{
			}

			bool operator()(const SimpleSphere& sphere) const
			{
				return (face.has_d(d_number) && sphere_intersects_sphere(sphere, face.get_d_tangent_sphere(d_number)));
			}
		};

		template<typename SphereType>
		struct LeafCheckerForValidD
		{
			Face<SphereType>& face;
			const std::size_t d_number;
			std::tr1::unordered_set<std::size_t> safety_monitor;

			LeafCheckerForValidD(Face<SphereType>& target, const std::size_t d_number) : face(target), d_number(d_number)
			{
			}

			std::pair<bool, bool> operator()(const std::size_t id, const SphereType& sphere)
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
		template<typename SphereType>
		static bool find_valid_e(const BoundingSpheresHierarchy<SphereType>& bsh, Face<SphereType>& face)
		{
			NodeCheckerForValidE<SphereType> node_checker(face);
			LeafCheckerForValidE<SphereType> leaf_checker(face, bsh);
			return !bsh.search(node_checker, leaf_checker).empty();
		}

	private:
		template<typename SphereType>
		struct NodeCheckerForValidE
		{
			const Face<SphereType>& face;

			NodeCheckerForValidE(const Face<SphereType>& target) : face(target)
			{
			}

			bool operator()(const SimpleSphere& sphere) const
			{
				return face.sphere_may_contain_candidate_for_e(sphere);
			}
		};

		template<typename SphereType>
		struct LeafCheckerForValidE
		{
			Face<SphereType>& face;
			const BoundingSpheresHierarchy<SphereType>& bsh;

			LeafCheckerForValidE(Face<SphereType>& target, const BoundingSpheresHierarchy<SphereType>& bsh) : face(target), bsh(bsh)
			{
			}

			std::pair<bool, bool> operator()(const std::size_t id, const SphereType&)
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

	template<typename SphereType>
	static std::size_t select_starting_sphere_for_finding_first_valid_faces(const BoundingSpheresHierarchy<SphereType>& bsh)
	{
		const std::vector<SphereType>& spheres=bsh.leaves_spheres();
		if(spheres.empty())
		{
			return 0;
		}
		SimplePoint center(0.0, 0.0, 0.0);
		for(std::size_t i=0;i<spheres.size();i++)
		{
			center=center+SimplePoint(spheres[i]);
		}
		center=center*(1/static_cast<double>(spheres.size()));
		std::size_t result=0;
		double result_distance=distance_from_point_to_point(center, spheres[result]);
		for(std::size_t i=1;i<spheres.size();i++)
		{
			const double i_distance=distance_from_point_to_point(center, spheres[i]);
			if(i_distance<result_distance)
			{
				result=i;
				result_distance=i_distance;
			}
		}
		return result;
	}

	template<typename SphereType>
	static std::vector< Face<SphereType> > find_first_valid_faces(
			const BoundingSpheresHierarchy<SphereType>& bsh,
			const std::size_t starting_sphere_id,
			std::size_t& iterations_count,
			const bool fix_starting_sphere_id,
			const bool allow_quadruples_with_two_tangent_spheres,
			const std::size_t max_size_of_traversal=std::numeric_limits<std::size_t>::max())
	{
		const std::vector<SphereType>& spheres=bsh.leaves_spheres();
		std::vector< Face<SphereType> > result;
		if(spheres.size()>=4 && starting_sphere_id<spheres.size())
		{
			const std::vector<std::size_t> traversal=BoundingSpheresHierarchy<SphereType>::sort_objects_by_distance_to_one_of_them(spheres, starting_sphere_id, minimal_distance_from_sphere_to_sphere<SphereType, SphereType>);
			for(std::size_t d=3;d<std::min(traversal.size(), max_size_of_traversal);d++)
			{
				for(std::size_t a=0;a<(fix_starting_sphere_id ? 1 : d);a++)
				{
					for(std::size_t b=a+1;b<d;b++)
					{
						for(std::size_t c=b+1;c<d;c++)
						{
							iterations_count++;
							const Triple triple(traversal[a], traversal[b], traversal[c]);
							const Quadruple quadruple(triple, traversal[d]);
							const std::vector<SimpleSphere> tangents=TangentSphereOfFourSpheres::calculate(spheres[quadruple.get(0)], spheres[quadruple.get(1)], spheres[quadruple.get(2)], spheres[quadruple.get(3)]);
							if(
									(tangents.size()==1 && SearchForSphericalCollisions::find_any_collision(bsh, tangents.front()).empty())
									|| (allow_quadruples_with_two_tangent_spheres && tangents.size()==2 && (SearchForSphericalCollisions::find_any_collision(bsh, tangents.front()).empty() || SearchForSphericalCollisions::find_any_collision(bsh, tangents.back()).empty()))
								)
							{
								result.push_back(Face<SphereType>(bsh.leaves_spheres(), triple, bsh.min_input_radius()));
								return result;
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
			if(quadruple_tangent_spheres_list.size()==1 && !spheres_equal(quadruple_tangent_spheres_list.front(), quadruple_tangent_sphere))
			{
				quadruple_tangent_spheres_list.push_back(quadruple_tangent_sphere);
				quadruple_tangent_sphere_added=true;
			}
		}
		return std::make_pair(quadruple_added, quadruple_tangent_sphere_added);
	}

	template<typename SphereType>
	static QuadruplesSearchLog find_valid_quadruples(const BoundingSpheresHierarchy<SphereType>& bsh, QuadruplesMap& quadruples_map)
	{
		typedef SphereType Sphere;
		typedef std::tr1::unordered_map<Triple, std::size_t, Triple::HashFunctor> TriplesMap;

		QuadruplesSearchLog log=QuadruplesSearchLog();

		std::vector< Face<Sphere> > stack=find_first_valid_faces(bsh, select_starting_sphere_for_finding_first_valid_faces(bsh), log.performed_iterations_for_finding_first_faces, false, true);
		std::tr1::unordered_set<Triple, Triple::HashFunctor> processed_triples_set;
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
				Face<Sphere> face=stack.back();
				stack.pop_back();
				stack_map.erase(face.abc_ids());
				processed_triples_set.insert(face.abc_ids());
				log.processed_faces++;
				if(!face.can_have_d())
				{
					log.encountered_difficult_faces++;
				}
				const bool found_d0=face.can_have_d() && !face.has_d(0) && SearchForAnyDOfFace::find_any_d<Sphere>(bsh, face, 0) && SearchForValidDOfFace::find_valid_d<Sphere>(bsh, face, 0);
				const bool found_d1=face.can_have_d() && !face.has_d(1) && SearchForAnyDOfFace::find_any_d<Sphere>(bsh, face, 1) && SearchForValidDOfFace::find_valid_d<Sphere>(bsh, face, 1);
				const bool found_e=face.can_have_e() && SearchForValidEOfFace::find_valid_e<Sphere>(bsh, face);
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
							if(processed_triples_set.count(produced_preface.first)==0)
							{
								TriplesMap::const_iterator sm_it=stack_map.find(produced_preface.first);
								if(sm_it==stack_map.end())
								{
									stack_map[produced_preface.first]=stack.size();
									stack.push_back(Face<Sphere>(bsh.leaves_spheres(), produced_preface.first, bsh.min_input_radius()));
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
				if(spheres_usage_mapping[i]==0 && ignorable_spheres_ids.count(i)==0)
				{
					stack=find_first_valid_faces(bsh, i, log.performed_iterations_for_finding_first_faces, true, true, 25);
					ignorable_spheres_ids.insert(i);
				}
			}
		}
		while(!stack.empty());

		return log;
	}

	template<typename SphereType>
	static SurplusQuadruplesSearchLog find_surplus_valid_quadruples(const BoundingSpheresHierarchy<SphereType>& bsh, QuadruplesMap& quadruples_map)
	{
		SurplusQuadruplesSearchLog log=SurplusQuadruplesSearchLog();
		std::vector< std::pair<Quadruple, SimpleSphere> > surplus_candidates;
		for(QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const std::vector<SimpleSphere>& tangent_spheres=it->second;
			for(std::size_t i=0;i<tangent_spheres.size();i++)
			{
				const SimpleSphere& tangent_sphere=tangent_spheres[i];
				const SimpleSphere expanded_tangent_sphere(tangent_sphere, tangent_sphere.r+(3*comparison_epsilon()));
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
};

}

#endif /* APOLLOTA_TRIANGULATION_H_ */
