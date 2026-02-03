#ifndef CADSCORELT_PARALLELIZATION_CONFIGURATION_H_
#define CADSCORELT_PARALLELIZATION_CONFIGURATION_H_

#ifndef CADSCORELT_DISABLE_OPENMP
#ifdef _OPENMP
#define CADSCORELT_OPENMP
#endif
#endif

#ifdef CADSCORELT_OPENMP
#include <omp.h>
#endif

namespace cadscorelt
{

bool openmp_enabled() noexcept
{
#ifdef CADSCORELT_OPENMP
	return true;
#else
	return false;
#endif
}

unsigned int openmp_setup(const unsigned int max_number_of_processors) noexcept
{
#ifdef CADSCORELT_OPENMP
	omp_set_num_threads(max_number_of_processors);
	omp_set_max_active_levels(1);
	return max_number_of_processors;
#else
	return 1;
#endif
}

}

#endif /* CADSCORELT_PARALLELIZATION_CONFIGURATION_H_ */
