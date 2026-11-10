#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"

static char *states[] = {
  [0] = "UNUSED",
  [1] = "USED",
  [2] = "SLEEPING",
  [3] = "RUNNABLE",
  [4] = "RUNNING",
  [5] = "ZOMBIE"
};

static char*
state_str(int s)
{
  if(s >= 0 && s < sizeof(states)/sizeof(states[0]) && states[s])
    return states[s];
  return "UNKNOWN";
}

void
test_basic(void)
{
  struct procinfo info;
  int mypid = getpid();

  printf("=== PI-01: Usage Example (Self PID: %d) ===\n", mypid);
  if(procinfo(mypid, &info) < 0){
    printf("FAIL: procinfo(getpid()) failed\n");
    exit(1);
  }

  printf("Process: %s\n", info.name);
  printf("PID: %d, PPID: %d\n", info.pid, info.ppid);
  printf("State: %d (%s)\n", info.state, state_str(info.state));
  printf("Memory: %d bytes\n", (int)info.sz);

  if(info.pid != mypid){
    printf("FAIL: info.pid (%d) != mypid (%d)\n", info.pid, mypid);
    exit(1);
  }
  if(strcmp(info.name, "procinfotest") != 0){
    printf("FAIL: expected name procinfotest, got %s\n", info.name);
    exit(1);
  }
  if(info.state != 4){ // RUNNING
    printf("FAIL: expected state 4 (RUNNING), got %d\n", info.state);
    exit(1);
  }
  uint64 expected_sz = (uint64)sbrk(0);
  if(info.sz != expected_sz){
    printf("FAIL: info.sz (%d) != sbrk(0) (%d)\n", (int)info.sz, (int)expected_sz);
    exit(1);
  }
  printf("PI-01 PASSED\n\n");
}

void
test_parent(void)
{
  struct procinfo self, parent;
  if(procinfo(getpid(), &self) < 0){
    printf("FAIL: procinfo failed\n");
    exit(1);
  }

  printf("=== PI-02: Query Parent PID (%d) ===\n", self.ppid);
  if(procinfo(self.ppid, &parent) < 0){
    printf("FAIL: procinfo(parent_pid) failed\n");
    exit(1);
  }

  printf("Parent Process: %s\n", parent.name);
  printf("Parent PID: %d, PPID: %d\n", parent.pid, parent.ppid);
  printf("Parent State: %d (%s)\n", parent.state, state_str(parent.state));
  printf("Parent Memory: %d bytes\n", (int)parent.sz);

  if(parent.pid != self.ppid){
    printf("FAIL: parent.pid != self.ppid\n");
    exit(1);
  }
  printf("PI-02 PASSED\n\n");
}

void
test_init(void)
{
  struct procinfo info;
  printf("=== PI-03: Query init (PID 1) ===\n");
  if(procinfo(1, &info) < 0){
    printf("FAIL: procinfo(1) failed\n");
    exit(1);
  }

  printf("Process: %s, PID: %d, PPID: %d, State: %d (%s)\n",
         info.name, info.pid, info.ppid, info.state, state_str(info.state));

  if(strcmp(info.name, "init") != 0){
    printf("FAIL: expected name init, got %s\n", info.name);
    exit(1);
  }
  if(info.ppid != 0){
    printf("FAIL: init should have ppid 0, got %d\n", info.ppid);
    exit(1);
  }
  printf("PI-03 PASSED\n\n");
}

void
test_invalid_pid(void)
{
  struct procinfo info;
  memset(&info, 0xAA, sizeof(info));

  printf("=== PI-04: Invalid PIDs (9999, 0, -1) ===\n");
  if(procinfo(9999, &info) != -1){
    printf("FAIL: procinfo(9999) should return -1\n");
    exit(1);
  }
  if(procinfo(0, &info) != -1){
    printf("FAIL: procinfo(0) should return -1\n");
    exit(1);
  }
  if(procinfo(-1, &info) != -1){
    printf("FAIL: procinfo(-1) should return -1\n");
    exit(1);
  }
  printf("PI-04 PASSED\n\n");
}

void
test_invalid_pointer(void)
{
  printf("=== PI-05: Invalid Pointers (NULL, >= MAXVA) ===\n");
  if(procinfo(getpid(), (struct procinfo*)0) != -1){
    printf("FAIL: procinfo with null pointer should return -1\n");
    exit(1);
  }
  if(procinfo(getpid(), (struct procinfo*)0xffffffffffffffff) != -1){
    printf("FAIL: procinfo with bad pointer should return -1\n");
    exit(1);
  }
  printf("PI-05 PASSED\n\n");
}

void
test_zombie(void)
{
  printf("=== PI-06: Zombie Process ===\n");
  int pid = fork();
  if(pid < 0){
    printf("FAIL: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    // Child process exits immediately
    exit(0);
  }

  // Parent sleeps briefly to allow child to exit and become ZOMBIE
  sleep(5);

  struct procinfo zinfo;
  if(procinfo(pid, &zinfo) < 0){
    printf("FAIL: procinfo(zombie_pid) failed\n");
    wait(0);
    exit(1);
  }

  printf("Zombie Process: %s, PID: %d, PPID: %d, State: %d (%s)\n",
         zinfo.name, zinfo.pid, zinfo.ppid, zinfo.state, state_str(zinfo.state));

  if(zinfo.state != 5){ // ZOMBIE
    printf("FAIL: expected state 5 (ZOMBIE), got %d\n", zinfo.state);
    wait(0);
    exit(1);
  }
  if(zinfo.ppid != getpid()){
    printf("FAIL: zombie ppid (%d) != parent getpid (%d)\n", zinfo.ppid, getpid());
    wait(0);
    exit(1);
  }

  // Clean up child
  wait(0);
  printf("PI-06 PASSED\n\n");
}

int
main(int argc, char *argv[])
{
  printf("Starting procinfotest...\n\n");
  test_basic();
  test_parent();
  test_init();
  test_invalid_pid();
  test_invalid_pointer();
  test_zombie();
  printf("ALL PROCINFO TESTS PASSED!\n");
  exit(0);
}
