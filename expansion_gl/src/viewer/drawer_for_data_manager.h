#ifndef VIEWER_DRAWER_FOR_DATA_MANAGER_H_
#define VIEWER_DRAWER_FOR_DATA_MANAGER_H_

#include "../../../src/scripting/data_manager.h"
#include "../../../src/common/construction_of_structural_cartoon.h"

#include "../uv/drawing_controller.h"
#include "../uv/drawing_with_instancing_controller.h"
#include "../uv/drawing_id.h"

namespace voronota
{

namespace viewer
{

class DrawerForDataManager
{
public:
	struct ElementID
	{
		scripting::DataManager* data_manager_ptr;
		std::size_t atom_id;
		std::size_t contact_id;
		std::size_t figure_id;

		ElementID() :
			data_manager_ptr(0),
			atom_id(std::numeric_limits<std::size_t>::max()),
			contact_id(std::numeric_limits<std::size_t>::max()),
			figure_id(std::numeric_limits<std::size_t>::max())
		{
		}

		bool valid_atom_id() const
		{
			return (data_manager_ptr!=0 && atom_id<data_manager_ptr->atoms().size());
		}

		bool valid_contact_id() const
		{
			return (data_manager_ptr!=0 && contact_id<data_manager_ptr->contacts().size());
		}

		bool valid_figure_id() const
		{
			return (data_manager_ptr!=0 && figure_id<data_manager_ptr->figures().size());
		}

		bool valid() const
		{
			return (valid_atom_id() || valid_contact_id() || valid_figure_id());
		}
	};

	struct DrawingRequest
	{
		bool atoms_balls;
		bool atoms_sticks;
		bool atoms_trace;
		bool atoms_cartoon;
		bool atoms_points;
		bool contacts_faces;
		bool contacts_sasmesh;
		bool contacts_edges;
		bool contacts_skinshape;
		bool figures_solid;
		bool figures_mesh;

		explicit DrawingRequest(const bool status) :
			atoms_balls(status),
			atoms_sticks(status),
			atoms_trace(status),
			atoms_cartoon(status),
			atoms_points(status),
			contacts_faces(status),
			contacts_sasmesh(status),
			contacts_edges(status),
			contacts_skinshape(status),
			figures_solid(status),
			figures_mesh(status)
		{
		}

		DrawingRequest(const bool atoms_status, const bool contacts_status, const bool figures_status) :
			atoms_balls(atoms_status),
			atoms_sticks(atoms_status),
			atoms_trace(atoms_status),
			atoms_cartoon(atoms_status),
			atoms_points(atoms_status),
			contacts_faces(contacts_status),
			contacts_sasmesh(contacts_status),
			contacts_edges(contacts_status),
			contacts_skinshape(contacts_status),
			figures_solid(figures_status),
			figures_mesh(figures_status)
		{
		}
	};

	struct RenderingParameters
	{
		unsigned int ball_sphere_quality;
		unsigned int stick_sphere_quality;
		unsigned int stick_cylinder_quality;
		unsigned int trace_sphere_quality;
		unsigned int trace_cylinder_quality;
		int cartoon_style;

		RenderingParameters() :
			ball_sphere_quality(3),
			stick_sphere_quality(3),
			stick_cylinder_quality(36),
			trace_sphere_quality(3),
			trace_cylinder_quality(36),
			cartoon_style(0)
		{
		}

		static RenderingParameters& default_rendering_parameters()
		{
			static RenderingParameters p;
			return p;
		}
	};

	explicit DrawerForDataManager(scripting::DataManager& data_manager) :
		data_manager_(data_manager),
		rendering_parameters_(RenderingParameters::default_rendering_parameters()),
		dc_atoms_balls_spheres_(0),
		dc_atoms_sticks_spheres_(1),
		dc_atoms_sticks_cylinders_(1),
		dc_atoms_trace_spheres_(2),
		dc_atoms_trace_cylinders_(2),
		dc_atoms_cartoon_(3),
		dc_atoms_points_(4),
		dc_contacts_faces_(0),
		dc_contacts_sasmesh_(1),
		dc_contacts_edges_(2),
		dc_contacts_skinshape_(3),
		dc_figures_solid_(0),
		dc_figures_mesh_(1)
	{
	}

	scripting::DataManager* data_manager_ptr()
	{
		return &data_manager_;
	}

	const RenderingParameters& rendering_parameters() const
	{
		return rendering_parameters_;
	}

	void set_rendering_parameters(const RenderingParameters& rendering_parameters)
	{
		rendering_parameters_=rendering_parameters;
	}

