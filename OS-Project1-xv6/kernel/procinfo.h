#ifndef _PROCINFO_H_
#define _PROCINFO_H_

#include "types.h"

struct procinfo {
  int pid;        // Process ID
  int ppid;       // Parent process ID
  int state;      // Process state (see kernel/proc.h)
  uint64 sz;      // Size of process memory (bytes)
  char name[16];  // Process name
};

#endif
