#ifndef DUKTAPER_TERMINAL_UTILITIES_H_
#define DUKTAPER_TERMINAL_UTILITIES_H_

#include <string>
#include <unistd.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

namespace voronota
{

namespace duktaper
{

class TerminalUtilities
{
public:
	static bool is_stdin_from_terminal()
	{
		return (isatty(fileno(stdin))==1);
	}

	static std::string read_line_from_stdin(bool& failed)
	{
		char* line=readline("> ");
		failed=(line==0);
		if(!failed)
		{
			std::string result;
			if(*line)
			{
				add_history(line);
				result=line;
			}
			free(static_cast<void*>(line));
			return result;
		}
		return std::string();
	}
};

}

}

#endif /* DUKTAPER_TERMINAL_UTILITIES_H_ */
