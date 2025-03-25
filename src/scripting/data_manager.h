#ifndef SCRIPTING_DATA_MANAGER_H_
#define SCRIPTING_DATA_MANAGER_H_

#include "../auxiliaries/color_utilities.h"

#include "../common/construction_of_secondary_structure.h"
#include "../common/construction_of_bonding_links.h"

#include "selection_manager.h"

namespace voronota
{

namespace scripting
{

class DataManager
{
public:
	class ChangeIndicator
	{
	public:
		ChangeIndicator() :
			changed_atoms_(false),
			changed_contacts_(false),
			changed_figures_(false),
			changed_atoms_tags_(false),
			changed_contacts_tags_(false),
			changed_atoms_adjuncts_(false),
			changed_contacts_adjuncts_(false),
			changed_atoms_display_states_marking_(false),
			changed_atoms_display_states_coloring_(false),
			changed_atoms_display_states_visibility_(false),
			changed_atoms_display_states_(false),
			changed_contacts_display_states_marking_(false),
			changed_contacts_display_states_coloring_(false),
			changed_contacts_display_states_visibility_(false),
			changed_contacts_display_states_(false),
			changed_figures_display_states_marking_(false),
			changed_figures_display_states_coloring_(false),
			changed_figures_display_states_visibility_(false),
			changed_figures_display_states_(false)
		{
		}

		bool changed_atoms() const { return changed_atoms_; }
		bool changed_contacts() const { return changed_contacts_; }
		bool changed_figures() const { return changed_figures_; }
		bool changed_atoms_tags() const { return changed_atoms_tags_; }
		bool changed_contacts_tags() const { return changed_contacts_tags_; }
		bool changed_atoms_adjuncts() const { return changed_atoms_adjuncts_; }
		bool changed_contacts_adjuncts() const { return changed_contacts_adjuncts_; }
		bool changed_atoms_display_states_marking() const { return changed_atoms_display_states_marking_; }
		bool changed_atoms_display_states_coloring() const { return changed_atoms_display_states_coloring_; }
		bool changed_atoms_display_states_visibility() const { return changed_atoms_display_states_visibility_; }
		bool changed_atoms_display_states() const { return changed_atoms_display_states_; }
		bool changed_contacts_display_states_marking() const { return changed_contacts_display_states_marking_; }
		bool changed_contacts_display_states_coloring() const { return changed_contacts_display_states_coloring_; }
		bool changed_contacts_display_states_visibility() const { return changed_contacts_display_states_visibility_; }
		bool changed_contacts_display_states() const { return changed_contacts_display_states_; }
		bool changed_figures_display_states_marking() const { return changed_figures_display_states_marking_; }
		bool changed_figures_display_states_coloring() const { return changed_figures_display_states_coloring_; }
		bool changed_figures_display_states_visibility() const { return changed_figures_display_states_visibility_; }
		bool changed_figures_display_states() const { return changed_figures_display_states_; }

		void set_changed_atoms(const bool value) { changed_atoms_=value; ensure_correctness(); }
		void set_changed_contacts(const bool value) { changed_contacts_=value; ensure_correctness(); }
		void set_changed_figures(const bool value) { changed_figures_=value; ensure_correctness(); }
		void set_changed_atoms_tags(const bool value) { changed_atoms_tags_=value; ensure_correctness(); }
		void set_changed_contacts_tags(const bool value) { changed_contacts_tags_=value; ensure_correctness(); }
		void set_changed_atoms_adjuncts(const bool value) { changed_atoms_adjuncts_=value; ensure_correctness(); }
		void set_changed_contacts_adjuncts(const bool value) { changed_contacts_adjuncts_=value; ensure_correctness(); }
		void set_changed_atoms_display_states_marking(const bool value) { changed_atoms_display_states_marking_=value; ensure_correctness(); }
		void set_changed_atoms_display_states_coloring(const bool value) { changed_atoms_display_states_coloring_=value; ensure_correctness(); }
		void set_changed_atoms_display_states_visibility(const bool value) { changed_atoms_display_states_visibility_=value; ensure_correctness(); }
		void set_changed_atoms_display_states(const bool value) { changed_atoms_display_states_=value; ensure_correctness(); }
		void set_changed_contacts_display_states_marking(const bool value) { changed_contacts_display_states_marking_=value; ensure_correctness(); }
		void set_changed_contacts_display_states_coloring(const bool value) { changed_contacts_display_states_coloring_=value; ensure_correctness(); }
		void set_changed_contacts_display_states_visibility(const bool value) { changed_contacts_display_states_visibility_=value; ensure_correctness(); }
		void set_changed_contacts_display_states(const bool value) { changed_contacts_display_states_=value; ensure_correctness(); }
		void set_changed_figures_display_states_marking(const bool value) { changed_figures_display_states_marking_=value; ensure_correctness(); }
		void set_changed_figures_display_states_coloring(const bool value) { changed_figures_display_states_coloring_=value; ensure_correctness(); }
		void set_changed_figures_display_states_visibility(const bool value) { changed_figures_display_states_visibility_=value; ensure_correctness(); }
		void set_changed_figures_display_states(const bool value) { changed_figures_display_states_=value; ensure_correctness(); }

		void update_changed_atoms_display_states_info(const bool marking, const bool coloring, const bool visibility)
		{
			changed_atoms_display_states_marking_=(changed_atoms_display_states_marking_ || marking);
			changed_atoms_display_states_coloring_=(changed_atoms_display_states_coloring_ || coloring);
			changed_atoms_display_states_visibility_=(changed_atoms_display_states_visibility_ || visibility);
			ensure_correctness();
		}

		void update_changed_contacts_display_states_info(const bool marking, const bool coloring, const bool visibility)
		{
			changed_contacts_display_states_marking_=(changed_contacts_display_states_marking_ || marking);
			changed_contacts_display_states_coloring_=(changed_contacts_display_states_coloring_ || coloring);
			changed_contacts_display_states_visibility_=(changed_contacts_display_states_visibility_ || visibility);
			ensure_correctness();
		}

		void update_changed_figures_display_states_info(const bool marking, const bool coloring, const bool visibility)
		{
			changed_figures_display_states_marking_=(changed_figures_display_states_marking_ || marking);
			changed_figures_display_states_coloring_=(changed_figures_display_states_coloring_ || coloring);
			changed_figures_display_states_visibility_=(changed_figures_display_states_visibility_ || visibility);
			ensure_correctness();
		}

		bool changed() const
		{
			return (changed_atoms_
					|| changed_contacts_
					|| changed_figures_
					|| changed_atoms_tags_
					|| changed_contacts_tags_
					|| changed_atoms_adjuncts_
					|| changed_contacts_adjuncts_
					|| changed_atoms_display_states_ || changed_atoms_display_states_marking_ || changed_atoms_display_states_coloring_ || changed_atoms_display_states_visibility_
					|| changed_contacts_display_states_ || changed_contacts_display_states_marking_ || changed_contacts_display_states_coloring_ || changed_contacts_display_states_visibility_
					|| changed_figures_display_states_ || changed_figures_display_states_marking_ || changed_figures_display_states_coloring_ || changed_figures_display_states_visibility_);
		}

	private:
		void ensure_correctness()
		{
			changed_contacts_=(changed_contacts_ || changed_atoms_);
			changed_atoms_tags_=(changed_atoms_tags_ || changed_atoms_);
			changed_contacts_tags_=(changed_contacts_tags_ || changed_contacts_);
			changed_atoms_adjuncts_=(changed_atoms_adjuncts_ || changed_atoms_);
			changed_contacts_adjuncts_=(changed_contacts_adjuncts_ || changed_contacts_);
			changed_atoms_display_states_marking_=(changed_atoms_display_states_marking_ || changed_atoms_);
			changed_atoms_display_states_coloring_=(changed_atoms_display_states_coloring_ || changed_atoms_);
			changed_atoms_display_states_visibility_=(changed_atoms_display_states_visibility_ || changed_atoms_);
			changed_atoms_display_states_=(changed_atoms_display_states_ || changed_atoms_ || changed_atoms_display_states_marking_ || changed_atoms_display_states_coloring_ || changed_atoms_display_states_visibility_);
			changed_contacts_display_states_marking_=(changed_contacts_display_states_marking_ || changed_contacts_);
			changed_contacts_display_states_coloring_=(changed_contacts_display_states_coloring_ || changed_contacts_);
			changed_contacts_display_states_visibility_=(changed_contacts_display_states_visibility_ || changed_contacts_);
			changed_contacts_display_states_=(changed_contacts_display_states_ || changed_contacts_ || changed_contacts_display_states_marking_ || changed_contacts_display_states_coloring_ || changed_contacts_display_states_visibility_);
			changed_figures_display_states_marking_=(changed_figures_display_states_marking_ || changed_figures_);
			changed_figures_display_states_coloring_=(changed_figures_display_states_coloring_ || changed_figures_);
			changed_figures_display_states_visibility_=(changed_figures_display_states_visibility_ || changed_figures_);
			changed_figures_display_states_=(changed_figures_display_states_ || changed_figures_ || changed_figures_display_states_marking_ || changed_figures_display_states_coloring_ || changed_figures_display_states_visibility_);
		}

		bool changed_atoms_;
		bool changed_contacts_;
		bool changed_figures_;
		bool changed_atoms_tags_;
		bool changed_contacts_tags_;
		bool changed_atoms_adjuncts_;
		bool changed_contacts_adjuncts_;
		bool changed_atoms_display_states_marking_;
		bool changed_atoms_display_states_coloring_;
		bool changed_atoms_display_states_visibility_;
		bool changed_atoms_display_states_;
		bool changed_contacts_display_states_marking_;
		bool changed_contacts_display_states_coloring_;
		bool changed_contacts_display_states_visibility_;
		bool changed_contacts_display_states_;
		bool changed_figures_display_states_marking_;
		bool changed_figures_display_states_coloring_;
		bool changed_figures_display_states_visibility_;
		bool changed_figures_display_states_;
	};

	struct RepresentationsDescriptor
	{
		std::vector<std::string> names;

		std::size_t id_by_name(const std::string& name) const
		{
			for(std::size_t i=0;i<names.size();i++)
			{
				if(names[i]==name)
				{
					return i;
				}
			}
			return names.size();
		}

		std::set<std::size_t> ids_by_names(const std::vector<std::string>& input_names) const
		{
			std::set<std::size_t> ids;
			for(std::size_t i=0;i<input_names.size();i++)
			{
				const std::size_t id=id_by_name(input_names[i]);
				if(id<names.size())
				{
					ids.insert(id);
				}
				else
				{
					throw std::runtime_error(std::string("Representation '")+input_names[i]+"' does not exist.");
				}
			}
			return ids;
		}
	};

	struct DisplayState
	{
		struct Visual
		{
			bool implemented;
			bool visible;
			unsigned int color;

			Visual() : implemented(false), visible(false), color(0x7F7F7F)
			{
			}

