#ifndef DUKTAPER_TERMINAL_UTILITIES_H_
#define DUKTAPER_TERMINAL_UTILITIES_H_

#include <string>
#include <unistd.h>
#include <stdio.h>

#include "../linenoise/linenoise.h"

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
		ManagedLineBuffer mlb(linenoise("> "));
		failed=(mlb.line==0);
		if(!failed)
		{
			std::string result;
			if(*mlb.line)
			{
				linenoiseHistoryAdd(mlb.line);
				result=mlb.line;
			}
			return result;
		}
		return std::string();
	}

private:
	struct ManagedLineBuffer
	{
		char* line;

		ManagedLineBuffer(char* line) : line(line)
		{
		}

		~ManagedLineBuffer()
		{
			if(line!=0)
			{
				free(static_cast<void*>(line));
			}
		}
	};
};

}

}

#endif /* DUKTAPER_TERMINAL_UTILITIES_H_ */
