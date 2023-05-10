#ifndef __BUFFER_H__
#define __BUFFER_H__

// Atomically reserve space in a buffer
// we allow for negative reservation to allow us to
// use the same fuction to reset a trace
static inline void *
buffer_reserve(uint32_t bytes, void * volatile *cur, void *end)
{
  uint8_t *old, *new;
 retry:
  old  = *cur;
  new = old + bytes;
  if (new > (uint8_t *)end) return NULL;
  if (!__sync_bool_compare_and_swap(cur, (void *)old, (void *)new))
    goto retry;
  return (void *)old;
}

// Atomically reserve space in a buffer
// we allow for negative reservation to allow us to
// use the same fuction to reset a trace
static inline int 
buffer_try_set(void * volatile *cur, void *new)
{
  void *old = *cur;
  return  __sync_bool_compare_and_swap(cur, old, new);
}

#endif
