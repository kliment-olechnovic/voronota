#ifndef SVG_WRITER_H_
#define SVG_WRITER_H_

#include "../auxiliaries/xml_writer.h"

namespace
{

class SVGWriter : public auxiliaries::XMLWriter
{
public:
	SVGWriter(const unsigned int width, const unsigned int height) : XMLWriter("svg")
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

	SVGWriter& add_rect(const unsigned int x, const unsigned int y, const unsigned int width, const unsigned int height, const std::string& color)
	{
		add_child(XMLWriter("rect").set("x", x).set("y", y).set("width", width).set("height", height).set("fill", color));
		return (*this);
	}
};

}

#endif /* SVG_WRITER_H_ */
