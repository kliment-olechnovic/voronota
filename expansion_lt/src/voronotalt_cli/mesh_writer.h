#ifndef VORONOTALT_MESH_WRITER_H_
#define VORONOTALT_MESH_WRITER_H_

#include <vector>
#include <set>
#include <map>
#include <fstream>

#include "../voronotalt/basic_types_and_functions.h"

namespace voronotalt
{

class MeshWriter
{
public:
	class ChooserOfBestCoordinateID
	{
	public:
		ChooserOfBestCoordinateID()
		{
		}

		void feed(const SimplePoint& p) noexcept
		{
			stats_x_.add(p.x);
			stats_y_.add(p.y);
			stats_z_.add(p.z);
		}

		unsigned int choose_best_coordinate_id() const noexcept
		{
			const Float dx=stats_x_.sample_variance_of_value();
			const Float dy=stats_y_.sample_variance_of_value();
			const Float dz=stats_z_.sample_variance_of_value();
			return ((dx>=dy && dx>=dz) ? 0 : ((dy>=dx && dy>=dz) ? 1 : 2));
		}

	private:
		struct ValueStatistics
		{
			Float mean_value;
			Float aggregate_for_variance_of_value;
			int count;

			ValueStatistics() noexcept : mean_value(FLOATCONST(0.0)), aggregate_for_variance_of_value(FLOATCONST(0.0)), count(0)
			{
			}

			void add(const Float single_value) noexcept
			{
				count++;
				if(count==1)
				{
					mean_value=single_value;
					aggregate_for_variance_of_value=FLOATCONST(0.0);
				}
				else
				{
					const Float old_mean_value=mean_value;
					mean_value+=(single_value-mean_value)/static_cast<Float>(count);
					aggregate_for_variance_of_value+=(single_value-mean_value)*(single_value-old_mean_value);
				}
			}

			inline Float sample_variance_of_value() const noexcept
			{
				return (count>1 ? (aggregate_for_variance_of_value/static_cast<Float>(count-1)) : FLOATCONST(0.0));
			}
		};

		ValueStatistics stats_x_;
		ValueStatistics stats_y_;
		ValueStatistics stats_z_;
	};

	explicit MeshWriter(const bool enabled) noexcept :
		enabled_(enabled),
		number_of_vertex_joins_(0),
		comparator_of_vertices_(0),
		vertices_map_(comparator_of_vertices_)
	{
	}

	MeshWriter(const bool enabled, const unsigned int coordinate_id) noexcept :
		enabled_(enabled),
		number_of_vertex_joins_(0),
		comparator_of_vertices_(coordinate_id),
		vertices_map_(comparator_of_vertices_)
	{
	}

	bool enabled() const noexcept
	{
		return enabled_;
	}

	bool add_triangle_fan(const std::vector<SimplePoint>& outer_points, const std::vector<int>& boundary_mask, const SimplePoint& center) noexcept
	{
		if(!enabled_ || outer_points.size()<2 || boundary_mask.size()!=outer_points.size())
		{
			return false;
		}
		const UnsignedInt center_id=add_vertex(center, 1);
		const int first_bm=boundary_mask.front();
		const UnsignedInt first_id=add_vertex(outer_points.front(), (first_bm>0 ? 2 : 0));
		const int second_bm=boundary_mask[1];
		const UnsignedInt second_id=add_vertex(outer_points[1], (second_bm>0 ? 2 : 0));
		add_face(Face(center_id, first_id, second_id));
		if((first_bm==1 || first_bm==3) && (second_bm==2 || second_bm==3))
		{
			boundary_edges_.insert(Edge(first_id, second_id));
		}
		int prev_bm=second_bm;
		UnsignedInt prev_id=second_id;
		for(std::size_t i=1;(i+1)<outer_points.size();i++)
		{
			const int next_bm=boundary_mask[i+1];
			const UnsignedInt next_id=add_vertex(outer_points[i+1], (next_bm>0 ? 2 : 0));
			add_face(Face(center_id, prev_id, next_id));
			if((prev_bm==1 || prev_bm==3) && (next_bm==2 || next_bm==3))
			{
				boundary_edges_.insert(Edge(prev_id, next_id));
			}
			prev_bm=next_bm;
			prev_id=next_id;
		}
		add_face(Face(center_id, prev_id, first_id));
		if((prev_bm==1 || prev_bm==3) && (first_bm==2 || first_bm==3))
		{
			boundary_edges_.insert(Edge(prev_id, first_id));
		}
		return true;
	}

