#ifndef COMMON_CONSTRUCTION_OF_CONTACTS_H_
#define COMMON_CONSTRUCTION_OF_CONTACTS_H_

#include "../apollota/constrained_contacts_construction.h"
#include "../apollota/constrained_contacts_utilities.h"
#include "../apollota/spheres_boundary_construction.h"

#include "../auxiliaries/opengl_printer.h"

#include "contact_value.h"

namespace common
{

class ConstructionOfContacts
{
public:
	struct Contact
	{
		std::size_t ids[2];
		ContactValue value;

		Contact(const std::size_t id_a, const std::size_t id_b, const ContactValue& value) : value(value)
		{
			ids[0]=id_a;
			ids[1]=id_b;
		}

		Contact(const std::size_t id, const ContactValue& value) : value(value)
		{
			ids[0]=id;
			ids[1]=id;
		}

		bool solvent() const
		{
			return (ids[0]==ids[1]);
		}
	};

	struct BundleOfContactInformation
	{
		std::size_t number_of_input_spheres;
		std::vector<apollota::SimpleSphere> spheres;
		apollota::Triangulation::Result triangulation_result;
		std::vector<Contact> contacts;
		std::vector<double> volumes;

		BundleOfContactInformation() : number_of_input_spheres(0)
		{
		}
	};

	struct BundleOfContactsMeshInformation
	{
		std::vector<float> global_buffer_of_vertices;
		std::vector<float> global_buffer_of_normals;
		std::vector<unsigned int> global_buffer_of_indices;
		std::vector< std::vector<unsigned int> > mapped_indices;

		void clear()
		{
			global_buffer_of_vertices.clear();
			global_buffer_of_normals.clear();
			global_buffer_of_indices.clear();
			mapped_indices.clear();
		}
	};

	class construct_bundle_of_contact_information
	{
	public:
		double probe;
		bool exclude_hidden_balls;
		double init_radius_for_BSH;
		double step;
		int projections;
		int sih_depth;
		bool calculate_volumes;

		construct_bundle_of_contact_information() :
			probe(1.4),
			exclude_hidden_balls(false),
			init_radius_for_BSH(3.5),
			step(0.2),
			projections(5),
			sih_depth(3),
			calculate_volumes(false)
		{
		}

		template<typename ContainerOfBalls>
		bool operator()(const ContainerOfBalls& balls, BundleOfContactInformation& bundle) const
		{
			bundle=BundleOfContactInformation();

			if(balls.size()<4)
			{
				return false;
			}

			bundle.spheres.reserve(balls.size());
			for(typename ContainerOfBalls::const_iterator it=balls.begin();it!=balls.end();++it)
			{
				bundle.spheres.push_back(apollota::SimpleSphere(*it));
			}

			bundle.number_of_input_spheres=bundle.spheres.size();
			{
				const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(bundle.spheres, probe*2.0);
				bundle.spheres.insert(bundle.spheres.end(), artificial_boundary.begin(), artificial_boundary.end());
			}

			bundle.triangulation_result=apollota::Triangulation::construct_result(bundle.spheres, init_radius_for_BSH, exclude_hidden_balls, false);
			const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle.triangulation_result.quadruples_map);

			std::map<apollota::Pair, double> interactions_map;
			std::pair< bool, std::map<std::size_t, double> > volumes_map_bundle(calculate_volumes, std::map<std::size_t, double>());

			{
				const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(bundle.spheres, vertices_vector, probe, step, projections, std::set<std::size_t>(), volumes_map_bundle);
				for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
				{
					const std::size_t id_a=it->first.get(0);
					const std::size_t id_b=it->first.get(1);
					if(id_a<bundle.number_of_input_spheres && id_b<bundle.number_of_input_spheres)
					{
						interactions_map[it->first]=it->second;
					}
				}
			}

			{
				const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(bundle.spheres, vertices_vector, probe, sih_depth, volumes_map_bundle);
				for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
				{
					const std::size_t id=it->first;
					if(id<bundle.number_of_input_spheres)
					{
						interactions_map[apollota::Pair(id, id)]=it->second;
					}
				}
			}

			bundle.contacts.reserve(interactions_map.size());
			for(std::map<apollota::Pair, double>::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
			{
				const double area=it->second;
				if(area>0.0)
				{
					const std::size_t id_a=it->first.get(0);
					const std::size_t id_b=it->first.get(1);
					Contact contact(id_a, id_b, ContactValue());
					contact.value.area=area;
					if(!contact.solvent())
					{
						contact.value.dist=apollota::distance_from_point_to_point(bundle.spheres[id_a], bundle.spheres[id_b]);
					}
					else
					{
						contact.value.dist=(bundle.spheres[id_a].r+(probe*3.0));
					}
					bundle.contacts.push_back(contact);
				}
			}

			if(volumes_map_bundle.first && !volumes_map_bundle.second.empty())
			{
				bundle.volumes.resize(bundle.number_of_input_spheres, 0.0);
				const std::map<std::size_t, double>& volumes_map=volumes_map_bundle.second;
				for(std::map<std::size_t, double>::const_iterator it=volumes_map.begin();it!=volumes_map.end();++it)
				{
					const double volume=it->second;
					if(volume>0.0)
					{
						const std::size_t id=it->first;
						if(id<bundle.volumes.size())
						{
							bundle.volumes[id]=volume;
						}
					}
				}
			}

			return true;
		}
	};

