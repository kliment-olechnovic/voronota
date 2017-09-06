#ifndef COMMON_COLLECTION_OF_CONTACTS_H_
#define COMMON_COLLECTION_OF_CONTACTS_H_

#include "../apollota/constrained_contacts_construction.h"
#include "../apollota/constrained_contacts_utilities.h"
#include "../apollota/spheres_boundary_construction.h"

#include "../auxiliaries/opengl_printer.h"

#include "collection_of_balls.h"
#include "contact_value.h"

namespace common
{

class CollectionOfContacts
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

	typedef std::vector<Contact> VectorOfContacts;

	class FunctionConstructContactsFromBalls
	{
	public:
		struct Result
		{
			bool valid;
			VectorOfContacts vector_of_contacts;
			std::vector<double> vector_of_volumes;

			Result() : valid(false)
			{
			}
		};

		double probe;
		bool exclude_hidden_balls;
		double init_radius_for_BSH;
		double step;
		int projections;
		int sih_depth;
		bool draw;
		bool tag_centrality;
		bool tag_peripherial;
		bool calculate_volumes;

		FunctionConstructContactsFromBalls() :
			probe(1.4),
			exclude_hidden_balls(false),
			init_radius_for_BSH(3.5),
			step(0.2),
			projections(5),
			sih_depth(3),
			draw(false),
			tag_centrality(false),
			tag_peripherial(false),
			calculate_volumes(false)
		{
		}

		Result run(const CollectionOfBalls::VectorOfBalls& input) const
		{
			Result result;

			std::vector<apollota::SimpleSphere> spheres;
			spheres.reserve(input.size());
			for(std::size_t i=0;i<input.size();i++)
			{
				spheres.push_back(apollota::SimpleSphere(input[i].value));
			}

			if(spheres.size()<4)
			{
				return result;
			}

			const std::size_t input_spheres_count=spheres.size();
			{
				const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
				spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());
			}

