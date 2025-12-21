#ifndef VORONOTALT_CLI_CONTACT_PLOTTER_H_
#define VORONOTALT_CLI_CONTACT_PLOTTER_H_

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>

#include "spheres_input.h"

namespace voronotalt
{

class ContactPlotter
{
public:
	struct LevelMode
	{
		enum ID
		{
			inter_atom,
			inter_residue,
			inter_chain
		};
	};

	struct ConfigFlags
	{
		bool valid;
		bool dark;
		bool compact;
		bool interactive;
		bool gradient;
		bool gradient_in_background;
		bool colored;
		bool xlabeled;
		bool ylabeled;

		ConfigFlags() noexcept : valid(false), dark(false), compact(false), interactive(false), gradient(false), gradient_in_background(false), colored(false), xlabeled(false), ylabeled(false)
		{
		}

		static ConfigFlags read(const std::set<std::string>& config) noexcept
		{
			ConfigFlags cf;
			for(std::set<std::string>::const_iterator it=config.begin();it!=config.end();++it)
			{
				if((*it)=="dark")
				{
					cf.dark=true;
				}
				else if((*it)=="compact")
				{
					cf.compact=true;
				}
				else if((*it)=="interactive")
				{
					cf.interactive=true;
				}
				else if((*it)=="gradient")
				{
					cf.gradient=true;
				}
				else if((*it)=="gradient-in-background")
				{
					cf.gradient_in_background=true;
				}
				else if((*it)=="colored")
				{
					cf.colored=true;
				}
				else if((*it)=="labeled")
				{
					cf.xlabeled=true;
					cf.ylabeled=true;
				}
				else if((*it)=="xlabeled")
				{
					cf.xlabeled=true;
				}
				else if((*it)=="ylabeled")
				{
					cf.ylabeled=true;
				}
				else
				{
					return cf;
				}
			}
			cf.valid=true;
			return cf;
		}
	};

	ContactPlotter(const LevelMode::ID mode) noexcept : mode_(mode)
	{
	}

	template<class ContactsContainer>
	bool add_contact(const std::size_t i, const ContactsContainer& contacts, const std::vector<SphereLabeling::SphereLabel>& sphere_labels, const unsigned int color) noexcept
	{
		if(i<contacts.size() && contacts[i].id_a<sphere_labels.size() && contacts[i].id_b<sphere_labels.size())
		{
			add_point(sphere_labels[contacts[i].id_a], sphere_labels[contacts[i].id_b], contacts[i].area, color);
			return true;
		}
		return false;
	}

