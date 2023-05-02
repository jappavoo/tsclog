#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <inttypes.h>
#include "now.h"
#include "cacheline.h"


void pinCpu(int cpu)
{
  cpu_set_t  mask;
  CPU_ZERO(&mask);

  if (cpu == -1 ) {
    cpu = sched_getcpu();
    if (cpu == -1) {
      err(1, "could not determine current cpu");
    }
  }

  CPU_SET(cpu, &mask);
  if (sched_setaffinity(0, sizeof(mask), &mask) != 0) {
    err(1, "could not pin to cpu=%d",cpu);
  }
  
#ifdef VERBOSE
    fprintf(stderr, "PINNED TO CPU: %d\n", cpu);
#endif
    
}

#ifdef __TSCLOG_LIB__
#include "tsclog.h"

JNIEXPORT jint JNICALL
Java_tsclog_availcpus(JNIEnv *, jclass)
{
  return get_nprocs();
}

JNIEXPORT void JNICALL
Java_tsclog_pin(JNIEnv *env, jclass jcl, jint cpu)
{
  pinCpu(cpu);
}

JNIEXPORT jint JNICALL
Java_tsclog_cpu(JNIEnv *, jclass)
{
  unsigned int cpu, node;
  getcpu(&cpu, &node);
  return cpu;
}


JNIEXPORT jint JNICALL
Java_tsclog_tid(JNIEnv *, jclass)
{
  int tid;
  tid = gettid();
  return tid;
}

JNIEXPORT jlong JNICALL
Java_tsclog_now(JNIEnv *env, jclass jcl)
{
  return  now();
}

JNIEXPORT jlong JNICALL
Java_tsclog_stdout_1now(JNIEnv *env, jclass jcl)
{
  uint64_t t=now();
  unsigned int cpu, node;
  int tid = gettid();
  getcpu(&cpu, &node);
  printf("tsclog: %d %d %lu\n", cpu, tid, t);
  return t;
}

JNIEXPORT jlong JNICALL
Java_tsclog_stderr_1now(JNIEnv *env, jclass jcl)
{
  uint64_t t=now();
  unsigned int cpu, node;
  int tid = gettid();
  getcpu(&cpu, &node);
  fprintf(stderr, "tsclog: %d %d %lu\n", cpu, tid, t);
  return t;
}

JNIEXPORT jlong JNICALL
Java_tsclog_stdout_1label_1now(JNIEnv *env, jclass jcl,
					     jstring label)
{
  const char * cp = (*env)->GetStringUTFChars(env, label, NULL);
  uint64_t t=now();
  unsigned int cpu, node;
  int tid = gettid();
  getcpu(&cpu, &node);
  printf("tsclog: %s %d %d %lu\n", cp, cpu, tid, t);
  (*env)->ReleaseStringUTFChars(env, label, cp);
  return t;
  
}

JNIEXPORT jlong JNICALL
Java_tsclog_stderr_1label_1now(JNIEnv *env,
			       jclass jcl,
			       jstring label)
{
  const char * cp = (*env)->GetStringUTFChars(env, label, NULL);
  uint64_t t=now();
  unsigned int cpu, node;
  int tid = gettid();
  getcpu(&cpu, &node);
  fprintf(stderr, "tsclog: %s %d %d %lu\n", cp, cpu, tid, t);
  (*env)->ReleaseStringUTFChars(env, label, cp);
  return t;
}

#else


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
	 
    
