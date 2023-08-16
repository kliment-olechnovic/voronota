#ifndef AUXILIARIES_COLOR_UTILITIES_H_
#define AUXILIARIES_COLOR_UTILITIES_H_

#include <vector>
#include <cmath>
#include <string>
#include <sstream>

namespace voronota
{

namespace auxiliaries
{

class ColorUtilities
{
public:
	typedef unsigned int ColorInteger;

	static ColorInteger null_color()
	{
		return 0x1FFFFFF;
	}

	static bool color_valid(const ColorInteger color)
	{
		return (color<=0xFFFFFF);
	}

	template<typename T>
	static void color_to_components(const ColorInteger color, T* components, const bool normalized)
	{
		components[0]=static_cast<T>((color&0xFF0000) >> 16);
		components[1]=static_cast<T>((color&0x00FF00) >> 8);
		components[2]=static_cast<T>(color&0x0000FF);

		if(normalized)
		{
			components[0]/=static_cast<T>(0xFF);
			components[1]/=static_cast<T>(0xFF);
			components[2]/=static_cast<T>(0xFF);
		}
	}

	template<typename T>
	static ColorInteger color_from_components(const T* components, const bool normalized)
	{
		if(normalized)
		{
			return ((static_cast<ColorInteger>(components[0]*static_cast<T>(0xFF)) << 16)+
					(static_cast<ColorInteger>(components[1]*static_cast<T>(0xFF)) << 8)+
					static_cast<ColorInteger>(components[2]*static_cast<T>(0xFF)));
		}
		else
		{
			return ((static_cast<ColorInteger>(components[0]) << 16)+
					(static_cast<ColorInteger>(components[1]) << 8)+
					static_cast<ColorInteger>(components[2]));
		}
	}

	static ColorInteger color_from_name(const char name)
	{
		if(name=='r')
		{
			return 0xFF0000;
		}
		else if(name=='g')
		{
			return 0x00FF00;
		}
		else if(name=='b')
		{
			return 0x0000FF;
		}
		else if(name=='y')
		{
			return 0xFFFF00;
		}
		else if(name=='c')
		{
			return 0x00FFFF;
		}
		else if(name=='m')
		{
			return 0xFF00FF;
		}
		else if(name=='o')
		{
			return 0xFF8000;
		}
		else if(name=='a')
		{
			return 0x0080FF;
		}
		else if(name=='p')
		{
			return 0x8000FF;
		}
		else if(name=='e')
		{
			return 0xFF0080;
		}
		else if(name=='w')
		{
			return 0xFFFFFF;
		}
		else if(name=='l')
		{
			return 0xAFAFAF;
		}
		else if(name=='s')
		{
			return 0x7F7F7F;
		}
		else if(name=='d')
		{
			return 0x4F4F4F;
		}
		else if(name=='n')
		{
			return 0x000000;
		}
		return null_color();
	}

	static ColorInteger color_from_name(const std::string& name)
	{
		if(name=="null")
		{
			return null_color();
		}
		else if(name.size()==1)
		{
			return color_from_name(name[0]);
		}
		else if(name=="red")
		{
			return 0xFF0000;
		}
		else if(name=="green")
		{
			return 0x00FF00;
		}
		else if(name=="blue")
		{
			return 0x0000FF;
		}
		else if(name=="yellow")
		{
			return 0xFFFF00;
		}
		else if(name=="cyan")
		{
			return 0x00FFFF;
		}
		else if(name=="magenta")
		{
			return 0xFF00FF;
		}
		else if(name=="orange")
		{
			return 0xFF8000;
		}
		else if(name=="white")
		{
			return 0xFFFFFF;
		}
		else if(name=="black")
		{
			return 0x000000;
		}
		else if(name.size()==8 && name.rfind("0x", 0)==0 && name.find_first_not_of("0123456789ABCDEF", 2)==std::string::npos)
		{
			ColorInteger color_int=0;
			std::istringstream color_input(name);
			color_input >> std::hex >> color_int;
			if(!color_input.fail() && color_int<=0xFFFFFF)
			{
				return color_int;
			}
		}
		else if(name.size()==6 && name.find_first_not_of("0123456789ABCDEF")==std::string::npos)
		{
			return color_from_name(std::string("0x")+name);
		}
		return null_color();
	}

