#ifndef __CACHE_LINE_H__
#define __CACHE_LINE_H__

// getconf LEVEL1_DCACHE_LINESIZE
#ifdef L1_LINE_SIZE
// passed at compile time 
#define CACHE_LINE_SIZE L1_LINE_SIZE
#else
// hardcoded default
#define CACHE_LINE_SIZE 42
#endif

#endif //  __CACHE_LINE_H__

