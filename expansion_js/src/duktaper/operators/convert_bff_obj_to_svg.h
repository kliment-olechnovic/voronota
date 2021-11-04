#ifndef DUKTAPER_OPERATORS_CONVERT_BFF_OBJ_TO_SVG_H_
#define DUKTAPER_OPERATORS_CONVERT_BFF_OBJ_TO_SVG_H_

#include "../operators_common.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ConvertBFFObjToSVG : public scripting::OperatorBase<ConvertBFFObjToSVG>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		Result()
		{
		}

		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	std::string input_obj_file;
	std::string input_colors_file;
	double scaling_factor;
	std::string output_svg_file;

	ConvertBFFObjToSVG() : scaling_factor(1000.0)
	{
	}

	void initialize(scripting::CommandInput& input, const bool managed)
	{
		if(!managed)
		{
			input_obj_file=input.get_value<std::string>("input-obj-file");
			scripting::assert_file_name_input(input_obj_file, false);
			input_colors_file=input.get_value_or_default<std::string>("input-colors-file", "");
			scripting::assert_file_name_input(input_colors_file, true);
		}
		scaling_factor=input.get_value_or_default<double>("scaling-factor", 1000.0);
		output_svg_file=input.get_value<std::string>("output-svg-file");
		scripting::assert_file_name_input(output_svg_file, false);
	}

	void initialize(scripting::CommandInput& input)
	{
		initialize(input, false);
	}

	void document(scripting::CommandDocumentation& doc, const bool managed) const
	{
		if(!managed)
		{
			doc.set_option_decription(CDOD("input-obj-file", CDOD::DATATYPE_STRING, "input BFF Obj file"));
			doc.set_option_decription(CDOD("input-colors-file", CDOD::DATATYPE_STRING, "input colors file", ""));
		}
		doc.set_option_decription(CDOD("scaling-factor", CDOD::DATATYPE_FLOAT, "scaling of max output box size", 1000.0));
		doc.set_option_decription(CDOD("output-svg-file", CDOD::DATATYPE_STRING, "output SVG file"));
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		document(doc, false);
	}

	Result run(void*) const
	{
		if(input_obj_file.empty())
		{
			throw std::runtime_error(std::string("No input BFF Obj file"));
		}

		if(output_svg_file.empty())
		{
			throw std::runtime_error(std::string("No output SVG file"));
		}

		scripting::assert_file_name_input(input_obj_file, false);
		scripting::assert_file_name_input(input_colors_file, true);
		scripting::assert_file_name_input(output_svg_file, false);

		std::vector<VTPoint> vt_points(1, VTPoint());
		std::map<FaceID, FaceContent> faces;

		{
			scripting::InputSelector finput_selector(input_obj_file);
			std::istream& finput=finput_selector.stream();
			scripting::assert_io_stream(input_obj_file, finput);

			while(finput.good())
			{
				std::string line;
				std::getline(finput, line);
				if(line.rfind("vt ", 0)==0)
				{
					std::istringstream linput(line);
					std::string token;
					VTPoint vtp;
					linput >> token >> vtp.x >> vtp.y;
					if(linput.fail())
					{
						throw std::runtime_error(std::string("Invalid obj line '")+line+"'.");
					}
					vt_points.push_back(vtp);
				}
				else if(line.rfind("f ", 0)==0)
				{
					std::string modline=line;
					int slashes=0;
					for(std::size_t i=0;i<modline.size();i++)
					{
						if(modline[i]=='/')
						{
							slashes++;
							modline[i]=' ';
						}
					}
					if(slashes!=3)
					{
						throw std::runtime_error(std::string("Invalid structure of obj line '")+line+"'.");
					}
					std::istringstream linput(modline);
					std::string token;
					FaceID fid;
					FaceContent fc;
					linput >> token;
					for(unsigned int i=0;i<3;i++)
					{
						linput >> fid.v_indices[i] >> fc.vt_indices[i];
					}
					if(linput.fail())
					{
						throw std::runtime_error(std::string("Invalid obj line '")+line+"'.");
					}
					faces[fid]=fc;
				}
			}
		}

		if(!input_colors_file.empty())
		{
			scripting::InputSelector finput_selector(input_colors_file);
			std::istream& finput=finput_selector.stream();
			scripting::assert_io_stream(input_colors_file, finput);

			while(finput.good())
			{
				std::string line;
				std::getline(finput, line);
				if(!line.empty())
				{
					std::istringstream linput(line);
					FaceID fid;
					unsigned int color=0;
					for(unsigned int i=0;i<3;i++)
					{
						linput >> fid.v_indices[i];
					}
					linput >> color;
					if(linput.fail())
					{
						throw std::runtime_error(std::string("Invalid color line '")+line+"'.");
					}
					std::map<FaceID, FaceContent>::iterator faces_it=faces.find(fid);
					if(faces_it!=faces.end())
					{
						faces_it->second.color=color;
					}
				}
			}
		}

		VTPoint vt_max(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest());
		{
			VTPoint vt_min(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
			for(std::size_t i=1;i<vt_points.size();i++)
			{
				const VTPoint& vt=vt_points[i];
				vt_min.x=std::min(vt.x, vt_min.x);
				vt_min.y=std::min(vt.y, vt_min.y);
				vt_max.x=std::max(vt.x, vt_max.x);
				vt_max.y=std::max(vt.y, vt_max.y);
			}
			for(std::size_t i=1;i<vt_points.size();i++)
			{
				VTPoint& vt=vt_points[i];
				vt.x-=vt_min.x;
				vt.y-=vt_min.y;
			}
			vt_max.x-=vt_min.x;
			vt_max.y-=vt_min.y;
			const double max_coord=std::max(vt_max.x, vt_max.y);
			for(std::size_t i=1;i<vt_points.size();i++)
			{
				VTPoint& vt=vt_points[i];
				vt.x=vt.x/max_coord*scaling_factor;
				vt.y=vt.y/max_coord*scaling_factor;
			}
			vt_max.x=vt_max.x/max_coord*scaling_factor;
			vt_max.y=vt_max.y/max_coord*scaling_factor;
		}

		{
			scripting::OutputSelector output_selector(output_svg_file);
			std::ostream& foutput=output_selector.stream();
			scripting::assert_io_stream(output_svg_file, foutput);

			const int width=static_cast<int>(vt_max.x+0.5);
			const int height=static_cast<int>(vt_max.y+0.5);

			foutput << "<svg";
			foutput << " viewBox=\"0 0 " << width << " " << height << "\"";
			foutput << " shape-rendering=\"crispEdges\"";
			foutput << " xmlns=\"http://www.w3.org/2000/svg\"";
			foutput << " >\n";
			foutput << "<rect width=\"" << width << "\"";
			foutput << " height=\"" << height << "\"";
			foutput << " fill=\"rgb(0,0,0)\"";
			foutput << " />\n";
			for(std::map<FaceID, FaceContent>::const_iterator faces_it=faces.begin();faces_it!=faces.end();++faces_it)
			{
				const FaceContent& fc=faces_it->second;
				if(fc.valid_vt_indices(vt_points.size()))
				{
					foutput << "<polygon";
					foutput << " points=\"";
					for(unsigned int i=0;i<3;i++)
					{
						const VTPoint& vt=vt_points[fc.vt_indices[i]];
						foutput << vt.x << "," << vt.y << (i<2 ? " " : "");
					}
					foutput << "\"";
					foutput << " fill=\"rgb(";
					foutput << fc.red() << ",";
					foutput << fc.green() << ",";
					foutput << fc.blue() << ")\"";
					foutput << " />\n";
				}
			}
			foutput << "</svg>\n";
		}

		Result result;
		return result;
	}

private:
	struct VTPoint
	{
		double x;
		double y;

		VTPoint() : x(0.0), y(0.0)
		{
		}

		VTPoint(const float x, const float y) : x(x), y(y)
		{
		}
	};

	struct FaceID
	{
		long v_indices[3];

		FaceID()
		{
			v_indices[0]=0;
			v_indices[1]=0;
			v_indices[2]=0;
		}

		bool operator<(const FaceID& fid) const
		{
			for(unsigned int i=0;i<3;i++)
			{
				if(v_indices[i]<fid.v_indices[i])
				{
					return true;
				}
				else if(v_indices[i]>fid.v_indices[i])
				{
					return false;
				}
			}
			return false;
		}

		bool operator==(const FaceID& fid) const
		{
			for(unsigned int i=0;i<3;i++)
			{
				if(v_indices[i]!=fid.v_indices[i])
				{
					return false;
				}
			}
			return true;
		}
	};

	struct FaceContent
	{
		long vt_indices[3];
		unsigned int color;

		FaceContent() : color(0)
		{
			vt_indices[0]=0;
			vt_indices[1]=0;
			vt_indices[2]=0;
		}

		bool valid_vt_indices(const long array_size) const
		{
			for(unsigned int i=0;i<3;i++)
			{
				if(vt_indices[i]>=array_size)
				{
					return false;
				}
			}
			return true;
		}

		unsigned int red() const
		{
			return ((color&0xFF0000) >> 16);
		}

		unsigned int green() const
		{
			return ((color&0x00FF00) >> 8);
		}

		unsigned int blue() const
		{
			return (color&0x0000FF);
		}
	};
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CONVERT_BFF_OBJ_TO_SVG_H_ */