	void draw(const DrawingRequest& drawing_request, const uv::ShadingMode::Mode shading_mode)
	{
		if(shading_mode==uv::ShadingMode::simple)
		{
			if(drawing_request.atoms_cartoon)
			{
				if(!dc_atoms_cartoon_.valid() && data_manager_.is_any_atom_visible(dc_atoms_cartoon_.representation_id))
				{
					reset_drawing_atoms_cartoon();
					update_drawing_atoms();
				}
				dc_atoms_cartoon_.draw();
			}
			if(drawing_request.contacts_faces)
			{
				dc_contacts_faces_.draw();
			}
			if(drawing_request.contacts_sasmesh)
			{
				dc_contacts_sasmesh_.draw();
			}
			if(drawing_request.contacts_edges)
			{
				dc_contacts_edges_.draw();
			}
			if(drawing_request.contacts_skinshape)
			{
				dc_contacts_skinshape_.draw();
			}
			if(drawing_request.figures_solid)
			{
				dc_figures_solid_.draw();
			}
			if(drawing_request.figures_mesh)
			{
				dc_figures_mesh_.draw();
			}
		}
		else if(shading_mode==uv::ShadingMode::with_instancing)
		{
			if(drawing_request.atoms_balls)
			{
				dc_atoms_balls_spheres_.draw();
			}
			if(drawing_request.atoms_sticks)
			{
				dc_atoms_sticks_spheres_.draw();
				dc_atoms_sticks_cylinders_.draw();
			}
			if(drawing_request.atoms_trace)
			{
				dc_atoms_trace_cylinders_.draw();
				dc_atoms_trace_spheres_.draw();
			}
		}
		else if(shading_mode==uv::ShadingMode::with_impostoring)
		{
			if(drawing_request.atoms_points)
			{
				dc_atoms_points_.draw();
			}
		}
	}

	bool update()
	{
		reset_drawing_atoms();
		reset_drawing_contacts();
		reset_drawing_figures();
		update_drawing_atoms();
		update_drawing_contacts();
		update_drawing_figures();

		return true;
	}

	bool update(const scripting::DataManager::ChangeIndicator& ci)
	{
		bool updated=false;

		if(ci.changed_atoms())
		{
			reset_drawing_atoms();
			updated=true;
		}

		if(ci.changed_contacts())
		{
			reset_drawing_contacts();
			updated=true;
		}

		if(ci.changed_figures())
		{
			reset_drawing_figures();
			updated=true;
		}

		if(ci.changed_atoms_display_states())
		{
			update_drawing_atoms();
			updated=true;
		}

		if(ci.changed_contacts_display_states())
		{
			update_drawing_contacts();
			updated=true;
		}

		if(ci.changed_figures_display_states())
		{
			update_drawing_figures();
			updated=true;
		}

		return updated;
	}

	ElementID resolve_drawing_id(const uv::DrawingID drawing_id)
	{
		ElementID eid;
		{
			const std::size_t id=find_atom_id_by_drawing_id(drawing_id);
			if(id<data_manager_.atoms().size())
			{
				eid.data_manager_ptr=&data_manager_;
				eid.atom_id=id;
				return eid;
			}
		}
		{
			const std::size_t id=find_contact_id_by_drawing_id(drawing_id);
			if(id<data_manager_.contacts().size())
			{
				eid.data_manager_ptr=&data_manager_;
				eid.contact_id=id;
				return eid;
			}
		}
		{
			const std::size_t id=find_figure_id_by_drawing_id(drawing_id);
			if(id<data_manager_.figures().size())
			{
				eid.data_manager_ptr=&data_manager_;
				eid.figure_id=id;
				return eid;
			}
		}
		return eid;
	}

private:
	template<typename DrawingController>
	class WrapperForGenericDrawingController : private uv::Noncopyable
	{
	public:
		const std::size_t representation_id;
		DrawingController* controller_ptr;
		std::vector<uv::DrawingID> drawing_ids;
		std::map<uv::DrawingID, std::size_t> map_of_drawing_ids;

		explicit WrapperForGenericDrawingController(const std::size_t representation_id) :
			representation_id(representation_id),
			controller_ptr(0)
		{
		}

		~WrapperForGenericDrawingController()
		{
			unset();
		}

		bool valid() const
		{
			return (controller_ptr!=0);
		}

		void reset(const std::size_t number_of_drawing_ids)
		{
			unset();
			controller_ptr=new DrawingController();
			drawing_ids.resize(number_of_drawing_ids, 0);
		}

		void unset()
		{
			if(controller_ptr!=0)
			{
				delete controller_ptr;
				controller_ptr=0;
			}
			drawing_ids.clear();
			map_of_drawing_ids.clear();
		}

		void draw()
		{
			if(controller_ptr!=0)
			{
				controller_ptr->draw();
			}
		}
	};

	typedef WrapperForGenericDrawingController<uv::DrawingController> WrappedDrawingController;
	typedef WrapperForGenericDrawingController<uv::DrawingWithInstancingController> WrappedDrawingWithInstancingController;
	typedef WrapperForGenericDrawingController<uv::DrawingWithImpostoringController> WrappedDrawingWithImpostoringController;

	void reset_drawing_atoms()
	{
		if(!data_manager_.atoms().empty())
		{
			data_manager_.reset_bonding_links_info_by_creating(common::ConstructionOfBondingLinks::ParametersToConstructBundleOfBondingLinks());
		}

		reset_drawing_atoms_balls();
		reset_drawing_atoms_sticks();
		reset_drawing_atoms_trace();
		//reset_drawing_atoms_cartoon();
		reset_drawing_atoms_points();
	}