	bool write_to_file(const std::string& filename, const std::set<std::string>& config) noexcept
	{
		if(map_of_points_.empty())
		{
			return false;
		}

		const ConfigFlags cf=ConfigFlags::read(config);
		if(!cf.valid)
		{
			return false;
		}

		std::ofstream output(filename, std::ios::out);
		if(!output.good())
		{
			return false;
		}

		const double scale=10.0;

		const std::string style_background_base(cf.dark ? "fill:#000000;" : "fill:#FFFFFF;");
		const std::string style_background_level0(cf.dark ? "fill:#191919;" : "fill:#F9F9F9;");
		const std::string style_background_level1(cf.dark ? "fill:#494949;" : "fill:#E9E9E9;");
		const std::string style_background_level2(cf.dark ? "fill:#797979;" : "fill:#C0C0C0;");
		const std::string style_rect_default(cf.dark ? "fill:#F9F9F9;" : "fill:#090909;");
		const std::string style_text_default(cf.dark ? "font-size:10px; fill:#FFFFFF;" : "font-size:10px; fill:#000000;");
		const std::string style_info_text_default(cf.dark ? "font-size:10px; fill:#FFFFFF;" : "font-size:10px; fill:#000000;");

		std::map<CoordKey, double> map_of_coords_main;
		std::map<CoordKey, double> map_of_coords_secondary;

		std::map<CoordKey, double>& map_of_coords_horizontal=map_of_coords_main;
		std::map<CoordKey, double>& map_of_coords_vertical=(cf.compact ? map_of_coords_secondary : map_of_coords_main);

		double max_area=0.0;
		for(std::map<PointKey, PointValue>::iterator it=map_of_points_.begin();it!=map_of_points_.end();++it)
		{
			map_of_coords_horizontal[it->first.a]=0.0;
			map_of_coords_vertical[it->first.b]=0.0;
			max_area=std::max(max_area, it->second.area);
		}

		double length_horizontal=0;
		double max_caption_width_vertical=0.0;
		for(std::map<CoordKey, double>::iterator it=map_of_coords_horizontal.begin();it!=map_of_coords_horizontal.end();++it)
		{
			it->second=length_horizontal;
			length_horizontal+=scale;
			max_caption_width_vertical=std::max(max_caption_width_vertical, it->first.unscaled_caption_width()*scale*0.9);
		}

		double length_vertical=0;
		double max_caption_width_horizontal=0.0;
		for(std::map<CoordKey, double>::iterator it=map_of_coords_vertical.begin();it!=map_of_coords_vertical.end();++it)
		{
			it->second=length_vertical;
			length_vertical+=scale;
			max_caption_width_horizontal=std::max(max_caption_width_horizontal, it->first.unscaled_caption_width()*scale*0.9);
		}

		const double full_width=(cf.ylabeled ? (length_horizontal+max_caption_width_horizontal) : length_horizontal);
		const double full_height=(cf.xlabeled ? (length_vertical+max_caption_width_vertical) : length_vertical)+(cf.interactive ? scale*3.0 : 0.0);
		const double shift_y=(cf.xlabeled ? max_caption_width_vertical : 0.0);

		SVGWriter svg(full_width, full_height, "font-family:monospace;", cf.interactive);

		if(cf.interactive)
		{
			svg.add_child(XMLWriter("script").set("<![CDATA[ function init(evt) { window.info = document.getElementById('info'); } function show(text) { info.textContent = text; } ]]>"));
		}

		svg.add_rect(0, 0, full_width, full_height, style_background_base);
		svg.add_rect(0, 0+shift_y, length_horizontal, length_vertical, style_background_level0);

		if(mode_==LevelMode::inter_atom || mode_==LevelMode::inter_residue)
		{
			std::map<PointKey, PointValue> map_of_points_on_chain_level;
			for(std::map<PointKey, PointValue>::iterator it=map_of_points_.begin();it!=map_of_points_.end();++it)
			{
				CoordKey a(LevelMode::inter_chain, it->first.a);
				CoordKey b(LevelMode::inter_chain, it->first.b);
				if(a!=b)
				{
					map_of_points_on_chain_level[PointKey(a, b)].add(it->second.area);
				}
			}

			double max_area_on_chain_level=0.0;
			for(std::map<PointKey, PointValue>::iterator it=map_of_points_on_chain_level.begin();it!=map_of_points_on_chain_level.end();++it)
			{
				max_area_on_chain_level=std::max(max_area_on_chain_level, it->second.area);
			}

			std::map<CoordKey, RegionValue> map_of_coord_regions_on_chain_level_horizontal;
			for(std::map<CoordKey, double>::iterator it=map_of_coords_horizontal.begin();it!=map_of_coords_horizontal.end();++it)
			{
				RegionValue& rv=map_of_coord_regions_on_chain_level_horizontal[CoordKey(LevelMode::inter_chain, it->first)];
				rv.update(it->second);
			}

			std::map<CoordKey, RegionValue> map_of_coord_regions_on_chain_level_vertical;
			for(std::map<CoordKey, double>::iterator it=map_of_coords_vertical.begin();it!=map_of_coords_vertical.end();++it)
			{
				RegionValue& rv=map_of_coord_regions_on_chain_level_vertical[CoordKey(LevelMode::inter_chain, it->first)];
				rv.update(it->second);
			}

			for(std::map<PointKey, PointValue>::iterator it=map_of_points_on_chain_level.begin();it!=map_of_points_on_chain_level.end();++it)
			{
				const RegionValue& x=map_of_coord_regions_on_chain_level_horizontal[it->first.a];
				const RegionValue& y=map_of_coord_regions_on_chain_level_vertical[it->first.b];
				std::string style_rect_colored;
				if(cf.gradient_in_background && mode_==LevelMode::inter_residue)
				{
					style_rect_colored=std::string("fill:")+color_from_gradient(it->second.area, 0.0, max_area_on_chain_level, GradientMode::cyan_blue)+std::string(";");
				}
				svg.add_rect(x.a, y.a+shift_y, x.b+scale-x.a, y.b+scale-y.a, style_rect_colored.empty() ? style_background_level1 : style_rect_colored);
				if(!cf.compact)
				{
					svg.add_rect(y.a, x.a+shift_y, y.b+scale-y.a, x.b+scale-x.a, style_rect_colored.empty() ? style_background_level1 : style_rect_colored);
				}
			}
		}

		if(mode_==LevelMode::inter_atom)
		{
			std::map<PointKey, PointValue> map_of_points_on_residue_level;
			for(std::map<PointKey, PointValue>::iterator it=map_of_points_.begin();it!=map_of_points_.end();++it)
			{
				CoordKey a(LevelMode::inter_residue, it->first.a);
				CoordKey b(LevelMode::inter_residue, it->first.b);
				if(a!=b)
				{
					map_of_points_on_residue_level[PointKey(a, b)].add(it->second.area);
				}
			}

			double max_area_on_residue_level=0.0;
			for(std::map<PointKey, PointValue>::iterator it=map_of_points_on_residue_level.begin();it!=map_of_points_on_residue_level.end();++it)
			{
				max_area_on_residue_level=std::max(max_area_on_residue_level, it->second.area);
			}

			std::map<CoordKey, RegionValue> map_of_coord_regions_on_residue_level_horizontal;
			for(std::map<CoordKey, double>::iterator it=map_of_coords_horizontal.begin();it!=map_of_coords_horizontal.end();++it)
			{
				RegionValue& rv=map_of_coord_regions_on_residue_level_horizontal[CoordKey(LevelMode::inter_residue, it->first)];
				rv.update(it->second);
			}

			std::map<CoordKey, RegionValue> map_of_coord_regions_on_residue_level_vertical;
			for(std::map<CoordKey, double>::iterator it=map_of_coords_vertical.begin();it!=map_of_coords_vertical.end();++it)
			{
				RegionValue& rv=map_of_coord_regions_on_residue_level_vertical[CoordKey(LevelMode::inter_residue, it->first)];
				rv.update(it->second);
			}

			for(std::map<PointKey, PointValue>::iterator it=map_of_points_on_residue_level.begin();it!=map_of_points_on_residue_level.end();++it)
			{
				const RegionValue& x=map_of_coord_regions_on_residue_level_horizontal[it->first.a];
				const RegionValue& y=map_of_coord_regions_on_residue_level_vertical[it->first.b];
				std::string style_rect_colored;
				if(cf.gradient_in_background)
				{
					style_rect_colored=std::string("fill:")+color_from_gradient(it->second.area, 0.0, max_area_on_residue_level, GradientMode::cyan_blue)+std::string(";");
				}
				svg.add_rect(x.a, y.a+shift_y, x.b+scale-x.a, y.b+scale-y.a, style_rect_colored.empty() ? style_background_level2 : style_rect_colored);
				if(!cf.compact)
				{
					svg.add_rect(y.a, x.a+shift_y, y.b+scale-y.a, x.b+scale-x.a, style_rect_colored.empty() ? style_background_level2 : style_rect_colored);
				}
			}
		}

		for(std::map<PointKey, PointValue>::const_iterator it=map_of_points_.begin();it!=map_of_points_.end();++it)
		{
			const double x=map_of_coords_horizontal[it->first.a];
			const double y=map_of_coords_vertical[it->first.b];
			std::string style_rect_colored;
			std::string onclick_action;

			if(cf.colored)
			{
				style_rect_colored=std::string("fill:")+it->second.mean_color()+std::string(";");
			}
			else if(cf.gradient)
			{
				style_rect_colored=std::string("fill:")+color_from_gradient(it->second.area, 0.0, max_area, GradientMode::yellow_red)+std::string(";");
			}

			if(cf.interactive)
			{
				std::ostringstream info_output;
				info_output << "show('area([" << it->first.a.caption() << "], [" << it->first.b.caption() << "]) = " << it->second.area << "')";
				onclick_action=info_output.str();
			}

			svg.add_rect(x, y+shift_y, scale, scale, (style_rect_colored.empty() ? style_rect_default : style_rect_colored), onclick_action);

			if(!cf.compact)
			{
				svg.add_rect(y, x+shift_y, scale, scale, (style_rect_colored.empty() ? style_rect_default : style_rect_colored), onclick_action);
			}
		}

		if(cf.xlabeled)
		{
			for(std::map<CoordKey, double>::iterator it=map_of_coords_horizontal.begin();it!=map_of_coords_horizontal.end();++it)
			{
				const double x=it->second+scale;
				const double y=0.0-scale*0.1+shift_y;
				svg.add_text(it->first.caption(), x, y, -90.0, x, y, style_text_default);
			}
		}

		if(cf.ylabeled)
		{
			for(std::map<CoordKey, double>::iterator it=map_of_coords_vertical.begin();it!=map_of_coords_vertical.end();++it)
			{
				const double x=length_horizontal+scale*0.1;
				const double y=scale+it->second-scale*0.2+shift_y;
				svg.add_text(it->first.caption(), x, y, style_text_default);
			}
		}

		if(cf.interactive)
		{
			svg.add_child(XMLWriter("text").set("id", "info").set("x", scale).set("y", full_height-scale).set("style", style_info_text_default).set(""));
		}

		output << "<?xml version='1.0' encoding='UTF-8'?>\n";
		svg.write(output);

		return true;
	}

private:
	class XMLWriter
	{
	public:
		explicit XMLWriter(const std::string& type) noexcept : type_(type)
		{
		}

