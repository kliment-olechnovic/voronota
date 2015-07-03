#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/opengl_printer.h"

#include "modescommon/contact_value.h"
#include "modescommon/drawing_utilities.h"

void draw_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts graphics')");
	pohw.describe_io("stdout", false, true, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts graphics')");

	const std::string drawing_for_pymol=poh.argument<std::string>(pohw.describe_option("--drawing-for-pymol", "string", "file path to output drawing as pymol script"), "");
	const std::string drawing_for_jmol=poh.argument<std::string>(pohw.describe_option("--drawing-for-jmol", "string", "file path to output drawing as jmol script"), "");
	const std::string drawing_for_scenejs=poh.argument<std::string>(pohw.describe_option("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"), "");
	const std::string drawing_name=poh.argument<std::string>(pohw.describe_option("--drawing-name", "string", "graphics object name for drawing output"), "contacts");
	const unsigned int drawing_color=poh.convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>(pohw.describe_option("--drawing-color", "string", "color for drawing output, in hex format, white is 0xFFFFFF"), "0xFFFFFF"));
	const std::string drawing_adjunct_gradient=poh.argument<std::string>(pohw.describe_option("--drawing-adjunct-gradient", "string", "adjunct name to use for gradient-based coloring"), "");
	const bool drawing_reverse_gradient=poh.contains_option(pohw.describe_option("--drawing-reverse-gradient", "", "flag to use reversed gradient for drawing"));
	const bool drawing_random_colors=poh.contains_option(pohw.describe_option("--drawing-random-colors", "", "flag to use random color for each drawn contact"));
	const double drawing_alpha=poh.argument<double>(pohw.describe_option("--drawing-alpha", "number", "alpha opacity value for drawing output"), 1.0);
	const bool drawing_labels=poh.contains_option(pohw.describe_option("--drawing-labels", "", "flag to use labels in drawing if possible"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	enabled_output_of_ContactValue_graphics()=true;

	std::map<CRADsPair, ContactValue> map_of_contacts;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, map_of_contacts);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	auxiliaries::OpenGLPrinter opengl_printer;
	opengl_printer.add_color(drawing_color);
	opengl_printer.add_alpha(drawing_alpha);
	for(std::map< CRADsPair, ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const CRADsPair& crads=it->first;
		const ContactValue& value=it->second;
		if(!value.graphics.empty())
		{
			if(drawing_labels)
			{
				opengl_printer.add_label(construct_label_from_two_crads(crads.a, crads.b));
			}

			if(!drawing_adjunct_gradient.empty())
			{
				if(value.props.adjuncts.count(drawing_adjunct_gradient)>0)
				{
					const double gradient_value=value.props.adjuncts.find(drawing_adjunct_gradient)->second;
					opengl_printer.add_color_from_blue_white_red_gradient(drawing_reverse_gradient ? (1.0-gradient_value) : gradient_value);
				}
				else
				{
					opengl_printer.add_color(drawing_color);
				}
			}
			else if(drawing_random_colors)
			{
				opengl_printer.add_color(calc_two_crads_color_integer(crads.a, crads.b));
			}

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

	auxiliaries::IOUtilities().write_map(map_of_contacts, std::cout);
}
