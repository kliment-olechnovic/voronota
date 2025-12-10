#ifndef VORONOTALT_CLI_CONTACT_PLOTTER_H_
#define VORONOTALT_CLI_CONTACT_PLOTTER_H_

#include <sstream>
#include <string>
#include <vector>
#include <map>
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

	ContactPlotter(const LevelMode::ID mode) : mode_(mode)
	{
	}

	template<class ContactsContainer>
	bool add_contact(const std::size_t i, const ContactsContainer& contacts, const std::vector<SphereLabeling::SphereLabel>& sphere_labels) noexcept
	{
		if(i<contacts.size() && contacts[i].id_a<sphere_labels.size() && contacts[i].id_b<sphere_labels.size())
		{
			add_point(sphere_labels[contacts[i].id_a], sphere_labels[contacts[i].id_b], contacts[i].area);
			return true;
		}
		return false;
	}

	bool write_to_file(const std::string& filename, const std::string& config) noexcept
	{
		if(map_of_points_.empty())
		{
			return false;
		}

		std::ofstream output(filename, std::ios::out);
		if(!output.good())
		{
			return false;
		}

		const bool xlabeled=(config.find("xlabeled")!=std::string::npos);
		const bool ylabeled=(config.find("ylabeled")!=std::string::npos);
		const bool allgradient=(config.find("allgradient")!=std::string::npos);
		const bool halfgradient=(config.find("halfgradient")!=std::string::npos);
		const bool dark=(config.find("dark")!=std::string::npos);

		const double scale=20.0;

		const std::string style_background_level0(dark ? "fill:#090909;" : "fill:#F0F0F0;");
		const std::string style_background_level1(dark ? "fill:#393939;" : "fill:#D9D9D9;");
		const std::string style_background_level2(dark ? "fill:#696969;" : "fill:#C0C0C0;");
		const std::string style_rect_default(dark ? "fill:#F9F9F9;" : "fill:#090909;");
		const std::string style_text_default("font-size:20px; fill:#000000;");

		std::map<CoordKey, double> map_of_coords;
		double max_area=0.0;
		for(std::map<PointKey, PointValue>::iterator it=map_of_points_.begin();it!=map_of_points_.end();++it)
		{
			map_of_coords[it->first.a]=0.0;
			map_of_coords[it->first.b]=0.0;
			max_area=std::max(max_area, it->second.area);
		}

		double length=0;
		double max_caption_width=0.0;
		for(std::map<CoordKey, double>::iterator it=map_of_coords.begin();it!=map_of_coords.end();++it)
		{
			it->second=length;
			length+=scale;
			max_caption_width=std::max(max_caption_width, it->first.unscaled_caption_width()*scale);
		}

		const double full_width=(ylabeled ? (length+max_caption_width) : length);
		const double full_height=(xlabeled ? (length+max_caption_width) : length);
		const double shift_y=(xlabeled ? max_caption_width : 0.0);

		SVGWriter svg(full_width, full_height);
		svg.set("style", "font-family:monospace;");
		svg.add_rect(0, 0+shift_y, length, length, style_background_level0);

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

			std::map<CoordKey, RegionValue> map_of_coord_regions_on_chain_level;
			for(std::map<CoordKey, double>::iterator it=map_of_coords.begin();it!=map_of_coords.end();++it)
			{
				RegionValue& rv=map_of_coord_regions_on_chain_level[CoordKey(LevelMode::inter_chain, it->first)];
				rv.update(it->second);
			}

			for(std::map<PointKey, PointValue>::iterator it=map_of_points_on_chain_level.begin();it!=map_of_points_on_chain_level.end();++it)
			{
				const RegionValue& x=map_of_coord_regions_on_chain_level[it->first.a];
				const RegionValue& y=map_of_coord_regions_on_chain_level[it->first.b];
				svg.add_rect(x.a, y.a+shift_y, x.b+scale-x.a, y.b+scale-y.a, style_background_level1);
				svg.add_rect(y.a, x.a+shift_y, y.b+scale-y.a, x.b+scale-x.a, style_background_level1);
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

			std::map<CoordKey, RegionValue> map_of_coord_regions_on_residue_level;
			for(std::map<CoordKey, double>::iterator it=map_of_coords.begin();it!=map_of_coords.end();++it)
			{
				RegionValue& rv=map_of_coord_regions_on_residue_level[CoordKey(LevelMode::inter_residue, it->first)];
				rv.update(it->second);
			}

			for(std::map<PointKey, PointValue>::iterator it=map_of_points_on_residue_level.begin();it!=map_of_points_on_residue_level.end();++it)
			{
				const RegionValue& x=map_of_coord_regions_on_residue_level[it->first.a];
				const RegionValue& y=map_of_coord_regions_on_residue_level[it->first.b];
				svg.add_rect(x.a, y.a+shift_y, x.b+scale-x.a, y.b+scale-y.a, style_background_level2);
				svg.add_rect(y.a, x.a+shift_y, y.b+scale-y.a, x.b+scale-x.a, style_background_level2);
			}
		}

		for(std::map<PointKey, PointValue>::const_iterator it=map_of_points_.begin();it!=map_of_points_.end();++it)
		{
			const double x=map_of_coords[it->first.a];
			const double y=map_of_coords[it->first.b];

			if(allgradient || halfgradient)
			{
				const std::string style_rect_colored=std::string("fill:")+SVGWriter::color_from_gradient(it->second.area, 0.0, max_area)+std::string(";");
				svg.add_rect(x, y+shift_y, scale, scale, (allgradient ? style_rect_colored : (y<x ? style_rect_colored : style_rect_default)));
				svg.add_rect(y, x+shift_y, scale, scale, (allgradient ? style_rect_colored : (x<y ? style_rect_colored : style_rect_default)));
			}
			else
			{
				svg.add_rect(x, y+shift_y, scale, scale, style_rect_default);
				svg.add_rect(y, x+shift_y, scale, scale, style_rect_default);
			}
		}

		if(xlabeled)
		{
			for(std::map<CoordKey, double>::iterator it=map_of_coords.begin();it!=map_of_coords.end();++it)
			{
				const double x=it->second+scale;
				const double y=0.0-scale*0.1+shift_y;
				svg.add_text(it->first.caption(), x, y, -90.0, x, y, style_text_default);
			}
		}

		if(ylabeled)
		{
			for(std::map<CoordKey, double>::iterator it=map_of_coords.begin();it!=map_of_coords.end();++it)
			{
				const double x=length+scale*0.1;
				const double y=scale+it->second-scale*0.2+shift_y;
				svg.add_text(it->first.caption(), x, y, style_text_default);
			}
		}

		svg.write(output);

		return true;
	}

