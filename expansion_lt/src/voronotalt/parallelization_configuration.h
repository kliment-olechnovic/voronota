#ifndef VORONOTALT_PARALLELIZATION_CONFIGURATION_H_
#define VORONOTALT_PARALLELIZATION_CONFIGURATION_H_

#ifndef VORONOTALT_DISABLE_OPENMP
#ifdef _OPENMP
#define VORONOTALT_OPENMP
#endif
#endif

#ifdef VORONOTALT_OPENMP
#include <omp.h>
#endif

namespace voronotalt
{

bool openmp_enabled() noexcept
{
#ifdef VORONOTALT_OPENMP
	return true;
#else
	return false;
#endif
}

unsigned int openmp_set_num_threads_if_possible(const unsigned int max_number_of_processors) noexcept
{
#ifdef VORONOTALT_OPENMP
	omp_set_num_threads(max_number_of_processors);
	return max_number_of_processors;
#else
	return 1;
#endif
}

int openmp_get_max_threads() noexcept
{
#ifdef VORONOTALT_OPENMP
	return omp_get_max_threads();
#else
	return 1;
#endif
}

}

#endif /* VORONOTALT_PARALLELIZATION_CONFIGURATION_H_ */