	UnsignedInt get_number_of_vertices() const noexcept
	{
		if(!enabled_)
		{
			return 0;
		}
		return vertices_map_.size();
	}

	UnsignedInt get_number_of_vertex_joins() const noexcept
	{
		if(!enabled_)
		{
			return 0;
		}
		return number_of_vertex_joins_;
	}

	UnsignedInt get_number_of_faces() const noexcept
	{
		if(!enabled_)
		{
			return 0;
		}
		return faces_.size();
	}

	UnsignedInt get_number_of_edges() const noexcept
	{
		if(!enabled_)
		{
			return 0;
		}
		return edges_.size();
	}

	long get_euler_characteristic() const noexcept
	{
		if(!enabled_)
		{
			return 0;
		}
		return (static_cast<long>(get_number_of_vertices())-static_cast<long>(get_number_of_edges())+static_cast<long>(get_number_of_faces()));
	}

	long get_number_of_connected_components() noexcept
	{
		if(!enabled_)
		{
			return 0;
		}
		if(vertex_grouping_.number_of_connected_components<=0)
		{
			calculate_connected_components();
		}
		return vertex_grouping_.number_of_connected_components;
	}

	long get_number_of_boundary_components() noexcept
	{
		if(!enabled_)
		{
			return 0;
		}
		if(vertex_grouping_.number_of_boundary_components<=0)
		{
			calculate_boundary_components();
		}
		return vertex_grouping_.number_of_boundary_components;
	}

	bool extract_connected_component(const long selected_component_id, MeshWriter& output) noexcept
	{
		if(!enabled_)
		{
			return false;
		}
		if(vertex_grouping_.number_of_connected_components<=0)
		{
			calculate_connected_components();
		}
		if(selected_component_id>vertex_grouping_.number_of_connected_components)
		{
			return false;
		}
		output=MeshWriter(true, comparator_of_vertices_.get_coordinate_id());
		std::map<UnsignedInt, UnsignedInt> map_of_old_to_new_ids;
		for(std::size_t i=0;i<vertex_infos_.size();i++)
		{
			if(vertex_grouping_.coloring_by_connected_components[i]==selected_component_id)
			{
				const VertexInfo new_vertex_info(output.vertices_map_.size(), vertex_infos_[i].indicator);
				output.vertices_map_.insert(std::pair<SimplePoint, VertexInfo>(vertex_points_[i], new_vertex_info));
				output.vertex_points_.push_back(vertex_points_[i]);
				output.vertex_infos_.push_back(new_vertex_info);
				map_of_old_to_new_ids[i]=new_vertex_info.id;
			}
		}
		for(std::set<Face>::const_iterator it=faces_.begin();it!=faces_.end();++it)
		{
			const Face& face=(*it);
			if(vertex_grouping_.coloring_by_connected_components[face.ids[0]]==selected_component_id
					&& vertex_grouping_.coloring_by_connected_components[face.ids[1]]==selected_component_id
					&& vertex_grouping_.coloring_by_connected_components[face.ids[2]]==selected_component_id)
			{
				const Face new_face(map_of_old_to_new_ids[face.ids[0]], map_of_old_to_new_ids[face.ids[1]], map_of_old_to_new_ids[face.ids[2]]);
				output.add_face(new_face);
			}
		}
		for(std::set<Edge>::const_iterator it=boundary_edges_.begin();it!=boundary_edges_.end();++it)
		{
			const Edge& edge=(*it);
			if(vertex_grouping_.coloring_by_connected_components[edge.ids[0]]==selected_component_id
					&& vertex_grouping_.coloring_by_connected_components[edge.ids[1]]==selected_component_id)
			{
				const Edge new_edge(map_of_old_to_new_ids[edge.ids[0]], map_of_old_to_new_ids[edge.ids[1]]);
				output.boundary_edges_.insert(new_edge);
			}
		}
		return true;
	}