	void reset_drawing_atoms_balls()
	{
		dc_atoms_balls_spheres_.unset();
		if(!data_manager_.atoms().empty())
		{
			const std::size_t number_of_atoms=data_manager_.atoms().size();
			dc_atoms_balls_spheres_.reset(number_of_atoms);
			if(dc_atoms_balls_spheres_.controller_ptr->init_as_sphere(rendering_parameters_.ball_sphere_quality))
			{
				std::vector<bool> drawing_statuses(number_of_atoms, false);
				for(std::size_t i=0;i<number_of_atoms;i++)
				{
					const float x=static_cast<float>(data_manager_.atoms()[i].value.x);
					const float y=static_cast<float>(data_manager_.atoms()[i].value.y);
					const float z=static_cast<float>(data_manager_.atoms()[i].value.z);
					const float r=static_cast<float>(data_manager_.atoms()[i].value.r);
					const uv::DrawingID drawing_id=uv::get_free_drawing_id();
					dc_atoms_balls_spheres_.drawing_ids[i]=drawing_id;
					dc_atoms_balls_spheres_.map_of_drawing_ids[drawing_id]=i;
					dc_atoms_balls_spheres_.controller_ptr->object_register(drawing_id);
					dc_atoms_balls_spheres_.controller_ptr->object_set_transformation_as_for_sphere(drawing_id, r, glm::vec3(x, y, z));
					drawing_statuses[i]=true;
				}
				data_manager_.set_atoms_representation_implemented(dc_atoms_balls_spheres_.representation_id, drawing_statuses);
			}
		}
	}

	void reset_drawing_atoms_sticks()
	{
		dc_atoms_sticks_spheres_.unset();
		dc_atoms_sticks_cylinders_.unset();
		if(!data_manager_.atoms().empty() && !data_manager_.bonding_links_info().bonds_links.empty())
		{
			const std::size_t number_of_atoms=data_manager_.atoms().size();
			const std::size_t number_of_links=data_manager_.bonding_links_info().bonds_links.size();
			dc_atoms_sticks_spheres_.reset(number_of_atoms);
			dc_atoms_sticks_cylinders_.reset(number_of_links);
			if(dc_atoms_sticks_spheres_.controller_ptr->init_as_sphere(rendering_parameters_.stick_sphere_quality)
					&& dc_atoms_sticks_cylinders_.controller_ptr->init_as_cylinder(rendering_parameters_.stick_cylinder_quality))
			{
				std::vector<bool> drawing_statuses(number_of_atoms, false);
				for(std::size_t i=0;i<number_of_atoms;i++)
				{
					const float x=static_cast<float>(data_manager_.atoms()[i].value.x);
					const float y=static_cast<float>(data_manager_.atoms()[i].value.y);
					const float z=static_cast<float>(data_manager_.atoms()[i].value.z);
					const uv::DrawingID drawing_id=uv::get_free_drawing_id();
					dc_atoms_sticks_spheres_.drawing_ids[i]=drawing_id;
					dc_atoms_sticks_spheres_.map_of_drawing_ids[drawing_id]=i;
					dc_atoms_sticks_spheres_.controller_ptr->object_register(drawing_id);
					dc_atoms_sticks_spheres_.controller_ptr->object_set_transformation_as_for_sphere(drawing_id, 0.3, glm::vec3(x, y, z));
					drawing_statuses[i]=true;
				}
				for(std::size_t i=0;i<number_of_links;i++)
				{
					const common::ConstructionOfBondingLinks::DirectedLink& dl=data_manager_.bonding_links_info().bonds_links[i];
					const float x1=static_cast<float>(data_manager_.atoms()[dl.a].value.x);
					const float y1=static_cast<float>(data_manager_.atoms()[dl.a].value.y);
					const float z1=static_cast<float>(data_manager_.atoms()[dl.a].value.z);
					const float x2=static_cast<float>(data_manager_.atoms()[dl.b].value.x);
					const float y2=static_cast<float>(data_manager_.atoms()[dl.b].value.y);
					const float z2=static_cast<float>(data_manager_.atoms()[dl.b].value.z);
					const uv::DrawingID drawing_id=uv::get_free_drawing_id();
					dc_atoms_sticks_cylinders_.drawing_ids[i]=drawing_id;
					dc_atoms_sticks_cylinders_.map_of_drawing_ids[drawing_id]=i;
					dc_atoms_sticks_cylinders_.controller_ptr->object_register(drawing_id);
					if(dl.a<dl.b)
					{
						dc_atoms_sticks_cylinders_.controller_ptr->object_set_transformation_as_for_cylinder(drawing_id, 0.3, glm::vec3(x1, y1, z1), glm::vec3((x1+x2)*0.5f, (y1+y2)*0.5f, (z1+z2)*0.5f));
					}
					else
					{
						dc_atoms_sticks_cylinders_.controller_ptr->object_set_transformation_as_for_cylinder(drawing_id, 0.3, glm::vec3((x1+x2)*0.5f, (y1+y2)*0.5f, (z1+z2)*0.5f), glm::vec3(x1, y1, z1));
					}
				}
				data_manager_.set_atoms_representation_implemented(dc_atoms_sticks_spheres_.representation_id, drawing_statuses);
			}
		}
	}