		template<typename T>
		XMLWriter& set(const std::string& name, const T& value, const std::string& units="") noexcept
		{
			std::ostringstream output;
			output << value << units;
			parameters_[name]=output.str();
			return (*this);
		}

		XMLWriter& set(const std::string& contents) noexcept
		{
			contents_=contents;
			return (*this);
		}

		XMLWriter& add_child(const XMLWriter& child) noexcept
		{
			children_.push_back(child);
			return (*this);
		}

		void write(std::ostream& output, const std::size_t tabs) const noexcept
		{
			output << std::string(tabs, ' ') << "<" << type_;
			for(std::map<std::string, std::string>::const_iterator it=parameters_.begin();it!=parameters_.end();++it)
			{
				output << " " << it->first << "=\"" << it->second << "\"";
			}
			if(contents_.empty() && children_.empty())
			{
				output << "/>\n";
			}
			else
			{
				output << ">\n";
				if(!contents_.empty())
				{
					output << std::string(tabs+2, ' ') << contents_ << "\n";
				}
				for(std::size_t i=0;i<children_.size();i++)
				{
					children_[i].write(output, tabs+2);
				}
				output << "</" << type_ << ">\n";
			}
		}

		void write(std::ostream& output) const
		{
			write(output, 0);
		}

	private:
		std::string type_;
		std::map<std::string, std::string> parameters_;
		std::string contents_;
		std::vector<XMLWriter> children_;
	};

