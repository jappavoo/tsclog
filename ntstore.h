#ifndef __NT_STORE_H__
#define __NT_STORE_H__


/*********************************************************************************
* non-temporal store instruction to avoid cache polution when writing to memory
* that will not be used again (eg. logs/traces)
*********************************************************************************/
inline static void write_nti64(void *p, const uint64_t v)
{
asm volatile("movnti %0, (%1)\n\t"
:
: "r"(v), "r"(p)
: "memory");
}

inline static void write_nti32(void *p, const uint32_t v)
{
asm volatile("movnti %0, (%1)\n\t"
:
: "r"(v), "r"(p)
: "memory");
}

#endif