			bool operator<(const Visual& v) const
			{
				if(implemented<v.implemented) { return true; }
				else if(implemented==v.implemented)
				{
					if(visible<v.visible) { return true; }
					else if(visible==v.visible)
					{
						return (color<v.color);
					}
				}
				return false;
			}
		};

		bool drawable;
		bool marked;
		std::vector<Visual> visuals;

		DisplayState() : drawable(false), marked(false)
		{
		}

		bool visible(const std::size_t visual_id) const
		{
			return (drawable && visual_id<visuals.size() && visuals[visual_id].visible);
		}

		bool visible() const
		{
			bool result=false;
			if(drawable)
			{
				for(std::size_t i=0;i<visuals.size() && !result;i++)
				{
					result=(result || visuals[i].visible);
				}
			}
			return result;
		}

		bool implemented(const std::size_t visual_id) const
		{
			return (drawable && visual_id<visuals.size() && visuals[visual_id].implemented);
		}

		bool implemented() const
		{
			bool result=false;
			if(drawable)
			{
				for(std::size_t i=0;i<visuals.size() && !result;i++)
				{
					result=(result || visuals[i].implemented);
				}
			}
			return result;
		}

		bool operator<(const DisplayState& v) const
		{
			if(drawable<v.drawable) { return true; }
			else if(drawable==v.drawable)
			{
				if(marked<v.marked) { return true; }
				else if(marked==v.marked)
				{
					return (visuals<v.visuals);
				}
			}
			return false;
		}

		friend std::ostream& operator<<(std::ostream& output, const DisplayState& ds)
		{
			output << ds.drawable << " " << ds.marked << "\n";
			output << ds.visuals.size() << "\n";
			for(std::size_t i=0;i<ds.visuals.size();i++)
			{
				output << ds.visuals[i].color << " " << ds.visuals[i].implemented << " " << ds.visuals[i].visible << "\n";
			}
			return output;
		}

		friend std::istream& operator>>(std::istream& input, DisplayState& ds)
		{
			input >> ds.drawable >> ds.marked;
			{
				int n=0;
				input >> n;
				if(n>0)
				{
					ds.visuals.reserve(n);
					for(int i=0;i<n;i++)
					{
						Visual val;
						input >> val.color >> val.implemented >> val.visible;
						ds.visuals.push_back(val);
					}
				}
			}
			return input;
		}
	};

	class DisplayStateUpdater
	{
	public:
		struct UpdateStatus
		{
			bool updated_marking;
			bool updated_coloring;
			bool updated_visibility;

			UpdateStatus() : updated_marking(false), updated_coloring(false), updated_visibility(false)
			{
			}

			void add(const UpdateStatus& other)
			{
				updated_marking=(updated_marking || other.updated_marking);
				updated_coloring=(updated_coloring || other.updated_coloring);
				updated_visibility=(updated_visibility || other.updated_visibility);
			}

			bool updated() const
			{
				return (updated_marking || updated_coloring || updated_visibility);
			}
		};

		bool mark;
		bool unmark;
		bool show;
		bool hide;
		bool additive_color;
		auxiliaries::ColorUtilities::ColorInteger color;
		std::set<std::size_t> visual_ids;

		DisplayStateUpdater() :
			mark(false),
			unmark(false),
			show(false),
			hide(false),
			additive_color(false),
			color(auxiliaries::ColorUtilities::null_color())
		{
		}

		DisplayStateUpdater& set_mark(const bool value)
		{
			mark=value;
			return (*this);
		}

		DisplayStateUpdater& set_unmark(const bool value)
		{
			unmark=value;
			return (*this);
		}

		DisplayStateUpdater& set_show(const bool value)
		{
			show=value;
			return (*this);
		}

		DisplayStateUpdater& set_hide(const bool value)
		{
			hide=value;
			return (*this);
		}

		DisplayStateUpdater& set_color(const auxiliaries::ColorUtilities::ColorInteger value)
		{
			color=value;
			return (*this);
		}

		DisplayStateUpdater& set_visual_ids(const std::set<std::size_t>& value)
		{
			visual_ids=value;
			return (*this);
		}

		bool color_valid() const
		{
			return auxiliaries::ColorUtilities::color_valid(color);
		}

		void assert_correctness() const
		{
			if(hide && show)
			{
				throw std::runtime_error(std::string("Cannot show and hide at the same time."));
			}

			if(mark && unmark)
			{
				throw std::runtime_error(std::string("Cannot mark and unmark at the same time."));
			}
		}

		UpdateStatus update_display_state(DisplayState& ds) const
		{
			UpdateStatus update_status;
			if(show || hide || mark || unmark || color_valid())
			{
				if(mark || unmark)
				{
					update_status.updated_marking=(update_status.updated_marking || (ds.marked!=mark));
					ds.marked=mark;
				}

				{
					if(show || hide || color_valid())
					{
						if(visual_ids.empty())
						{
							for(std::size_t i=0;i<ds.visuals.size();i++)
							{
								update_status.add(update_display_state_visual(ds.visuals[i]));
							}
						}
						else
						{
							for(std::set<std::size_t>::const_iterator jt=visual_ids.begin();jt!=visual_ids.end();++jt)
							{
								const std::size_t visual_id=(*jt);
								if(visual_id<ds.visuals.size())
								{
									update_status.add(update_display_state_visual(ds.visuals[visual_id]));
								}
							}
						}
					}
				}
			}
			return update_status;
		}

		UpdateStatus update_display_state(const std::size_t id, std::vector<DisplayState>& display_states) const
		{
			assert_correctness();
			UpdateStatus update_status;
			if(id<display_states.size())
			{
				update_status.add(update_display_state(display_states[id]));
			}
			return update_status;
		}

		UpdateStatus update_display_states(const std::set<std::size_t>& ids, std::vector<DisplayState>& display_states) const
		{
			assert_correctness();
			UpdateStatus update_status;
			if(show || hide || mark || unmark || color_valid())
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					if((*it)<display_states.size())
					{
						update_status.add(update_display_state(display_states[*it]));
					}
				}
			}
			return update_status;
		}

		UpdateStatus update_display_states(std::vector<DisplayState>& display_states) const
		{
			assert_correctness();
			UpdateStatus update_status;
			if(show || hide || mark || unmark || color_valid())
			{
				for(std::size_t i=0;i<display_states.size();i++)
				{
					update_status.add(update_display_state(display_states[i]));
				}
			}
			return update_status;
		}

	private:
		UpdateStatus update_display_state_visual(DisplayState::Visual& visual) const
		{
			UpdateStatus update_status;

			{
				if(show || hide)
				{
					update_status.updated_visibility=(update_status.updated_visibility || (visual.visible!=show));
					visual.visible=show;
				}

				if(color_valid())
				{
					if(!additive_color)
					{
						update_status.updated_coloring=(update_status.updated_coloring || (visual.color!=color));
						visual.color=color;
					}
					else
					{
						auxiliaries::ColorUtilities::ColorInteger sum_of_colors=auxiliaries::ColorUtilities::color_sum(visual.color, color);
						update_status.updated_coloring=(update_status.updated_coloring || (visual.color!=sum_of_colors));
						visual.color=sum_of_colors;
					}
				}
			}

			return update_status;
		}
	};

	struct TransformationOfCoordinates
	{
		std::vector<double> pre_translation_vector;
		std::vector<double> rotation_matrix;
		std::vector<double> rotation_axis_and_angle;
		std::vector<double> rotation_three_angles;
		std::vector<double> rotation_ztwist_theta_phi;
		std::vector<double> post_translation_vector;
		double pre_translation_scale;
		double post_translation_scale;

		TransformationOfCoordinates() : pre_translation_scale(1.0), post_translation_scale(1.0)
		{
		}

		void assert_validity() const
		{
			if(pre_translation_vector.empty() && post_translation_vector.empty() && rotation_matrix.empty() && rotation_axis_and_angle.empty() && rotation_three_angles.empty() && rotation_ztwist_theta_phi.empty())
			{
				throw std::runtime_error(std::string("No transformations provided to transform atoms."));
			}

			if(!pre_translation_vector.empty() && pre_translation_vector.size()!=3)
			{
				throw std::runtime_error(std::string("Invalid translation-before vector provided to transform atoms."));
			}

			if(!post_translation_vector.empty() && post_translation_vector.size()!=3)
			{
				throw std::runtime_error(std::string("Invalid translation-after vector provided to transform atoms."));
			}

			if(!rotation_matrix.empty() && rotation_matrix.size()!=9)
			{
				throw std::runtime_error(std::string("Invalid rotation matrix provided to transform atoms."));
			}

			if(!rotation_axis_and_angle.empty() && rotation_axis_and_angle.size()!=4)
			{
				throw std::runtime_error(std::string("Invalid rotation axis and angle vector provided to transform atoms."));
			}

			if(!rotation_three_angles.empty() && rotation_three_angles.size()!=3)
			{
				throw std::runtime_error(std::string("Invalid rotation three angles vector provided to transform atoms."));
			}

			if(!rotation_ztwist_theta_phi.empty() && rotation_ztwist_theta_phi.size()!=3)
			{
				throw std::runtime_error(std::string("Invalid rotation ztwist-theta-phi vector provided to transform atoms."));
			}
		}

		void transform_coordinates_of_atoms(std::vector<Atom>& mutable_atoms, const std::set<std::size_t>& ids) const
		{
			if(mutable_atoms.empty() || ids.empty())
			{
				return;
			}

			if(*ids.rbegin()>=mutable_atoms.size())
			{
				throw std::runtime_error(std::string("Invalid ids provided to transform atoms."));
			}

			assert_validity();

			if(!pre_translation_vector.empty())
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					common::BallValue& ball=mutable_atoms[*it].value;
					ball.x+=pre_translation_vector[0]*pre_translation_scale;
					ball.y+=pre_translation_vector[1]*pre_translation_scale;
					ball.z+=pre_translation_vector[2]*pre_translation_scale;
				}
			}

