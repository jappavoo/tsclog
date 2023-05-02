#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <inttypes.h>
#include "now.h"
#include "cacheline.h"

#define LOG_CPU
#define LOG_TID

struct logentry {
#ifdef LOG_CPU
  uint32_t cpu;
#endif
#ifdef LOG_TID
  uint32_t tid;
#endif
  uint64_t tsc;
  uint64_t data[];
};

struct Log {
  struct Info {
    uint64_t len;
    uint64_t *cur;
  } info;
  struct logentry entries[];
};

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

JNIEXPORT jlong JNICALL
Java_tsclog_mklog(JNIEnv *env, jclass jcl, jlong n)
{
  uint64_t mem = 0;
  if (n) {
    mem = (uint64_t)malloc(n*sizeof(uint64_t));
  }
  return mem;
}

JNIEXPORT void JNICALL
Java_tsclog_log(JNIEnv *env, jclass jcl, jlong lptr)
{
}

JNIEXPORT void JNICALL
Java_tsclog_log1(JNIEnv *, jclass, jlong lptr, jlong v1)
{
}


#if 0
// Get a reference to this object's class
   jclass thisClass = (*env)->GetObjectClass(env, thisObj);
 
   // int
   // Get the Field ID of the instance variables "number"
   jfieldID fidNumber = (*env)->GetFieldID(env, thisClass, "number", "I");
   if (NULL == fidNumber) return;
 
   // Get the int given the Field ID
   jint number = (*env)->GetIntField(env, thisObj, fidNumber);
   printf("In C, the int is %d\n", number);
 
   // Change the variable
   number = 99;
   (*env)->SetIntField(env, thisObj, fidNumber, number);
#endif

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
	 
    