	struct GradientMode
	{
		enum ID
		{
			yellow_red,
			cyan_green,
			cyan_blue
		};
	};

	class SVGWriter : public XMLWriter
	{
	public:
		SVGWriter(const double width, const double height, const std::string& style, const bool interactive) noexcept : XMLWriter("svg")
		{
			set("xmlns", "http://www.w3.org/2000/svg");
			set("width", width);
			set("height", height);
			if(!style.empty())
			{
				set("style", style);
			}
			if(interactive)
			{
				set("onload", "init(evt)");
			}
		}

		SVGWriter& add_rect(const double x, const double y, const double width, const double height, const std::string& style, const std::string& onclick) noexcept
		{
			XMLWriter obj("rect");
			obj.set("x", x).set("y", y).set("width", width).set("height", height).set("style", style);
			if(!onclick.empty())
			{
				obj.set("onclick", onclick);
			}
			add_child(obj);
			return (*this);
		}

		SVGWriter& add_rect(const double x, const double y, const double width, const double height, const std::string& style) noexcept
		{
			return add_rect(x, y, width, height, style, std::string());
		}

		SVGWriter& add_circle(const double cx, const double cy, const double r, const std::string& style) noexcept
		{
			add_child(XMLWriter("circle").set("cx", cx).set("cy", cy).set("r", r).set("style", style));
			return (*this);
		}

