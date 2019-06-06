#ifndef COMPATABILITY_TR1_USAGE_H_
#define COMPATABILITY_TR1_USAGE_H_

#ifndef USE_TR1
#if __cplusplus >= 201103L
#define USE_TR1 0
#elif defined(__clang__)
#define USE_TR1 0
#elif defined(__GNUC__)
#define USE_TR1 1
#else
#define USE_TR1 0
#endif
#endif

#endif /* COMPATABILITY_TR1_USAGE_H_ */
