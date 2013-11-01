#ifndef THREADS_TEST_C
#define THREADS_TEST_C

#include <stdint.h>

__inline__ uint64_t rdtsc (void)
{
  uint32_t lo, hi;
  __asm__ __volatile__ (
      " xorl %%eax,%%eax \n"
      " cpuid" // serialize
      ::: "%rax", "%rbx", "%rcx", "%rdx");
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return (uint64_t)hi << 32 | lo;
}

#endif
