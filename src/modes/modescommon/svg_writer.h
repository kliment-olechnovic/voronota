#ifndef MODESCOMMON_SVG_WRITER_H_
#define MODESCOMMON_SVG_WRITER_H_

#include "../../auxiliaries/xml_writer.h"

namespace voronota
{

namespace modescommon
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

	SVGWriter& add_rect(const unsigned int x, const unsigned int y, const unsigned int width, const unsigned int height, const std::string& style)
	{
		add_child(XMLWriter("rect").set("x", x).set("y", y).set("width", width).set("height", height).set("style", style));
		return (*this);
	}

	SVGWriter& add_circle(const unsigned int cx, const unsigned int cy, const unsigned int r, const std::string& style)
	{
		add_child(XMLWriter("circle").set("cx", cx).set("cy", cy).set("r", r).set("style", style));
		return (*this);
	}

	SVGWriter& add_line(const unsigned int x1, const unsigned int y1, const unsigned int x2, const unsigned int y2, const std::string& style)
	{
		add_child(XMLWriter("line").set("x1", x1).set("y1", y1).set("x2", x2).set("y2", y2).set("style", style));
		return (*this);
	}
};

}

}

#endif /* MODESCOMMON_SVG_WRITER_H_ */
