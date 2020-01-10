#include <fstream>
#include <set>
#include <map>

#include "../auxiliaries/program_options_handler.h"

namespace
{

std::set<std::string> read_file_as_set(const std::string& filename)
{
	std::set<std::string> result;
	std::ifstream input(filename.c_str(), std::ios::in);
	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		if(!line.empty())
		{
			{
				const std::size_t comments_pos=line.find("#", 0);
				if(comments_pos!=std::string::npos)
				{
					line=line.substr(0, comments_pos);
				}
			}
			if(!line.empty())
			{
				result.insert(line);
			}
		}
	}
	return result;
}

std::map< std::string, std::pair<int, int> > join_two_sets(const std::set<std::string>& set1, const std::set<std::string>& set2)
{
	std::map< std::string, std::pair<int, int> > result;
	for(std::set<std::string>::const_iterator it=set1.begin();it!=set1.end();++it)
	{
		result[*it].first=1;
	}
	for(std::set<std::string>::const_iterator it=set2.begin();it!=set2.end();++it)
	{
		result[*it].second=1;
	}
	return result;
}

std::map< std::pair<int, int>, int > summarize_join_map(const std::map< std::string, std::pair<int, int> >& join_map)
{
	std::map< std::pair<int, int>, int > result;
	for(std::map< std::string, std::pair<int, int> >::const_iterator it=join_map.begin();it!=join_map.end();++it)
	{
		result[it->second]++;
	}
	return result;
}

}

void compare_files_as_sets(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "nothing");
	pohw.describe_io("stdout", false, true, "results");

	const std::string file1=poh.argument<std::string>(pohw.describe_option("--file1", "string", "first file path"));
	const std::string file2=poh.argument<std::string>(pohw.describe_option("--file2", "string", "second file path"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::map< std::pair<int, int>, int > summary=summarize_join_map(join_two_sets(read_file_as_set(file1), read_file_as_set(file2)));

	std::cout << "m11 m10 m01\n";
	std::cout << summary[std::make_pair(1, 1)] << " ";
	std::cout << summary[std::make_pair(1, 0)] << " ";
	std::cout << summary[std::make_pair(0, 1)] << "\n";
}