	class enhance_bundle_of_contact_information
	{
	public:
		double probe;
		double step;
		int projections;
		int sih_depth;
		bool draw;
		bool tag_centrality;
		bool tag_peripherial;

		enhance_bundle_of_contact_information() :
			probe(1.4),
			step(0.2),
			projections(5),
			sih_depth(3),
			draw(false),
			tag_centrality(false),
			tag_peripherial(false)
		{
		}

		template<typename ContainerOfIds>
		bool operator()(const ContainerOfIds& ids, BundleOfContactInformation& bundle) const
		{
			bool modified=false;

			if(!(draw || tag_centrality || tag_peripherial) || ids.empty())
			{
				return modified;
			}

			const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle.triangulation_result.quadruples_map);
			const apollota::TriangulationQueries::PairsMap pairs_vertices=((draw || tag_peripherial) ? apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::PairsMap());
			const apollota::TriangulationQueries::IDsMap ids_vertices=(draw ? apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::IDsMap());
			const apollota::SubdividedIcosahedron sih(draw ? sih_depth : 0);
			const apollota::TriangulationQueries::PairsMap pairs_neighbors=(tag_centrality ? apollota::TriangulationQueries::collect_pairs_neighbors_map_from_quadruples_map(bundle.triangulation_result.quadruples_map) : apollota::TriangulationQueries::PairsMap());

			for(typename ContainerOfIds::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id=(*it);
				if(id<bundle.contacts.size())
				{
					Contact& contact=bundle.contacts[id];
					const std::size_t id_a=contact.ids[0];
					const std::size_t id_b=contact.ids[1];
					if(!contact.solvent())
					{
						if(draw)
						{
							contact.value.graphics=apollota::draw_inter_atom_contact<auxiliaries::OpenGLPrinter>(bundle.spheres, vertices_vector, pairs_vertices, id_a, id_b, probe, step, projections);
						}

						if(tag_centrality && apollota::check_inter_atom_contact_centrality(bundle.spheres, pairs_neighbors, id_a, id_b))
						{
							contact.value.props.tags.insert("central");
						}

						if(tag_peripherial && apollota::check_inter_atom_contact_peripherial(bundle.spheres, vertices_vector, pairs_vertices, id_a, id_b, probe))
						{
							contact.value.props.tags.insert("peripherial");
						}
					}
					else
					{
						if(draw)
						{
							contact.value.graphics=apollota::draw_solvent_contact<auxiliaries::OpenGLPrinter>(bundle.spheres, vertices_vector, ids_vertices, id_a, probe, sih);
						}
					}
					modified=true;
				}
			}

			return modified;
		}
	};

	bool construct_bundle_of_contacts_mesh_information(const std::vector<Contact>& contacts, BundleOfContactsMeshInformation& bundle)
	{
		bundle.clear();

		if(contacts.empty())
		{
			return false;
		}

		bundle.mapped_indices.resize(contacts.size());

		{
			std::vector<float> buffer_of_vertices;
			std::vector<float> buffer_of_normals;
			std::vector<unsigned int> buffer_of_indices;

			for(std::size_t id=0;id<contacts.size();id++)
			{
				const ContactValue& value=contacts[id].value;
				if(!value.graphics.empty())
				{
					auxiliaries::OpenGLPrinter opengl_printer;
					opengl_printer.add(value.graphics);
					if(opengl_printer.write_to_low_level_triangle_buffers(buffer_of_vertices, buffer_of_normals, buffer_of_indices, true))
					{
						for(std::size_t j=0;j<buffer_of_indices.size();j++)
						{
							buffer_of_indices[j]+=bundle.global_buffer_of_vertices.size()/3;
						}
						bundle.global_buffer_of_vertices.insert(bundle.global_buffer_of_vertices.end(), buffer_of_vertices.begin(), buffer_of_vertices.end());
						bundle.global_buffer_of_normals.insert(bundle.global_buffer_of_normals.end(), buffer_of_normals.begin(), buffer_of_normals.end());
						bundle.global_buffer_of_indices.insert(bundle.global_buffer_of_indices.end(), buffer_of_indices.begin(), buffer_of_indices.end());
						bundle.mapped_indices[id]=buffer_of_indices;
					}
				}
			}
		}

		return (!bundle.global_buffer_of_indices.empty());
	}
};

inline std::ostream& operator<<(std::ostream& output, const ConstructionOfContacts::Contact& contact)
{
	output << contact.ids[0] << " " << contact.ids[1] << " " << contact.value;
	return output;
}

inline std::istream& operator>>(std::istream& input, ConstructionOfContacts::Contact& contact)
{
	input >> contact.ids[0] >> contact.ids[1] >> contact.value;
	return input;
}

}

#endif /* COMMON_CONSTRUCTION_OF_CONTACTS_H_ */
