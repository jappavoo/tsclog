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
#include <sys/syscall.h>
#include <assert.h>
#include "now.h"
#include "cacheline.h"
#include "ntstore.h"
#include "buffer.h"
#include "tsclogc.h"

// if gettid optimization avaiable use it
#if __GLIBC_PREREQ(2,30)
#define _GNU_SOURCE
#include <unistd.h>
#else
#include <sys/syscall.h>
pid_t
gettid(void)
{
    return syscall(SYS_gettid);
}
#endif

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

struct tsclog_exitargs {
  void *log;
  FILE *stream;
  char *valhdrs;
  int  binary;
};

static void tsclog_exit(int status, void *args)
{
  struct tsclog_exitargs *a = args;
  tsclog_write(a->log, a->stream, a->binary, a->valhdrs);
}

void *
tsclog_newlog(uint32_t n, uint32_t values_per_entry, int logonexit,
	      FILE *stream, int binary, char *valhdrs) {
  struct TscLog *log;
  uint64_t now;
  uint64_t entrybytes = n * TscLogEntrySize(values_per_entry);
  uint64_t bytes = sizeof(struct TscLog) + entrybytes;
  
  if (!n)  return NULL;
  
  log = aligned_alloc(CACHE_LINE_SIZE, bytes);
  assert(log);
  log->hdr.info.cur = &(log->entries[0]);
  log->hdr.info.overflow = 0;
  log->hdr.info.end = &(log->entries[entrybytes]);
  log->hdr.info.valperentry = values_per_entry;
  
  // initialize dynamic facts
  now = now_and_procid(&(log->hdr.info.cpuid));
  log->hdr.info.tid = gettid();
  log->hdr.info.migrations = 0;

  if (logonexit) {
    struct tsclog_exitargs *args = malloc(sizeof(struct tsclog_exitargs));
    args->log = log;
    args->binary = binary;
    args->stream = stream;
    args->valhdrs = valhdrs; 
    on_exit(tsclog_exit, (void *)args);
  }
#ifdef VERBOSE
  tsclog_write(log, stderr, 0, NULL);
  /*
  fprintf(stderr,
	  "tsclog: now:%lu: new(n=%u, values_per_entry=%u):\n   bytes=%"
	  PRIu64  "\n   log=%p cur=%p\n   tid=%u\n   "
	  "cpuid=%d\n   migrations=%u\n"
	  "   sizeof(struct LogEntry)=%lu "
	  "sizeof(struct Log)=%lu\n",  now,
	  log->hdr.info.len, log->hdr.info.valperentry,
	  log->hdr.info.bytes, log, log->hdr.info.cur,
	  log->hdr.info.tid, log->hdr.info.cpuid,
	  log->hdr.info.migrations,
	  sizeof(struct TscLogEntry), sizeof(struct TscLog));
  */
#endif
  return log;
}

uint32_t
tsclog_write(void *log, FILE *stream, int binary, char *valhdrs)
{
  struct TscLog *l = log;
  int numvals = l->hdr.info.valperentry;
  struct TscLogEntry *e = (struct TscLogEntry *)&(l->entries[0]);
  struct TscLogEntry *end = (struct TscLogEntry *)l->hdr.info.end;
  struct TscLogEntry *cur = (struct TscLogEntry *)l->hdr.info.cur;
  fprintf(stream, "TSC LOG: tid:%u cpuid:%u"
	  " valsperentry:%u migrations:%u entries:%p end:%p cur:%p\n",
	  l->hdr.info.tid, l->hdr.info.cpuid,
	  l->hdr.info.valperentry, l->hdr.info.migrations,
	  &(l->entries[0]), l->hdr.info.cur, l->hdr.info.end);
  
  if (l->hdr.info.overflow>0) {
    fprintf(stream, "WARNING: overflow dumping all entries\n");
    cur = end;
  }

  fprintf(stream, "tsc");
#ifdef LOG_CPU
  fprintf(stream, ",cpu");
#endif    
#ifdef LOG_TID
  fprintf(stream, ",tid");
#endif
  if (valhdrs) fprintf(stream, "%s", valhdrs);
  else {
    for (int i=0; i<numvals; i++) {
      fprintf(stream, ",val%d", i);
    }
  }
  fprintf(stream, "\n");

  while (e!=cur) {
    fprintf(stream, "%"PRIu64, e->tsc);
#ifdef LOG_CPU
    fprintf(stream, ",%u", e->cpu);
#endif    
#ifdef LOG_TID
    fprintf(stream, ",%u", e->tid);
#endif
    for (int i=0; i<numvals; i++) {
      fprintf(stream, ",%"PRId64, (int64_t)e->values[i]);
    }
    e = (struct TscLogEntry *)((uint8_t *)e +
			       TscLogEntrySize(numvals));
    fprintf(stream, "\n");
  }
}

#ifdef __TSCLOG_LIB__
#include "tsclog.h"

JNIEXPORT jint JNICALL
Java_tsclog_availcpus(JNIEnv *env, jclass jcl)
{
  return get_nprocs();
}

JNIEXPORT void JNICALL
Java_tsclog_pin(JNIEnv *env, jclass jcl, jint cpu)
{
  tsclog_pinCpu(cpu);
}

JNIEXPORT jint JNICALL
Java_tsclog_cpu(JNIEnv *env, jclass jcl)
{
  unsigned int cpu, node;
  getcpu(&cpu, &node);
  return cpu;
}


JNIEXPORT jint JNICALL
Java_tsclog_tid(JNIEnv *env, jclass jcl)
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
Java_tsclog_mklog(JNIEnv *env, jclass jcl, jlong n, jint valsperentry)
{
  void * log = (void *)tsclog_newlog(n, // num entries
				     valsperentry,  // num vals per entry
				     1,  // log on exit
				     stderr, // stream to write log
				     0,      // log as binary
				     NULL    // val headers
				     );
  return (jlong)log;
}

JNIEXPORT void JNICALL
Java_tsclog_log(JNIEnv *env, jclass jcl, jlong lptr)
{
  void *log = (void *)lptr;
  tsclog_0(log);
}


JNIEXPORT void JNICALL
Java_tsclog_log1(JNIEnv *env, jclass jcl, jlong lptr, jlong v1)
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

  void * log = (void *)tsclog_newlog(10, // num entries
				     0,  // num vals per entry
				     1,  // log on exit
				     stderr, // stream to write log
				     0,      // log as binary
				     NULL    // val headers
				     );

  for (int j=0; j<10; j++) {
    tsclog_0(log);
  }
  //tsclog_write(log, stderr, 0, NULL);
  
  return i;
}
#endif
	 
    
