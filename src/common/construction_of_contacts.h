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
		bool calculate_bounding_arcs;
		bool calculate_adjacencies;
		bool skip_sas;
		bool skip_same_group;
		std::vector<int> lookup_groups;
		std::vector<int> sas_mask;

		ParametersToConstructBundleOfContactInformation() :
			probe(1.4),
			step(0.2),
			projections(5),
			sih_depth(3),
			calculate_volumes(false),
			calculate_bounding_arcs(false),
			calculate_adjacencies(false),
			skip_sas(false),
			skip_same_group(false)
		{
		}

		bool equals(const ParametersToConstructBundleOfContactInformation& b) const
		{
			return (probe==b.probe
					&& step==b.step
					&& projections==b.projections
					&& sih_depth==b.sih_depth
					&& calculate_volumes==b.calculate_volumes
					&& calculate_bounding_arcs==b.calculate_bounding_arcs
					&& calculate_adjacencies==b.calculate_adjacencies
					&& skip_sas==b.skip_sas
					&& skip_same_group==b.skip_same_group);
		}

		bool supersedes(const ParametersToConstructBundleOfContactInformation& b) const
		{
			return (probe!=b.probe
					|| step!=b.step
					|| projections!=b.projections
					|| sih_depth!=b.sih_depth
					|| (calculate_volumes && !b.calculate_volumes)
					|| (calculate_bounding_arcs && !b.calculate_bounding_arcs)
					|| (calculate_adjacencies && !b.calculate_adjacencies)
					|| (!skip_sas && b.skip_sas)
					|| (!skip_same_group && b.skip_same_group));
		}
	};

	struct BundleOfContactInformation
	{
		ParametersToConstructBundleOfContactInformation parameters_of_construction;
		std::vector<Contact> contacts;
		std::vector<double> volumes;
		std::vector<double> bounding_arcs;
		std::vector< std::map<std::size_t, double> > adjacencies;
		std::vector<double> adjacency_perimeters;
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

		if(bundle_of_triangulation_information.number_of_input_spheres<1 || bundle_of_triangulation_information.quadruples_map.empty())
		{
			return false;
		}

		const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);

		std::map<apollota::Pair, double> interactions_map;
		std::pair< bool, std::map<std::size_t, double> > volumes_map_bundle(parameters.calculate_volumes, std::map<std::size_t, double>());
		std::pair< bool, std::map<apollota::Pair, double> > bounding_arcs_map_bundle(parameters.calculate_bounding_arcs, std::map<apollota::Pair, double>());
		std::pair< bool, std::map<apollota::Triple, double> > edge_strips_map_bundle(parameters.calculate_adjacencies, std::map<apollota::Triple, double>());

		{
			const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(bundle_of_triangulation_information.spheres, vertices_vector, parameters.probe, parameters.step, parameters.projections, std::set<std::size_t>(), (parameters.skip_same_group ?  parameters.lookup_groups : std::vector<int>()), volumes_map_bundle, bounding_arcs_map_bundle, edge_strips_map_bundle);
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

		if(!(parameters.skip_sas && parameters.sas_mask.empty()))
		{
			const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(bundle_of_triangulation_information.spheres, vertices_vector, parameters.probe, parameters.sih_depth, (parameters.skip_sas ? parameters.sas_mask : std::vector<int>()), volumes_map_bundle);
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

		if(bounding_arcs_map_bundle.first && !bounding_arcs_map_bundle.second.empty())
		{
			bundle_of_contact_information.bounding_arcs.resize(bundle_of_contact_information.contacts.size(), 0.0);
			const std::map<apollota::Pair, double>& bounding_arcs_map=bounding_arcs_map_bundle.second;
			for(std::size_t i=0;i<bundle_of_contact_information.contacts.size();i++)
			{
				const Contact& contact=bundle_of_contact_information.contacts[i];
				std::map<apollota::Pair, double>::const_iterator it=bounding_arcs_map.find(apollota::Pair(contact.ids[0], contact.ids[1]));
				if(it!=bounding_arcs_map.end())
				{
					bundle_of_contact_information.bounding_arcs[i]=it->second;
				}
			}
		}

		if(edge_strips_map_bundle.first && !edge_strips_map_bundle.second.empty())
		{
			bundle_of_contact_information.adjacencies.resize(bundle_of_contact_information.contacts.size());
			bundle_of_contact_information.adjacency_perimeters.resize(bundle_of_contact_information.contacts.size());
			const std::map<apollota::Triple, double>& edge_strips_map=edge_strips_map_bundle.second;
			std::map<apollota::Pair, std::size_t> map_of_pairs_to_ids;
			for(std::size_t i=0;i<bundle_of_contact_information.contacts.size();i++)
			{
				const Contact& contact=bundle_of_contact_information.contacts[i];
				map_of_pairs_to_ids[apollota::Pair(contact.ids[0], contact.ids[1])]=i;
			}
			for(std::map<apollota::Triple, double>::const_iterator it=edge_strips_map.begin();it!=edge_strips_map.end();++it)
			{
				const apollota::Triple& t=it->first;
				std::map<apollota::Pair, std::size_t>::const_iterator id01_it=map_of_pairs_to_ids.find(apollota::Pair(t.get(0), t.get(1)));
				std::map<apollota::Pair, std::size_t>::const_iterator id02_it=map_of_pairs_to_ids.find(apollota::Pair(t.get(0), t.get(2)));
				std::map<apollota::Pair, std::size_t>::const_iterator id12_it=map_of_pairs_to_ids.find(apollota::Pair(t.get(1), t.get(2)));
				if(id01_it!=map_of_pairs_to_ids.end() && id02_it!=map_of_pairs_to_ids.end())
				{
					bundle_of_contact_information.adjacencies[id01_it->second][id02_it->second]=it->second;
					bundle_of_contact_information.adjacencies[id02_it->second][id01_it->second]=it->second;
				}
				if(id01_it!=map_of_pairs_to_ids.end() && id12_it!=map_of_pairs_to_ids.end())
				{
					bundle_of_contact_information.adjacencies[id01_it->second][id12_it->second]=it->second;
					bundle_of_contact_information.adjacencies[id12_it->second][id01_it->second]=it->second;
				}
				if(id02_it!=map_of_pairs_to_ids.end() && id12_it!=map_of_pairs_to_ids.end())
				{
					bundle_of_contact_information.adjacencies[id02_it->second][id12_it->second]=it->second;
					bundle_of_contact_information.adjacencies[id12_it->second][id02_it->second]=it->second;
				}
				if(id01_it!=map_of_pairs_to_ids.end())
				{
					bundle_of_contact_information.adjacency_perimeters[id01_it->second]+=it->second;
				}
				if(id02_it!=map_of_pairs_to_ids.end())
				{
					bundle_of_contact_information.adjacency_perimeters[id02_it->second]+=it->second;
				}
				if(id12_it!=map_of_pairs_to_ids.end())
				{
					bundle_of_contact_information.adjacency_perimeters[id12_it->second]+=it->second;
				}
			}
		}

		if(bounding_arcs_map_bundle.first && !bounding_arcs_map_bundle.second.empty() && edge_strips_map_bundle.first && !edge_strips_map_bundle.second.empty())
		{
			std::map<std::size_t, std::size_t> map_of_atom_ids_to_solvent_contact_ids;
			for(std::size_t i=0;i<bundle_of_contact_information.contacts.size();i++)
			{
				const Contact& contact=bundle_of_contact_information.contacts[i];
				if(contact.solvent())
				{
					map_of_atom_ids_to_solvent_contact_ids[contact.ids[0]]=i;
				}
			}
			for(std::size_t i=0;i<bundle_of_contact_information.contacts.size();i++)
			{
				const Contact& contact=bundle_of_contact_information.contacts[i];
				if(!contact.solvent())
				{
					const double bounding_arc=bundle_of_contact_information.bounding_arcs[i];
					if(bounding_arc>0.0)
					{
						std::map<std::size_t, std::size_t>::const_iterator its[2]={map_of_atom_ids_to_solvent_contact_ids.find(contact.ids[0]), map_of_atom_ids_to_solvent_contact_ids.find(contact.ids[1])};
						for(int j=0;j<2;j++)
						{
							if(its[j]!=map_of_atom_ids_to_solvent_contact_ids.end())
							{
								const std::size_t solvent_contact_id=its[j]->second;
								bundle_of_contact_information.adjacencies[i][solvent_contact_id]=bounding_arc;
								bundle_of_contact_information.adjacencies[solvent_contact_id][i]=bounding_arc;
								bundle_of_contact_information.adjacency_perimeters[solvent_contact_id]+=bounding_arc;
							}
						}
						if(its[0]!=map_of_atom_ids_to_solvent_contact_ids.end() && its[1]!=map_of_atom_ids_to_solvent_contact_ids.end())
						{
							bundle_of_contact_information.adjacencies[its[0]->second][its[1]->second]=bounding_arc;
							bundle_of_contact_information.adjacencies[its[1]->second][its[0]->second]=bounding_arc;
						}
						bundle_of_contact_information.adjacency_perimeters[i]+=bounding_arc;
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
		bool enable_alt;
		double circular_angle_step;

		ParametersToDrawContacts() :
			probe(1.4),
			step(0.2),
			projections(5),
			simplify(false),
			sih_depth(3),
			enable_alt(false),
			circular_angle_step(0.1)
		{
		}

		bool equals(const ParametersToDrawContacts& b) const
		{
			return (probe==b.probe
					&& step==b.step
					&& projections==b.projections
					&& simplify==b.simplify
					&& sih_depth==b.sih_depth
					&& enable_alt==b.enable_alt
					&& circular_angle_step==b.circular_angle_step);
		}

		bool operator==(const ParametersToDrawContacts& v) const
		{
			return equals(v);
		}

		bool operator!=(const ParametersToDrawContacts& v) const
		{
			return (!((*this)==v));
		}

		bool operator<(const ParametersToDrawContacts& v) const
		{
			if(probe<v.probe) { return true; }
			else if(probe==v.probe)
			{
				if(step<v.step) { return true; }
				else if(step==v.step)
				{
					if(projections<v.projections) { return true; }
					else if(projections==v.projections)
					{
						if(sih_depth<v.sih_depth) { return true; }
						else if(sih_depth==v.sih_depth)
						{
							if(circular_angle_step<v.circular_angle_step) { return true; }
							else if(circular_angle_step==v.circular_angle_step)
							{
								if(simplify<v.simplify) { return true; }
								else if(simplify==v.simplify)
								{
									return (enable_alt<v.enable_alt);
								}
							}
						}
					}
				}
			}
			return false;
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

		if(!draw_solvent_ids.empty())
		{
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
					if(parameters.enable_alt)
					{
						contact.value.graphics_alt.resize(2);
						apollota::draw_solvent_contact_in_two_scales<auxiliaries::OpenGLPrinter>(bundle_of_triangulation_information.spheres, vertices_vector, ids_vertices, contact.ids[0], parameters.probe, sih, contact.value.graphics, contact.value.graphics_alt[0]);
					}
					else
					{
						apollota::draw_solvent_contact<auxiliaries::OpenGLPrinter>(bundle_of_triangulation_information.spheres, vertices_vector, ids_vertices, contact.ids[0], parameters.probe, sih, contact.value.graphics);
					}
				}
			}

			if(parameters.enable_alt)
			{
				voronota::apollota::TriangulationQueries::IDsMap singles_map=voronota::apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);
				const voronota::apollota::TriangulationQueries::PairsMap pairs_map=voronota::apollota::TriangulationQueries::collect_pairs_neighbors_map_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);
				std::map< std::size_t, std::vector<std::size_t> > map_of_rolling_descriptors;
				for(typename std::vector<std::size_t>::const_iterator it=draw_solvent_ids.begin();it!=draw_solvent_ids.end();++it)
				{
					map_of_rolling_descriptors[contacts[*it].ids[0]].reserve(3);
				}
				std::vector<voronota::apollota::RollingTopology::RollingDescriptor> rolling_descriptors;
				for(voronota::apollota::TriangulationQueries::PairsMap::const_iterator pairs_map_it=pairs_map.begin();pairs_map_it!=pairs_map.end();++pairs_map_it)
				{
					const voronota::apollota::Pair& pair=pairs_map_it->first;
					std::map< std::size_t, std::vector<std::size_t> >::iterator need_it0=map_of_rolling_descriptors.find(pair.get(0));
					std::map< std::size_t, std::vector<std::size_t> >::iterator need_it1=map_of_rolling_descriptors.find(pair.get(1));
					if(need_it0!=map_of_rolling_descriptors.end() || need_it1!=map_of_rolling_descriptors.end())
					{
						const std::set<std::size_t>& neighbor_ids=pairs_map_it->second;
						const voronota::apollota::RollingTopology::RollingDescriptor rolling_descriptor=
								voronota::apollota::RollingTopology::calculate_rolling_descriptor(bundle_of_triangulation_information.spheres, pair.get(0), pair.get(1), neighbor_ids, singles_map[pair.get(0)], singles_map[pair.get(1)], parameters.probe);
						if(rolling_descriptor.possible && (!rolling_descriptor.strips.empty() || rolling_descriptor.detached))
						{
							if(need_it0!=map_of_rolling_descriptors.end())
							{
								need_it0->second.push_back(rolling_descriptors.size());
							}
							if(need_it1!=map_of_rolling_descriptors.end())
							{
								need_it1->second.push_back(rolling_descriptors.size());
							}
							rolling_descriptors.push_back(rolling_descriptor);
						}
					}
				}
				for(typename std::vector<std::size_t>::const_iterator it=draw_solvent_ids.begin();it!=draw_solvent_ids.end();++it)
				{
					Contact& contact=contacts[*it];
					contact.value.graphics_alt.resize(2);
					apollota::draw_solvent_alt_contact<auxiliaries::OpenGLPrinter>(bundle_of_triangulation_information.spheres, rolling_descriptors, contact.ids[0], map_of_rolling_descriptors[contact.ids[0]], parameters.probe, parameters.circular_angle_step, 0.5/4.0, 0.8, contact.value.graphics_alt[1]);
				}
			}
		}

		if(!draw_nonsolvent_ids.empty())
		{
			if(vertices_vector.empty())
			{
				vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(bundle_of_triangulation_information.quadruples_map);
			}
			const apollota::TriangulationQueries::PairsMap pairs_vertices=apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);
			for(typename std::vector<std::size_t>::const_iterator it=draw_nonsolvent_ids.begin();it!=draw_nonsolvent_ids.end();++it)
			{
				Contact& contact=contacts[*it];
				apollota::draw_inter_atom_contact<auxiliaries::OpenGLPrinter>(bundle_of_triangulation_information.spheres, vertices_vector, pairs_vertices, contact.ids[0], contact.ids[1], parameters.probe, parameters.step, parameters.projections, parameters.simplify, contact.value.graphics);
			}
		}

		return true;
	}

	static bool construct_bundle_of_contacts_mesh_information(const std::vector<Contact>& contacts, const bool use_graphics_alt, BundleOfContactsMeshInformation& bundle)
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
				const Contact& contact=contacts[id];
				if((!use_graphics_alt && !contact.value.graphics.empty()) || (use_graphics_alt && !contact.value.graphics_alt.empty()))
				{
					auxiliaries::OpenGLPrinter opengl_printer;
					if(use_graphics_alt)
					{
						for(std::size_t i=0;i<contact.value.graphics_alt.size();i++)
						{
							if(!contact.value.graphics_alt[i].empty())
							{
								opengl_printer.add(contact.value.graphics_alt[i]);
							}
						}
					}
					else
					{
						if(!contact.value.graphics.empty())
						{
							opengl_printer.add(contact.value.graphics);
						}
					}
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

inline std::ostream& operator<<(std::ostream& output, const ConstructionOfContacts::ParametersToConstructBundleOfContactInformation& params)
{
	output << params.probe;
	output << " " << params.step;
	output << " " << params.projections;
	output << " " << params.sih_depth;
	output << " " << params.calculate_volumes;
	output << " " << params.calculate_bounding_arcs;
	output << " " << params.calculate_adjacencies;
	output << " " << params.skip_sas;
	output << " " << params.skip_same_group;
	output << " " << params.lookup_groups.size();
	for(std::size_t i=0;i<params.lookup_groups.size();i++)
	{
		output << " " << params.lookup_groups[i];
	}
	output << " " << params.sas_mask.size();
	for(std::size_t i=0;i<params.sas_mask.size();i++)
	{
		output << " " << params.sas_mask[i];
	}
	return output;
}

inline std::istream& operator>>(std::istream& input, ConstructionOfContacts::ParametersToConstructBundleOfContactInformation& params)
{
	input >> params.probe;
	input >> params.step;
	input >> params.projections;
	input >> params.sih_depth;
	input >> params.calculate_volumes;
	input >> params.calculate_bounding_arcs;
	input >> params.calculate_adjacencies;
	input >> params.skip_sas;
	input >> params.skip_same_group;
	{
		int n=0;
		input >> n;
		if(n>0)
		{
			params.lookup_groups.reserve(n);
			for(int i=0;i<n;i++)
			{
				int val=0;
				input >> val;
				params.lookup_groups.push_back(val);
			}
		}
	}
	{
		int n=0;
		input >> n;
		if(n>0)
		{
			params.sas_mask.reserve(n);
			for(int i=0;i<n;i++)
			{
				int val=0;
				input >> val;
				params.sas_mask.push_back(val);
			}
		}
	}
	return input;
}

inline std::ostream& operator<<(std::ostream& output, const ConstructionOfContacts::ParametersToEnhanceContacts& params)
{
	output << params.probe;
	output << " " << params.sih_depth;
	output << " " << params.tag_centrality;
	output << " " << params.tag_peripherial;
	output << " " << params.adjunct_solvent_direction;
	return output;
}

inline std::istream& operator>>(std::istream& input, ConstructionOfContacts::ParametersToEnhanceContacts& params)
{
	input >> params.probe;
	input >> params.sih_depth;
	input >> params.tag_centrality;
	input >> params.tag_peripherial;
	input >> params.adjunct_solvent_direction;
	return input;
}

inline std::ostream& operator<<(std::ostream& output, const ConstructionOfContacts::ParametersToDrawContacts& params)
{
	output << params.probe;
	output << " " << params.step;
	output << " " << params.projections;
	output << " " << params.simplify;
	output << " " << params.sih_depth;
	output << " " << params.enable_alt;
	output << " " << params.circular_angle_step;
	return output;
}

inline std::istream& operator>>(std::istream& input, ConstructionOfContacts::ParametersToDrawContacts& params)
{
	input >> params.probe;
	input >> params.step;
	input >> params.projections;
	input >> params.simplify;
	input >> params.sih_depth;
	input >> params.enable_alt;
	input >> params.circular_angle_step;
	return input;
}

}

}

#endif /* COMMON_CONSTRUCTION_OF_CONTACTS_H_ */
