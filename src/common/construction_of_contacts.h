#ifndef COMMON_CONSTRUCTION_OF_CONTACTS_H_
#define COMMON_CONSTRUCTION_OF_CONTACTS_H_

#include "../apollota/constrained_contacts_construction.h"
#include "../apollota/constrained_contacts_utilities.h"

#include "../auxiliaries/opengl_printer.h"

#include "construction_of_triangulation.h"
#include "contact_value.h"

namespace voronota
{

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

	struct ParametersToConstructBundleOfContactInformation
	{
		double probe;
		double step;
		int projections;
		int sih_depth;
		bool calculate_volumes;

		ParametersToConstructBundleOfContactInformation() :
			probe(1.4),
			step(0.2),
			projections(5),
			sih_depth(3),
			calculate_volumes(false)
		{
		}

		bool equals(const ParametersToConstructBundleOfContactInformation& b) const
		{
			return (probe==b.probe
					&& step==b.step
					&& projections==b.projections
					&& sih_depth==b.sih_depth
					&& calculate_volumes==b.calculate_volumes);
		}

		bool supersedes(const ParametersToConstructBundleOfContactInformation& b) const
		{
			return (probe!=b.probe
					|| step!=b.step
					|| projections!=b.projections
					|| sih_depth!=b.sih_depth
					|| (calculate_volumes && !b.calculate_volumes));
		}
	};

	struct BundleOfContactInformation
	{
		ParametersToConstructBundleOfContactInformation parameters_of_construction;
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

	static bool construct_bundle_of_contact_information(
			const ParametersToConstructBundleOfContactInformation& parameters,
			const ConstructionOfTriangulation::BundleOfTriangulationInformation& bundle_of_triangulation_information,
			BundleOfContactInformation& bundle_of_contact_information)
	{
		bundle_of_contact_information=BundleOfContactInformation();
		bundle_of_contact_information.parameters_of_construction=parameters;

		if(bundle_of_triangulation_information.number_of_input_spheres<4 || bundle_of_triangulation_information.quadruples_map.empty())
		{
			return false;
		}

		const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);

		std::map<apollota::Pair, double> interactions_map;
		std::pair< bool, std::map<std::size_t, double> > volumes_map_bundle(parameters.calculate_volumes, std::map<std::size_t, double>());

		{
			const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(bundle_of_triangulation_information.spheres, vertices_vector, parameters.probe, parameters.step, parameters.projections, std::set<std::size_t>(), volumes_map_bundle);
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
			const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(bundle_of_triangulation_information.spheres, vertices_vector, parameters.probe, parameters.sih_depth, volumes_map_bundle);
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
					contact.value.dist=(bundle_of_triangulation_information.spheres[id_a].r+(parameters.probe*3.0));
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
	static bool construct_bundle_of_contact_information(
			const ParametersToConstructBundleOfContactInformation& parameters,
			const ContainerOfBalls& balls,
			const ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation& parameters_to_construct_triangulation,
			ConstructionOfTriangulation::BundleOfTriangulationInformation& bundle_of_triangulation_information,
			BundleOfContactInformation& bundle_of_contact_information)
	{
		ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation parameters_to_construct_triangulation_safely=parameters_to_construct_triangulation;
		if(parameters_to_construct_triangulation_safely.artificial_boundary_shift<parameters.probe*2.0)
		{
			parameters_to_construct_triangulation_safely.artificial_boundary_shift=parameters.probe*2.0;
		}

		if(ConstructionOfTriangulation::construct_bundle_of_triangulation_information(parameters_to_construct_triangulation_safely, balls, bundle_of_triangulation_information))
		{
			return construct_bundle_of_contact_information(parameters, bundle_of_triangulation_information, bundle_of_contact_information);
		}

		return false;
	}

	template<typename ContainerOfBalls>
	static bool construct_bundle_of_contact_information(
			const ParametersToConstructBundleOfContactInformation& parameters,
			const ContainerOfBalls& balls,
			ConstructionOfTriangulation::BundleOfTriangulationInformation& bundle_of_triangulation_information,
			BundleOfContactInformation& bundle_of_contact_information)
	{
		ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation parameters_to_construct_triangulation;
		parameters_to_construct_triangulation.artificial_boundary_shift=parameters.probe*2.0;

		return construct_bundle_of_contact_information(parameters, balls, parameters_to_construct_triangulation, bundle_of_triangulation_information, bundle_of_contact_information);
	}

	struct ParametersToEnhanceContacts
	{
		bool tag_centrality;
		bool tag_peripherial;
		bool adjunct_solvent_direction;
		double probe;
		int sih_depth;

		ParametersToEnhanceContacts() :
			tag_centrality(true),
			tag_peripherial(false),
			adjunct_solvent_direction(false),
			probe(1.4),
			sih_depth(3)
		{
		}

		bool equals(const ParametersToEnhanceContacts& b) const
		{
			return (tag_centrality==b.tag_centrality
					&& tag_peripherial==b.tag_peripherial
					&& adjunct_solvent_direction==b.adjunct_solvent_direction
					&& probe==b.probe
					&& sih_depth==b.sih_depth);
		}

		bool supersedes(const ParametersToEnhanceContacts& b) const
		{
			return ((tag_centrality && !b.tag_centrality)
					|| (tag_peripherial && !b.tag_peripherial)
					|| (tag_peripherial && probe!=b.probe)
					|| (adjunct_solvent_direction && !b.adjunct_solvent_direction)
					|| (adjunct_solvent_direction && probe!=b.probe)
					|| (adjunct_solvent_direction && sih_depth!=b.sih_depth));
		}
	};

	static bool enhance_contacts(
			const ParametersToEnhanceContacts& parameters,
			const ConstructionOfTriangulation::BundleOfTriangulationInformation& bundle_of_triangulation_information,
			std::vector<Contact>& contacts)
	{
		if(contacts.empty())
		{
			return false;
		}

		if(!parameters.tag_centrality && !parameters.tag_peripherial && !parameters.adjunct_solvent_direction)
		{
			return false;
		}

		bool modify_nonsolvent=false;
		if(parameters.tag_centrality || parameters.tag_peripherial)
		{
			for(std::size_t i=0;i<contacts.size() && !modify_nonsolvent;i++)
			{
				modify_nonsolvent=!contacts[i].solvent();
			}
		}

		bool modify_solvent=false;
		if(parameters.adjunct_solvent_direction)
		{
			for(std::size_t i=0;i<contacts.size() && !modify_solvent;i++)
			{
				modify_solvent=contacts[i].solvent();
			}
		}

		if(!modify_nonsolvent && !modify_solvent)
		{
			return false;
		}

		apollota::Triangulation::VerticesVector vertices_vector;
		apollota::TriangulationQueries::PairsMap pairs_vertices;

		if(modify_nonsolvent && parameters.tag_peripherial)
		{
			if(vertices_vector.empty())
			{
				vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);
			}
			if(pairs_vertices.empty())
			{
				pairs_vertices=apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);
			}
			for(std::size_t i=0;i<contacts.size();i++)
			{
				Contact& contact=contacts[i];
				if(!contact.solvent() && apollota::check_inter_atom_contact_peripherial(bundle_of_triangulation_information.spheres, vertices_vector, pairs_vertices, contact.ids[0], contact.ids[1], parameters.probe))
				{
					contact.value.props.tags.insert("peripherial");
				}
			}
		}

