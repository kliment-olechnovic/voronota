#include "../auxiliaries/program_options_handler.h"

#include "../common/contact_value.h"

#include "modescommon/drawing_utilities.h"

namespace
{

typedef common::ChainResidueAtomDescriptorsPair CRADsPair;

}

void draw_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts graphics')");
	pohw.describe_io("stdout", false, true, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts graphics')");

	const std::string drawing_for_pymol=poh.argument<std::string>(pohw.describe_option("--drawing-for-pymol", "string", "file path to output drawing as pymol script"), "");
	const std::string drawing_for_jmol=poh.argument<std::string>(pohw.describe_option("--drawing-for-jmol", "string", "file path to output drawing as jmol script"), "");
	const std::string drawing_for_scenejs=poh.argument<std::string>(pohw.describe_option("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"), "");
	const std::string drawing_name=poh.argument<std::string>(pohw.describe_option("--drawing-name", "string", "graphics object name for drawing output"), "contacts");
	modescommon::DrawingParametersWrapper drawing_parameters_wrapper;
	drawing_parameters_wrapper.default_color=poh.convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>(pohw.describe_option("--default-color", "string", "default color for drawing output, in hex format, white is 0xFFFFFF"), "0xFFFFFF"));
	drawing_parameters_wrapper.adjunct_gradient=poh.argument<std::string>(pohw.describe_option("--adjunct-gradient", "string", "adjunct name to use for gradient-based coloring"), "");
	drawing_parameters_wrapper.adjunct_gradient_blue=poh.argument<double>(pohw.describe_option("--adjunct-gradient-blue", "number", "blue adjunct gradient value"), 0.0);
	drawing_parameters_wrapper.adjunct_gradient_red=poh.argument<double>(pohw.describe_option("--adjunct-gradient-red", "number", "red adjunct gradient value"), 1.0);
	drawing_parameters_wrapper.adjuncts_rgb=poh.contains_option(pohw.describe_option("--adjuncts-rgb", "", "flag to use RGB color values from adjuncts"));
	drawing_parameters_wrapper.random_colors=poh.contains_option(pohw.describe_option("--random-colors", "", "flag to use random color for each drawn contact"));
	drawing_parameters_wrapper.alpha_opacity=poh.argument<double>(pohw.describe_option("--alpha", "number", "alpha opacity value for drawing output"), 1.0);
	drawing_parameters_wrapper.use_labels=poh.contains_option(pohw.describe_option("--use-labels", "", "flag to use labels in drawing if possible"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	common::enabled_output_of_ContactValue_graphics()=true;

	std::map<CRADsPair, common::ContactValue> map_of_contacts;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, map_of_contacts);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(!(drawing_for_pymol.empty() && drawing_for_jmol.empty() && drawing_for_scenejs.empty()))
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(drawing_parameters_wrapper.default_color);
		opengl_printer.add_alpha(drawing_parameters_wrapper.alpha_opacity);
		for(std::map< CRADsPair, common::ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair& crads=it->first;
			const common::ContactValue& value=it->second;
			if(!value.graphics.empty())
			{
				drawing_parameters_wrapper.process(std::make_pair(crads.a, crads.b), value.props.adjuncts, opengl_printer);
				opengl_printer.add(value.graphics);
			}
		}

		if(!drawing_for_pymol.empty())
		{
			std::ofstream foutput(drawing_for_pymol.c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_pymol_script(drawing_name, true, foutput);
			}
		}

		if(!drawing_for_jmol.empty())
		{
			std::ofstream foutput(drawing_for_jmol.c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_jmol_script(drawing_name, foutput);
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
	}

	auxiliaries::IOUtilities().write_map(map_of_contacts, std::cout);
}