private:
	class XMLWriter
	{
	public:
		explicit XMLWriter(const std::string& type) : type_(type)
		{
		}

		template<typename T>
		XMLWriter& set(const std::string& name, const T& value, const std::string& units="")
		{
			std::ostringstream output;
			output << value << units;
			parameters_[name]=output.str();
			return (*this);
		}

		XMLWriter& set(const std::string& contents)
		{
			contents_=contents;
			return (*this);
		}

		XMLWriter& add_child(const XMLWriter& child)
		{
			children_.push_back(child);
			return (*this);
		}

		void write(std::ostream& output, const std::size_t tabs) const
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

	class SVGWriter : public XMLWriter
	{
	public:
		SVGWriter(const double width, const double height) : XMLWriter("svg")
		{
			set("width", width);
			set("height", height);
		}

		static std::string color_from_red_green_blue_components(const double r, const double g, const double b, const double scale)
		{
			std::ostringstream output;
			output << "rgb(" << static_cast<unsigned int>(r*scale) << "," << static_cast<unsigned int>(g*scale) << "," << static_cast<unsigned int>(b*scale) << ")";
			return output.str();
		}

		static std::string color_from_gradient(const double input_value, const double left_value, const double right_value)
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
			return color_from_red_green_blue_components(r, g, b, 255.0);
		}

		SVGWriter& add_rect(const double x, const double y, const double width, const double height, const std::string& style)
		{
			add_child(XMLWriter("rect").set("x", x).set("y", y).set("width", width).set("height", height).set("style", style));
			return (*this);
		}

		SVGWriter& add_circle(const double cx, const double cy, const double r, const std::string& style)
		{
			add_child(XMLWriter("circle").set("cx", cx).set("cy", cy).set("r", r).set("style", style));
			return (*this);
		}

		SVGWriter& add_line(const double x1, const double y1, const double x2, const double y2, const std::string& style)
		{
			add_child(XMLWriter("line").set("x1", x1).set("y1", y1).set("x2", x2).set("y2", y2).set("style", style));
			return (*this);
		}

		SVGWriter& add_text(const std::string& text, const double x, const double y, const std::string& style)
		{
			add_child(XMLWriter("text").set("x", x).set("y", y).set("style", style).set(text));
			return (*this);
		}

		SVGWriter& add_text(const std::string& text, const double x, const double y, const double rotation_angle, const double rotation_cx, const double rotation_cy, const std::string& style)
		{
			std::ostringstream rotation_output;
			rotation_output << "rotate(" << rotation_angle << " " << rotation_cx << " " << rotation_cy << ")";
			add_child(XMLWriter("text").set("x", x).set("y", y).set("transform", rotation_output.str()).set("style", style).set(text));
			return (*this);
		}
	};

	struct CoordKey
	{
		std::string chain;
		int residue_number;
		std::string icode;
		std::string residue_id;
		std::string atom_name;

		CoordKey(const LevelMode::ID mode, const SphereLabeling::SphereLabel& sl) : residue_number(0)
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

		CoordKey(const LevelMode::ID mode, const CoordKey& ck) : residue_number(0)
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

		bool operator==(const CoordKey& v) const
		{
			return (chain==v.chain && residue_number==v.residue_number && icode==v.icode && residue_id==v.residue_id && atom_name==v.atom_name);
		}

		bool operator!=(const CoordKey& v) const
		{
			return (!((*this)==v));
		}

		bool operator<(const CoordKey& v) const
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

		std::string caption() const
		{
			return (chain+(residue_id.empty() ? std::string() : std::string(" ")+residue_id)+(atom_name.empty() ? std::string() : std::string(" ")+atom_name));
		}

		double unscaled_caption_width() const
		{
			return (static_cast<double>(chain.size()+residue_id.size()+atom_name.size()+2));
		}
	};

	struct PointKey
	{
		CoordKey a;
		CoordKey b;
		PointKey(const CoordKey& ck1, const CoordKey& ck2) : a(ck1<ck2 ? ck1 : ck2), b(ck1<ck2 ? ck2 : ck1)
		{
		}

		bool operator<(const PointKey& v) const
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

		PointValue() : area(0.0)
		{
		}

		void add(const double more_area)
		{
			area+=more_area;
		}
	};

	struct RegionValue
	{
		bool initialized;
		double a;
		double b;

		RegionValue() : initialized(false), a(0.0), b(0.0)
		{
		}

		void update(const double v)
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

	void add_point(const SphereLabeling::SphereLabel& sl1, const SphereLabeling::SphereLabel& sl2, const double area)
	{
		CoordKey a(mode_, sl1);
		CoordKey b(mode_, sl2);
		if(a!=b)
		{
			map_of_points_[PointKey(a, b)].add(area);
		}
	}

	LevelMode::ID mode_;
	std::map<PointKey, PointValue> map_of_points_;
};

}

#endif /* VORONOTALT_CLI_CONTACT_PLOTTER_H_ */