	void reset_drawing_atoms_trace()
	{
		dc_atoms_trace_spheres_.unset();
		dc_atoms_trace_cylinders_.unset();
		if(!data_manager_.atoms().empty() && !data_manager_.bonding_links_info().residue_trace_links.empty())
		{
			const std::size_t number_of_atoms=data_manager_.atoms().size();
			const std::size_t number_of_links=data_manager_.bonding_links_info().residue_trace_links.size();
			dc_atoms_trace_spheres_.reset(number_of_atoms);
			dc_atoms_trace_cylinders_.reset(number_of_links);
			if(dc_atoms_trace_spheres_.controller_ptr->init_as_sphere(rendering_parameters_.trace_sphere_quality)
					&& dc_atoms_trace_cylinders_.controller_ptr->init_as_cylinder(rendering_parameters_.trace_cylinder_quality))
			{
				std::vector<bool> drawing_statuses(number_of_atoms, false);
				for(std::size_t i=0;i<number_of_atoms;i++)
				{
					if(!data_manager_.bonding_links_info().map_of_atoms_to_residue_trace_links[i].empty())
					{
						const float x=static_cast<float>(data_manager_.atoms()[i].value.x);
						const float y=static_cast<float>(data_manager_.atoms()[i].value.y);
						const float z=static_cast<float>(data_manager_.atoms()[i].value.z);
						const uv::DrawingID drawing_id=uv::get_free_drawing_id();
						dc_atoms_trace_spheres_.drawing_ids[i]=drawing_id;
						dc_atoms_trace_spheres_.map_of_drawing_ids[drawing_id]=i;
						dc_atoms_trace_spheres_.controller_ptr->object_register(drawing_id);
						dc_atoms_trace_spheres_.controller_ptr->object_set_transformation_as_for_sphere(drawing_id, 0.5, glm::vec3(x, y, z));
						drawing_statuses[i]=true;
					}
				}
				for(std::size_t i=0;i<number_of_links;i++)
				{
					const common::ConstructionOfBondingLinks::DirectedLink& dl=data_manager_.bonding_links_info().residue_trace_links[i];
					const float x1=static_cast<float>(data_manager_.atoms()[dl.a].value.x);
					const float y1=static_cast<float>(data_manager_.atoms()[dl.a].value.y);
					const float z1=static_cast<float>(data_manager_.atoms()[dl.a].value.z);
					const float x2=static_cast<float>(data_manager_.atoms()[dl.b].value.x);
					const float y2=static_cast<float>(data_manager_.atoms()[dl.b].value.y);
					const float z2=static_cast<float>(data_manager_.atoms()[dl.b].value.z);
					const uv::DrawingID drawing_id=uv::get_free_drawing_id();
					dc_atoms_trace_cylinders_.drawing_ids[i]=drawing_id;
					dc_atoms_trace_cylinders_.map_of_drawing_ids[drawing_id]=i;
					dc_atoms_trace_cylinders_.controller_ptr->object_register(drawing_id);
					if(dl.a<dl.b)
					{
						dc_atoms_trace_cylinders_.controller_ptr->object_set_transformation_as_for_cylinder(drawing_id, 0.5, glm::vec3(x1, y1, z1), glm::vec3((x1+x2)*0.5f, (y1+y2)*0.5f, (z1+z2)*0.5f));
					}
					else
					{
						dc_atoms_trace_cylinders_.controller_ptr->object_set_transformation_as_for_cylinder(drawing_id, 0.5, glm::vec3((x1+x2)*0.5f, (y1+y2)*0.5f, (z1+z2)*0.5f), glm::vec3(x1, y1, z1));
					}
				}
				data_manager_.set_atoms_representation_implemented(dc_atoms_trace_spheres_.representation_id, drawing_statuses);
			}
		}
	}

	void reset_drawing_atoms_cartoon()
	{
		dc_atoms_cartoon_.unset();
		if(!data_manager_.atoms().empty() && data_manager_.bonding_links_info().valid(data_manager_.atoms(), data_manager_.primary_structure_info()))
		{
			common::ConstructionOfStructuralCartoon::BundleOfMeshInformation bundle;
			if(common::ConstructionOfStructuralCartoon::construct_bundle_of_mesh_information(
					common::ConstructionOfStructuralCartoon::Parameters::styled(rendering_parameters_.cartoon_style),
					data_manager_.atoms(),
					data_manager_.primary_structure_info(),
					data_manager_.secondary_structure_info(),
					data_manager_.bonding_links_info(),
					bundle))
			{
				const std::size_t number_of_atoms=data_manager_.atoms().size();
				dc_atoms_cartoon_.reset(number_of_atoms);
				if(dc_atoms_cartoon_.controller_ptr->init(bundle.global_buffer_of_vertices, bundle.global_buffer_of_normals, bundle.global_buffer_of_indices))
				{
					std::vector<bool> drawing_statuses(number_of_atoms, false);
					for(std::size_t i=0;i<number_of_atoms;i++)
					{
						if(!bundle.mapped_indices[i].empty())
						{
							const uv::DrawingID drawing_id=uv::get_free_drawing_id();
							dc_atoms_cartoon_.drawing_ids[i]=drawing_id;
							dc_atoms_cartoon_.map_of_drawing_ids[drawing_id]=i;
							dc_atoms_cartoon_.controller_ptr->object_register(drawing_id, bundle.mapped_indices[i]);
							drawing_statuses[i]=true;
						}
					}
					data_manager_.set_atoms_representation_implemented(dc_atoms_cartoon_.representation_id, drawing_statuses);
				}
			}
		}
	}

