#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"
#include "../auxiliaries/color_utilities.h"

namespace
{

class PPMImage
{
public:
	PPMImage(const std::size_t width, const std::size_t height) :
		width_(width),
		height_(height),
		pixels_(width*height*3, 0)
	{
	}

	std::size_t height() const
	{
		return height_;
	}

	std::size_t width() const
	{
		return width_;
	}

	char* color(const std::size_t x, const std::size_t y)
	{
		if(y<height_ || x<width_)
		{
			return (&(pixels_[(y*width_+x)*3]));
		}
		return 0;
	}

	void write(std::ostream& output) const
	{
		output << "P6\n" << width_ << " " << height_ << "\n255\n";
		output.write(&(pixels_[0]), pixels_.size());
	}

private:
	std::size_t width_;
	std::size_t height_;
	std::vector<char> pixels_;
};

}

void draw_values_bitmap(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "values");
	pohw.describe_io("stdout", false, true, "picture in PPM format");

	const std::string color_stops=poh.argument<std::string>(pohw.describe_option("--color-stops", "string", "string of coded color stops"), "rygcb");
	const int max_lines=poh.restrict_value_in_range(1, 10000, poh.argument<int>(pohw.describe_option("--max-lines", "number", "maximum number of lines, default is 1"), 1));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<double> values;
	voronota::auxiliaries::IOUtilities().read_lines_to_set(std::cin, values);
	if(values.size()<2)
	{
		throw std::runtime_error("Less than 2 points provided to stdin.");
	}

	const int N=static_cast<int>(values.size());
	const int number_of_lines=std::min(N, max_lines);
	const int number_of_columns=((N%number_of_lines==0) ? (N/number_of_lines) : ((N/number_of_lines)+1));

	PPMImage image(number_of_columns, number_of_lines);
	for(int i=0;i<N;i++)
	{
		const int y=i/number_of_columns;
		const int x=i%number_of_columns;
		unsigned char the_color[3]={0, 0, 0};
		voronota::auxiliaries::ColorUtilities::color_to_components(voronota::auxiliaries::ColorUtilities::color_from_gradient(color_stops, values[i]), the_color, false);
		char* color=image.color(x, y);
		for(int j=0;j<3;j++)
		{
			color[j]=static_cast<char>(the_color[j]);
		}
	}
	image.write(std::cout);
}
