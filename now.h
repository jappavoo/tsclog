#ifndef __CLOCK_H__
#define __CLOCK_H__
/******************************************************************************
* Copyright (C) 2022 by Jonathan Appavoo, Boston University
*****************************************************************************/

// use cat /proc/cpuinfo | grep flags | head -1 | sed 's/ /\n/g' | grep tsc
// to determine the behaviour of your tsc clock
// Eg.
// $ cat /proc/cpuinfo | grep flags | head -1 | sed 's/ /\n/g' | grep tsc
// tsc
// rdtscp
// constant_tsc
// nonstop_tsc
// tsc_deadline_timer
// tsc_adjust



// now using rdtsc: Read Time-Stamp Counter
//
// NOTE WE USE cpuid instruction to force serialization (worth
// exploring using fences instead)
//
// Documentation from INTEL SDM volume 2B
//
// Reads the current value of the processor’s time-stamp counter (a
// 64-bit MSR) into the EDX:EAX registers. The EDX register is loaded
// with the high-order 32 bits of the MSR and the EAX register is
// loaded with the low-order 32 bits. (On processors that support the
// Intel 64 architecture, the high-order 32 bits of each of RAX and
// RDX are cleared.)
//
// The processor monotonically increments the
// time-stamp counter MSR every clock cycle and resets it to 0
// whenever the processor is reset. See “Time Stamp Counter” in
// Chapter 17 of the Intel® 64 and IA-32 Architectures Software
// Developer’s Manual, Volume 3B, for specific details of the time
// stamp counter behavior.
//
// The time stamp disable (TSD) flag in
// register CR4 restricts the use of the RDTSC instruction as
// follows. When the flag is clear, the RDTSC instruction can be
// executed at any privilege level; when the flag is set, the
// instruction can only be executed at privilege level 0.
//
// The time-stamp counter can also be read with the RDMSR instruction,
// when executing at privilege level 0.
//
// The RDTSC instruction is not a serializing instruction. It does not
// necessarily wait until all previous instructions have been executed
// before reading the counter. Similarly, subsequent instructions may
// begin execution before the read operation is performed. The
// following items may guide software seeking to order executions of
// RDTSC:
// - If software requires RDTSC to be executed only after all previous
//   instructions have executed and all previous loads are globally
//   visible,1 it can execute LFENCE immediately before RDTSC.
// - If software requires RDTSC to be executed only after all previous
//   instructions have executed and all previous loads and stores are
//   globally visible, it can execute the sequence MFENCE;LFENCE
//   immediately before RDTSC.
// - If software requires RDTSC to be executed prior to execution of
//   any subsequent instruction (including any memory accesses), it
//   can execute the sequence LFENCE immediately after RDTSC.
uint64_t inline  __attribute__((always_inline))  now() {
  uint32_t cycles_high1, cycles_low1;
  asm volatile("rdtsc;"
               "mov %%edx, %0;"
               "mov %%eax, %1;"
               "cpuid;"
	       : "=r"(cycles_high1), "=r"(cycles_low1):
	       : "%rax", "%rbx", "%rcx", "%rdx");  // cpuid clobbers more registers than rdtsc		 
  return  (((uint64_t)cycles_high1 << 32 | cycles_low1));
}



// a version of now that uses rdtscp Read Time-Stamp and Processor ID
// rdtsp documentation from INTEL SDM Vol 2B
//
// NOTE WE USE cpuid instruction to force serialization (worth
// exploring using fences instead)
//
//
// Reads the current value of the processor’s time-stamp counter (a
// 64-bit MSR) into the EDX:EAX registers and also reads the value of
// the IA32_TSC_AUX MSR (address C0000103H) into the ECX register. The
// EDX register is loaded with the high-order 32 bits of the IA32_TSC
// MSR; the EAX register is loaded with the low-order 32 bits of the
// IA32_TSC MSR; and the ECX register is loaded with the low-order
// 32-bits of IA32_TSC_AUX MSR. On processors that support the Intel
// 64 architecture, the high-order 32 bits of each of RAX, RDX, and
// RCX are cleared.
//
// The processor monotonically increments the time-stamp counter MSR
// every clock cycle and resets it to 0 whenever the processor is
// reset. See “Time Stamp Counter” in Chapter 17 of the Intel® 64 and
// IA-32 Architectures Software Developer’s Manual, Volume 3B, for
// specific details of the time stamp counter behavior.
//
// The time stamp disable (TSD) flag in register CR4 restricts the use
// of the RDTSCP instruction as follows. When the flag is clear, the
// RDTSCP instruction can be executed at any privilege level; when the
// flag is set, the instruction can only be executed at privilege
// level 0.
//																								   
// The RDTSCP instruction is not a serializing instruction, but it
// does wait until all previous instructions have executed and all
// previous loads are globally visible.1 But it does not wait for
// previous stores to be globally visible, and subse- quent
// instructions may begin execution before the read operation is
// performed. The following items may guide software seeking to order
// executions of RDTSCP:
//
// - If software requires RDTSCP to be executed only after all previous
//   stores are globally visible, it can execute MFENCE immediately
//   before RDTSCP.
// - If software requires RDTSCP to be executed prior to execution of
//   any subsequent instruction (including any memory accesses), it
//   can execute LFENCE immediately after RDTSCP.
uint64_t inline  __attribute__((always_inline))  now_and_procid(uint32_t *cpuid) {
  uint32_t cycles_high1, cycles_low1, tsc_sig;
  asm volatile("rdtscp;"
               "mov %%edx, %0;"
               "mov %%eax, %1;"
	       "mov %%ecx, %2;"
               "cpuid;"
	       : "=r"(cycles_high1), "=r"(cycles_low1), "=r"(tsc_sig):
	       : "%rax", "%rbx", "%rcx", "%rdx");  // cpuid clobbers more registers than rdtscp
  *cpuid=tsc_sig;
  return  (((uint64_t)cycles_high1 << 32 | cycles_low1));
}


#endif