	void reset_drawing_atoms_points()
	{
		dc_atoms_points_.unset();
		if(!data_manager_.atoms().empty())
		{
			const std::size_t number_of_atoms=data_manager_.atoms().size();
			dc_atoms_points_.reset(number_of_atoms);
			std::vector<float> data(number_of_atoms*4, 0.0f);
			for(std::size_t i=0;i<number_of_atoms;i++)
			{
				data[i*4+0]=static_cast<float>(data_manager_.atoms()[i].value.x);
				data[i*4+1]=static_cast<float>(data_manager_.atoms()[i].value.y);
				data[i*4+2]=static_cast<float>(data_manager_.atoms()[i].value.z);
				data[i*4+3]=static_cast<float>(data_manager_.atoms()[i].value.r);
			}
			if(dc_atoms_points_.controller_ptr->init(data, data))
			{
				std::vector<bool> drawing_statuses(number_of_atoms, false);
				for(std::size_t i=0;i<number_of_atoms;i++)
				{
					const uv::DrawingID drawing_id=uv::get_free_drawing_id();
					dc_atoms_points_.drawing_ids[i]=drawing_id;
					dc_atoms_points_.map_of_drawing_ids[drawing_id]=i;
					dc_atoms_points_.controller_ptr->object_register(drawing_id, i);
					drawing_statuses[i]=true;
				}
				data_manager_.set_atoms_representation_implemented(dc_atoms_points_.representation_id, drawing_statuses);
			}
		}
	}

	void reset_drawing_contacts()
	{
		dc_contacts_faces_.unset();
		dc_contacts_sasmesh_.unset();
		dc_contacts_edges_.unset();
		dc_contacts_skinshape_.unset();
		if(!data_manager_.contacts().empty())
		{
			common::ConstructionOfContacts::BundleOfContactsMeshInformation bundle;
			if(common::ConstructionOfContacts::construct_bundle_of_contacts_mesh_information(data_manager_.contacts(), false, bundle))
			{
				const std::size_t number_of_contacts=data_manager_.contacts().size();
				dc_contacts_faces_.reset(number_of_contacts);
				dc_contacts_sasmesh_.reset(number_of_contacts);
				dc_contacts_edges_.reset(number_of_contacts);
				if(dc_contacts_faces_.controller_ptr->init(bundle.global_buffer_of_vertices, bundle.global_buffer_of_normals, bundle.global_buffer_of_indices))
				{
					std::vector<bool> drawing_statuses(number_of_contacts, false);
					for(std::size_t i=0;i<number_of_contacts;i++)
					{
						if(!bundle.mapped_indices[i].empty())
						{
							const uv::DrawingID drawing_id=uv::get_free_drawing_id();
							dc_contacts_faces_.drawing_ids[i]=drawing_id;
							dc_contacts_faces_.map_of_drawing_ids[drawing_id]=i;
							dc_contacts_faces_.controller_ptr->object_register(drawing_id, bundle.mapped_indices[i]);
							drawing_statuses[i]=true;
						}
					}
					data_manager_.set_contacts_representation_implemented(dc_contacts_faces_.representation_id, drawing_statuses);
				}
				if(dc_contacts_sasmesh_.controller_ptr->init(bundle.global_buffer_of_vertices, bundle.global_buffer_of_normals, bundle.global_buffer_of_indices))
				{
					std::vector<bool> drawing_statuses(number_of_contacts, false);
					for(std::size_t i=0;i<number_of_contacts;i++)
					{
						if(!bundle.mapped_indices[i].empty() && data_manager_.contacts()[i].solvent())
						{
							const uv::DrawingID drawing_id=uv::get_free_drawing_id();
							dc_contacts_sasmesh_.drawing_ids[i]=drawing_id;
							dc_contacts_sasmesh_.map_of_drawing_ids[drawing_id]=i;
							dc_contacts_sasmesh_.controller_ptr->object_register(drawing_id, bundle.mapped_indices[i]);
							drawing_statuses[i]=true;
						}
					}
					data_manager_.set_contacts_representation_implemented(dc_contacts_sasmesh_.representation_id, drawing_statuses);
					dc_contacts_sasmesh_.controller_ptr->set_wire_mode(true);
				}
				if(dc_contacts_edges_.controller_ptr->init(bundle.global_buffer_of_vertices, bundle.global_buffer_of_normals, bundle.global_buffer_of_indices))
				{
					std::vector<bool> drawing_statuses(number_of_contacts, false);
					for(std::size_t i=0;i<number_of_contacts;i++)
					{
						if(!bundle.mapped_indices[i].empty() && !data_manager_.contacts()[i].solvent())
						{
							const uv::DrawingID drawing_id=uv::get_free_drawing_id();
							dc_contacts_edges_.drawing_ids[i]=drawing_id;
							dc_contacts_edges_.map_of_drawing_ids[drawing_id]=i;
							dc_contacts_edges_.controller_ptr->object_register(drawing_id, bundle.mapped_indices[i]);
							drawing_statuses[i]=true;
						}
					}
					data_manager_.set_contacts_representation_implemented(dc_contacts_edges_.representation_id, drawing_statuses);
					dc_contacts_edges_.controller_ptr->set_wire_mode(true);
					dc_contacts_edges_.controller_ptr->set_wire_mode_outline(true);
				}
			}
		}
		if(!data_manager_.contacts().empty())
		{
			common::ConstructionOfContacts::BundleOfContactsMeshInformation bundle;
			if(common::ConstructionOfContacts::construct_bundle_of_contacts_mesh_information(data_manager_.contacts(), true, bundle))
			{
				const std::size_t number_of_contacts=data_manager_.contacts().size();
				dc_contacts_skinshape_.reset(number_of_contacts);
				if(dc_contacts_skinshape_.controller_ptr->init(bundle.global_buffer_of_vertices, bundle.global_buffer_of_normals, bundle.global_buffer_of_indices))
				{
					std::vector<bool> drawing_statuses(number_of_contacts, false);
					for(std::size_t i=0;i<number_of_contacts;i++)
					{
						if(!bundle.mapped_indices[i].empty())
						{
							const uv::DrawingID drawing_id=uv::get_free_drawing_id();
							dc_contacts_skinshape_.drawing_ids[i]=drawing_id;
							dc_contacts_skinshape_.map_of_drawing_ids[drawing_id]=i;
							dc_contacts_skinshape_.controller_ptr->object_register(drawing_id, bundle.mapped_indices[i]);
							drawing_statuses[i]=true;
						}
					}
					data_manager_.set_contacts_representation_implemented(dc_contacts_skinshape_.representation_id, drawing_statuses);
				}
			}
		}
	}