	int calculate_genus() noexcept
	{
		if(!enabled_)
		{
			return -1;
		}
		const long N=get_number_of_connected_components();
		int sum_of_genuses=0;
		if(N==1)
		{
			const long x=get_euler_characteristic();
			const long b=get_number_of_boundary_components();
			const Float raw_g=static_cast<Float>(2-b-x)/FLOATCONST(2.0);
			if(raw_g<FLOATCONST(0.0) || raw_g!=std::floor(raw_g))
			{
				return -1;
			}
			return static_cast<int>(std::floor(raw_g));
		}
		else
		{
			for(long id=1;id<=N;id++)
			{
				MeshWriter submesh(false);
				if(extract_connected_component(id, submesh) && submesh.get_number_of_connected_components()==1)
				{
					const Float raw_g=submesh.calculate_genus();
					if(raw_g<FLOATCONST(0.0) || raw_g!=std::floor(raw_g))
					{
						return -1;
					}
					sum_of_genuses+=static_cast<int>(std::floor(raw_g));
				}
			}
		}
		return sum_of_genuses;
	}

	bool write_to_obj_file(const std::string& filename) const noexcept
	{
		if(!enabled_)
		{
			return false;
		}

		if(filename.empty())
		{
			return false;
		}

		std::ofstream output(filename, std::ios::out);

		if(!output.good())
		{
			return false;
		}

		for(UnsignedInt i=0;i<vertex_points_.size();i++)
		{
			const SimplePoint& p=vertex_points_[i];
			output << "v " << p.x << " " << p.y << " " << p.z << "\n";
		}

		for(std::set<Face>::const_iterator it=faces_.begin();it!=faces_.end();++it)
		{
			const Face& face=(*it);
			output << "f " << (face.ids[0]+1) << " " << (face.ids[1]+1) << " " << (face.ids[2]+1) << "\n";
		}

		return true;
	}

private:
	struct VertexInfo
	{
		UnsignedInt id;
		int indicator;

		VertexInfo() noexcept : id(0), indicator(0)
		{
		}

		VertexInfo(const UnsignedInt id, const int indicator) noexcept : id(id), indicator(indicator)
		{
		}
	};

	struct Edge
	{
		UnsignedInt ids[2];

		Edge(const UnsignedInt a, const UnsignedInt b) noexcept
		{
			ids[0]=a;
			ids[1]=b;
			if(ids[0]>ids[1])
			{
				std::swap(ids[0], ids[1]);
			}
		}

		bool operator<(const Edge& e) const noexcept
		{
			return (ids[0]<e.ids[0] || (ids[0]==e.ids[0] && ids[1]<e.ids[1]));
		}
	};

	struct Face
	{
		UnsignedInt ids[3];

		Face(const UnsignedInt a, const UnsignedInt b, const UnsignedInt c) noexcept
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

		bool operator<(const Face& f) const noexcept
		{
			return (ids[0]<f.ids[0] || (ids[0]==f.ids[0] && ids[1]<f.ids[1]) || (ids[0]==f.ids[0] && ids[1]==f.ids[1] && ids[2]<f.ids[2]));
		}
	};

	class ComparatorOfVertices
	{
	public:
		explicit ComparatorOfVertices(const unsigned int coordinate_id) noexcept :
			coordinate_id_(coordinate_id%3),
			mesh_epsilon_(FLOATEPSILONROUGH)
		{
		}

