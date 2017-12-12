#ifndef AUXILIARIES_COLOR_UTILITIES_H_
#define AUXILIARIES_COLOR_UTILITIES_H_

#include <vector>
#include <cmath>
#include <string>

namespace auxiliaries
{

class ColorUtilities
{
public:
	typedef unsigned int ColorInteger;

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

	static ColorInteger color_from_name(const std::string& name)
	{
		if(name=="r" || name=="red")
		{
			return 0xFF0000;
		}
		else if(name=="g" || name=="green")
		{
			return 0x00FF00;
		}
		else if(name=="b" || name=="blue")
		{
			return 0x0000FF;
		}
		else if(name=="y" || name=="yellow")
		{
			return 0xFFFF00;
		}
		else if(name=="c" || name=="cyan")
		{
			return 0x00FFFF;
		}
		else if(name=="m" || name=="magenta")
		{
			return 0xFF00FF;
		}
		else if(name=="w" || name=="white")
		{
			return 0xFFFFFF;
		}
		return 0;
	}

	static ColorInteger color_from_gradient(const std::vector<ColorInteger>& anchors, const double value)
	{
		if(anchors.empty())
		{
			return 0;
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

		{
			unsigned int max_of_mix=std::max(mix[0], std::max(mix[1], mix[2]));
			if(max_of_mix<static_cast<unsigned int>(0xFF))
			{
				const double k=static_cast<double>(0xFF)/static_cast<double>(max_of_mix);
				for(int a=0;a<3;a++)
				{
					mix[a]=std::min(static_cast<unsigned int>(static_cast<double>(mix[a])*k), static_cast<unsigned int>(0xFF));
				}
			}
		}

		return color_from_components(mix, false);
	}

	static ColorInteger color_from_gradient(const ColorInteger& anchor0, const ColorInteger& anchor1, const double value)
	{
		static std::vector<ColorInteger> anchors(2, 0);
		anchors[0]=anchor0;
		anchors[1]=anchor1;
		return color_from_gradient(anchors, value);
	}

	static ColorInteger color_from_gradient(const ColorInteger& anchor0, const ColorInteger& anchor1, const ColorInteger& anchor2, const double value)
	{
		static std::vector<ColorInteger> anchors(3, 0);
		anchors[0]=anchor0;
		anchors[1]=anchor1;
		anchors[2]=anchor2;
		return color_from_gradient(anchors, value);
	}

private:
};

}

#endif /* AUXILIARIES_COLOR_UTILITIES_H_ */