	void reset_drawing_figures()
	{
		dc_figures_solid_.unset();
		dc_figures_mesh_.unset();
		if(!data_manager_.figures().empty())
		{
			const std::size_t number_of_figures=data_manager_.figures().size();

			std::vector<float> global_buffer_of_vertices;
			std::vector<float> global_buffer_of_normals;
			std::vector<unsigned int> global_buffer_of_indices;
			std::vector< std::vector<unsigned int> > mapped_indices(number_of_figures);

			{
				std::size_t total_number_of_vertices=0;
				std::size_t total_number_of_indices=0;
				for(std::size_t i=0;i<number_of_figures;i++)
				{
					if(data_manager_.figures()[i].valid())
					{
						total_number_of_vertices+=data_manager_.figures()[i].vertices.size();
						total_number_of_indices+=data_manager_.figures()[i].indices.size();
					}
				}
				global_buffer_of_vertices.reserve(total_number_of_vertices);
				global_buffer_of_normals.reserve(total_number_of_vertices);
				global_buffer_of_indices.reserve(total_number_of_indices);
			}

			for(std::size_t i=0;i<number_of_figures;i++)
			{
				if(data_manager_.figures()[i].valid())
				{
					const std::size_t offset=global_buffer_of_vertices.size()/3;
					global_buffer_of_vertices.insert(global_buffer_of_vertices.end(), data_manager_.figures()[i].vertices.begin(), data_manager_.figures()[i].vertices.end());
					global_buffer_of_normals.insert(global_buffer_of_normals.end(), data_manager_.figures()[i].normals.begin(), data_manager_.figures()[i].normals.end());
					mapped_indices[i].reserve(data_manager_.figures()[i].indices.size());
					for(std::size_t j=0;j<data_manager_.figures()[i].indices.size();j++)
					{
						const std::size_t id=(data_manager_.figures()[i].indices[j]+offset);
						global_buffer_of_indices.push_back(id);
						mapped_indices[i].push_back(id);
					}
				}
			}

			{
				dc_figures_solid_.reset(number_of_figures);
				dc_figures_mesh_.reset(number_of_figures);
				if(dc_figures_solid_.controller_ptr->init(global_buffer_of_vertices, global_buffer_of_normals, global_buffer_of_indices))
				{
					std::vector<bool> drawing_statuses(number_of_figures, false);
					for(std::size_t i=0;i<number_of_figures;i++)
					{
						if(!mapped_indices[i].empty())
						{
							const uv::DrawingID drawing_id=uv::get_free_drawing_id();
							dc_figures_solid_.drawing_ids[i]=drawing_id;
							dc_figures_solid_.map_of_drawing_ids[drawing_id]=i;
							dc_figures_solid_.controller_ptr->object_register(drawing_id, mapped_indices[i]);
							drawing_statuses[i]=true;
						}
					}
					data_manager_.set_figures_representation_implemented(dc_figures_solid_.representation_id, drawing_statuses);
				}
				if(dc_figures_mesh_.controller_ptr->init(global_buffer_of_vertices, global_buffer_of_normals, global_buffer_of_indices))
				{
					std::vector<bool> drawing_statuses(number_of_figures, false);
					for(std::size_t i=0;i<number_of_figures;i++)
					{
						if(!mapped_indices[i].empty())
						{
							const uv::DrawingID drawing_id=uv::get_free_drawing_id();
							dc_figures_mesh_.drawing_ids[i]=drawing_id;
							dc_figures_mesh_.map_of_drawing_ids[drawing_id]=i;
							dc_figures_mesh_.controller_ptr->object_register(drawing_id, mapped_indices[i]);
							drawing_statuses[i]=true;
						}
					}
					data_manager_.set_figures_representation_implemented(dc_figures_mesh_.representation_id, drawing_statuses);
					dc_figures_mesh_.controller_ptr->set_wire_mode(true);
				}
			}
		}
	}

