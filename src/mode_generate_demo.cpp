#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
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
		unsigned long count_all;
		unsigned long count_helix;
		unsigned long count_sheet;

		Bin() : sum_energy(0.0), count_all(0), count_helix(0), count_sheet(0)
		{
		}
	};

	typedef std::vector< std::vector<Bin> > Data;

	static const int size=360;

	Data data;

	Matrix() : data(size, std::vector<Bin>(size))
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
				image[i][j]=Color::from_blue_white_red_gradient(gradient_value);
			}
		}
	}
	return image;
}

Image construct_image_from_matrix_by_ss(const Matrix& m)
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
				color.r=static_cast<unsigned int>(static_cast<double>(bin.count_helix)/static_cast<double>(bin.count_all)*255.0);
				color.g=static_cast<unsigned int>(static_cast<double>(bin.count_sheet)/static_cast<double>(bin.count_all)*255.0);
				color.b=static_cast<unsigned int>(static_cast<double>(bin.count_all-(bin.count_helix+bin.count_sheet))/static_cast<double>(bin.count_all)*255.0);
			}
		}
	}
	return image;
}

Image construct_image_from_matrix_by_ss_without_loops(const Matrix& m)
{
	Image image(m.size, std::vector<Color>(m.size));
	for(int i=0;i<m.size;i++)
	{
		for(int j=0;j<m.size;j++)
		{
			const Matrix::Bin& bin=m.data[i][j];
			if((bin.count_helix+bin.count_sheet)>0)
			{
				Color& color=image[i][j];
				color.r=static_cast<unsigned int>(static_cast<double>(bin.count_helix)/static_cast<double>(bin.count_helix+bin.count_sheet)*255.0);
				color.g=static_cast<unsigned int>(static_cast<double>(bin.count_sheet)/static_cast<double>(bin.count_helix+bin.count_sheet)*255.0);
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
				const double mean_energy=(bin.sum_energy/static_cast<double>(bin.count_all));
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
				const double mean_energy=(bin.sum_energy/static_cast<double>(bin.count_all));
				const double gradient_value=(mean_energy-gradient_range.first)/(gradient_range.second-gradient_range.first);
				image[i][j]=Color::from_blue_white_red_gradient(gradient_value);
			}
		}
	}
	return image;
}

void print_image(const Image& image)
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
		svg.write(std::cout);
	}
}

}

void generate_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--print-counts-image", "", "flag to print counts image"));
		ods.push_back(OD("--print-ss-image", "", "flag to print secondary structures image"));
		ods.push_back(OD("--print-ss-without-loops-image", "", "flag to print secondary structures image, without loops"));
		ods.push_back(OD("--print-energies-image", "", "flag to print mean energies image"));
		if(!poh.assert(ods, false))
		{
			return;
		}
	}

	const bool print_ss_image=poh.contains_option("--print-ss-image");
	const bool print_counts_image=poh.contains_option("--print-counts-image");
	const bool print_ss_without_loops_image=poh.contains_option("--print-ss-without-loops-image");
	const bool print_energies_image=poh.contains_option("--print-energies-image");

	Matrix matrix;

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

	if(print_counts_image)
	{
		print_image(construct_image_from_matrix_by_counts(matrix));
	}

	if(print_ss_image)
	{
		print_image(construct_image_from_matrix_by_ss(matrix));
	}

	if(print_ss_without_loops_image)
	{
		print_image(construct_image_from_matrix_by_ss_without_loops(matrix));
	}

	if(print_energies_image)
	{
		print_image(construct_image_from_matrix_by_mean_energy(matrix));
	}
}
