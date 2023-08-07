#ifndef SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_PYMOL_CGO_H_
#define SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_PYMOL_CGO_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportContactsAsPymolCGO : public OperatorBase<ExportContactsAsPymolCGO>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;
		SummaryOfContacts contacts_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			else
			{
				heterostorage.forwarding_strings["download"].push_back(file);
			}
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	std::string name;
	bool wireframe;
	bool fat_wireframe;
	bool fat_wireframe_peaks_only;
	double fat_wireframe_transform_a;
	double fat_wireframe_transform_b;
	double fat_wireframe_min_radius;
	double fat_wireframe_max_radius;
	SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> representation_names;
	std::string file;

	ExportContactsAsPymolCGO() :
		wireframe(false),
		fat_wireframe(false),
		fat_wireframe_peaks_only(false),
		fat_wireframe_transform_a(1.0),
		fat_wireframe_transform_b(0.0),
		fat_wireframe_min_radius(0.0),
		fat_wireframe_max_radius(std::numeric_limits<double>::max())
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value_or_default<std::string>("name", "contacts");
		wireframe=input.get_flag("wireframe");
		fat_wireframe=input.get_flag("fat-wireframe");
		fat_wireframe_peaks_only=input.get_flag("fat-wireframe-peaks-only");
		fat_wireframe_transform_a=input.get_value_or_default<double>("fat-wireframe-transform-a", 1.0);
		fat_wireframe_transform_b=input.get_value_or_default<double>("fat-wireframe-transform-b", 0.0);
		fat_wireframe_min_radius=input.get_value_or_default<double>("fat-wireframe-min-radius", 0.0);
		fat_wireframe_max_radius=input.get_value_or_default<double>("fat-wireframe-max-radius", std::numeric_limits<double>::max());
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		file=input.get_value<std::string>("file");
		assert_file_name_input(file, false);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "name of CGO object", "atoms"));
		doc.set_option_decription(CDOD("wireframe", CDOD::DATATYPE_BOOL, "flag use wireframe representation"));
		doc.set_option_decription(CDOD("fat-wireframe", CDOD::DATATYPE_BOOL, "flag use fat wireframe representation"));
		doc.set_option_decription(CDOD("fat-wireframe-peaks-only", CDOD::DATATYPE_BOOL, "flag use fat wireframe representation in peaks-only mode"));
		doc.set_option_decription(CDOD("fat-wireframe-transform-a", CDOD::DATATYPE_FLOAT, "fat wireframe radius display transform multiplier", "1"));
		doc.set_option_decription(CDOD("fat-wireframe-transform-b", CDOD::DATATYPE_FLOAT, "fat wireframe radius display transform addition", "0"));
		doc.set_option_decription(CDOD("fat-wireframe-min-radius", CDOD::DATATYPE_FLOAT, "fat wireframe minimum radius for display", "0"));
		doc.set_option_decription(CDOD("fat-wireframe-max-radius", CDOD::DATATYPE_FLOAT, "fat wireframe maximum radius for display", "+inf"));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("rep", CDOD::DATATYPE_STRING_ARRAY, "representation names", ""));
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_representations_availability();

		assert_file_name_input(file, false);

		if(name.empty())
		{
			throw std::runtime_error(std::string("Missing object name."));
		}

		std::set<std::size_t> representation_ids=data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

		if(representation_ids.empty())
		{
			representation_ids.insert(0);
		}

		if(representation_ids.size()>1)
		{
			throw std::runtime_error(std::string("More than one representation requested."));
		}

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		{
			common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts;
			parameters_to_draw_contacts.probe=data_manager.history_of_actions_on_contacts().probe();
			parameters_to_draw_contacts.enable_alt=(representation_ids.count(data_manager.contacts_representation_descriptor().id_by_name("skin-shape"))>0);
			data_manager.reset_contacts_graphics_by_creating(parameters_to_draw_contacts, ids, true);
		}

		auxiliaries::OpenGLPrinter opengl_printer;
		{
			unsigned int prev_color=0;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id=(*it);
				for(std::set<std::size_t>::const_iterator jt=representation_ids.begin();jt!=representation_ids.end();++jt)
				{
					const std::size_t visual_id=(*jt);
					if(visual_id<data_manager.contacts_display_states()[id].visuals.size())
					{
						const DataManager::DisplayState::Visual& dsv=data_manager.contacts_display_states()[id].visuals[visual_id];
						if(prev_color==0 || dsv.color!=prev_color)
						{
							opengl_printer.add_color(dsv.color);
						}
						prev_color=dsv.color;
						if(wireframe || fat_wireframe)
						{
							if(wireframe)
							{
								opengl_printer.add_as_wireframe(data_manager.contacts()[id].value.graphics);
							}
							if(fat_wireframe)
							{
								opengl_printer.add_as_fat_wireframe(
										data_manager.contacts()[id].value.graphics,
										FatWireframeSphereGenerator(data_manager.atoms()[data_manager.contacts()[id].ids[0]].value, fat_wireframe_min_radius, fat_wireframe_max_radius),
										fat_wireframe_peaks_only,
										fat_wireframe_transform_a,
										fat_wireframe_transform_b);
							}
						}
						else
						{
							opengl_printer.add(data_manager.contacts()[id].value.graphics);
						}
					}
				}
			}
		}

		OutputSelector output_selector(file);

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);
			opengl_printer.print_pymol_script(name, true, output);
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}

private:
	class FatWireframeSphereGenerator
	{
	public:
		typedef apollota::SimpleSphere ResultSphereType;

		template<typename Sphere>
		explicit FatWireframeSphereGenerator(
				const Sphere& touching_sphere,
				const double min_allowed_radius,
				const double max_allowed_radius) :
					touching_sphere_(touching_sphere),
					min_allowed_radius_(min_allowed_radius),
					max_allowed_radius_(max_allowed_radius)
		{
		}

		template<typename Point>
		ResultSphereType operator()(const Point& point) const
		{
			const double radius=apollota::minimal_distance_from_point_to_sphere(point, touching_sphere_);
			return ResultSphereType(point, ((radius>=min_allowed_radius_ && radius<=max_allowed_radius_) ? radius : 0.0));
		}

	private:
		ResultSphereType touching_sphere_;
		double min_allowed_radius_;
		double max_allowed_radius_;
	};
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_PYMOL_CGO_H_ */