		SVGWriter& add_line(const double x1, const double y1, const double x2, const double y2, const std::string& style) noexcept
		{
			add_child(XMLWriter("line").set("x1", x1).set("y1", y1).set("x2", x2).set("y2", y2).set("style", style));
			return (*this);
		}

		SVGWriter& add_text(const std::string& text, const double x, const double y, const double rotation_angle, const double rotation_cx, const double rotation_cy, const std::string& style) noexcept
		{
			XMLWriter obj("text");
			obj.set("x", x).set("y", y);
			if(rotation_angle!=0.0)
			{
				std::ostringstream rotation_output;
				rotation_output << "rotate(" << rotation_angle << " " << rotation_cx << " " << rotation_cy << ")";
				obj.set("transform", rotation_output.str());
			}
			obj.set("style", style).set(text);
			add_child(obj);
			return (*this);
		}

		SVGWriter& add_text(const std::string& text, const double x, const double y, const std::string& style) noexcept
		{
			return add_text(text, x, y, 0.0, 0.0, 0.0, style);
		}
	};

	struct CoordKey
	{
		std::string chain;
		int residue_number;
		std::string icode;
		std::string residue_id;
		std::string atom_name;

		CoordKey(const LevelMode::ID mode, const SphereLabeling::SphereLabel& sl) noexcept : residue_number(0)
		{
			chain=sl.chain_id;
			if(mode==LevelMode::inter_residue || mode==LevelMode::inter_atom)
			{
				if(sl.expanded_residue_id.valid)
				{
					residue_number=sl.expanded_residue_id.rnum;
					icode=sl.expanded_residue_id.icode;
				}
				residue_id=sl.residue_id;
				if(mode==LevelMode::inter_atom)
				{
					atom_name=sl.atom_name;
				}
			}
		}

		CoordKey(const LevelMode::ID mode, const CoordKey& ck) noexcept : residue_number(0)
		{
			chain=ck.chain;
			if(mode==LevelMode::inter_residue || mode==LevelMode::inter_atom)
			{
				residue_number=ck.residue_number;
				icode=ck.icode;
				residue_id=ck.residue_id;
				if(mode==LevelMode::inter_atom)
				{
					atom_name=ck.atom_name;
				}
			}
		}

		bool operator==(const CoordKey& v) const noexcept
		{
			return (chain==v.chain && residue_number==v.residue_number && icode==v.icode && residue_id==v.residue_id && atom_name==v.atom_name);
		}

		bool operator!=(const CoordKey& v) const noexcept
		{
			return (!((*this)==v));
		}

		bool operator<(const CoordKey& v) const noexcept
		{
			if(chain<v.chain)
			{
				return true;
			}
			else if(chain==v.chain)
			{
				if(residue_number<v.residue_number)
				{
					return true;
				}
				else if(residue_number==v.residue_number)
				{
					if(icode<v.icode)
					{
						return true;
					}
					else if(icode==v.icode)
					{
						if(residue_id<v.residue_id)
						{
							return true;
						}
						else if(residue_id==v.residue_id)
						{
							return (atom_name<v.atom_name);
						}
					}
				}
			}
			return false;
		}

		std::string caption() const noexcept
		{
			return (chain+(residue_id.empty() ? std::string() : std::string(" ")+residue_id)+(atom_name.empty() ? std::string() : std::string(" ")+atom_name));
		}

		double unscaled_caption_width() const noexcept
		{
			return (static_cast<double>(chain.size()+residue_id.size()+atom_name.size()+(residue_id.empty() ? 0 : 1)+(atom_name.empty() ? 0 : 1)));
		}
	};

