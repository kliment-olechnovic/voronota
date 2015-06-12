#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>

#include "auxiliaries/program_options_handler.h"

#include "modescommon/svg_writer.h"

namespace
{

struct Matrix
{
	struct Bin
	{
		double sum_energy;
		double count_all;
		double count_helix;
		double count_sheet;

		Bin() : sum_energy(0.0), count_all(0), count_helix(0), count_sheet(0)
		{
		}

		void add(const Bin& bin)
		{
			sum_energy+=bin.sum_energy;
			count_all+=bin.count_all;
			count_helix+=bin.count_helix;
			count_sheet+=bin.count_sheet;
		}

		void multiply(const double v)
		{
			sum_energy*=v;
			count_all*=v;
			count_helix*=v;
			count_sheet*=v;
		}
	};

	typedef std::vector< std::vector<Bin> > Data;

	const int size;
	Data data;

	Matrix(const int size) : size(size), data(size, std::vector<Bin>(size))
	{
	}

	int angle_to_index(const double angle) const
	{
		if(fabs(angle)>180.0)
		{
			throw std::runtime_error("Invalid angle for index.");
		}
		return std::max(0, std::min(size-1, static_cast<int>(floor((angle+180.0)/(360.0/static_cast<double>(size))))));
	}

	void feed(const std::string ss, const double phi, const double psi, const double energy)
	{
		if(fabs(phi)<=180.0 && fabs(psi)<=180.0)
		{
			Bin& bin=data[angle_to_index(phi)][angle_to_index(psi)];
			bin.count_all++;
			bin.sum_energy+=energy;
			if(ss=="H")
			{
				bin.count_helix++;
			}
			else if(ss=="S")
			{
				bin.count_sheet++;
			}
		}
	}

	void remove_noise(const int window)
	{
		Data cleaned_data=data;
		for(int i=0;i<size;i++)
		{
			for(int j=0;j<size;j++)
			{
				if(data[i][j].count_all>0)
				{
					int empty_neighbors=0;
					int filled_neighbors=0;
					for(int wi=(i-window);wi<=(i+window);wi++)
					{
						for(int wj=(j-window);wj<=(j+window);wj++)
						{
							if(wi>=0 && wi<size && wj>=0 && wj<size)
							{
								if(data[wi][wj].count_all<=0)
								{
									empty_neighbors++;
								}
								else
								{
									filled_neighbors++;
								}
							}
						}
					}
					if(empty_neighbors>filled_neighbors)
					{
						cleaned_data[i][j]=Bin();
					}
				}
			}
		}
		data=cleaned_data;
	}

	void smooth(const int window)
	{
		Data smoothed_data(size, std::vector<Bin>(size));
		for(int i=0;i<size;i++)
		{
			for(int j=0;j<size;j++)
			{
				int empty_neighbors=0;
				int filled_neighbors=0;
				Bin neighbors_bin;
				for(int wi=(i-window);wi<=(i+window);wi++)
				{
					for(int wj=(j-window);wj<=(j+window);wj++)
					{
						if(wi>=0 && wi<size && wj>=0 && wj<size)
						{
							if(data[wi][wj].count_all<=0)
							{
								empty_neighbors++;
							}
							else
							{
								filled_neighbors++;
								neighbors_bin.add(data[wi][wj]);
							}
						}
					}
				}
				if(filled_neighbors>empty_neighbors)
				{
					neighbors_bin.multiply(1.0/static_cast<double>((2*window+1)*(2*window+1)));
					smoothed_data[i][j]=neighbors_bin;
				}
			}
		}
		data=smoothed_data;
	}
};

struct Color
{
	unsigned int r;
	unsigned int g;
	unsigned int b;

	Color() : r(0), g(0), b(0)
	{
	}

	Color(const unsigned int rgb) : r((rgb&0xFF0000) >> 16), g((rgb&0x00FF00) >> 8), b(rgb&0x0000FF)
	{
	}

	static Color from_blue_white_red_gradient(const double value)
	{
		Color color;
		if(value<0)
		{
			color.b=255;
		}
		else if(value>1)
		{
			color.r=255;
		}
		else if(value<=0.5)
		{
			color.b=255;
			color.r=static_cast<unsigned char>(255*(value/0.5));
			color.g=color.r;
		}
		else if(value>0.5)
		{
			color.r=255;
			color.b=static_cast<unsigned char>(255*(1-(value-0.5)/0.5));
			color.g=color.b;
		}
		return color;
	}