			const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, init_radius_for_BSH, exclude_hidden_balls, false);
			const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);

			std::map<apollota::Pair, double> interactions_map;
			std::pair< bool, std::map<std::size_t, double> > volumes_map_bundle(calculate_volumes, std::map<std::size_t, double>());

			{
				const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(spheres, vertices_vector, probe, step, projections, std::set<std::size_t>(), volumes_map_bundle);
				for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
				{
					const std::size_t id_a=it->first.get(0);
					const std::size_t id_b=it->first.get(1);
					if(id_a<input_spheres_count && id_b<input_spheres_count)
					{
						interactions_map[it->first]=it->second;
					}
				}
			}

			{
				const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(spheres, vertices_vector, probe, sih_depth, volumes_map_bundle);
				for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
				{
					const std::size_t id=it->first;
					if(id<input_spheres_count)
					{
						interactions_map[apollota::Pair(id, id)]=it->second;
					}
				}
			}

			{
				const apollota::TriangulationQueries::PairsMap pairs_vertices=((draw || tag_peripherial) ? apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::PairsMap());
				const apollota::TriangulationQueries::IDsMap ids_vertices=(draw ? apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::IDsMap());
				const apollota::SubdividedIcosahedron sih(draw ? sih_depth : 0);
				const apollota::TriangulationQueries::PairsMap pairs_neighbors=(tag_centrality ? apollota::TriangulationQueries::collect_pairs_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map) : apollota::TriangulationQueries::PairsMap());

				result.vector_of_contacts.reserve(interactions_map.size());
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
							contact.value.dist=apollota::distance_from_point_to_point(spheres[id_a], spheres[id_b]);

							if(draw)
							{
								contact.value.graphics=apollota::draw_inter_atom_contact<auxiliaries::OpenGLPrinter>(spheres, vertices_vector, pairs_vertices, id_a, id_b, probe, step, projections);
							}

							if(tag_centrality && apollota::check_inter_atom_contact_centrality(spheres, pairs_neighbors, id_a, id_b))
							{
								contact.value.props.tags.insert("central");
							}

							if(tag_peripherial && apollota::check_inter_atom_contact_peripherial(spheres, vertices_vector, pairs_vertices, id_a, id_b, probe))
							{
								contact.value.props.tags.insert("peripherial");
							}
						}
						else
						{
							contact.value.dist=(spheres[id_a].r+(probe*3.0));

							if(draw)
							{
								contact.value.graphics=apollota::draw_solvent_contact<auxiliaries::OpenGLPrinter>(spheres, vertices_vector, ids_vertices, id_a, probe, sih);
							}
						}
						result.vector_of_contacts.push_back(contact);
					}
				}
			}

			if(volumes_map_bundle.first && !volumes_map_bundle.second.empty())
			{
				result.vector_of_volumes.resize(input.size(), 0.0);
				const std::map<std::size_t, double>& volumes_map=volumes_map_bundle.second;
				for(std::map<std::size_t, double>::const_iterator it=volumes_map.begin();it!=volumes_map.end();++it)
				{
					const double volume=it->second;
					if(volume>0.0)
					{
						const std::size_t id=it->first;
						if(id<result.vector_of_volumes.size())
						{
							result.vector_of_volumes[id]=volume;
						}
					}
				}
			}

			result.valid=true;
			return result;
		}
	};

	class FunctionDeriveIndexedMeshFromVectorOfContacts
	{
		struct Result
		{
			bool valid;
			std::vector<float> global_buffer_of_vertices;
			std::vector<float> global_buffer_of_normals;
			std::vector<unsigned int> global_buffer_of_indices;
			std::vector< std::vector<unsigned int> > map_of_drawing_locations;

			Result() : valid(false)
			{
			}
		};

		Result run(const VectorOfContacts& vector_of_contacts) const
		{
			Result result;

			if(vector_of_contacts.empty())
			{
				return result;
			}

			result.map_of_drawing_locations.resize(vector_of_contacts.size());

			{
				std::vector<float> buffer_of_vertices;
				std::vector<float> buffer_of_normals;
				std::vector<unsigned int> buffer_of_indices;

				for(std::size_t id=0;id<vector_of_contacts.size();id++)
				{
					const ContactValue& value=vector_of_contacts[id].value;
					if(!value.graphics.empty())
					{
						auxiliaries::OpenGLPrinter opengl_printer;
						opengl_printer.add(value.graphics);
						if(opengl_printer.write_to_low_level_triangle_buffers(buffer_of_vertices, buffer_of_normals, buffer_of_indices, true))
						{
							for(std::size_t j=0;j<buffer_of_indices.size();j++)
							{
								buffer_of_indices[j]+=result.global_buffer_of_vertices.size()/3;
							}
							result.global_buffer_of_vertices.insert(result.global_buffer_of_vertices.end(), buffer_of_vertices.begin(), buffer_of_vertices.end());
							result.global_buffer_of_normals.insert(result.global_buffer_of_normals.end(), buffer_of_normals.begin(), buffer_of_normals.end());
							result.global_buffer_of_indices.insert(result.global_buffer_of_indices.end(), buffer_of_indices.begin(), buffer_of_indices.end());
							result.map_of_drawing_locations[id]=buffer_of_indices;
						}
					}
				}
			}

			result.valid=(!result.global_buffer_of_indices.empty());

			return result;
		}
	};
};

inline std::ostream& operator<<(std::ostream& output, const CollectionOfContacts::Contact& contact)
{
	output << contact.ids[0] << " " << contact.ids[1] << " " << contact.value;
	return output;
}

inline std::istream& operator>>(std::istream& input, CollectionOfContacts::Contact& contact)
{
	input >> contact.ids[0] >> contact.ids[1] >> contact.value;
	return input;
}

}

#endif /* COMMON_COLLECTION_OF_CONTACTS_H_ */
