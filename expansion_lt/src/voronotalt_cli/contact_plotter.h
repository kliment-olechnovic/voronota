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
	template<class ContactsContainer>
	bool add_contact(
			const std::size_t i,
			const ContactsContainer& contacts,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels) noexcept
	{
		if(i<contacts.size() && contacts[i].id_a<sphere_labels.size() && contacts[i].id_b<sphere_labels.size())
		{
			const SphereLabeling::SphereLabel& sl1=sphere_labels[contacts[i].id_a];
			const SphereLabeling::SphereLabel& sl2=sphere_labels[contacts[i].id_b];
			if(sl1.expanded_residue_id.valid && sl2.expanded_residue_id.valid)
			{
				add_point(CoordKey(sl1.chain_id, sl1.expanded_residue_id.rnum, sl1.expanded_residue_id.icode, sl1.atom_name), CoordKey(sl2.chain_id, sl2.expanded_residue_id.rnum, sl2.expanded_residue_id.icode, sl2.atom_name), contacts[i].area);
			}
			else
			{
				add_point(CoordKey(sl1.chain_id, sl1.residue_id, sl1.atom_name), CoordKey(sl2.chain_id, sl2.residue_id, sl2.atom_name), contacts[i].area);
			}
			return true;
		}
		return false;
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	bool add_contact_residue_level(
			const std::size_t i,
			const ContactsContainer& contacts,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids,
			const GroupedContactsContainer& grouped_contacts) noexcept
	{
		if(i<grouped_contacts_representative_ids.size() && i<grouped_contacts.size())
		{
			const std::size_t j=grouped_contacts_representative_ids[i];
			if(j<contacts.size() && contacts[j].id_a<sphere_labels.size() && contacts[j].id_b<sphere_labels.size())
			{
				const SphereLabeling::SphereLabel& sl1=sphere_labels[contacts[j].id_a];
				const SphereLabeling::SphereLabel& sl2=sphere_labels[contacts[j].id_b];
				if(sl1.expanded_residue_id.valid && sl2.expanded_residue_id.valid)
				{
					add_point(CoordKey(sl1.chain_id, sl1.expanded_residue_id.rnum, sl1.expanded_residue_id.icode), CoordKey(sl2.chain_id, sl2.expanded_residue_id.rnum, sl2.expanded_residue_id.icode), grouped_contacts[i].area);
				}
				else
				{
					add_point(CoordKey(sl1.chain_id, sl1.residue_id), CoordKey(sl2.chain_id, sl2.residue_id), grouped_contacts[i].area);
				}
				return true;
			}
		}
		return false;
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	bool add_contact_chain_level(
			const std::size_t i,
			const ContactsContainer& contacts,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids,
			const GroupedContactsContainer& grouped_contacts) noexcept
	{
		if(i<grouped_contacts_representative_ids.size() && i<grouped_contacts.size())
		{
			const std::size_t j=grouped_contacts_representative_ids[i];
			if(j<contacts.size() && contacts[j].id_a<sphere_labels.size() && contacts[j].id_b<sphere_labels.size())
			{
				const SphereLabeling::SphereLabel& sl1=sphere_labels[contacts[j].id_a];
				const SphereLabeling::SphereLabel& sl2=sphere_labels[contacts[j].id_b];
				add_point(CoordKey(sl1.chain_id), CoordKey(sl2.chain_id), grouped_contacts[i].area);
				return true;
			}
		}
		return false;
	}

	bool write_to_file(const std::string& filename) noexcept
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

		const double scale=1.0;

		double length=0;
		for(std::map<CoordKey, double>::iterator it=map_of_coords_.begin();it!=map_of_coords_.end();++it)
		{
			it->second=length;
			length+=scale;
		}

		SVGWriter svg(length, length);
		//svg.set("style", "font-family:monospace;");
		svg.add_rect(0, 0, length, length, std::string("fill:#CCCCCC;"));

		for(std::map<PointKey, PointValue>::const_iterator it=map_of_points_.begin();it!=map_of_points_.end();++it)
		{
			const double x=map_of_coords_[it->first.a];
			const double y=map_of_coords_[it->first.b];
			svg.add_rect(x, y, scale, scale, std::string("fill:#000000;"));
			svg.add_rect(y, x, scale, scale, std::string("fill:#000000;"));
		}

//		for(std::map<CoordKey, double>::iterator it=map_of_coords_.begin();it!=map_of_coords_.end();++it)
//		{
//			const double x=length+5.0;
//			const double y=it->second;
//			const std::string text=it->first.chain;
//			svg.add_text(text, x, y, std::string("font-size:20px; fill:#000000;"));
//		}

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

		static std::string color_from_blue_white_red_gradient(const double input_value, const double blue_value, const double red_value)
		{
			double value=input_value;
			if(blue_value<red_value)
			{
				value=(value-blue_value)/(red_value-blue_value);
			}
			else
			{
				value=1.0-((value-red_value)/(blue_value-red_value));
			}
			double r=0;
			double g=0;
			double b=0;
			if(value<0.0)
			{
				b=1.0;
			}
			else if(value>1.0)
			{
				r=1.0;
			}
			else if(value<=0.5)
			{
				b=1.0;
				r=(value/0.5);
				g=r;
			}
			else
			{
				r=1.0;
				b=(1.0-(value-0.5)/0.5);
				g=b;
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
	};

	struct CoordKey
	{
		std::string chain;
		int residue_number;
		std::string icode;
		std::string residue_id;
		std::string atom_name;

		CoordKey() : residue_number(0)
		{
		}

		CoordKey(const std::string& chain) : chain(chain), residue_number(0)
		{
		}

		CoordKey(const std::string& chain, const std::string& residue_id) : chain(chain), residue_number(0), residue_id(residue_id)
		{
		}

		CoordKey(const std::string& chain, const std::string& residue_id, const std::string& atom_name) : chain(chain), residue_number(0), residue_id(residue_id), atom_name(atom_name)
		{
		}

		CoordKey(const std::string& chain, const int residue_number, const std::string& icode) : chain(chain), residue_number(residue_number), icode(icode)
		{
		}

		CoordKey(const std::string& chain, const int residue_number, const std::string& icode, const std::string& atom_name) : chain(chain), residue_number(residue_number), icode(icode), atom_name(atom_name)
		{
		}

		bool operator==(const CoordKey& v) const
		{
			return (chain==v.chain && residue_number==v.residue_number && icode==v.icode && atom_name==v.atom_name);
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
	};

	struct PointKey
	{
		CoordKey a;
		CoordKey b;
		PointKey(const CoordKey& phk1, const CoordKey& phk2) : a(phk1<phk2 ? phk1 : phk2), b(phk1<phk2 ? phk2 : phk1)
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

		PointValue(const double area) : area(area)
		{
		}
	};

	void add_point(const CoordKey& a, const CoordKey& b, const double area)
	{
		map_of_points_[PointKey(a, b)].area=area;
		map_of_coords_[a]=0;
		map_of_coords_[b]=0;
	}

	std::map<PointKey, PointValue> map_of_points_;
	std::map<CoordKey, double> map_of_coords_;
};

}

#endif /* VORONOTALT_CLI_CONTACT_PLOTTER_H_ */
