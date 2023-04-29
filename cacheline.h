#ifndef __CACHE_LINE_H__
#define __CACHE_LINE_H__

#ifdef COHERENCY_LINE_SIZE
// passed at compile time 
#define CACHE_LINE_SIZE COHERENCY_LINE_SIZE
#else
// hardcoded default
#define CACHE_LINE_SIZE 42
#endif

#endif //  __CACHE_LINE_H__

