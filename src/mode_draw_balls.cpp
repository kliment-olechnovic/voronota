#include "auxiliaries/program_options_handler.h"

#include "modescommon/drawing_links.h"
#include "modescommon/drawing_protein_cartoons.h"
#include "modescommon/drawing_nucleic_acid_cartoons.h"

void draw_balls(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const std::string representation=poh.argument<std::string>(pohw.describe_option("--representation", "string", "representation name: 'vdw', 'sticks', 'trace' or 'cartoon'"), "vdw");
	const std::string drawing_for_pymol=poh.argument<std::string>(pohw.describe_option("--drawing-for-pymol", "string", "file path to output drawing as pymol script"), "");
	const std::string drawing_for_scenejs=poh.argument<std::string>(pohw.describe_option("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"), "");
	const std::string drawing_name=poh.argument<std::string>(pohw.describe_option("--drawing-name", "string", "graphics object name for drawing output"), "balls");
	DrawingParametersWrapper drawing_parameters_wrapper;
	drawing_parameters_wrapper.default_color=poh.convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>(pohw.describe_option("--default-color", "string", "default color for drawing output, in hex format, white is 0xFFFFFF"), "0xFFFFFF"));
	drawing_parameters_wrapper.adjunct_gradient=poh.argument<std::string>(pohw.describe_option("--adjunct-gradient", "string", "adjunct name to use for gradient-based coloring"), "");
	drawing_parameters_wrapper.adjunct_gradient_blue=poh.argument<double>(pohw.describe_option("--adjunct-gradient-blue", "number", "blue adjunct gradient value"), 0.0);
	drawing_parameters_wrapper.adjunct_gradient_red=poh.argument<double>(pohw.describe_option("--adjunct-gradient-red", "number", "red adjunct gradient value"), 1.0);
	drawing_parameters_wrapper.rainbow_gradient=poh.contains_option(pohw.describe_option("--rainbow-gradient", "", "flag to use rainbow color gradient"));
	drawing_parameters_wrapper.adjuncts_rgb=poh.contains_option(pohw.describe_option("--adjuncts-rgb", "", "flag to use RGB color values from adjuncts"));
	drawing_parameters_wrapper.random_colors=poh.contains_option(pohw.describe_option("--random-colors", "", "flag to use random color for each drawn ball"));
	drawing_parameters_wrapper.random_colors_by_chain=poh.contains_option(pohw.describe_option("--random-colors-by-chain", "", "flag to use random color for each drawn chain"));
	drawing_parameters_wrapper.use_labels=poh.contains_option(pohw.describe_option("--use-labels", "", "flag to use labels in drawing if possible"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	typedef std::vector< std::pair<CRAD, BallValue> > ListOfBalls;
	const ListOfBalls list_of_balls=auxiliaries::IOUtilities().read_lines_to_map<ListOfBalls>(std::cin);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	auxiliaries::OpenGLPrinter opengl_printer;
	opengl_printer.add_color(drawing_parameters_wrapper.default_color);
	if(representation=="vdw")
	{
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const CRAD& crad=list_of_balls[i].first;
			const BallValue& value=list_of_balls[i].second;
			drawing_parameters_wrapper.process(crad, value.props.adjuncts, opengl_printer);
			opengl_printer.add_sphere(value);
		}
	}
	else if(representation=="sticks")
	{
		draw_links(list_of_balls, 0.8, 4.0, 0.2, 0.2, 9, false, drawing_parameters_wrapper, opengl_printer);
	}
	else if(representation=="trace")
	{
		draw_trace(list_of_balls, "CA", 4.0, 0.3, drawing_parameters_wrapper, opengl_printer);
		draw_trace(list_of_balls, "C3'", 8.0, 0.3, drawing_parameters_wrapper, opengl_printer);
	}
	else if(representation=="cartoon")
	{
		DrawingProteinCartoons().draw_cartoon(list_of_balls, drawing_parameters_wrapper, opengl_printer);
		DrawingNucleicAcidCartoons().draw_cartoon(list_of_balls, drawing_parameters_wrapper, opengl_printer);
	}
	else
	{
		throw std::runtime_error("Invalid representation name.");
	}

	if(!drawing_for_pymol.empty())
	{
		std::ofstream foutput(drawing_for_pymol.c_str(), std::ios::out);
		if(foutput.good())
		{
			opengl_printer.print_pymol_script(drawing_name, true, foutput);
		}
	}

	if(!drawing_for_scenejs.empty())
	{
		std::ofstream foutput(drawing_for_scenejs.c_str(), std::ios::out);
		if(foutput.good())
		{
			opengl_printer.print_scenejs_script(drawing_name, true, foutput);
		}
	}

	auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
}