	struct PointKey
	{
		CoordKey a;
		CoordKey b;
		PointKey(const CoordKey& ck1, const CoordKey& ck2) noexcept : a(ck1<ck2 ? ck1 : ck2), b(ck1<ck2 ? ck2 : ck1)
		{
		}

		bool operator<(const PointKey& v) const noexcept
		{
			if(a<v.a)
			{
				return true;
			}
			else if(a==v.a)
			{
				return (b<v.b);
			}
			return false;
		}
	};

	struct PointValue
	{
		double area;
		double wr;
		double wg;
		double wb;

		PointValue() noexcept : area(0.0), wr(0.0), wg(0.0), wb(0.0)
		{
		}

		void add(const double more_area, const double r, const double g, const double b) noexcept
		{
			area+=more_area;
			wr+=r*more_area;
			wg+=g*more_area;
			wb+=b*more_area;
		}

		void add(const double more_area, const unsigned int rgb) noexcept
		{
			add(more_area, static_cast<double>((rgb&0xFF0000) >> 16)/static_cast<double>(0xFF), static_cast<double>((rgb&0x00FF00) >> 8)/static_cast<double>(0xFF), static_cast<double>(rgb&0x0000FF)/static_cast<double>(0xFF));
		}

		void add(const double more_area) noexcept
		{
			add(more_area, 0.0, 0.0, 0.0);
		}

		std::string mean_color() const noexcept
		{
			return color_from_red_green_blue_components(wr, wg, wb, (area>0.0 ? (255.0/area) : 255.0));
		}
	};

	struct RegionValue
	{
		bool initialized;
		double a;
		double b;

		RegionValue() noexcept : initialized(false), a(0.0), b(0.0)
		{
		}

		void update(const double v) noexcept
		{
			if(!initialized)
			{
				a=v;
				b=v;
				initialized=true;
			}
			else
			{
				a=std::min(a, v);
				b=std::max(b, v);
			}
		}
	};

	static std::string color_from_red_green_blue_components(const double r, const double g, const double b, const double scale) noexcept
	{
		std::ostringstream output;
		output << "rgb(" << static_cast<unsigned int>(r*scale) << "," << static_cast<unsigned int>(g*scale) << "," << static_cast<unsigned int>(b*scale) << ")";
		return output.str();
	}

	static std::string color_from_gradient(const double input_value, const double left_value, const double right_value, const GradientMode::ID gradient_mode) noexcept
	{
		double value=input_value;
		if(left_value<right_value)
		{
			value=(value-left_value)/(right_value-left_value);
		}
		else
		{
			value=1.0-((value-right_value)/(left_value-right_value));
		}
		double r=0;
		double g=0;
		double b=0;
		if(gradient_mode==GradientMode::yellow_red)
		{
			if(value<0.0)
			{
				r=1.0;
				g=1.0;
			}
			else if(value>1.0)
			{
				r=1.0;
			}
			else
			{
				r=1.0;
				g=1.0-value;
			}
		}
		else if(gradient_mode==GradientMode::cyan_green)
		{
			r=0.1;
			if(value<0.0)
			{
				g=1.0;
				b=1.0;
			}
			else if(value>1.0)
			{
				g=1.0;
			}
			else
			{
				g=1.0;
				b=1.0-value;
			}
		}
		else if(gradient_mode==GradientMode::cyan_blue)
		{
			r=0.25;
			if(value<0.0)
			{
				g=1.0;
				b=1.0;
			}
			else if(value>1.0)
			{
				b=1.0;
			}
			else
			{
				g=1.0-value;
				b=1.0;
			}
		}
		return color_from_red_green_blue_components(r, g, b, 255.0);
	}

	void add_point(const SphereLabeling::SphereLabel& sl1, const SphereLabeling::SphereLabel& sl2, const double area, const unsigned int color) noexcept
	{
		CoordKey a(mode_, sl1);
		CoordKey b(mode_, sl2);
		if(a!=b)
		{
			map_of_points_[PointKey(a, b)].add(area, color);
		}
	}

	LevelMode::ID mode_;
	std::map<PointKey, PointValue> map_of_points_;
};

}

#endif /* VORONOTALT_CLI_CONTACT_PLOTTER_H_ */
