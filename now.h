#ifndef __CLOCK_H__
#define __CLOCK_H__

uint64_t inline  __attribute__((always_inline))  now() {
  uint32_t cycles_high1, cycles_low1;
  asm volatile("rdtscp;"
               "mov %%edx, %0;"
               "mov %%eax, %1;"
               "cpuid;"
	       : "=r"(cycles_high1), "=r"(cycles_low1)::"%rax", "%rbx",
		 "%rcx", "%rdx");
  return  (((uint64_t)cycles_high1 << 32 | cycles_low1));
}
#endif
