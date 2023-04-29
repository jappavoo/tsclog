#define _GNU_SOURCE
#include <sched.h>
#include <sys/sysinfo.h>
#include <inttypes.h>
#include "now.h"
#include "cacheline.h"


#ifdef __TSCLOG_LIB__
#include "tsclog.h"

JNIEXPORT jint JNICALL Java_tsclog_init(JNIEnv *env, jclass jcl)
{
  return 1;
}

JNIEXPORT jint JNICALL Java_tsclog_log(JNIEnv *env, jclass jcl)
{
  return 2;
}

JNIEXPORT jint JNICALL Java_tsclog_done(JNIEnv *env, jclass jcl)
{
  return 3;
}

#else
#include <stdio.h>

unsigned long long sum = 0;
int
main(int argc, char **argv)
{
  unsigned long long start, end;
  int i;
  unsigned int totalcpus, availcpus, cpu, node;

  totalcpus = get_nprocs_conf();
  availcpus = get_nprocs();
  getcpu(&cpu, &node);
  
  start = now();
  for (i=0;i<1000000;i++) {
    sum += now();
  }
  end = now();

  printf("totalcpu:%u availcpus:%u cpu:%u,node:%u: %u start:%llu end:%llu diff:%llu sum:%llu\n",
	 totalcpus, availcpus, cpu, node,
	 CACHE_LINE_SIZE, start, end, end - start, sum);
  return i;
}
#endif
	 
    
