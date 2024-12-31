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
		comparator_of_vertices_(0, FLOATCONST(0.000001)),
		vertices_(comparator_of_vertices_)
	{
	}

	MeshWriter(const bool enabled, const unsigned int coordinate_id) noexcept :
		enabled_(enabled),
		comparator_of_vertices_(coordinate_id, FLOATCONST(0.000001)),
		vertices_(comparator_of_vertices_)
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
		const UnsignedInt first_id=add_vertex(outer_points.front(), (boundary_mask.front()>0 ? 2 : 0));
		const UnsignedInt second_id=add_vertex(outer_points[1], (boundary_mask[1]>0 ? 2 : 0));
		add_face(Face(center_id, first_id, second_id));
		UnsignedInt prev_id=second_id;
		for(std::size_t i=1;(i+1)<outer_points.size();i++)
		{
			const UnsignedInt next_id=add_vertex(outer_points[i+1], (boundary_mask[i+1]>0 ? 2 : 0));
			add_face(Face(center_id, prev_id, next_id));
			prev_id=next_id;
		}
		add_face(Face(center_id, prev_id, first_id));
		return true;
	}

	bool collect_vertex_points(std::vector<SimplePoint>& points, std::vector<int>& indicators) const noexcept
	{
		if(!enabled_)
		{
			return false;
		}
		points.resize(vertices_.size());
		indicators.resize(vertices_.size());
		for(typename Multimap::const_iterator it=vertices_.begin();it!=vertices_.end();++it)
		{
			points[it->second.id]=it->first;
			indicators[it->second.id]=it->second.indicator;
		}
		return true;
	}

	UnsignedInt get_number_of_vertices() const noexcept
	{
		if(!enabled_)
		{
			return 0;
		}
		return vertices_.size();
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

	long calculate_euler_characteristic() const noexcept
	{
		if(!enabled_)
		{
			return 0;
		}
		return (static_cast<long>(get_number_of_vertices())-static_cast<long>(get_number_of_edges())+static_cast<long>(get_number_of_faces()));
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

		std::vector<SimplePoint> points;
		std::vector<int> indicators;

		collect_vertex_points(points, indicators);

		if(points.size()!=indicators.size())
		{
			return false;
		}

		for(UnsignedInt i=0;i<points.size();i++)
		{
			const SimplePoint& p=points[i];
			output << "v " << p.x << " " << p.y << " " << p.z << "\n";
		}

		for(UnsignedInt i=0;i<faces_.size();i++)
		{
			output << "f " << (faces_[i].ids[0]+1) << " " << (faces_[i].ids[1]+1) << " " << (faces_[i].ids[2]+1) << "\n";
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
	};

	class ComparatorOfVertices
	{
	public:
		explicit ComparatorOfVertices(const unsigned int coordinate_id, const Float mesh_epsilon) noexcept :
			coordinate_id_(coordinate_id%3),
			mesh_epsilon_(mesh_epsilon)
		{
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

	typedef std::multimap<SimplePoint, VertexInfo, ComparatorOfVertices> Multimap;

	UnsignedInt add_vertex(const SimplePoint& p, const int indicator) noexcept
	{
		std::pair<typename Multimap::iterator, typename Multimap::iterator> range=vertices_.equal_range(p);
		typename Multimap::iterator matched_it=vertices_.end();
		for(typename Multimap::iterator it=range.first;it!=vertices_.end() && it!=range.second && matched_it==vertices_.end();++it)
		{
			if(comparator_of_vertices_.equal(p, it->first))
			{
				matched_it=it;
			}
		}
		if(matched_it==vertices_.end())
		{
			matched_it=vertices_.insert(std::pair<SimplePoint, VertexInfo>(p, VertexInfo(vertices_.size(), indicator)));
		}
		return matched_it->second.id;
	}

	void add_face(const Face& face) noexcept
	{
		faces_.push_back(face);
		edges_.insert(Edge(face.ids[0], face.ids[1]));
		edges_.insert(Edge(face.ids[0], face.ids[2]));
		edges_.insert(Edge(face.ids[1], face.ids[2]));
	}

	bool enabled_;
	ComparatorOfVertices comparator_of_vertices_;
	Multimap vertices_;
	std::vector<Face> faces_;
	std::set<Edge> edges_;

};

}

#endif /* VORONOTALT_MESH_WRITER_H_ */
