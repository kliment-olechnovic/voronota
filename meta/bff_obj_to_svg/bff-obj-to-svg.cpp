#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <limits>
#include <exception>

namespace
{

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

	FaceID() : v_indices{0, 0, 0}
	{
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

	FaceContent() : vt_indices{0, 0, 0}, color(0)
	{
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

}

int main(const int argc, const char** argv)
{
	try
	{
		std::vector<std::string> command_args;
		for(int i=1;i<argc;i++)
		{
			command_args.push_back(argv[i]);
		}

		if(command_args.size()!=3)
		{
			throw std::runtime_error(std::string("Invalid number of parameters, must be 3: input obj file, input colors file, output svg file."));
		}

		std::vector<VTPoint> vt_points(1, VTPoint());
		std::map<FaceID, FaceContent> faces;

		{
			std::ifstream finput(command_args[0], std::ios::in);
			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+command_args[0]+"'.");
			}
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

		std::cout << "Points: " << vt_points.size() << "\n";
		std::cout << "Faces: " << faces.size() << "\n";

		{
			std::ifstream finput(command_args[1], std::ios::in);
			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+command_args[1]+"'.");
			}
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

		const double scaling_factor=1000.0;

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
			std::ofstream foutput(command_args[2], std::ios::out);
			if(!foutput.good())
			{
				throw std::runtime_error(std::string("Failed to write file '")+command_args[2]+"'.");
			}
			foutput << "<svg viewBox=\"0 0 " << static_cast<int>(vt_max.x+0.5) << " " << static_cast<int>(vt_max.y+0.5) << "\"";
			foutput << " xmlns=\"http://www.w3.org/2000/svg\"";
			foutput << ">\n";
			for(std::map<FaceID, FaceContent>::const_iterator faces_it=faces.begin();faces_it!=faces.end();++faces_it)
			{
				const FaceContent& fc=faces_it->second;
				if(fc.valid_vt_indices(vt_points.size()))
				{
					foutput << "<polygon points=\"";
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
					foutput << " shape-rendering=\"crispEdges\"";
					foutput << " />\n";
				}
			}
			foutput << "</svg>\n";
		}

		return 0;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown exception caught." << std::endl;
	}

	return 1;
}