	void update_drawing_atom(const std::size_t i)
	{
		if(i<data_manager_.atoms_display_states().size())
		{
			const scripting::DataManager::DisplayState& ds=data_manager_.atoms_display_states()[i];
			if(ds.implemented())
			{
				if(dc_atoms_balls_spheres_.valid() && ds.visuals[dc_atoms_balls_spheres_.representation_id].implemented)
				{
					const uv::DrawingID drawing_id=dc_atoms_balls_spheres_.drawing_ids[i];
					if(drawing_id>0)
					{
						const std::size_t rep_id=dc_atoms_balls_spheres_.representation_id;
						dc_atoms_balls_spheres_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
						dc_atoms_balls_spheres_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
						dc_atoms_balls_spheres_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
					}
				}

				if(dc_atoms_sticks_spheres_.valid() && dc_atoms_sticks_cylinders_.valid() && ds.visuals[dc_atoms_sticks_spheres_.representation_id].implemented)
				{
					const std::size_t rep_id=dc_atoms_sticks_spheres_.representation_id;
					{
						const uv::DrawingID drawing_id=dc_atoms_sticks_spheres_.drawing_ids[i];
						if(drawing_id>0)
						{
							dc_atoms_sticks_spheres_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
							dc_atoms_sticks_spheres_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
							dc_atoms_sticks_spheres_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
						}
					}
					for(std::size_t j=0;j<data_manager_.bonding_links_info().map_of_atoms_to_bonds_links[i].size();j++)
					{
						const uv::DrawingID drawing_id=dc_atoms_sticks_cylinders_.drawing_ids[data_manager_.bonding_links_info().map_of_atoms_to_bonds_links[i][j]];
						if(drawing_id>0)
						{
							dc_atoms_sticks_cylinders_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
							dc_atoms_sticks_cylinders_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
							dc_atoms_sticks_cylinders_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
						}
					}
				}

				if(dc_atoms_trace_spheres_.valid() && dc_atoms_trace_cylinders_.valid() && ds.visuals[dc_atoms_trace_spheres_.representation_id].implemented)
				{
					const std::size_t rep_id=dc_atoms_trace_spheres_.representation_id;
					{
						const uv::DrawingID drawing_id=dc_atoms_trace_spheres_.drawing_ids[i];
						if(drawing_id>0)
						{
							dc_atoms_trace_spheres_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
							dc_atoms_trace_spheres_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
							dc_atoms_trace_spheres_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
						}
					}
					for(std::size_t j=0;j<data_manager_.bonding_links_info().map_of_atoms_to_residue_trace_links[i].size();j++)
					{
						const uv::DrawingID drawing_id=dc_atoms_trace_cylinders_.drawing_ids[data_manager_.bonding_links_info().map_of_atoms_to_residue_trace_links[i][j]];
						if(drawing_id>0)
						{
							dc_atoms_trace_cylinders_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
							dc_atoms_trace_cylinders_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
							dc_atoms_trace_cylinders_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
						}
					}
				}

				if(dc_atoms_cartoon_.valid() && ds.visuals[dc_atoms_cartoon_.representation_id].implemented)
				{
					const uv::DrawingID drawing_id=dc_atoms_cartoon_.drawing_ids[i];
					if(drawing_id>0)
					{
						const std::size_t rep_id=dc_atoms_cartoon_.representation_id;
						dc_atoms_cartoon_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
						dc_atoms_cartoon_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
						dc_atoms_cartoon_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
					}
				}

				if(dc_atoms_points_.valid() && ds.visuals[dc_atoms_points_.representation_id].implemented)
				{
					const uv::DrawingID drawing_id=dc_atoms_points_.drawing_ids[i];
					if(drawing_id>0)
					{
						const std::size_t rep_id=dc_atoms_points_.representation_id;
						dc_atoms_points_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
						dc_atoms_points_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
						dc_atoms_points_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
					}
				}
			}
		}
	}

	void update_drawing_atoms()
	{
		for(std::size_t i=0;i<data_manager_.atoms_display_states().size();i++)
		{
			update_drawing_atom(i);
		}
	}

	template<typename T>
	void update_drawing_atoms(const T& set_of_ids)
	{
		for(typename T::const_iterator it=set_of_ids.begin();it!=set_of_ids.end();++it)
		{
			update_drawing_atom(*it);
		}
	}

	void update_drawing_contact(const std::size_t i)
	{
		if(i<data_manager_.contacts_display_states().size())
		{
			const scripting::DataManager::DisplayState& ds=data_manager_.contacts_display_states()[i];
			if(ds.implemented())
			{
				if(dc_contacts_faces_.valid() && ds.visuals[dc_contacts_faces_.representation_id].implemented)
				{
					const uv::DrawingID drawing_id=dc_contacts_faces_.drawing_ids[i];
					if(drawing_id>0)
					{
						const std::size_t rep_id=dc_contacts_faces_.representation_id;
						dc_contacts_faces_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
						dc_contacts_faces_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
						dc_contacts_faces_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
					}
				}

				if(dc_contacts_sasmesh_.valid() && ds.visuals[dc_contacts_sasmesh_.representation_id].implemented)
				{
					const uv::DrawingID drawing_id=dc_contacts_sasmesh_.drawing_ids[i];
					if(drawing_id>0)
					{
						const std::size_t rep_id=dc_contacts_sasmesh_.representation_id;
						dc_contacts_sasmesh_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
						dc_contacts_sasmesh_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
						dc_contacts_sasmesh_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
					}
				}

				if(dc_contacts_edges_.valid() && ds.visuals[dc_contacts_edges_.representation_id].implemented)
				{
					const uv::DrawingID drawing_id=dc_contacts_edges_.drawing_ids[i];
					if(drawing_id>0)
					{
						const std::size_t rep_id=dc_contacts_edges_.representation_id;
						dc_contacts_edges_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
						dc_contacts_edges_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
						dc_contacts_edges_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 1.0, 0.0);
					}
				}

				if(dc_contacts_skinshape_.valid() && ds.visuals[dc_contacts_skinshape_.representation_id].implemented)
				{
					const uv::DrawingID drawing_id=dc_contacts_skinshape_.drawing_ids[i];
					if(drawing_id>0)
					{
						const std::size_t rep_id=dc_contacts_skinshape_.representation_id;
						dc_contacts_skinshape_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
						dc_contacts_skinshape_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
						dc_contacts_skinshape_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
					}
				}
			}
		}
	}