		ComparatorOfVertices(const unsigned int coordinate_id, const Float mesh_epsilon) noexcept :
			coordinate_id_(coordinate_id%3),
			mesh_epsilon_(mesh_epsilon)
		{
		}

		unsigned int get_coordinate_id() const
		{
			return coordinate_id_;
		}

		bool equal(const SimplePoint& a, const SimplePoint& b) const noexcept
		{
			return (voronotalt::equal(a.x, b.x, mesh_epsilon_) && voronotalt::equal(a.y, b.y, mesh_epsilon_) && voronotalt::equal(a.z, b.z, mesh_epsilon_));
		}

		bool less(const SimplePoint& a, const SimplePoint& b) const noexcept
		{
			return ((coordinate_id_==0) ? ((a.x+mesh_epsilon_)<b.x) : ((coordinate_id_==1) ? ((a.y+mesh_epsilon_)<b.y) : ((a.z+mesh_epsilon_)<b.z)));
		}

		bool operator()(const SimplePoint& a, const SimplePoint& b) const noexcept
		{
			return less(a, b);
		}

	private:
		unsigned int coordinate_id_;
		Float mesh_epsilon_;
	};

	struct VertexGrouping
	{
		long number_of_connected_components;
		long number_of_boundary_components;
		std::vector<long> coloring_by_connected_components;
		std::vector<long> coloring_by_boundary_components;

		VertexGrouping() noexcept :
				number_of_connected_components(-1),
				number_of_boundary_components(-1)
		{
		}

		void invalidate() noexcept
		{
			number_of_connected_components=-1;
			number_of_boundary_components=-1;
		}
	};

	typedef std::multimap<SimplePoint, VertexInfo, ComparatorOfVertices> Multimap;

	UnsignedInt add_vertex(const SimplePoint& p, const int indicator) noexcept
	{
		vertex_grouping_.invalidate();
		std::pair<typename Multimap::iterator, typename Multimap::iterator> range=vertices_map_.equal_range(p);
		typename Multimap::iterator matched_it=vertices_map_.end();
		for(typename Multimap::iterator it=range.first;it!=vertices_map_.end() && it!=range.second && matched_it==vertices_map_.end();++it)
		{
			if(comparator_of_vertices_.equal(p, it->first))
			{
				matched_it=it;
				matched_it->second.indicator=std::max(matched_it->second.indicator, indicator);
				number_of_vertex_joins_++;
			}
		}
		if(matched_it==vertices_map_.end())
		{
			const VertexInfo new_vertex_info(vertices_map_.size(), indicator);
			matched_it=vertices_map_.insert(std::pair<SimplePoint, VertexInfo>(p, new_vertex_info));
			vertex_points_.push_back(p);
			vertex_infos_.push_back(new_vertex_info);
		}
		return matched_it->second.id;
	}

	void add_face(const Face& face) noexcept
	{
		vertex_grouping_.invalidate();
		faces_.insert(face);
		edges_.insert(Edge(face.ids[0], face.ids[1]));
		edges_.insert(Edge(face.ids[0], face.ids[2]));
		edges_.insert(Edge(face.ids[1], face.ids[2]));
	}

