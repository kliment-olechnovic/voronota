#include <iostream>

#include "apollota/apollonius_triangulation.h"

#include "auxiliaries/command_line_options.h"

void calculate_triangulation(const auxiliaries::CommandLineOptions& clo)
{
	clo.check_allowed_options("--epsilon: --bounding-spheres-hierarchy-first-radius: --allow-redundant-quadruples --skip-output --print-log --check");
}