	static ColorInteger color_from_gradient(const std::vector<ColorInteger>& anchors, const double value)
	{
		if(anchors.empty())
		{
			return null_color();
		}

		for(std::size_t i=0;i<anchors.size();i++)
		{
			if(!color_valid(anchors[i]))
			{
				return null_color();
			}
		}

		const std::size_t N=anchors.size();

		if(N==1)
		{
			return anchors[0];
		}

		if(value<=0.0)
		{
			return anchors[0];
		}
		else if(value>=1.0)
		{
			return anchors[N-1];
		}

		const std::size_t i=static_cast<std::size_t>(floor(static_cast<double>(N-1)*value));

		if(i>=(N-1))
		{
			return anchors[N-1];
		}

		const std::size_t j=(i+1);

		double subvalue=0.0;

		{
			const double pos_i=static_cast<double>(i)/static_cast<double>(N-1);
			const double pos_j=static_cast<double>(j)/static_cast<double>(N-1);
			subvalue=(value-pos_i)/(pos_j-pos_i);
		}

		if(subvalue<=0.0)
		{
			return anchors[i];
		}
		else if(subvalue>=1.0)
		{
			return anchors[j];
		}

		unsigned int mix[3]={0, 0, 0};

		{
			unsigned int c1[3]={0, 0, 0};
			color_to_components(anchors[i], c1, false);

			unsigned int c2[3]={0, 0, 0};
			color_to_components(anchors[j], c2, false);

			double w1=(1.0-subvalue);
			double w2=subvalue;

			for(int a=0;a<3;a++)
			{
				const unsigned int raw=static_cast<unsigned int>(floor(static_cast<double>(c1[a])*w1+static_cast<double>(c2[a])*w2));
				mix[a]=std::min(raw, static_cast<unsigned int>(0xFF));
			}
		}

		return color_from_components(mix, false);
	}

	static bool set_gradient_anchors_from_name(const std::string& name, std::vector<ColorInteger>& anchors)
	{
		if(name.empty())
		{
			return false;
		}

		if(name=="rainbow")
		{
			return set_gradient_anchors_from_name("rygcb", anchors);
		}
		else if(name=="reverse-rainbow")
		{
			return set_gradient_anchors_from_name("bcgyr", anchors);
		}
		else if(name=="blue-white-red")
		{
			return set_gradient_anchors_from_name("bwr", anchors);
		}
		else if(name=="red-white-blue")
		{
			return set_gradient_anchors_from_name("rwb", anchors);
		}

		for(std::size_t i=0;i<name.size();i++)
		{
			if(!color_valid(color_from_name(name[i])))
			{
				return false;
			}
		}

		anchors.resize(name.size(), 0);
		for(std::size_t i=0;i<name.size();i++)
		{
			anchors[i]=color_from_name(name[i]);
		}

		return true;
	}

	static ColorInteger color_from_gradient(const std::string& name, const double value)
	{
		std::vector<ColorInteger> anchors;

		if(!set_gradient_anchors_from_name(name, anchors))
		{
			return null_color();
		}

		return color_from_gradient(anchors, value);
	}

	static ColorInteger color_sum(const ColorInteger a, const ColorInteger b)
	{
		return ((std::min(static_cast<ColorInteger>(((a&0xFF0000) >> 16)+((b&0xFF0000) >> 16)), static_cast<ColorInteger>(0xFF)) << 16)+
				(std::min(static_cast<ColorInteger>(((a&0x00FF00) >> 8) +((b&0x00FF00) >> 8)),  static_cast<ColorInteger>(0xFF)) << 8)+
				(std::min(static_cast<ColorInteger>(((a&0x0000FF))      +((b&0x0000FF))),       static_cast<ColorInteger>(0xFF))));
	}
};

}

}

#endif /* AUXILIARIES_COLOR_UTILITIES_H_ */