		if(modify_nonsolvent && parameters.tag_centrality)
		{
			const apollota::TriangulationQueries::PairsMap pairs_neighbors=apollota::TriangulationQueries::collect_pairs_neighbors_map_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);
			for(std::size_t i=0;i<contacts.size();i++)
			{
				Contact& contact=contacts[i];
				if(!contact.solvent() && apollota::check_inter_atom_contact_centrality(bundle_of_triangulation_information.spheres, pairs_neighbors, contact.ids[0], contact.ids[1]))
				{
					contact.value.props.tags.insert("central");
				}
			}
		}

		if(modify_solvent && parameters.adjunct_solvent_direction)
		{
			if(vertices_vector.empty())
			{
				vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);
			}
			const apollota::TriangulationQueries::IDsMap ids_vertices=apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector);
			const apollota::SubdividedIcosahedron sih(parameters.sih_depth);
			for(std::size_t i=0;i<contacts.size();i++)
			{
				Contact& contact=contacts[i];
				if(contact.solvent())
				{
					const apollota::SimplePoint direction=apollota::calculate_direction_of_solvent_contact(bundle_of_triangulation_information.spheres, vertices_vector, ids_vertices, contact.ids[0], parameters.probe, sih);
					if(!(direction==apollota::SimplePoint()))
					{
						contact.value.props.adjuncts["solvdir_x"]=direction.x;
						contact.value.props.adjuncts["solvdir_y"]=direction.y;
						contact.value.props.adjuncts["solvdir_z"]=direction.z;
					}
				}
			}
		}

		return true;
	}

	struct ParametersToDrawContacts
	{
		double probe;
		double step;
		int projections;
		bool simplify;
		int sih_depth;

		ParametersToDrawContacts() :
			probe(1.4),
			step(0.2),
			projections(5),
			simplify(false),
			sih_depth(3)
		{
		}

		bool equals(const ParametersToDrawContacts& b) const
		{
			return (probe==b.probe
					&& step==b.step
					&& projections==b.projections
					&& simplify==b.simplify
					&& sih_depth==b.sih_depth);
		}
	};

	template<typename ContainerOfIds>
	static bool draw_contacts(
			const ParametersToDrawContacts& parameters,
			const ConstructionOfTriangulation::BundleOfTriangulationInformation& bundle_of_triangulation_information,
			const ContainerOfIds& draw_ids,
			std::vector<Contact>& contacts)
	{
		if(contacts.empty() || draw_ids.empty())
		{
			return false;
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

		if(draw_solvent_ids.empty() && draw_nonsolvent_ids.empty())
		{
			return false;
		}

		apollota::Triangulation::VerticesVector vertices_vector;
		apollota::TriangulationQueries::PairsMap pairs_vertices;

		if(!draw_solvent_ids.empty())
		{
			if(vertices_vector.empty())
			{
				vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);
			}
			const apollota::TriangulationQueries::IDsMap ids_vertices=apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector);
			const apollota::SubdividedIcosahedron sih(parameters.sih_depth);
			for(typename std::vector<std::size_t>::const_iterator it=draw_solvent_ids.begin();it!=draw_solvent_ids.end();++it)
			{
				Contact& contact=contacts[*it];
				contact.value.graphics=apollota::draw_solvent_contact<auxiliaries::OpenGLPrinter>(bundle_of_triangulation_information.spheres, vertices_vector, ids_vertices, contact.ids[0], parameters.probe, sih);
			}
		}

		if(!draw_nonsolvent_ids.empty())
		{
			if(vertices_vector.empty())
			{
				vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);
			}
			if(pairs_vertices.empty())
			{
				pairs_vertices=apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);
			}
			for(typename std::vector<std::size_t>::const_iterator it=draw_nonsolvent_ids.begin();it!=draw_nonsolvent_ids.end();++it)
			{
				Contact& contact=contacts[*it];
				contact.value.graphics=apollota::draw_inter_atom_contact<auxiliaries::OpenGLPrinter>(bundle_of_triangulation_information.spheres, vertices_vector, pairs_vertices, contact.ids[0], contact.ids[1], parameters.probe, parameters.step, parameters.projections, parameters.simplify);
			}
		}

		return true;
	}

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

	static bool construct_bundle_of_contacts_scaled_mesh_information(const std::vector<Contact>& contacts, const std::vector<apollota::SimpleSphere>& centers, const float scale_factor, BundleOfContactsMeshInformation& bundle)
	{
		bundle.clear();

		if(contacts.empty())
		{
			return false;
		}

		bundle.mapped_indices.resize(contacts.size());

		{
			std::vector<float> buffer_of_vertices_raw;
			std::vector<float> buffer_of_vertices;
			std::vector<float> buffer_of_normals;
			std::vector<unsigned int> buffer_of_indices_raw;
			std::vector<unsigned int> buffer_of_indices;

			for(std::size_t id=0;id<contacts.size();id++)
			{
				const Contact& contact=contacts[id];
				if(!contact.value.graphics.empty())
				{
					auxiliaries::OpenGLPrinter opengl_printer;
					opengl_printer.add(contact.value.graphics);
					if(opengl_printer.write_to_low_level_triangle_buffers(buffer_of_vertices_raw, buffer_of_normals, buffer_of_indices_raw, true))
					{
						for(int e=0;e<(contact.solvent() ? 1 : 2);e++)
						{
							if(contact.ids[e]<centers.size())
							{
								const apollota::SimpleSphere& center=centers[contact.ids[e]];
								buffer_of_vertices=buffer_of_vertices_raw;
								for(std::size_t j=0;j<buffer_of_vertices.size();j+=3)
								{
									buffer_of_vertices[j]-=static_cast<float>(center.x);
									buffer_of_vertices[j+1]-=static_cast<float>(center.y);
									buffer_of_vertices[j+2]-=static_cast<float>(center.z);
									buffer_of_vertices[j]*=scale_factor;
									buffer_of_vertices[j+1]*=scale_factor;
									buffer_of_vertices[j+2]*=scale_factor;
									buffer_of_vertices[j]+=static_cast<float>(center.x);
									buffer_of_vertices[j+1]+=static_cast<float>(center.y);
									buffer_of_vertices[j+2]+=static_cast<float>(center.z);
								}
								buffer_of_indices=buffer_of_indices_raw;
								for(std::size_t j=0;j<buffer_of_indices.size();j++)
								{
									buffer_of_indices[j]+=bundle.global_buffer_of_vertices.size()/3;
								}
								bundle.global_buffer_of_vertices.insert(bundle.global_buffer_of_vertices.end(), buffer_of_vertices.begin(), buffer_of_vertices.end());
								bundle.global_buffer_of_normals.insert(bundle.global_buffer_of_normals.end(), buffer_of_normals.begin(), buffer_of_normals.end());
								bundle.global_buffer_of_indices.insert(bundle.global_buffer_of_indices.end(), buffer_of_indices.begin(), buffer_of_indices.end());
								bundle.mapped_indices[id].insert(bundle.mapped_indices[id].end(), buffer_of_indices.begin(), buffer_of_indices.end());
							}
						}
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

}

#endif /* COMMON_CONSTRUCTION_OF_CONTACTS_H_ */