	void calculate_connected_components() noexcept
	{
		vertex_grouping_.number_of_connected_components=0;
		vertex_grouping_.coloring_by_connected_components.clear();
		vertex_grouping_.coloring_by_connected_components.resize(vertex_infos_.size(), 0);

		std::vector< std::vector<UnsignedInt> > graph(vertex_infos_.size());
		for(std::set<Edge>::const_iterator it=edges_.begin();it!=edges_.end();++it)
		{
			const Edge& e=(*it);
			graph[e.ids[0]].push_back(e.ids[1]);
			graph[e.ids[1]].push_back(e.ids[0]);
		}

		std::vector<UnsignedInt> searchstack;
		for(std::size_t i=0;i<graph.size();i++)
		{
			if(vertex_grouping_.coloring_by_connected_components[i]==0)
			{
				vertex_grouping_.number_of_connected_components++;
				searchstack.push_back(i);
				while(!searchstack.empty())
				{
					const UnsignedInt ci=searchstack.back();
					searchstack.pop_back();
					vertex_grouping_.coloring_by_connected_components[ci]=vertex_grouping_.number_of_connected_components;
					for(std::size_t j=0;j<graph[ci].size();j++)
					{
						const UnsignedInt ni=graph[ci][j];
						if(vertex_grouping_.coloring_by_connected_components[ni]==0)
						{
							searchstack.push_back(ni);
						}
					}
				}
			}
		}

		{
			std::map<long, long> map_of_component_sizes;
			for(std::size_t i=0;i<vertex_grouping_.coloring_by_connected_components.size();i++)
			{
				map_of_component_sizes[vertex_grouping_.coloring_by_connected_components[i]]++;
			}

			std::vector< std::pair<long, long> > sizes_of_components;
			sizes_of_components.reserve(map_of_component_sizes.size());
			for(std::map<long, long>::const_iterator it=map_of_component_sizes.begin();it!=map_of_component_sizes.end();++it)
			{
				sizes_of_components.push_back(std::pair<long, long>(0-it->second, it->first));
			}
			std::sort(sizes_of_components.begin(), sizes_of_components.end());

			std::map<long, long> remapping_of_component_ids;
			for(std::size_t i=0;i<sizes_of_components.size();i++)
			{
				remapping_of_component_ids[sizes_of_components[i].second]=static_cast<long>(i+1);
			}

			for(std::size_t i=0;i<vertex_grouping_.coloring_by_connected_components.size();i++)
			{
				vertex_grouping_.coloring_by_connected_components[i]=remapping_of_component_ids[vertex_grouping_.coloring_by_connected_components[i]];
			}
		}
	}

	void calculate_boundary_components() noexcept
	{
		vertex_grouping_.number_of_boundary_components=0;
		vertex_grouping_.coloring_by_boundary_components.clear();
		vertex_grouping_.coloring_by_boundary_components.resize(vertex_infos_.size(), 0);

		std::vector< std::vector<UnsignedInt> > graph(vertex_infos_.size());
		for(std::set<Edge>::const_iterator it=boundary_edges_.begin();it!=boundary_edges_.end();++it)
		{
			const Edge& e=(*it);
			if(vertex_infos_[e.ids[0]].indicator==2 && vertex_infos_[e.ids[1]].indicator==2)
			{
				graph[e.ids[0]].push_back(e.ids[1]);
				graph[e.ids[1]].push_back(e.ids[0]);
			}
		}

		std::vector<UnsignedInt> searchstack;
		for(std::size_t i=0;i<vertex_infos_.size();i++)
		{
			if(vertex_infos_[i].indicator==2 && vertex_grouping_.coloring_by_boundary_components[i]==0)
			{
				vertex_grouping_.number_of_boundary_components++;
				searchstack.push_back(i);
				while(!searchstack.empty())
				{
					const UnsignedInt ci=searchstack.back();
					searchstack.pop_back();
					vertex_grouping_.coloring_by_boundary_components[ci]=vertex_grouping_.number_of_boundary_components;
					for(std::size_t j=0;j<graph[ci].size();j++)
					{
						const UnsignedInt ni=graph[ci][j];
						if(vertex_infos_[ni].indicator==2 && vertex_grouping_.coloring_by_boundary_components[ni]==0)
						{
							searchstack.push_back(ni);
						}
					}
				}
			}
		}
	}

	bool enabled_;
	UnsignedInt number_of_vertex_joins_;
	ComparatorOfVertices comparator_of_vertices_;
	Multimap vertices_map_;
	std::vector<SimplePoint> vertex_points_;
	std::vector<VertexInfo> vertex_infos_;
	std::set<Face> faces_;
	std::set<Edge> edges_;
	std::set<Edge> boundary_edges_;
	VertexGrouping vertex_grouping_;
};

}

#endif /* VORONOTALT_MESH_WRITER_H_ */