			if(!rotation_matrix.empty())
			{
				const std::vector<double>& m=rotation_matrix;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					common::BallValue& ball=mutable_atoms[*it].value;
					const apollota::SimplePoint p(ball);
					ball.x=p.x*m[0]+p.y*m[1]+p.z*m[2];
					ball.y=p.x*m[3]+p.y*m[4]+p.z*m[5];
					ball.z=p.x*m[6]+p.y*m[7]+p.z*m[8];
				}
			}

			if(!rotation_axis_and_angle.empty())
			{
				const apollota::Rotation rotation(
						apollota::SimplePoint(rotation_axis_and_angle[0], rotation_axis_and_angle[1], rotation_axis_and_angle[2]),
						rotation_axis_and_angle[3]);

				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					common::BallValue& ball=mutable_atoms[*it].value;
					const apollota::SimplePoint p=rotation.rotate<apollota::SimplePoint>(ball);
					ball.x=p.x;
					ball.y=p.y;
					ball.z=p.z;
				}
			}

			if(!rotation_three_angles.empty())
			{
				const apollota::SimplePoint ox(1, 0, 0);
				const apollota::SimplePoint oz(0, 0, 1);
				const apollota::SimplePoint axis_step1=apollota::Rotation(oz, rotation_three_angles[0]).rotate<apollota::SimplePoint>(ox);
				const apollota::SimplePoint axis_step2=apollota::Rotation(oz&axis_step1, rotation_three_angles[1]).rotate<apollota::SimplePoint>(axis_step1);

				const apollota::Rotation rotation(axis_step2, rotation_three_angles[2]);

				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					common::BallValue& ball=mutable_atoms[*it].value;
					const apollota::SimplePoint p=rotation.rotate<apollota::SimplePoint>(ball);
					ball.x=p.x;
					ball.y=p.y;
					ball.z=p.z;
				}
			}

			if(!rotation_ztwist_theta_phi.empty())
			{
				const double degree_radians=apollota::pi_value()/180.0;
				const double z_twist=rotation_ztwist_theta_phi[0]*degree_radians;
				const double theta=rotation_ztwist_theta_phi[1]*degree_radians;
				const double phi=rotation_ztwist_theta_phi[2]*degree_radians;

				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					common::BallValue& ball=mutable_atoms[*it].value;

					const double post_z_twist_x=ball.x*std::cos(z_twist)-ball.y*std::sin(z_twist);
					const double post_z_twist_y=ball.x*std::sin(z_twist)+ball.y*std::cos(z_twist);
					const double post_z_twist_z=ball.z;

					const double post_theta_x=post_z_twist_z*std::sin(theta)+post_z_twist_x*std::cos(theta);
					const double post_theta_y=post_z_twist_y;
					const double post_theta_z=post_z_twist_z*std::cos(theta)-post_z_twist_x*std::sin(theta);

					ball.x=post_theta_x*std::cos(phi)-post_theta_y*std::sin(phi);
					ball.y=post_theta_x*std::sin(phi)+post_theta_y*std::cos(phi);
					ball.z=post_theta_z;
				}
			}

			if(!post_translation_vector.empty())
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					common::BallValue& ball=mutable_atoms[*it].value;
					ball.x+=post_translation_vector[0]*post_translation_scale;
					ball.y+=post_translation_vector[1]*post_translation_scale;
					ball.z+=post_translation_vector[2]*post_translation_scale;
				}
			}
		}

		void transform_coordinates_of_atoms(std::vector<Atom>& mutable_atoms) const
		{
			std::set<std::size_t> ids;
			for(std::size_t i=0;i<mutable_atoms.size();i++)
			{
				ids.insert(ids.end(), i);
			}
			transform_coordinates_of_atoms(mutable_atoms, ids);
		}
	};

	struct HistoryOfActionsOnContacts
	{
		std::vector<common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation> constructing;
		std::vector<common::ConstructionOfContacts::ParametersToEnhanceContacts> enhancing;
		std::map<std::size_t, common::ConstructionOfContacts::ParametersToDrawContacts> graphics_creating;

		void clear()
		{
			constructing.clear();
			enhancing.clear();
			graphics_creating.clear();
		}

		double probe() const
		{
			if(!constructing.empty())
			{
				return constructing.back().probe;
			}
			return common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation().probe;
		}

	};

	DataManager()
	{
		add_atoms_representation("balls");
		add_atoms_representation("sticks");
		add_atoms_representation("trace");
		add_atoms_representation("cartoon");
		add_atoms_representation("points");
		add_atoms_representation("molsurf");
		add_atoms_representation("molsurf-mesh");

		add_contacts_representation("faces");
		add_contacts_representation("sas-mesh");
		add_contacts_representation("edges");
		add_contacts_representation("skin-shape");

		add_figures_representation("solid");
		add_figures_representation("mesh");
	}

	DataManager(const DataManager& dm)
	{
		(*this)=dm;
	}

	virtual ~DataManager()
	{
	}

	DataManager& operator=(const DataManager& dm)
	{
		atoms_representations_descriptor_=dm.atoms_representations_descriptor_;
		contacts_representations_descriptor_=dm.contacts_representations_descriptor_;
		figures_representations_descriptor_=dm.figures_representations_descriptor_;
		atoms_=dm.atoms_;
		contacts_=dm.contacts_;
		contacts_adjacencies_=dm.contacts_adjacencies_;
		figures_=dm.figures_;
		global_numeric_adjuncts_=dm.global_numeric_adjuncts_;
		atoms_display_states_=dm.atoms_display_states_;
		contacts_display_states_=dm.contacts_display_states_;
		figures_display_states_=dm.figures_display_states_;
		primary_structure_info_=dm.primary_structure_info_;
		secondary_structure_info_=dm.secondary_structure_info_;
		bonding_links_info_=dm.bonding_links_info_;
		triangulation_info_=dm.triangulation_info_;
		selection_manager_=dm.selection_manager_.make_adjusted_copy(atoms_, contacts_);
		history_of_actions_on_contacts_=dm.history_of_actions_on_contacts_;
		return (*this);
	}

	const std::vector<Atom>& atoms() const
	{
		return atoms_;
	}

	const std::vector<Contact>& contacts() const
	{
		return contacts_;
	}

	const std::vector< std::map<std::size_t, double> >& contacts_adjacencies() const
	{
		return contacts_adjacencies_;
	}

	const std::vector<Figure>& figures() const
	{
		return figures_;
	}

	const std::map<std::string, double>& global_numeric_adjuncts() const
	{
		return global_numeric_adjuncts_;
	}

	const std::vector<DisplayState>& atoms_display_states() const
	{
		return atoms_display_states_;
	}

	const std::vector<DisplayState>& contacts_display_states() const
	{
		return contacts_display_states_;
	}

	const std::vector<DisplayState>& figures_display_states() const
	{
		return figures_display_states_;
	}

	const common::ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& primary_structure_info() const
	{
		return primary_structure_info_;
	}

	const common::ConstructionOfSecondaryStructure::BundleOfSecondaryStructure& secondary_structure_info() const
	{
		return secondary_structure_info_;
	}

	const common::ConstructionOfBondingLinks::BundleOfBondingLinks& bonding_links_info() const
	{
		return bonding_links_info_;
	}

	const common::ConstructionOfTriangulation::BundleOfTriangulationInformation& triangulation_info() const
	{
		return triangulation_info_;
	}

	const RepresentationsDescriptor& atoms_representation_descriptor() const
	{
		return atoms_representations_descriptor_;
	}

	const RepresentationsDescriptor& contacts_representation_descriptor() const
	{
		return contacts_representations_descriptor_;
	}

	const RepresentationsDescriptor& figures_representation_descriptor() const
	{
		return figures_representations_descriptor_;
	}

	const ChangeIndicator& change_indicator() const
	{
		return change_indicator_;
	}

	const HistoryOfActionsOnContacts& history_of_actions_on_contacts() const
	{
		return history_of_actions_on_contacts_;
	}

	void assert_atoms_representations_availability() const
	{
		if(atoms_representations_descriptor_.names.empty())
		{
			throw std::runtime_error(std::string("No atoms representations available."));
		}
	}

	void assert_atoms_availability() const
	{
		if(atoms_.empty())
		{
			throw std::runtime_error(std::string("No atoms available."));
		}
	}

	void assert_atoms_selections_availability() const
	{
		if(selection_manager_.map_of_atoms_selections().empty())
		{
			throw std::runtime_error(std::string("No atoms selections available."));
		}
	}

	void assert_atoms_selections_availability(const std::vector<std::string>& names) const
	{
		for(std::size_t i=0;i<names.size();i++)
		{
			if(selection_manager_.map_of_atoms_selections().count(names[i])==0)
			{
				throw std::runtime_error(std::string("Invalid atoms selection name '")+names[i]+"'.");
			}
		}
	}

	void assert_atoms_tag_availability(const std::string& tag) const
	{
		if(!is_any_atom_with_tag(tag))
		{
			throw std::runtime_error(std::string("No atoms with tag '")+tag+"'.");
		}
	}

	void assert_atoms_adjunct_availability(const std::string& adjunct) const
	{
		if(!is_any_atom_with_adjunct(adjunct))
		{
			throw std::runtime_error(std::string("No atoms with adjunct '")+adjunct+"'.");
		}
	}

	void assert_triangulation_info_availability() const
	{
		if(triangulation_info_.quadruples_map.empty())
		{
			throw std::runtime_error(std::string("No triangulation info available."));
		}
	}

	void assert_contacts_representations_availability() const
	{
		if(contacts_representations_descriptor_.names.empty())
		{
			throw std::runtime_error(std::string("No contacts representations available."));
		}
	}

	void assert_contacts_availability() const
	{
		if(contacts_.empty())
		{
			throw std::runtime_error(std::string("No contacts available."));
		}
	}

	void assert_contacts_adjacencies_availability() const
	{
		if(contacts_adjacencies_.empty())
		{
			throw std::runtime_error(std::string("No contacts adjacencies available."));
		}
	}

	void assert_contacts_selections_availability() const
	{
		if(selection_manager_.map_of_contacts_selections().empty())
		{
			throw std::runtime_error(std::string("No contacts selections available."));
		}
	}

	void assert_contacts_selections_availability(const std::vector<std::string>& names) const
	{
		for(std::size_t i=0;i<names.size();i++)
		{
			if(selection_manager_.map_of_contacts_selections().count(names[i])==0)
			{
				throw std::runtime_error(std::string("Invalid contacts selection name '")+names[i]+"'.");
			}
		}
	}

	void assert_contacts_tag_availability(const std::string& tag) const
	{
		if(!is_any_contact_with_tag(tag))
		{
			throw std::runtime_error(std::string("No contacts with tag '")+tag+"'.");
		}
	}

	void assert_contacts_adjunct_availability(const std::string& adjunct) const
	{
		if(!is_any_contact_with_adjunct(adjunct))
		{
			throw std::runtime_error(std::string("No contacts with adjunct '")+adjunct+"'.");
		}
	}

	void assert_figures_representations_availability() const
	{
		if(figures_representations_descriptor_.names.empty())
		{
			throw std::runtime_error(std::string("No figures representations available."));
		}
	}

	void assert_figures_availability() const
	{
		if(figures_.empty())
		{
			throw std::runtime_error(std::string("No figures available."));
		}
	}

	void assert_primary_structure_info_valid() const
	{
		if(!primary_structure_info_.valid(atoms_))
		{
			throw std::runtime_error(std::string("Invalid primary structure information."));
		}
	}

	bool is_any_atom_visible() const
	{
		for(std::size_t i=0;i<atoms_display_states_.size();i++)
		{
			if(atoms_display_states_[i].visible())
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_atom_visible(const std::size_t representation_id) const
	{
		for(std::size_t i=0;i<atoms_display_states_.size();i++)
		{
			if(atoms_display_states_[i].visible(representation_id))
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_atom_marked() const
	{
		for(std::size_t i=0;i<atoms_display_states_.size();i++)
		{
			if(atoms_display_states_[i].marked)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_atom_with_tag(const std::string& tag) const
	{
		for(std::size_t i=0;i<atoms_.size();i++)
		{
			if(atoms_[i].value.props.tags.count(tag)>0)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_atom_with_adjunct(const std::string& adjunct) const
	{
		for(std::size_t i=0;i<atoms_.size();i++)
		{
			if(atoms_[i].value.props.adjuncts.count(adjunct)>0)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_contact_visible() const
	{
		for(std::size_t i=0;i<contacts_display_states_.size();i++)
		{
			if(contacts_display_states_[i].visible())
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_contact_marked() const
	{
		for(std::size_t i=0;i<contacts_display_states_.size();i++)
		{
			if(contacts_display_states_[i].marked)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_contact_with_tag(const std::string& tag) const
	{
		for(std::size_t i=0;i<contacts_.size();i++)
		{
			if(contacts_[i].value.props.tags.count(tag)>0)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_contact_with_adjunct(const std::string& adjunct) const
	{
		for(std::size_t i=0;i<contacts_.size();i++)
		{
			if(contacts_[i].value.props.adjuncts.count(adjunct)>0)
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_figure_visible() const
	{
		for(std::size_t i=0;i<figures_display_states_.size();i++)
		{
			if(figures_display_states_[i].visible())
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_figure_marked() const
	{
		for(std::size_t i=0;i<figures_display_states_.size();i++)
		{
			if(figures_display_states_[i].marked)
			{
				return true;
			}
		}
		return false;
	}

	std::size_t count_residues_by_atom_ids(const std::set<std::size_t>& atom_ids) const
	{
		std::set<std::size_t> residue_ids;
		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			residue_ids.insert(primary_structure_info().map_of_atoms_to_residues.at(*it));
		}
		return residue_ids.size();
	}

	const std::map< std::size_t, std::map<std::size_t, double> > extract_subset_of_contacts_adjacencies(const std::set<std::size_t>& contact_ids) const
	{
		std::map< std::size_t, std::map<std::size_t, double> > result;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const std::size_t contact_id=(*it);
			if(contact_id<contacts_adjacencies_.size())
			{
				const std::map<std::size_t, double>& all_neighbors=contacts_adjacencies_[contact_id];
				std::map<std::size_t, double>& selected_neighbors=result[contact_id];
				for(std::map<std::size_t, double>::const_iterator jt=all_neighbors.begin();jt!=all_neighbors.end();++jt)
				{
					if(contact_ids.count(jt->first))
					{
						selected_neighbors[jt->first]=jt->second;
					}
				}
			}
		}
		return result;
	}

	std::map< std::string, std::vector<std::size_t> > generate_ids_for_of_labels(const std::set<std::size_t>& atom_ids, const bool with_residue_info, const bool with_atom_info) const
	{
		std::map< std::string, std::vector<std::size_t> > map_of_ids;

		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			const std::size_t atom_id=(*it);
			const Atom& atom=atoms()[atom_id];
			std::ostringstream idstream;
			idstream << atom.crad.chainID;
			if(with_residue_info)
			{
				idstream << "." << atom.crad.resSeq << atom.crad.iCode;
			}
			if(with_atom_info)
			{
				idstream << "." << atom.crad.name;
			}
			map_of_ids[idstream.str()].push_back(atom_id);
		}

		return map_of_ids;
	}

	SelectionManager& selection_manager()
	{
		return selection_manager_;
	}

	std::set<std::string>& atom_tags_mutable(const std::size_t id)
	{
		change_indicator_.set_changed_atoms_tags(true);
		return atoms_[id].value.props.tags;
	}

	std::map<std::string, double>& atom_adjuncts_mutable(const std::size_t id)
	{
		change_indicator_.set_changed_atoms_adjuncts(true);
		return atoms_[id].value.props.adjuncts;
	}

	std::set<std::string>& contact_tags_mutable(const std::size_t id)
	{
		change_indicator_.set_changed_contacts_tags(true);
		return contacts_[id].value.props.tags;
	}

	std::map<std::string, double>& contact_adjuncts_mutable(const std::size_t id)
	{
		change_indicator_.set_changed_contacts_adjuncts(true);
		return contacts_[id].value.props.adjuncts;
	}

	std::map<std::string, double>& global_numeric_adjuncts_mutable()
	{
		return global_numeric_adjuncts_;
	}

	bool set_atoms_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		if(set_representation_implemented(atoms_representations_descriptor_.names, representation_id, statuses, atoms_display_states_))
		{
			change_indicator_.set_changed_atoms_display_states(true);
			return true;
		}
		return false;
	}

	bool set_contacts_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		if(set_representation_implemented(contacts_representations_descriptor_.names, representation_id, statuses, contacts_display_states_))
		{
			change_indicator_.set_changed_contacts_display_states(true);
			return true;
		}
		return false;
	}

	bool set_figures_representation_implemented(const std::size_t representation_id, const std::vector<bool>& statuses)
	{
		if(set_representation_implemented(figures_representations_descriptor_.names, representation_id, statuses, figures_display_states_))
		{
			change_indicator_.set_changed_figures_display_states(true);
			return true;
		}
		return false;
	}

	void update_atoms_display_state(const DisplayStateUpdater& dsu, const std::size_t id)
	{
		const DisplayStateUpdater::UpdateStatus update_status=dsu.update_display_state(id, atoms_display_states_);
		if(update_status.updated())
		{
			change_indicator_.update_changed_atoms_display_states_info(update_status.updated_marking, update_status.updated_coloring, update_status.updated_visibility);
		}
	}

	void update_atoms_display_states(const DisplayStateUpdater& dsu, const std::set<std::size_t>& ids)
	{
		const DisplayStateUpdater::UpdateStatus update_status=dsu.update_display_states(ids, atoms_display_states_);
		if(update_status.updated())
		{
			change_indicator_.update_changed_atoms_display_states_info(update_status.updated_marking, update_status.updated_coloring, update_status.updated_visibility);
		}
	}

	void update_atoms_display_states(const DisplayStateUpdater& dsu)
	{
		const DisplayStateUpdater::UpdateStatus update_status=dsu.update_display_states(atoms_display_states_);
		if(update_status.updated())
		{
			change_indicator_.update_changed_atoms_display_states_info(update_status.updated_marking, update_status.updated_coloring, update_status.updated_visibility);
		}
	}

	void update_contacts_display_state(const DisplayStateUpdater& dsu, const std::size_t id)
	{
		const DisplayStateUpdater::UpdateStatus update_status=dsu.update_display_state(id, contacts_display_states_);
		if(update_status.updated())
		{
			change_indicator_.update_changed_contacts_display_states_info(update_status.updated_marking, update_status.updated_coloring, update_status.updated_visibility);
		}
	}

	void update_contacts_display_states(const DisplayStateUpdater& dsu, const std::set<std::size_t>& ids)
	{
		const DisplayStateUpdater::UpdateStatus update_status=dsu.update_display_states(ids, contacts_display_states_);
		if(update_status.updated())
		{
			change_indicator_.update_changed_contacts_display_states_info(update_status.updated_marking, update_status.updated_coloring, update_status.updated_visibility);
		}
	}

	void update_contacts_display_states(const DisplayStateUpdater& dsu)
	{
		const DisplayStateUpdater::UpdateStatus update_status=dsu.update_display_states(contacts_display_states_);
		if(update_status.updated())
		{
			change_indicator_.update_changed_contacts_display_states_info(update_status.updated_marking, update_status.updated_coloring, update_status.updated_visibility);
		}
	}

	void update_figures_display_state(const DisplayStateUpdater& dsu, const std::size_t id)
	{
		const DisplayStateUpdater::UpdateStatus update_status=dsu.update_display_state(id, figures_display_states_);
		if(update_status.updated())
		{
			change_indicator_.update_changed_figures_display_states_info(update_status.updated_marking, update_status.updated_coloring, update_status.updated_visibility);
		}
	}

	void update_figures_display_states(const DisplayStateUpdater& dsu, const std::set<std::size_t>& ids)
	{
		const DisplayStateUpdater::UpdateStatus update_status=dsu.update_display_states(ids, figures_display_states_);
		if(update_status.updated())
		{
			change_indicator_.update_changed_figures_display_states_info(update_status.updated_marking, update_status.updated_coloring, update_status.updated_visibility);
		}
	}

	void update_figures_display_states(const DisplayStateUpdater& dsu)
	{
		const DisplayStateUpdater::UpdateStatus update_status=dsu.update_display_states(figures_display_states_);
		if(update_status.updated())
		{
			change_indicator_.update_changed_figures_display_states_info(update_status.updated_marking, update_status.updated_coloring, update_status.updated_visibility);
		}
	}

	void reset_change_indicator()
	{
		change_indicator_=ChangeIndicator();
	}

	void reset_atoms_by_swapping(std::vector<Atom>& atoms)
	{
		if(atoms.empty())
		{
			throw std::runtime_error(std::string("No atoms to set."));
		}
		change_indicator_.set_changed_atoms(true);
		atoms_.swap(atoms);
		reset_atoms_display_states();
		reset_data_dependent_on_atoms();
	}

	void reset_atoms_by_copying(const std::vector<Atom>& atoms)
	{
		std::vector<Atom> atoms_copy=atoms;
		reset_atoms_by_swapping(atoms_copy);
	}

	void reset_atoms_display_states()
	{
		change_indicator_.update_changed_atoms_display_states_info(true, true, true);
		atoms_display_states_.clear();
		atoms_display_states_.resize(atoms_.size());
		for(std::size_t i=0;i<atoms_display_states_.size();i++)
		{
			atoms_display_states_[i].drawable=true;
		}
		resize_visuals_in_display_states(atoms_representations_descriptor_.names.size(), atoms_display_states_);
	}

	void restrict_atoms(const std::set<std::size_t>& ids)
	{
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No ids provided to restrict atoms."));
		}

		if(*ids.rbegin()>=atoms_.size())
		{
			throw std::runtime_error(std::string("Invalid ids provided to restrict atoms."));
		}

		std::vector<Atom> restricted_atoms;
		restricted_atoms.reserve(ids.size());

		std::vector<DisplayState> restricted_atoms_display_states;
		restricted_atoms_display_states.reserve(ids.size());

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			restricted_atoms.push_back(atoms_[id]);
			restricted_atoms_display_states.push_back(atoms_display_states_[id]);
		}

		change_indicator_.set_changed_atoms(true);

		atoms_.swap(restricted_atoms);
		atoms_display_states_.swap(restricted_atoms_display_states);

		reset_data_dependent_on_atoms();
	}

	void restrict_atoms_and_renumber_residues_by_adjunct(const std::string& name)
	{
		std::set<std::size_t> ids;
		for(std::size_t i=0;i<atoms_.size();i++)
		{
			std::map<std::string, double>::const_iterator it=atoms_[i].value.props.adjuncts.find(name);
			if(it!=atoms_[i].value.props.adjuncts.end())
			{
				atoms_[i].crad.resSeq=static_cast<int>(it->second);
				ids.insert(i);
			}
		}

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms with adjunct '")+name+"'.");
		}

		restrict_atoms(ids);
	}

	void restrict_atoms_and_renumber_residues(const std::set<std::size_t>& atom_ids_to_keep, const std::map<std::size_t, int>& atom_ids_to_renumber)
	{
		if(atom_ids_to_keep.empty())
		{
			throw std::runtime_error(std::string("No atoms to keep after renumbering residues"));
		}

		for(std::map<std::size_t, int>::const_iterator it=atom_ids_to_renumber.begin();it!=atom_ids_to_renumber.end();++it)
		{
			const std::size_t atom_id=it->first;
			if(atom_id<atoms_.size())
			{
				atoms_[atom_id].crad.resSeq=it->second;
			}
		}

		restrict_atoms(atom_ids_to_keep);
	}

	void sort_atoms_by_residue_id()
	{
		std::map< common::ChainResidueAtomDescriptor, std::vector<std::size_t> > map_of_residue_ids;
		for(std::size_t i=0;i<atoms_.size();i++)
		{
			map_of_residue_ids[atoms_[i].crad.without_atom()].push_back(i);
		}

		std::vector<std::size_t> sorting_order;
		sorting_order.reserve(atoms_.size());
		for(std::map< common::ChainResidueAtomDescriptor, std::vector<std::size_t> >::const_iterator it=map_of_residue_ids.begin();it!=map_of_residue_ids.end();++it)
		{
			const std::vector<std::size_t>& ids=it->second;
			sorting_order.insert(sorting_order.end(), ids.begin(), ids.end());
		}

		bool reset_needed=(sorting_order.size()!=atoms_.size());
		for(std::size_t i=0;i<sorting_order.size() && !reset_needed;i++)
		{
			if(sorting_order[i]!=i)
			{
				reset_needed=true;
			}
		}

		if(reset_needed)
		{
			std::vector<Atom> sorted_atoms;
			sorted_atoms.reserve(atoms_.size());

			for(std::size_t i=0;i<sorting_order.size();i++)
			{
				sorted_atoms.push_back(atoms_[sorting_order[i]]);
			}

			reset_atoms_by_swapping(sorted_atoms);
		}
	}

	void append_atoms_from_other_data_managers(const std::vector<const DataManager*>& other_data_managers)
	{
		if(other_data_managers.empty())
		{
			throw std::runtime_error(std::string("No atom sources provided for appending."));
		}

		for(std::size_t i=0;i<other_data_managers.size();i++)
		{
			if(other_data_managers[i]==0)
			{
				throw std::runtime_error(std::string("Null source provided for appending."));
			}
			if(other_data_managers[i]==this)
			{
				throw std::runtime_error(std::string("Requested to append own atoms."));
			}
		}

		change_indicator_.set_changed_atoms(true);

		for(std::size_t i=0;i<other_data_managers.size();i++)
		{
			const DataManager& other_data_manager=*(other_data_managers[i]);
			atoms_.insert(atoms_.end(), other_data_manager.atoms().begin(), other_data_manager.atoms().end());
			atoms_display_states_.insert(atoms_display_states_.end(), other_data_manager.atoms_display_states().begin(), other_data_manager.atoms_display_states().end());
		}

		reset_data_dependent_on_atoms();
	}

	void transform_coordinates_of_atoms(const std::set<std::size_t>& ids, const TransformationOfCoordinates& transformation, const bool allow_recomputing_dependencies)
	{
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No ids provided to transform atoms."));
		}

		if(*ids.rbegin()>=atoms_.size())
		{
			throw std::runtime_error(std::string("Invalid ids provided to transform atoms."));
		}

		transformation.assert_validity();

		change_indicator_.set_changed_atoms(true);

		const std::size_t num_of_all_atoms=atoms_.size();

		transformation.transform_coordinates_of_atoms(atoms_, ids);

		if(allow_recomputing_dependencies && ids.size()==num_of_all_atoms && atoms_.size()==num_of_all_atoms)
		{
			refresh_by_saving_and_loading();
		}
		else
		{
			reset_data_dependent_on_atoms();
		}
	}

	void transform_coordinates_of_atoms(const std::set<std::size_t>& ids, const TransformationOfCoordinates& transformation)
	{
		transform_coordinates_of_atoms(ids, transformation, true);
	}

	void transform_coordinates_of_atoms(const TransformationOfCoordinates& transformation)
	{
		transformation.assert_validity();

		change_indicator_.set_changed_atoms(true);

		transformation.transform_coordinates_of_atoms(atoms_);

		refresh_by_saving_and_loading();
	}

	void remove_bonding_links_info()
	{
		bonding_links_info_=common::ConstructionOfBondingLinks::BundleOfBondingLinks();
	}

	void reset_bonding_links_info_by_swapping(common::ConstructionOfBondingLinks::BundleOfBondingLinks& bonding_links_info)
	{
		if(!bonding_links_info.valid(atoms_, primary_structure_info_))
		{
			throw std::runtime_error(std::string("Invalid bonding links info provided."));
		}

		bonding_links_info_.swap(bonding_links_info);
	}

	void reset_bonding_links_info_by_copying(const common::ConstructionOfBondingLinks::BundleOfBondingLinks& bonding_links_info)
	{
		common::ConstructionOfBondingLinks::BundleOfBondingLinks bonding_links_info_copy=bonding_links_info;
		reset_bonding_links_info_by_swapping(bonding_links_info_copy);
	}

	void reset_bonding_links_info_by_creating(const common::ConstructionOfBondingLinks::ParametersToConstructBundleOfBondingLinks& parameters)
	{
		common::ConstructionOfBondingLinks::BundleOfBondingLinks bundle_of_bonding_links;
		if(common::ConstructionOfBondingLinks::construct_bundle_of_bonding_links(
				parameters,
				atoms(),
				primary_structure_info(),
				bundle_of_bonding_links))
		{
			reset_bonding_links_info_by_swapping(bundle_of_bonding_links);
		}
		else
		{
			throw std::runtime_error(std::string("Failed to define bonding links."));
		}
	}

	void remove_triangulation_info()
	{
		triangulation_info_=common::ConstructionOfTriangulation::BundleOfTriangulationInformation();
		reset_data_dependent_on_triangulation_info();
	}

	void reset_triangulation_info_by_swapping(common::ConstructionOfTriangulation::BundleOfTriangulationInformation& triangulation_info)
	{
		if(triangulation_info.quadruples_map.empty())
		{
			throw std::runtime_error(std::string("No triangulation info to set."));
		}

		if(!triangulation_info.matching(common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atoms_)))
		{
			throw std::runtime_error(std::string("Triangulation info does not match atoms."));
		}

		triangulation_info_.swap(triangulation_info);

		reset_data_dependent_on_triangulation_info();
	}

	void reset_triangulation_info_by_copying(const common::ConstructionOfTriangulation::BundleOfTriangulationInformation& triangulation_info)
	{
		common::ConstructionOfTriangulation::BundleOfTriangulationInformation triangulation_info_copy=triangulation_info;
		reset_triangulation_info_by_swapping(triangulation_info_copy);
	}

	void reset_triangulation_info_by_creating(const common::ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation& parameters)
	{
		const std::vector<apollota::SimpleSphere> atomic_balls=common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atoms());

		if(triangulation_info().equivalent(parameters, atomic_balls))
		{
			return;
		}

		common::ConstructionOfTriangulation::BundleOfTriangulationInformation bundle_of_triangulation_information;

		if(!common::ConstructionOfTriangulation::construct_bundle_of_triangulation_information(parameters, atomic_balls, bundle_of_triangulation_information))
		{
			throw std::runtime_error(std::string("Failed to construct triangulation."));
		}

		reset_triangulation_info_by_swapping(bundle_of_triangulation_information);
	}

	void remove_contacts()
	{
		change_indicator_.set_changed_contacts(true);
		contacts_.clear();
		contacts_adjacencies_.clear();
		contacts_display_states_.clear();
		selection_manager_.set_contacts(0);
		history_of_actions_on_contacts_.clear();
	}

	void reset_contacts_by_swapping(std::vector<Contact>& contacts)
	{
		if(contacts.empty())
		{
			throw std::runtime_error(std::string("No contacts to set."));
		}
		assert_atoms_availability();
		if(!SelectionManager::check_contacts_compatibility_with_atoms(atoms_, contacts))
		{
			throw std::runtime_error(std::string("Contacts are not compatible with atoms."));
		}
		change_indicator_.set_changed_contacts(true);
		contacts_.swap(contacts);
		contacts_adjacencies_.clear();
		reset_contacts_display_states();
		selection_manager_.set_contacts(&contacts_);
		history_of_actions_on_contacts_.clear();
	}

	void reset_contacts_by_copying(const std::vector<Contact>& contacts)
	{
		std::vector<Contact> contacts_copy=contacts;
		reset_contacts_by_swapping(contacts_copy);
	}

	void reset_contacts_by_creating(
			const common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation& parameters_to_construct_contacts,
			const common::ConstructionOfContacts::ParametersToEnhanceContacts& parameters_to_enhance_contacts)
	{
		{
			common::ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation parameters_to_construct_triangulation;
			parameters_to_construct_triangulation.artificial_boundary_shift=std::max(parameters_to_construct_contacts.probe*2.0, 5.0);
			reset_triangulation_info_by_creating(parameters_to_construct_triangulation);
		}

		if(!contacts_.empty()
				&& !history_of_actions_on_contacts_.constructing.empty()
				&& !parameters_to_construct_contacts.supersedes(history_of_actions_on_contacts_.constructing.back())
				&& !history_of_actions_on_contacts_.enhancing.empty()
				&& !parameters_to_enhance_contacts.supersedes(history_of_actions_on_contacts_.enhancing.back())
				&& SelectionManager::check_contacts_compatibility_with_atoms(atoms_, contacts_))
		{
			return;
		}

		common::ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;

		if(!common::ConstructionOfContacts::construct_bundle_of_contact_information(parameters_to_construct_contacts, triangulation_info(), bundle_of_contact_information))
		{
			throw std::runtime_error(std::string("Failed to construct contacts."));
		}

		reset_contacts_by_swapping(bundle_of_contact_information.contacts);

		if(parameters_to_construct_contacts.calculate_volumes)
		{
			change_indicator_.set_changed_atoms_adjuncts(true);
			for(std::size_t i=0;i<bundle_of_contact_information.volumes.size() && i<atoms().size();i++)
			{
				atoms_[i].value.props.adjuncts["volume"]=bundle_of_contact_information.volumes[i];
			}
		}

		if(parameters_to_construct_contacts.calculate_bounding_arcs)
		{
			for(std::size_t i=0;i<bundle_of_contact_information.bounding_arcs.size() && i<contacts().size();i++)
			{
				if(!contacts_[i].solvent())
				{
					contacts_[i].value.props.adjuncts["boundary"]=bundle_of_contact_information.bounding_arcs[i];
				}
			}
		}

		if(parameters_to_construct_contacts.calculate_adjacencies)
		{
			contacts_adjacencies_=bundle_of_contact_information.adjacencies;
			for(std::size_t i=0;i<bundle_of_contact_information.adjacency_perimeters.size() && i<contacts().size();i++)
			{
				contacts_[i].value.props.adjuncts["adjacency"]=bundle_of_contact_information.adjacency_perimeters[i];
			}
		}

		history_of_actions_on_contacts_.constructing.clear();
		history_of_actions_on_contacts_.constructing.push_back(parameters_to_construct_contacts);

		common::ConstructionOfContacts::enhance_contacts(parameters_to_enhance_contacts, triangulation_info(), contacts_);

		history_of_actions_on_contacts_.enhancing.clear();
		history_of_actions_on_contacts_.enhancing.push_back(parameters_to_enhance_contacts);
	}

	void reset_contacts_adjacencies_by_swapping(std::vector< std::map<std::size_t, double> >& adjacencies, const std::vector<double>& adjacency_perimeters)
	{
		if(adjacencies.size()==contacts_.size() && adjacency_perimeters.size()==contacts_.size())
		{
			contacts_adjacencies_.swap(adjacencies);
			for(std::size_t i=0;i<contacts_.size();i++)
			{
				contacts_[i].value.props.adjuncts["adjacency"]=adjacency_perimeters[i];
			}
		}
	}

	void reset_contacts_display_states()
	{
		change_indicator_.update_changed_contacts_display_states_info(true, true, true);
		contacts_display_states_.clear();
		contacts_display_states_.resize(contacts_.size());
		for(std::size_t i=0;i<contacts_display_states_.size();i++)
		{
			contacts_display_states_[i].drawable=(!contacts_[i].value.graphics.empty());
		}
		resize_visuals_in_display_states(contacts_representations_descriptor_.names.size(), contacts_display_states_);
	}

	void remove_contacts_graphics(const std::set<std::size_t>& ids)
	{
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<contacts_.size())
			{
				if(!contacts_[id].value.graphics.empty())
				{
					change_indicator_.set_changed_contacts(true);
					contacts_[id].value.graphics.clear();
					history_of_actions_on_contacts_.graphics_creating.erase(id);
				}
			}
		}
		update_contacts_display_states_drawable(ids);
	}

	void reset_contacts_graphics_by_creating(
			const common::ConstructionOfContacts::ParametersToDrawContacts& parameters_to_draw_contacts,
			const std::set<std::size_t>& ids,
			const bool lazy)
	{
		assert_contacts_availability();

		std::set<std::size_t> ids_for_updating;

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<contacts_.size())
			{
				if(contacts_[id].value.graphics.empty())
				{
					ids_for_updating.insert(id);
				}
				else
				{
					std::map<std::size_t, common::ConstructionOfContacts::ParametersToDrawContacts>::const_iterator jt=history_of_actions_on_contacts_.graphics_creating.find(id);
					bool need_to_update=false;
					if(jt==history_of_actions_on_contacts_.graphics_creating.end())
					{
						if(lazy)
						{
							need_to_update=parameters_to_draw_contacts.enable_alt;
						}
						else
						{
							need_to_update=true;
						}
					}
					else
					{
						const common::ConstructionOfContacts::ParametersToDrawContacts& current_draw_parameters=jt->second;
						if(parameters_to_draw_contacts.enable_alt && !current_draw_parameters.enable_alt)
						{
							need_to_update=true;
						}
						else
						{
							if(lazy)
							{
								need_to_update=false;
							}
							else
							{
								need_to_update=!parameters_to_draw_contacts.equals(current_draw_parameters);
							}
						}
					}
					if(need_to_update)
					{
						ids_for_updating.insert(id);
					}
				}
			}
		}

		if(ids_for_updating.empty())
		{
			return;
		}

		assert_triangulation_info_availability();

		if(triangulation_info().parameters_of_construction.artificial_boundary_shift<(parameters_to_draw_contacts.probe*2))
		{
			throw std::runtime_error(std::string("The triangulation artificial boundary is not compatible with the probe radius."));
		}

		change_indicator_.set_changed_contacts(true);

		common::ConstructionOfContacts::draw_contacts(parameters_to_draw_contacts, triangulation_info(), ids_for_updating, contacts_);

		update_contacts_display_states_drawable(ids_for_updating);

		for(std::set<std::size_t>::const_iterator it=ids_for_updating.begin();it!=ids_for_updating.end();++it)
		{
			history_of_actions_on_contacts_.graphics_creating[*it]=parameters_to_draw_contacts;
		}
	}

	void remove_figures()
	{
		change_indicator_.set_changed_figures(true);
		figures_.clear();
		figures_display_states_.clear();
	}

	void remove_figures(const std::set<std::size_t>& ids)
	{
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No ids provided to remove figures."));
		}
		if(*ids.rbegin()>=figures_.size())
		{
			throw std::runtime_error(std::string("Invalid ids provided to remove atoms."));
		}
		change_indicator_.set_changed_figures(true);
		for(std::set<std::size_t>::const_reverse_iterator it=ids.rbegin();it!=ids.rend();++it)
		{
			figures_.erase(figures_.begin()+(*it));
			figures_display_states_.erase(figures_display_states_.begin()+(*it));
		}
	}

	void add_figures(const std::vector<Figure>& new_figures)
	{
		if(new_figures.empty())
		{
			throw std::runtime_error(std::string("No figures to add."));
		}
		change_indicator_.set_changed_figures(true);
		for(std::size_t i=0;i<new_figures.size();i++)
		{
			if(LongName::match(new_figures, new_figures[i].name).size()>1)
			{
				throw std::runtime_error(std::string("Repeating figure name."));
			}
			if(!LongName::match(figures(), new_figures[i].name).empty())
			{
				throw std::runtime_error(std::string("Redundant figure name."));
			}
			if(!new_figures[i].valid())
			{
				throw std::runtime_error(std::string("Figure is not valid."));
			}
		}
		std::vector<DisplayState> new_figures_display_states(new_figures.size());
		for(std::size_t i=0;i<new_figures_display_states.size();i++)
		{
			new_figures_display_states[i].drawable=true;
		}
		resize_visuals_in_display_states(figures_representations_descriptor_.names.size(), new_figures_display_states);
		figures_.insert(figures_.end(), new_figures.begin(), new_figures.end());
		figures_display_states_.insert(figures_display_states_.end(), new_figures_display_states.begin(), new_figures_display_states.end());
	}

	void add_figure(const Figure& figure)
	{
		add_figures(std::vector<Figure>(1, figure));
	}

	void reset_figures_display_states()
	{
		change_indicator_.update_changed_figures_display_states_info(true, true, true);
		figures_display_states_.clear();
		figures_display_states_.resize(contacts_.size());
		for(std::size_t i=0;i<figures_display_states_.size();i++)
		{
			figures_display_states_[i].drawable=true;
		}
		resize_visuals_in_display_states(figures_representations_descriptor_.names.size(), figures_display_states_);
	}

	void sync_atoms_selections_with_display_states()
	{
		if(!atoms_display_states_.empty())
		{
			std::set<std::size_t> ids_visible;
			std::set<std::size_t> ids_marked;
			for(std::size_t i=0;i<atoms_display_states_.size();i++)
			{
				const DisplayState& ds=atoms_display_states_[i];
				if(ds.visible())
				{
					ids_visible.insert(i);
				}
				if(ds.marked)
				{
					ids_marked.insert(i);
				}
			}

			if(ids_visible.empty())
			{
				selection_manager_.delete_atoms_selection("_visible");
			}
			else
			{
				selection_manager_.set_atoms_selection("_visible", ids_visible);
			}

			if(ids_marked.empty())
			{
				selection_manager_.delete_atoms_selection("_marked");
			}
			else
			{
				selection_manager_.set_atoms_selection("_marked", ids_marked);
			}
		}
	}

	void sync_contacts_selections_with_display_states()
	{
		if(!contacts_display_states_.empty())
		{
			std::set<std::size_t> ids_visible;
			std::set<std::size_t> ids_marked;
			std::set<std::size_t> ids_drawable;

			for(std::size_t i=0;i<contacts_display_states_.size();i++)
			{
				const DisplayState& ds=contacts_display_states_[i];

				if(ds.visible())
				{
					ids_visible.insert(i);
				}

				if(ds.marked)
				{
					ids_marked.insert(i);
				}

				if(ds.drawable)
				{
					ids_drawable.insert(i);
				}
			}

			if(ids_visible.empty())
			{
				selection_manager_.delete_contacts_selection("_visible");
			}
			else
			{
				selection_manager_.set_contacts_selection("_visible", ids_visible);
			}

			if(ids_marked.empty())
			{
				selection_manager_.delete_contacts_selection("_marked");
			}
			else
			{
				selection_manager_.set_contacts_selection("_marked", ids_marked);
			}

			if(ids_drawable.empty())
			{
				selection_manager_.delete_contacts_selection("_drawable");
			}
			else
			{
				selection_manager_.set_contacts_selection("_drawable", ids_drawable);
			}
		}
	}

	void sync_selections_with_display_states_if_requested_in_string(const std::string& request)
	{
		if(request.find("_marked")!=std::string::npos || request.find("_visible")!=std::string::npos || request.find("_drawable")!=std::string::npos)
		{
			sync_atoms_selections_with_display_states();
			sync_contacts_selections_with_display_states();
		}
	}

	bool is_saveable_to_stream() const
	{
		if(atoms_.empty() || (!contacts_.empty() && history_of_actions_on_contacts_.constructing.empty()))
		{
			return false;
		}
		return true;
	}

	void save_to_stream(std::ostream& output) const
	{
		if(!is_saveable_to_stream())
		{
			throw std::runtime_error(std::string("Object is not saveable to stream."));
		}

		output << atoms_.size() << "\n";
		for(std::size_t i=0;i<atoms_.size();i++)
		{
			output << atoms_[i] << "\n";
		}

		{
			std::map< DisplayState, std::vector<std::size_t> > map_of_atoms_display_states;
			for(std::size_t i=0;i<atoms_display_states_.size();i++)
			{
				map_of_atoms_display_states[atoms_display_states_[i]].push_back(i);
			}

			output << map_of_atoms_display_states.size() << "\n";
			for(std::map< DisplayState, std::vector<std::size_t> >::const_iterator it=map_of_atoms_display_states.begin();it!=map_of_atoms_display_states.end();++it)
			{
				output << it->first << "\n";
				output << it->second.size() << "\n";
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					output << it->second[i] << "\n";
				}
			}
		}

		{
			std::map< std::string, std::vector<std::size_t> > map_of_atoms_selections;
			for(std::map< std::string, std::set<std::size_t> >::const_iterator it=selection_manager_.map_of_atoms_selections().begin();it!=selection_manager_.map_of_atoms_selections().end();++it)
			{
				if(!it->first.empty() && it->first[0]!='_')
				{
					std::vector<std::size_t>& ids_vector=map_of_atoms_selections[it->first];
					ids_vector.reserve(it->second.size());
					for(std::set<std::size_t>::const_iterator jt=it->second.begin();jt!=it->second.end();++jt)
					{
						ids_vector.push_back(*jt);
					}
				}
			}

			output << map_of_atoms_selections.size() << "\n";
			for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=map_of_atoms_selections.begin();it!=map_of_atoms_selections.end();++it)
			{
				output << it->first << "\n";
				output << it->second.size() << "\n";
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					output << it->second[i] << "\n";
				}
			}
		}

		output << history_of_actions_on_contacts_.constructing.size() << "\n";
		for(std::size_t i=0;i<history_of_actions_on_contacts_.constructing.size();i++)
		{
			output << history_of_actions_on_contacts_.constructing[i] << "\n";
		}

		output << history_of_actions_on_contacts_.enhancing.size() << "\n";
		for(std::size_t i=0;i<history_of_actions_on_contacts_.enhancing.size();i++)
		{
			output << history_of_actions_on_contacts_.enhancing[i] << "\n";
		}

		{
			std::map< common::ConstructionOfContacts::ParametersToDrawContacts, std::vector< std::pair<std::size_t, std::size_t> > > map_of_contacts_graphics_creating_parameters;
			for(std::map<std::size_t, common::ConstructionOfContacts::ParametersToDrawContacts>::const_iterator it=history_of_actions_on_contacts_.graphics_creating.begin();it!=history_of_actions_on_contacts_.graphics_creating.end();++it)
			{
				map_of_contacts_graphics_creating_parameters[it->second].push_back(std::pair<std::size_t, std::size_t>(contacts_[it->first].ids[0], contacts_[it->first].ids[1]));
			}

			output << map_of_contacts_graphics_creating_parameters.size() << "\n";
			for(std::map< common::ConstructionOfContacts::ParametersToDrawContacts, std::vector< std::pair<std::size_t, std::size_t> > >::const_iterator it=map_of_contacts_graphics_creating_parameters.begin();it!=map_of_contacts_graphics_creating_parameters.end();++it)
			{
				output << it->first << "\n";
				output << it->second.size() << "\n";
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					output << it->second[i].first << " " << it->second[i].second << "\n";
				}
			}
		}

		{
			std::map< common::PropertiesValue, std::vector< std::pair<std::size_t, std::size_t> > > map_of_contacts_properties;
			for(std::size_t i=0;i<contacts_.size();i++)
			{
				map_of_contacts_properties[contacts_[i].value.props].push_back(std::pair<std::size_t, std::size_t>(contacts_[i].ids[0], contacts_[i].ids[1]));
			}

			output << map_of_contacts_properties.size() << "\n";
			for(std::map< common::PropertiesValue, std::vector< std::pair<std::size_t, std::size_t> > >::const_iterator it=map_of_contacts_properties.begin();it!=map_of_contacts_properties.end();++it)
			{
				output << it->first << "\n";
				output << it->second.size() << "\n";
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					output << it->second[i].first << " " << it->second[i].second << "\n";
				}
			}
		}

		{
			std::map< DisplayState, std::vector< std::pair<std::size_t, std::size_t> > > map_of_contacts_display_states;
			for(std::size_t i=0;i<contacts_display_states_.size();i++)
			{
				map_of_contacts_display_states[contacts_display_states_[i]].push_back(std::pair<std::size_t, std::size_t>(contacts_[i].ids[0], contacts_[i].ids[1]));
			}

			output << map_of_contacts_display_states.size() << "\n";
			for(std::map< DisplayState, std::vector< std::pair<std::size_t, std::size_t> > >::const_iterator it=map_of_contacts_display_states.begin();it!=map_of_contacts_display_states.end();++it)
			{
				output << it->first << "\n";
				output << it->second.size() << "\n";
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					output << it->second[i].first << " " << it->second[i].second << "\n";
				}
			}
		}

		{
			std::map< std::string, std::vector< std::pair<std::size_t, std::size_t> > > map_of_contacts_selections;
			for(std::map< std::string, std::set<std::size_t> >::const_iterator it=selection_manager_.map_of_contacts_selections().begin();it!=selection_manager_.map_of_contacts_selections().end();++it)
			{
				if(!it->first.empty() && it->first[0]!='_')
				{
					std::vector< std::pair<std::size_t, std::size_t> >& ids_vector=map_of_contacts_selections[it->first];
					ids_vector.reserve(it->second.size());
					for(std::set<std::size_t>::const_iterator jt=it->second.begin();jt!=it->second.end();++jt)
					{
						ids_vector.push_back(std::pair<std::size_t, std::size_t>(contacts_[*jt].ids[0], contacts_[*jt].ids[1]));
					}
				}
			}

			output << map_of_contacts_selections.size() << "\n";
			for(std::map< std::string, std::vector< std::pair<std::size_t, std::size_t> > >::const_iterator it=map_of_contacts_selections.begin();it!=map_of_contacts_selections.end();++it)
			{
				output << it->first << "\n";
				output << it->second.size() << "\n";
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					output << it->second[i].first << " " << it->second[i].second << "\n";
				}
			}
		}

		output << figures_.size() << "\n";
		for(std::size_t i=0;i<figures_.size();i++)
		{
			output << figures_[i] << "\n";
		}

		{
			std::map< DisplayState, std::vector<std::size_t> > map_of_figures_display_states;
			for(std::size_t i=0;i<figures_display_states_.size();i++)
			{
				map_of_figures_display_states[figures_display_states_[i]].push_back(i);
			}

			output << map_of_figures_display_states.size() << "\n";
			for(std::map< DisplayState, std::vector<std::size_t> >::const_iterator it=map_of_figures_display_states.begin();it!=map_of_figures_display_states.end();++it)
			{
				output << it->first << "\n";
				output << it->second.size() << "\n";
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					output << it->second[i] << "\n";
				}
			}
		}
	}

	void load_from_stream(std::istream& input)
	{
		(*this)=DataManager();

		std::vector<Atom> atoms;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				Atom atom;
				input >> atom;
				atoms.push_back(atom);
			}
		}

		std::map< DisplayState, std::vector<std::size_t> > map_of_atoms_display_states;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				DisplayState ds;
				input >> ds;
				int ids_count=0;
				input >> ids_count;
				if(ids_count>0)
				{
					std::vector<std::size_t>& ids_vector=map_of_atoms_display_states[ds];
					ids_vector.reserve(ids_count);
					for(int i=0;i<ids_count;i++)
					{
						std::size_t id_value=0;
						input >> id_value;
						ids_vector.push_back(id_value);
					}
				}
			}
		}

		std::map< std::string, std::vector<std::size_t> > map_of_atoms_selections;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				std::string name;
				input >> name;
				int ids_count=0;
				input >> ids_count;
				if(ids_count>0)
				{
					std::vector<std::size_t>& ids_vector=map_of_atoms_selections[name];
					ids_vector.reserve(ids_count);
					for(int i=0;i<ids_count;i++)
					{
						std::size_t id_value=0;
						input >> id_value;
						ids_vector.push_back(id_value);
					}
				}
			}
		}

		std::vector<common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation> contacts_params_constructing;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation params;
				input >> params;
				contacts_params_constructing.push_back(params);
			}
		}

		std::vector<common::ConstructionOfContacts::ParametersToEnhanceContacts> contacts_params_enhancing;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				common::ConstructionOfContacts::ParametersToEnhanceContacts params;
				input >> params;
				contacts_params_enhancing.push_back(params);
			}
		}

		std::map< common::ConstructionOfContacts::ParametersToDrawContacts, std::vector< std::pair<std::size_t, std::size_t> > > map_of_contacts_graphics_creating_parameters;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				common::ConstructionOfContacts::ParametersToDrawContacts params;
				params.probe=1.0;
				params.step=2.0;
				params.projections=3;
				params.simplify=true;
				params.sih_depth=5;
				params.enable_alt=true;
				params.circular_angle_step=6.0;
				input >> params;
				int pairs_count=0;
				input >> pairs_count;
				if(pairs_count>0)
				{
					std::vector< std::pair<std::size_t, std::size_t> >& pairs_vector=map_of_contacts_graphics_creating_parameters[params];
					pairs_vector.reserve(pairs_count);
					for(int i=0;i<pairs_count;i++)
					{
						std::pair<std::size_t, std::size_t> pair_value;
						input >> pair_value.first >> pair_value.second;
						pairs_vector.push_back(pair_value);
					}
				}
			}
		}

		std::map< common::PropertiesValue, std::vector< std::pair<std::size_t, std::size_t> > > map_of_contacts_properties;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				common::PropertiesValue props;
				input >> props;
				int pairs_count=0;
				input >> pairs_count;
				if(pairs_count>0)
				{
					std::vector< std::pair<std::size_t, std::size_t> >& pairs_vector=map_of_contacts_properties[props];
					pairs_vector.reserve(pairs_count);
					for(int i=0;i<pairs_count;i++)
					{
						std::pair<std::size_t, std::size_t> pair_value;
						input >> pair_value.first >> pair_value.second;
						pairs_vector.push_back(pair_value);
					}
				}
			}
		}

		std::map< DisplayState, std::vector< std::pair<std::size_t, std::size_t> > > map_of_contacts_display_states;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				DisplayState ds;
				input >> ds;
				int pairs_count=0;
				input >> pairs_count;
				if(pairs_count>0)
				{
					std::vector< std::pair<std::size_t, std::size_t> >& pairs_vector=map_of_contacts_display_states[ds];
					pairs_vector.reserve(pairs_count);
					for(int i=0;i<pairs_count;i++)
					{
						std::pair<std::size_t, std::size_t> pair_value;
						input >> pair_value.first >> pair_value.second;
						pairs_vector.push_back(pair_value);
					}
				}
			}
		}

		std::map< std::string, std::vector< std::pair<std::size_t, std::size_t> > > map_of_contacts_selections;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				std::string name;
				input >> name;
				int pairs_count=0;
				input >> pairs_count;
				if(pairs_count>0)
				{
					std::vector< std::pair<std::size_t, std::size_t> >& pairs_vector=map_of_contacts_selections[name];
					pairs_vector.reserve(pairs_count);
					for(int i=0;i<pairs_count;i++)
					{
						std::pair<std::size_t, std::size_t> pair_value;
						input >> pair_value.first >> pair_value.second;
						pairs_vector.push_back(pair_value);
					}
				}
			}
		}

		std::vector<Figure> figures;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				Figure figure;
				input >> figure;
				figures.push_back(figure);
			}
		}

		std::map< DisplayState, std::vector<std::size_t> > map_of_figures_display_states;
		{
			int count=0;
			input >> count;
			for(int i=0;i<count;i++)
			{
				DisplayState ds;
				input >> ds;
				int ids_count=0;
				input >> ids_count;
				if(ids_count>0)
				{
					std::vector<std::size_t>& ids_vector=map_of_figures_display_states[ds];
					ids_vector.reserve(ids_count);
					for(int i=0;i<ids_count;i++)
					{
						std::size_t id_value=0;
						input >> id_value;
						ids_vector.push_back(id_value);
					}
				}
			}
		}

		if(atoms.empty())
		{
			throw std::runtime_error(std::string("No atoms when loading from stream."));
		}

		if(contacts_params_constructing.empty()!=contacts_params_enhancing.empty())
		{
			throw std::runtime_error(std::string("Invalid parameters for constructing contacts when loading from stream."));
		}

		if(contacts_params_constructing.empty() && !map_of_contacts_graphics_creating_parameters.empty())
		{
			throw std::runtime_error(std::string("Invalid parameters for drawing contacts when loading from stream."));
		}

		if(contacts_params_constructing.empty()!=map_of_contacts_properties.empty())
		{
			throw std::runtime_error(std::string("Invalid contacts data when loading from stream."));
		}

		reset_atoms_by_copying(atoms);

		for(std::map< DisplayState, std::vector<std::size_t> >::const_iterator it=map_of_atoms_display_states.begin();it!=map_of_atoms_display_states.end();++it)
		{
			for(std::size_t i=0;i<(it->second.size());i++)
			{
				const std::size_t id=it->second[i];
				if(id<atoms_display_states_.size())
				{
					atoms_display_states_[id]=it->first;
				}
			}
		}

		for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=map_of_atoms_selections.begin();it!=map_of_atoms_selections.end();++it)
		{
			selection_manager_.set_atoms_selection(it->first, std::set<std::size_t>(it->second.begin(), it->second.end()));
		}

		if(!contacts_params_constructing.empty())
		{
			reset_contacts_by_creating(contacts_params_constructing.back(), contacts_params_enhancing.back());

			std::map<std::pair<std::size_t, std::size_t>, std::size_t> map_of_pairs_to_contact_ids;
			for(std::size_t i=0;i<contacts_.size();i++)
			{
				map_of_pairs_to_contact_ids[std::pair<std::size_t, std::size_t>(contacts_[i].ids[0], contacts_[i].ids[1])]=i;
			}

			for(std::map< common::ConstructionOfContacts::ParametersToDrawContacts, std::vector< std::pair<std::size_t, std::size_t> > >::const_iterator it=map_of_contacts_graphics_creating_parameters.begin();it!=map_of_contacts_graphics_creating_parameters.end();++it)
			{
				std::set<std::size_t> ids_for_params;
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					std::map<std::pair<std::size_t, std::size_t>, std::size_t>::const_iterator id_it=map_of_pairs_to_contact_ids.find(it->second[i]);
					if(id_it!=map_of_pairs_to_contact_ids.end())
					{
						ids_for_params.insert(id_it->second);
					}
				}
				reset_contacts_graphics_by_creating(it->first, ids_for_params, false);
			}

			for(std::map< common::PropertiesValue, std::vector< std::pair<std::size_t, std::size_t> > >::const_iterator it=map_of_contacts_properties.begin();it!=map_of_contacts_properties.end();++it)
			{
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					std::map<std::pair<std::size_t, std::size_t>, std::size_t>::const_iterator id_it=map_of_pairs_to_contact_ids.find(it->second[i]);
					if(id_it!=map_of_pairs_to_contact_ids.end())
					{
						contacts_[id_it->second].value.props=it->first;
					}
				}
			}

			for(std::map< DisplayState, std::vector< std::pair<std::size_t, std::size_t> > >::const_iterator it=map_of_contacts_display_states.begin();it!=map_of_contacts_display_states.end();++it)
			{
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					std::map<std::pair<std::size_t, std::size_t>, std::size_t>::const_iterator id_it=map_of_pairs_to_contact_ids.find(it->second[i]);
					if(id_it!=map_of_pairs_to_contact_ids.end())
					{
						contacts_display_states_[id_it->second]=it->first;
					}
				}
			}

			for(std::map< std::string, std::vector< std::pair<std::size_t, std::size_t> > >::const_iterator it=map_of_contacts_selections.begin();it!=map_of_contacts_selections.end();++it)
			{
				std::vector<std::size_t> ids_vector;
				ids_vector.reserve(it->second.size());
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					std::map<std::pair<std::size_t, std::size_t>, std::size_t>::const_iterator id_it=map_of_pairs_to_contact_ids.find(it->second[i]);
					if(id_it!=map_of_pairs_to_contact_ids.end())
					{
						ids_vector.push_back(id_it->second);
					}
				}
				selection_manager_.set_contacts_selection(it->first, std::set<std::size_t>(ids_vector.begin(), ids_vector.end()));
			}
		}

		if(!figures.empty())
		{
			for(std::size_t i=0;i<figures.size();i++)
			{
				Figure& figure=figures[i];
				if(figure.special_description_for_label.is_label)
				{
					FigureOfText::init_figure_of_text(
							figure.special_description_for_label.label_text,
							figure.special_description_for_label.label_outline,
							figure.special_description_for_label.label_origin,
							figure.special_description_for_label.label_scale,
							figure.special_description_for_label.label_centered,
							figure);
				}
			}

			add_figures(figures);

			for(std::map< DisplayState, std::vector<std::size_t> >::const_iterator it=map_of_figures_display_states.begin();it!=map_of_figures_display_states.end();++it)
			{
				for(std::size_t i=0;i<(it->second.size());i++)
				{
					const std::size_t id=it->second[i];
					if(id<figures_display_states_.size())
					{
						figures_display_states_[id]=it->first;
					}
				}
			}
		}
	}

