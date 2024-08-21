#ifndef VORONOTALT_TERMINAL_UTILITIES_H_
#define VORONOTALT_TERMINAL_UTILITIES_H_

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
#include <unistd.h>
#define ISATTY_AVAILABLE
#endif

namespace voronotalt
{

inline bool is_stdin_from_terminal() noexcept
{
#ifdef ISATTY_AVAILABLE
	return (isatty(fileno(stdin))==1);
#else
	return false;
#endif

}

}

#endif /* VORONOTALT_TERMINAL_UTILITIES_H_ */
