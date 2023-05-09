/******************************************************************************
* Copyright (C) 2023 by Jonathan Appavoo, Boston University
*****************************************************************************/


#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <inttypes.h>
#include <assert.h>
#include "now.h"
#include "cacheline.h"
#include "ntstore.h"

//#define LOG_CPU
//#define LOG_TID

struct TscLogEntry {
#ifdef LOG_CPU
  uint32_t cpu;
#endif
#ifdef LOG_TID
  uint32_t tid;
#endif
  uint64_t tsc;
  uint64_t values[];
};

struct TscLog {
  union Header {
    char raw[CACHE_LINE_SIZE];
    struct Info{
      void    *cur;
      uint64_t bytes;
      uint32_t migrations;
      uint32_t cpuid;
      uint32_t valperentry;
      uint32_t len;
      uint32_t tid;
    } info;
  } hdr;
  uint8_t entries[];
} __attribute__ ((aligned (CACHE_LINE_SIZE)));;

// Ensure header info fits within the raw size set aside 
_Static_assert(sizeof(((struct TscLog *)0)->hdr.raw) >=
	       sizeof(((struct TscLog *)0)->hdr.info),
	       "TsLog Info exceeds a cacheline");

void tsclog_pinCpu(int cpu)
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

uint64_t
tsclog_newlog(uint32_t n, uint32_t values_per_entry) {
  struct TscLog *log;
  uint64_t now;
  uint64_t mem = 0;
  uint64_t bytes = sizeof(struct TscLog) +
    n * (sizeof(struct TscLogEntry) + (values_per_entry * sizeof(uint64_t)));
  if (n) {
    mem = (uint64_t)aligned_alloc(CACHE_LINE_SIZE, bytes);
  }
  log = (void *)mem;
  assert(log);
  log->hdr.info.cur = &(log->entries[0]);
  log->hdr.info.valperentry = values_per_entry;
  log->hdr.info.len = n;
  log->hdr.info.bytes = bytes;
  
  // initialize dynamic facts
  now = now_and_procid(&(log->hdr.info.cpuid));
  log->hdr.info.tid = gettid();
  log->hdr.info.migrations = 0;
  
#ifdef VERBOSE
  fprintf(stderr,
	  "tsclog: now:%lu: new(n=%u, values_per_entry=%u):\n   bytes=%"
	  PRIu64  "\n   mem=%" PRIx64 " cur=%p\n   tid=%u\n   "
	  "cpuid=%d\n   migrations=%u\n"
	  "   sizeof(struct LogEntry)=%lu "
	  "sizeof(struct Log)=%lu\n",  now,
	  log->hdr.info.len, log->hdr.info.valperentry,
	  log->hdr.info.bytes, mem, log->hdr.info.cur,
	  log->hdr.info.tid, log->hdr.info.cpuid,
	  log->hdr.info.migrations,
	  sizeof(struct TscLogEntry), sizeof(struct TscLog));
#endif
  return mem;
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
  tsclog_pinCpu(cpu);
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
  return now();
}

JNIEXPORT jlong JNICALL
Java_tsclog_stdout_1now(JNIEnv *env, jclass jcl)
{
  uint32_t cpuid;
  uint64_t now = now_and_procid(&cpuid);
  int tid = gettid();

  printf("tsclog: %d %d %lu\n", tid, cpuid, now);
  return now;
}

JNIEXPORT jlong JNICALL
Java_tsclog_stderr_1now(JNIEnv *env, jclass jcl)
{
  uint32_t cpuid;
  uint64_t now = now_and_procid(&cpuid);
  int tid = gettid();

  fprintf(stderr, "tsclog: %d %d %lu\n", tid, cpuid, now);
  return now;
}

JNIEXPORT jlong JNICALL
Java_tsclog_stdout_1label_1now(JNIEnv *env, jclass jcl,
					     jstring label)
{
  const char * cp = (*env)->GetStringUTFChars(env, label, NULL);
  uint32_t cpuid;
  uint64_t now = now_and_procid(&cpuid);
  int tid = gettid();

  printf("tsclog: %s %d %d %lu\n", cp, tid, cpuid, now);
  (*env)->ReleaseStringUTFChars(env, label, cp);
  return now;
  
}

JNIEXPORT jlong JNICALL
Java_tsclog_stderr_1label_1now(JNIEnv *env,
			       jclass jcl,
			       jstring label)
{
  const char * cp = (*env)->GetStringUTFChars(env, label, NULL);
  uint32_t cpuid;
  uint64_t now = now_and_procid(&cpuid);
  int tid = gettid();

  fprintf(stderr, "tsclog: %s %d %d %lu\n", cp, tid, cpuid, now);
  (*env)->ReleaseStringUTFChars(env, label, cp);
  return now;
}

JNIEXPORT jlong JNICALL
Java_tsclog_mklog(JNIEnv *env, jclass jcl, jlong n)
{
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

  struct TscLog *log = (void *)tsclog_newlog(1024, 0); 
  return i;
}
#endif
	 
    