	static Color from_rainbow_gradient(const double value)
	{
		Color color;
		if(value<0)
		{
			color.b=255;
		}
		else if(value<=0.25)
		{
			color.g=static_cast<unsigned char>((value/0.25)*255);
			color.b=255;
		}
		else if(value<=0.5)
		{
			color.g=255;
			color.b=static_cast<unsigned char>((1.0-(value-0.25)/0.25)*255);
		}
		else if(value<=0.75)
		{
			color.r=static_cast<unsigned char>(((value-0.5)/0.25)*255);
			color.g=255;
		}
		else if(value<1.0)
		{
			color.r=255;
			color.g=static_cast<unsigned char>((1.0-(value-0.75)/0.25)*255);
		}
		else
		{
			color.r=255;
		}
		return color;
	}
};

typedef std::vector< std::vector<Color> > Image;

Image construct_image_from_matrix_by_counts(const Matrix& m)
{
	Image image(m.size, std::vector<Color>(m.size));
	std::vector<double> counts;
	counts.reserve(m.size*m.size);
	for(int i=0;i<m.size;i++)
	{
		for(int j=0;j<m.size;j++)
		{
			const Matrix::Bin& bin=m.data[i][j];
			if(bin.count_all>0)
			{
				counts.push_back(bin.count_all);
			}
		}
	}
	std::sort(counts.begin(), counts.end());
	const std::size_t q1=counts.size()/100*5;
	const std::size_t q2=counts.size()/100*95;
	const std::pair<double, double> gradient_range(counts[q1], counts[q2]);
	for(int i=0;i<m.size;i++)
	{
		for(int j=0;j<m.size;j++)
		{
			const Matrix::Bin& bin=m.data[i][j];
			if(bin.count_all>0)
			{
				const double count=bin.count_all;
				const double gradient_value=(count-gradient_range.first)/(gradient_range.second-gradient_range.first);
				image[i][j]=Color::from_rainbow_gradient(gradient_value);
			}
		}
	}
	return image;
}

Image construct_image_from_matrix_by_ss(const Matrix& m, const int mode)
{
	Image image(m.size, std::vector<Color>(m.size));
	for(int i=0;i<m.size;i++)
	{
		for(int j=0;j<m.size;j++)
		{
			const Matrix::Bin& bin=m.data[i][j];
			if(bin.count_all>0)
			{
				Color& color=image[i][j];
				if(mode==0)
				{
					color=Color::from_rainbow_gradient(bin.count_helix/bin.count_all);
				}
				else if(mode==1)
				{
					color=Color::from_rainbow_gradient(bin.count_sheet/bin.count_all);
				}
				else
				{
					color=Color::from_rainbow_gradient((bin.count_all-bin.count_helix-bin.count_sheet)/bin.count_all);
				}
			}
		}
	}
	return image;
}

Image construct_image_from_matrix_by_mean_energy(const Matrix& m)
{
	Image image(m.size, std::vector<Color>(m.size));
	std::vector<double> mean_energies;
	mean_energies.reserve(m.size*m.size);
	for(int i=0;i<m.size;i++)
	{
		for(int j=0;j<m.size;j++)
		{
			const Matrix::Bin& bin=m.data[i][j];
			if(bin.count_all>0)
			{
				const double mean_energy=(bin.sum_energy/bin.count_all);
				mean_energies.push_back(mean_energy);
			}
		}
	}
	std::sort(mean_energies.begin(), mean_energies.end());
	const std::size_t q1=mean_energies.size()/100*5;
	const std::size_t q2=mean_energies.size()/100*95;
	const std::pair<double, double> gradient_range(mean_energies[q1], mean_energies[q2]);
	for(int i=0;i<m.size;i++)
	{
		for(int j=0;j<m.size;j++)
		{
			const Matrix::Bin& bin=m.data[i][j];
			if(bin.count_all>0)
			{
				const double mean_energy=(bin.sum_energy/bin.count_all);
				const double gradient_value=(mean_energy-gradient_range.first)/(gradient_range.second-gradient_range.first);
				image[i][j]=Color::from_rainbow_gradient(gradient_value);
			}
		}
	}
	return image;
}

void print_image(const Image& image, std::ostream& output)
{
	if(!image.empty())
	{
		std::size_t w=image.size();
		std::size_t h=image.front().size();
		SVGWriter svg(w, h);
		svg.add_rect(0, 0, w, h, "#000000");
		for(std::size_t x=0;x<image.size();x++)
		{
			for(std::size_t y=0;y<image[x].size();y++)
			{
				const Color& color=image[x][y];
				svg.add_rect(x, (h-1-y), 1, 1, SVGWriter::color_from_red_green_blue_components(color.r, color.g, color.b, 1.0));
			}
		}
		svg.write(output);
	}
}

}

void generate_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--output-prefix", "string", "output file name prefix"));
		if(!poh.assert(ods, false))
		{
			return;
		}
	}

	const std::string output_prefix=poh.argument<std::string>("--output-prefix", "");

	Matrix matrix(360);

	int processed_lines=0;
	while(std::cin.good())
	{
		std::string line;
		std::getline(std::cin, line);
		if(!line.empty())
		{
			std::istringstream line_input(line);
			if(line_input.good())
			{
				std::string name;
				std::string ss;
				double phi=0.0;
				double psi=0.0;
				double energy=0.0;
				std::cin >> name >> ss >> phi >> psi >> energy;
				if(!line_input.fail() && !name.empty() && name[0]=='R')
				{
					matrix.feed(ss, phi, psi, energy);
				}
			}
		}
		processed_lines++;
	}

	matrix.remove_noise(10);
	matrix.smooth(10);

	{
		std::ofstream output((output_prefix+"counts.svg").c_str(), std::ios::out);
		print_image(construct_image_from_matrix_by_counts(matrix), output);
	}

	{
		std::ofstream output((output_prefix+"ss_helix.svg").c_str(), std::ios::out);
		print_image(construct_image_from_matrix_by_ss(matrix, 0), output);
	}

	{
		std::ofstream output((output_prefix+"ss_sheet.svg").c_str(), std::ios::out);
		print_image(construct_image_from_matrix_by_ss(matrix, 1), output);
	}

	{
		std::ofstream output((output_prefix+"ss_loop.svg").c_str(), std::ios::out);
		print_image(construct_image_from_matrix_by_ss(matrix, 2), output);
	}

	{
		std::ofstream output((output_prefix+"mean_energy.svg").c_str(), std::ios::out);
		print_image(construct_image_from_matrix_by_mean_energy(matrix), output);
	}
}
