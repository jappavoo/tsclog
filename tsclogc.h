#ifndef __TSC_CLOG_H__
#define __TSC_CLOG_H__

//#define LOG_CPU
//#define LOG_TID
typedef uint64_t TscLogValue_t;
struct TscLogEntry {
#ifdef LOG_CPU
  uint32_t cpu;
#endif
#ifdef LOG_TID
  uint32_t tid;
#endif
  uint64_t tsc;
  TscLogValue_t values[];
};

struct TscLog {
  union Header {
    char raw[CACHE_LINE_SIZE];
    struct Info{
      void    *cur;
      uint64_t n;
      void    *end;
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
	       "TscLog Info exceeds a cacheline");

_Static_assert(sizeof(void *) <= sizeof(uint64_t), "TscLog: void * too big");

extern void tsclog_pinCpu(int cpu);   // -1 for current cpu
extern void * tsclog_newlog(uint32_t n, uint32_t values_per_entry,
			    int logonexit, FILE *stream, int binary,
			    char *valhdrs); // create new log
extern uint32_t tsclog_write(void *log, FILE *stream, int binary, char *valhdrs);

static inline int
tsclog_0(void *log) {
  struct TscLog *lptr = log;
  uint32_t cpuid;
  uint64_t now            = now_and_procid(&cpuid);
  struct TscLogEntry *e   = lptr->hdr.info.cur;
  struct TscLogEntry *end = lptr->hdr.info.end; 
  uint32_t lcpuid         = lptr->hdr.info.cpuid;

  if (e>=end) {
#ifdef TSC_CIRCULAR_LOG
    // rotate back to the beginning
    lptr->hdr.info.cur = &(lptr->entries[0]);
#else
    return 0;  // stop logging
#endif
  }
  
  write_nti64(&(e->tsc), now);
#ifdef LOG_CPU
  write_nti32(&(e->cpu),cpuid);
#endif
#ifdef LOG_TID
  write_nti32(&(e->tid),gettid());
#endif
  lptr->hdr.info.cur = (void *)(e + 1);
  lptr->hdr.info.n++; 
  if (lcpuid != cpuid) lptr->hdr.info.migrations++;
  return 1;
}

static inline void
tsclog_appendval(void *log, TscLogValue_t v0) {
  struct TscLog *lptr = log;
  struct TscLogEntry *e   = lptr->hdr.info.cur;
  write_nti64(e, v0);
  e = (struct TscLogEntry *)((char *)e + sizeof(TscLogValue_t));  
}

static inline int
tsclog_1(void *log, TscLogValue_t v0) {
  if (!tsclog_0(log)) return 0;
  tsclog_appendval(log, v0);
  return 1;
}

static inline int
tsclog_2(void *log, TscLogValue_t v0, TscLogValue_t v1) {
  if (!tsclog_0(log)) return 0;
  tsclog_appendval(log, v0);
  tsclog_appendval(log, v1);
  return 1;
}

static inline int
tsclog_3(void *log, TscLogValue_t v0, TscLogValue_t v1,
	 TscLogValue_t v2) {
  if (!tsclog_0(log)) return 0;
  tsclog_appendval(log, v0);
  tsclog_appendval(log, v1);
  tsclog_appendval(log, v2);
  return 1;
}

static inline int
tsclog_4(void *log, TscLogValue_t v0, TscLogValue_t v1,
	 TscLogValue_t v2, TscLogValue_t v3) {
  if (!tsclog_0(log)) return 0;
  tsclog_appendval(log, v0);
  tsclog_appendval(log, v1);
  tsclog_appendval(log, v2);
  tsclog_appendval(log, v3);
  return 1;
}


#endif
