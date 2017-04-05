#ifndef COMPATABILITY_MACROS_H_
#define COMPATABILITY_MACROS_H_

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

#endif /* COMPATABILITY_MACROS_H_ */