private:
	static bool add_names_to_representations(const std::vector<std::string>& names, std::vector<std::string>& representations)
	{
		if(names.empty())
		{
			return false;
		}

		for(std::size_t i=0;i<names.size();i++)
		{
			const std::string& name=names[i];
			if(name.empty())
			{
				return false;
			}
			else if(std::find(representations.begin(), representations.end(), name)!=representations.end())
			{
				return false;
			}
		}

		representations.insert(representations.end(), names.begin(), names.end());

		return true;
	}

	static bool set_representation_implemented_always(
			const std::vector<std::string>& representations,
			const std::size_t representation_id,
			const bool status,
			std::set<std::size_t>& representations_implemented_always)
	{
		if(representation_id>=representations.size())
		{
			return false;
		}

		if(status)
		{
			representations_implemented_always.insert(representation_id);
		}
		else
		{
			representations_implemented_always.erase(representation_id);
		}

		return true;
	}

	static bool set_representation_implemented(
			const std::vector<std::string>& representations,
			const std::size_t representation_id,
			const std::vector<bool>& statuses,
			std::vector<DisplayState>& display_states)
	{
		if(statuses.size()!=display_states.size())
		{
			return false;
		}

		if(representation_id>=representations.size())
		{
			return false;
		}

		for(std::size_t i=0;i<display_states.size();i++)
		{
			if(display_states[i].drawable && representation_id>=display_states[i].visuals.size())
			{
				return false;
			}
		}

		for(std::size_t i=0;i<display_states.size();i++)
		{
			if(display_states[i].drawable)
			{
				display_states[i].visuals[representation_id].implemented=statuses[i];
			}
		}

		return true;
	}

	bool add_atoms_representation(const std::string& name)
	{
		if(add_names_to_representations(std::vector<std::string>(1, name), atoms_representations_descriptor_.names))
		{
			if(resize_visuals_in_display_states(atoms_representations_descriptor_.names.size(), atoms_display_states_))
			{
				change_indicator_.set_changed_atoms_display_states(true);
			}
			return true;
		}
		return false;
	}

	bool add_contacts_representation(const std::string& name)
	{
		if(add_names_to_representations(std::vector<std::string>(1, name), contacts_representations_descriptor_.names))
		{
			if(resize_visuals_in_display_states(contacts_representations_descriptor_.names.size(), contacts_display_states_))
			{
				change_indicator_.set_changed_contacts_display_states(true);
			}
			return true;
		}
		return false;
	}

	bool add_figures_representation(const std::string& name)
	{
		if(add_names_to_representations(std::vector<std::string>(1, name), figures_representations_descriptor_.names))
		{
			if(resize_visuals_in_display_states(figures_representations_descriptor_.names.size(), figures_display_states_))
			{
				change_indicator_.set_changed_figures_display_states(true);
			}
			return true;
		}
		return false;
	}

	static bool resize_visuals_in_display_states(const std::size_t size, std::vector<DisplayState>& display_states)
	{
		bool resized=false;
		for(std::size_t i=0;i<display_states.size();i++)
		{
			if(display_states[i].visuals.size()!=size)
			{
				display_states[i].visuals.resize(size);
				resized=true;
			}
		}
		return resized;
	}

	void reset_data_dependent_on_atoms()
	{
		primary_structure_info_=common::ConstructionOfPrimaryStructure::construct_bundle_of_primary_structure(atoms_);
		secondary_structure_info_=common::ConstructionOfSecondaryStructure::construct_bundle_of_secondary_structure(atoms_, primary_structure_info_);
		selection_manager_=SelectionManager(&atoms_, 0);

		remove_bonding_links_info();
		remove_triangulation_info();
		remove_contacts();
	}

	void reset_data_dependent_on_triangulation_info()
	{
		remove_contacts();
	}

	void update_contacts_display_states_drawable(const std::set<std::size_t>& ids)
	{
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			if(id<contacts_.size())
			{
				DisplayState& ds=contacts_display_states_[id];
				ds.drawable=(!contacts_[id].value.graphics.empty());
				for(std::size_t j=0;j<ds.visuals.size();j++)
				{
					ds.visuals[j].implemented=false;
				}
			}
		}
		resize_visuals_in_display_states(contacts_representations_descriptor_.names.size(), contacts_display_states_);
	}

	void refresh_by_saving_and_loading()
	{
		std::ostringstream output;
		save_to_stream(output);
		std::istringstream input(output.str());
		load_from_stream(input);
	}

	RepresentationsDescriptor atoms_representations_descriptor_;
	RepresentationsDescriptor contacts_representations_descriptor_;
	RepresentationsDescriptor figures_representations_descriptor_;
	std::vector<Atom> atoms_;
	std::vector<Contact> contacts_;
	std::vector< std::map<std::size_t, double> > contacts_adjacencies_;
	std::vector<Figure> figures_;
	std::map<std::string, double> global_numeric_adjuncts_;
	std::vector<DisplayState> atoms_display_states_;
	std::vector<DisplayState> contacts_display_states_;
	std::vector<DisplayState> figures_display_states_;
	common::ConstructionOfPrimaryStructure::BundleOfPrimaryStructure primary_structure_info_;
	common::ConstructionOfSecondaryStructure::BundleOfSecondaryStructure secondary_structure_info_;
	common::ConstructionOfBondingLinks::BundleOfBondingLinks bonding_links_info_;
	common::ConstructionOfTriangulation::BundleOfTriangulationInformation triangulation_info_;
	SelectionManager selection_manager_;
	HistoryOfActionsOnContacts history_of_actions_on_contacts_;
	ChangeIndicator change_indicator_;
};

}

}

#endif /* SCRIPTING_DATA_MANAGER_H_ */

