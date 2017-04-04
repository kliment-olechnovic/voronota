#ifndef COMPATABILITY_MACROS_H_
#define COMPATABILITY_MACROS_H_

#ifndef USE_TR1
#if __cplusplus >= 201103L
#define USE_TR1 0
#elif defined(__clang__) || defined(_MSC_VER)
#define USE_TR1 0
#elif defined(__GNUC__) && __GNUC__ > 4
#define USE_TR1 0
#else
#define USE_TR1 1
#endif
#endif

#endif /* COMPATABILITY_MACROS_H_ */