	void update_drawing_contacts()
	{
		for(std::size_t i=0;i<data_manager_.contacts_display_states().size();i++)
		{
			update_drawing_contact(i);
		}
	}

	template<typename T>
	void update_drawing_contacts(const T& set_of_ids)
	{
		for(typename T::const_iterator it=set_of_ids.begin();it!=set_of_ids.end();++it)
		{
			update_drawing_contact(*it);
		}
	}

	void update_drawing_figure(const std::size_t i)
	{
		if(i<data_manager_.figures_display_states().size())
		{
			const scripting::DataManager::DisplayState& ds=data_manager_.figures_display_states()[i];
			if(ds.implemented())
			{
				if(dc_figures_solid_.valid() && ds.visuals[dc_figures_solid_.representation_id].implemented)
				{
					const uv::DrawingID drawing_id=dc_figures_solid_.drawing_ids[i];
					if(drawing_id>0)
					{
						const std::size_t rep_id=dc_figures_solid_.representation_id;
						dc_figures_solid_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
						dc_figures_solid_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
						dc_figures_solid_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
					}
				}
				if(dc_figures_mesh_.valid() && ds.visuals[dc_figures_mesh_.representation_id].implemented)
				{
					const uv::DrawingID drawing_id=dc_figures_mesh_.drawing_ids[i];
					if(drawing_id>0)
					{
						const std::size_t rep_id=dc_figures_mesh_.representation_id;
						dc_figures_mesh_.controller_ptr->object_set_visible(drawing_id, ds.visuals[rep_id].visible);
						dc_figures_mesh_.controller_ptr->object_set_color(drawing_id, ds.visuals[rep_id].color);
						dc_figures_mesh_.controller_ptr->object_set_adjunct(drawing_id, ds.marked ? 1.0 : 0.0, 0.0, 0.0);
					}
				}
			}
		}
	}

	void update_drawing_figures()
	{
		for(std::size_t i=0;i<data_manager_.figures_display_states().size();i++)
		{
			update_drawing_figure(i);
		}
	}

	std::size_t find_atom_id_by_drawing_id(const uv::DrawingID drawing_id) const
	{
		typedef std::map<uv::DrawingID, std::size_t> Map;
		typedef std::map<uv::DrawingID, std::size_t>::const_iterator Iterator;

		{
			const Map& map=dc_atoms_balls_spheres_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		{
			const Map& map=dc_atoms_sticks_spheres_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		{
			const Map& map=dc_atoms_sticks_cylinders_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				if((it->second)<data_manager_.bonding_links_info().bonds_links.size())
				{
					return data_manager_.bonding_links_info().bonds_links[it->second].a;
				}
			}
		}

		{
			const Map& map=dc_atoms_trace_spheres_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		{
			const Map& map=dc_atoms_trace_cylinders_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				if((it->second)<data_manager_.bonding_links_info().residue_trace_links.size())
				{
					return data_manager_.bonding_links_info().residue_trace_links[it->second].a;
				}
			}
		}

		{
			const Map& map=dc_atoms_cartoon_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		{
			const Map& map=dc_atoms_points_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		return data_manager_.atoms().size();
	}

	std::size_t find_contact_id_by_drawing_id(const uv::DrawingID drawing_id) const
	{
		typedef std::map<uv::DrawingID, std::size_t> Map;
		typedef std::map<uv::DrawingID, std::size_t>::const_iterator Iterator;

		{
			const Map& map=dc_contacts_faces_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		{
			const Map& map=dc_contacts_sasmesh_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		{
			const Map& map=dc_contacts_edges_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		{
			const Map& map=dc_contacts_skinshape_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		return data_manager_.contacts().size();
	}

	std::size_t find_figure_id_by_drawing_id(const uv::DrawingID drawing_id) const
	{
		typedef std::map<uv::DrawingID, std::size_t> Map;
		typedef std::map<uv::DrawingID, std::size_t>::const_iterator Iterator;

		{
			const Map& map=dc_figures_solid_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		{
			const Map& map=dc_figures_mesh_.map_of_drawing_ids;
			Iterator it=map.find(drawing_id);
			if(it!=map.end())
			{
				return it->second;
			}
		}

		return data_manager_.figures().size();
	}

	scripting::DataManager& data_manager_;
	RenderingParameters rendering_parameters_;
	WrappedDrawingWithInstancingController dc_atoms_balls_spheres_;
	WrappedDrawingWithInstancingController dc_atoms_sticks_spheres_;
	WrappedDrawingWithInstancingController dc_atoms_sticks_cylinders_;
	WrappedDrawingWithInstancingController dc_atoms_trace_spheres_;
	WrappedDrawingWithInstancingController dc_atoms_trace_cylinders_;
	WrappedDrawingController dc_atoms_cartoon_;
	WrappedDrawingWithImpostoringController dc_atoms_points_;
	WrappedDrawingController dc_contacts_faces_;
	WrappedDrawingController dc_contacts_sasmesh_;
	WrappedDrawingController dc_contacts_edges_;
	WrappedDrawingController dc_contacts_skinshape_;
	WrappedDrawingController dc_figures_solid_;
	WrappedDrawingController dc_figures_mesh_;
};

}

}

#endif /* VIEWER_DRAWER_FOR_DATA_MANAGER_H_ */

