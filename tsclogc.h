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

#define TscLogEntrySize(numvals) ((sizeof(struct TscLogEntry) + (numvals * sizeof(TscLogValue_t))))

struct TscLog {
  union Header {
    char raw[CACHE_LINE_SIZE];
    struct Info{
      void    *cur;
      void    *end;
      uint32_t overflow;
      uint32_t migrations;
      uint32_t cpuid;
      uint32_t valperentry;
      uint32_t len;
      uint32_t tid;
    } info;
  } hdr;
  char name[CACHE_LINE_SIZE];
  uint8_t entries[];
} __attribute__ ((aligned (CACHE_LINE_SIZE)));;

// Ensure header info fits within the raw size set aside 
_Static_assert(sizeof(((struct TscLog *)0)->hdr.raw) >=
	       sizeof(((struct TscLog *)0)->hdr.info),
	       "TscLog Info exceeds a cacheline");

_Static_assert(sizeof(void *) <= sizeof(uint64_t), "TscLog: void * too big");

extern void tsclog_pinCpu(int cpu);   // -1 for current cpu
extern void * tsclog_newlog(char *name, uint32_t n, uint32_t values_per_entry,
			    int logonexit, FILE *stream, int binary,
			    char *valhdrs); // create new log
extern uint32_t tsclog_write(void *log, FILE *stream, int binary, char *valhdrs);
extern void tsclog_freelog(void *log);

static inline struct TscLogEntry *
tsclog_getentry(void *log, uint32_t numvals)
{
  struct TscLog *lptr = log;
  struct TscLogEntry *e;

 retry:
  e = buffer_reserve(TscLogEntrySize(numvals),
		     &(lptr->hdr.info.cur),
		     lptr->hdr.info.end);
  if (e == NULL) {
    // out of space
    lptr->hdr.info.overflow = 1;
#ifdef TSC_CIRCULAR_LOG
    // rotate back to the beginning
    buffer_try_set(&(lptr->hdr.info.cur),&(lptr->entries[0]));
    goto retry;
#endif
    // if we are not circular buffer then return NULL to indicate
    // we are out of space
  }
  return e;
}

static inline void
tsclog_hdr(struct TscLogEntry *e, uint64_t now, uint32_t cpuid)
{
  write_nti64(&(e->tsc), now);
#ifdef LOG_CPU
  write_nti32(&(e->cpu),cpuid);
#endif
#ifdef LOG_TID
  write_nti32(&(e->tid),gettid());
#endif
}

// FYI: migrations update is not thread safe so it should
//      not be treated as an exact count but rather indication
//      that at least one migration has been detected
#define TSCLOG_INFO(log, n, ...)			\
  struct TscLog *lptr = log;				\
  uint32_t cpuid;					\
  uint64_t now            = now_and_procid(&cpuid);	\
  uint32_t lcpuid         = lptr->hdr.info.cpuid;	\
  void *end = lptr->hdr.info.end;			\
  struct TscLogEntry *e;				\
  e = tsclog_getentry(log, n);				\
  if (e == NULL) return;				\
  tsclog_hdr(e, now, cpuid);				\
  if (lcpuid != cpuid) lptr->hdr.info.migrations++	

 
static inline void
tsclog_0(void *log)
{
  TSCLOG_INFO(log, 0);
}

static inline void
tsclog_writeval(struct TscLogEntry *e, int i, TscLogValue_t v)
{
    write_nti64(&(e->values[i]), v);
}

static inline void
tsclog_1(void *log, TscLogValue_t v0)
{
  TSCLOG_INFO(log,1);
  tsclog_writeval(e,0,v0);
}

static inline void
tsclog_2(void *log, TscLogValue_t v0, TscLogValue_t v1) {
  TSCLOG_INFO(log,2);
  tsclog_writeval(e,0,v0);
  tsclog_writeval(e,1,v1);
}

static inline void
tsclog_3(void *log, TscLogValue_t v0, TscLogValue_t v1,
	 TscLogValue_t v2) {
  TSCLOG_INFO(log,3);
  tsclog_writeval(e,0,v0);
  tsclog_writeval(e,1,v1);
  tsclog_writeval(e,2,v2);
}

static inline void
tsclog_4(void *log, TscLogValue_t v0, TscLogValue_t v1,
	 TscLogValue_t v2, TscLogValue_t v3) {
  TSCLOG_INFO(log,3);
  tsclog_writeval(e,0,v0);
  tsclog_writeval(e,1,v1);
  tsclog_writeval(e,2,v2);
  tsclog_writeval(e,3,v3);
}


#endif
