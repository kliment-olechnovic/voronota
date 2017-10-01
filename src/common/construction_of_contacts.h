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

		Contact()
		{
			ids[0]=0;
			ids[1]=0;
		}

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

	struct BundleOfTriangulationInformation
	{
		std::size_t number_of_input_spheres;
		std::vector<apollota::SimpleSphere> spheres;
		apollota::Triangulation::Result triangulation_result;

		BundleOfTriangulationInformation() : number_of_input_spheres(0)
		{
		}
	};

	struct BundleOfContactInformation
	{
		std::vector<Contact> contacts;
		std::vector<double> volumes;
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

	class construct_bundle_of_triangulation_information
	{
	public:
		double artificial_boundary_shift;
		bool exclude_hidden_balls;
		double init_radius_for_BSH;

		construct_bundle_of_triangulation_information() :
			artificial_boundary_shift(2.8),
			exclude_hidden_balls(false),
			init_radius_for_BSH(3.5)
		{
		}

		template<typename ContainerOfBalls>
		bool operator()(const ContainerOfBalls& balls, BundleOfTriangulationInformation& bundle_of_triangulation_information) const
		{
			bundle_of_triangulation_information=BundleOfTriangulationInformation();

			if(balls.size()<4)
			{
				return false;
			}

			bundle_of_triangulation_information.spheres.reserve(balls.size());
			for(typename ContainerOfBalls::const_iterator it=balls.begin();it!=balls.end();++it)
			{
				bundle_of_triangulation_information.spheres.push_back(apollota::SimpleSphere(*it));
			}

			bundle_of_triangulation_information.number_of_input_spheres=bundle_of_triangulation_information.spheres.size();
			if(artificial_boundary_shift>0.0)
			{
				const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(bundle_of_triangulation_information.spheres, artificial_boundary_shift);
				bundle_of_triangulation_information.spheres.insert(bundle_of_triangulation_information.spheres.end(), artificial_boundary.begin(), artificial_boundary.end());
			}

			bundle_of_triangulation_information.triangulation_result=apollota::Triangulation::construct_result(bundle_of_triangulation_information.spheres, init_radius_for_BSH, exclude_hidden_balls, false);

			if(bundle_of_triangulation_information.triangulation_result.quadruples_map.empty())
			{
				return false;
			}

			return true;
		}
	};

	class construct_bundle_of_contact_information
	{
	public:
		double probe;
		double step;
		int projections;
		int sih_depth;
		bool calculate_volumes;

		construct_bundle_of_contact_information() :
			probe(1.4),
			step(0.2),
			projections(5),
			sih_depth(3),
			calculate_volumes(false)
		{
		}

		bool operator()(const BundleOfTriangulationInformation& bundle_of_triangulation_information, BundleOfContactInformation& bundle_of_contact_information) const
		{
			bundle_of_contact_information=BundleOfContactInformation();

			if(bundle_of_triangulation_information.number_of_input_spheres<4 || bundle_of_triangulation_information.triangulation_result.quadruples_map.empty())
			{
				return false;
			}

			const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.triangulation_result.quadruples_map);

			std::map<apollota::Pair, double> interactions_map;
			std::pair< bool, std::map<std::size_t, double> > volumes_map_bundle(calculate_volumes, std::map<std::size_t, double>());

			{
				const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(bundle_of_triangulation_information.spheres, vertices_vector, probe, step, projections, std::set<std::size_t>(), volumes_map_bundle);
				for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
				{
					const std::size_t id_a=it->first.get(0);
					const std::size_t id_b=it->first.get(1);
					if(id_a<bundle_of_triangulation_information.number_of_input_spheres && id_b<bundle_of_triangulation_information.number_of_input_spheres)
					{
						interactions_map[it->first]=it->second;
					}
				}
			}

			{
				const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(bundle_of_triangulation_information.spheres, vertices_vector, probe, sih_depth, volumes_map_bundle);
				for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
				{
					const std::size_t id=it->first;
					if(id<bundle_of_triangulation_information.number_of_input_spheres)
					{
						interactions_map[apollota::Pair(id, id)]=it->second;
					}
				}
			}

			bundle_of_contact_information.contacts.reserve(interactions_map.size());
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
						contact.value.dist=apollota::distance_from_point_to_point(bundle_of_triangulation_information.spheres[id_a], bundle_of_triangulation_information.spheres[id_b]);
					}
					else
					{
						contact.value.dist=(bundle_of_triangulation_information.spheres[id_a].r+(probe*3.0));
					}
					bundle_of_contact_information.contacts.push_back(contact);
				}
			}

			if(volumes_map_bundle.first && !volumes_map_bundle.second.empty())
			{
				bundle_of_contact_information.volumes.resize(bundle_of_triangulation_information.number_of_input_spheres, 0.0);
				const std::map<std::size_t, double>& volumes_map=volumes_map_bundle.second;
				for(std::map<std::size_t, double>::const_iterator it=volumes_map.begin();it!=volumes_map.end();++it)
				{
					const double volume=it->second;
					if(volume>0.0)
					{
						const std::size_t id=it->first;
						if(id<bundle_of_contact_information.volumes.size())
						{
							bundle_of_contact_information.volumes[id]=volume;
						}
					}
				}
			}

			return true;
		}

		template<typename ContainerOfBalls>
		bool operator()(const ContainerOfBalls& balls, const construct_bundle_of_triangulation_information& construct_triangulation, BundleOfTriangulationInformation& bundle_of_triangulation_information, BundleOfContactInformation& bundle_of_contact_information) const
		{
			construct_bundle_of_triangulation_information construct_triangulation_safely=construct_triangulation;
			if(construct_triangulation_safely.artificial_boundary_shift<probe*2.0)
			{
				construct_triangulation_safely.artificial_boundary_shift=probe*2.0;
			}

			if(construct_triangulation_safely(balls, bundle_of_triangulation_information))
			{
				return this->operator()(bundle_of_triangulation_information, bundle_of_contact_information);
			}

			return false;
		}

		template<typename ContainerOfBalls>
		bool operator()(const ContainerOfBalls& balls, BundleOfTriangulationInformation& bundle_of_triangulation_information, BundleOfContactInformation& bundle_of_contact_information) const
		{
			construct_bundle_of_triangulation_information construct_triangulation;
			construct_triangulation.artificial_boundary_shift=probe*2.0;

			return this->operator()(balls, construct_triangulation, bundle_of_triangulation_information, bundle_of_contact_information);
		}
	};

	class enhance_contacts
	{
	public:
		double probe;
		double step;
		int projections;
		int sih_depth;
		bool tag_centrality;
		bool tag_peripherial;

		enhance_contacts() :
			probe(1.4),
			step(0.2),
			projections(5),
			sih_depth(3),
			tag_centrality(false),
			tag_peripherial(false)
		{
		}

		template<typename ContainerOfIds>
		bool operator()(const BundleOfTriangulationInformation& bundle_of_triangulation_information, const ContainerOfIds& draw_ids, std::vector<Contact>& contacts) const
		{
			if(contacts.empty())
			{
				return false;
			}

			if(!tag_centrality && !tag_peripherial && draw_ids.empty())
			{
				return false;
			}

			bool tag_nonsolvent=false;
			if(tag_centrality || tag_peripherial)
			{
				for(std::size_t i=0;i<contacts.size() && !tag_nonsolvent;i++)
				{
					tag_nonsolvent=!contacts[i].solvent();
				}
			}

			std::vector<std::size_t> draw_solvent_ids;
			std::vector<std::size_t> draw_nonsolvent_ids;
			for(typename ContainerOfIds::const_iterator it=draw_ids.begin();it!=draw_ids.end();++it)
			{
				const std::size_t id=(*it);
				if(id<contacts.size())
				{
					if(contacts[id].solvent())
					{
						draw_solvent_ids.push_back(id);
					}
					else
					{
						draw_nonsolvent_ids.push_back(id);
					}
				}
			}

			if(!tag_nonsolvent && draw_solvent_ids.empty() && draw_nonsolvent_ids.empty())
			{
				return false;
			}

			apollota::Triangulation::VerticesVector vertices_vector;
			apollota::TriangulationQueries::PairsMap pairs_vertices;

			if(!draw_solvent_ids.empty())
			{
				if(vertices_vector.empty())
				{
					vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.triangulation_result.quadruples_map);
				}
				const apollota::TriangulationQueries::IDsMap ids_vertices=apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector);
				const apollota::SubdividedIcosahedron sih(sih_depth);
				for(typename std::vector<std::size_t>::const_iterator it=draw_solvent_ids.begin();it!=draw_solvent_ids.end();++it)
				{
					Contact& contact=contacts[*it];
					contact.value.graphics=apollota::draw_solvent_contact<auxiliaries::OpenGLPrinter>(bundle_of_triangulation_information.spheres, vertices_vector, ids_vertices, contact.ids[0], probe, sih);
				}
			}

			if(!draw_nonsolvent_ids.empty())
			{
				if(vertices_vector.empty())
				{
					vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.triangulation_result.quadruples_map);
				}
				if(pairs_vertices.empty())
				{
					pairs_vertices=apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);
				}
				for(typename std::vector<std::size_t>::const_iterator it=draw_nonsolvent_ids.begin();it!=draw_nonsolvent_ids.end();++it)
				{
					Contact& contact=contacts[*it];
					contact.value.graphics=apollota::draw_inter_atom_contact<auxiliaries::OpenGLPrinter>(bundle_of_triangulation_information.spheres, vertices_vector, pairs_vertices, contact.ids[0], contact.ids[1], probe, step, projections);
				}
			}

			if(tag_nonsolvent && tag_peripherial)
			{
				if(vertices_vector.empty())
				{
					vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.triangulation_result.quadruples_map);
				}
				if(pairs_vertices.empty())
				{
					pairs_vertices=apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);
				}
				for(std::size_t i=0;i<contacts.size();i++)
				{
					Contact& contact=contacts[i];
					if(!contact.solvent() && apollota::check_inter_atom_contact_peripherial(bundle_of_triangulation_information.spheres, vertices_vector, pairs_vertices, contact.ids[0], contact.ids[1], probe))
					{
						contact.value.props.tags.insert("peripherial");
					}
				}
			}

			if(tag_nonsolvent && tag_centrality)
			{
				const apollota::TriangulationQueries::PairsMap pairs_neighbors=apollota::TriangulationQueries::collect_pairs_neighbors_map_from_quadruples_map(bundle_of_triangulation_information.triangulation_result.quadruples_map);
				for(std::size_t i=0;i<contacts.size();i++)
				{
					Contact& contact=contacts[i];
					if(!contact.solvent() && apollota::check_inter_atom_contact_centrality(bundle_of_triangulation_information.spheres, pairs_neighbors, contact.ids[0], contact.ids[1]))
					{
						contact.value.props.tags.insert("central");
					}
				}
			}

			return true;
		}
	};

	static bool construct_bundle_of_contacts_mesh_information(const std::vector<Contact>& contacts, BundleOfContactsMeshInformation& bundle)
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
