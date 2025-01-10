#ifndef VORONOTALT_PARALLELIZATION_CONFIGURATION_H_
#define VORONOTALT_PARALLELIZATION_CONFIGURATION_H_

#ifdef _OPENMP
#define VORONOTALT_OPENMP
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

}

#endif /* VORONOTALT_PARALLELIZATION_CONFIGURATION_H_ */
