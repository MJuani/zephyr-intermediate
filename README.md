# Zephyr Training Environment

Welcome to the Zephyr RTOS training! This repository includes a ready-to-use
development environment based on Zephyr **4.4.0**, which you can set up in one of
three ways:

---

## Manual Zephyr Setup

Follow the following guide:
- [Getting Started Guide](https://docs.zephyrproject.org/4.4.0/develop/getting_started/index.html#).

Make sure to select appropriate OS and to perform all steps till
[Build the Blinky Sample](https://docs.zephyrproject.org/4.4.0/develop/getting_started/index.html#build-the-blinky-sample).

---

## L2 Task 1: Mutex Protection Results

Two threads at the same priority (`worker_a`, `worker_b`, priority 5) each
increment a shared `counter` 1,000,000 times. The expected final value is
2,000,000. Switch between the two versions with `USE_MUTEX` in
[app/src/main.c](app/src/main.c).

### Unprotected (`USE_MUTEX 0`)

```
*** Booting Zephyr OS build v4.4.0 ***
[00:00:00.396,301] <inf> demo: === L2 Task 1: Mutex Protection (Unprotected) ===
[00:00:00.396,331] <inf> demo: Expected final value: 2000000
[00:00:00.693,847] <inf> demo: [worker_a] finished
[00:00:00.710,754] <inf> demo: [worker_b] finished
[00:00:00.710,784] <inf> demo: Actual final value: 1489706
[00:00:00.710,815] <err> demo: Race condition confirmed: lost 510294 updates
[00:00:00.710,815] <inf> demo: Execution time: 314 ms
```

### Protected (`USE_MUTEX 1`)

```
*** Booting Zephyr OS build v4.4.0 ***
[00:00:00.344,787] <inf> demo: === L2 Task 1: Mutex Protection (Protected) ===
[00:00:00.344,787] <inf> demo: Expected final value: 2000000
[00:01:21.912,902] <inf> demo: [worker_a] finished
[00:01:21.932,647] <inf> demo: [worker_b] finished
[00:01:21.932,678] <inf> demo: Actual final value: 2000000
[00:01:21.932,678] <wrn> demo: No race this run
[00:01:21.932,708] <inf> demo: Execution time: 81588 ms
```

### Comparison

| | Unprotected | Protected |
|---|---|---|
| Final value | 1,489,706 | 2,000,000 |
| Lost updates | 510,294 (~25.5%) | 0 |
| Execution time | 314 ms | 81,588 ms (~260x slower) |
| Correct | No | Yes |

### Why the unprotected version has a race condition

`counter++` is not atomic. On the Cortex-M4 it compiles to three separate
instructions: a **load** (`LDR`) of `counter` into a register, an **add**, and
a **store** (`STR`) back to memory. `volatile` only forces each access to go
to memory. It does not make the three steps happen as one.

The two workers have the same priority and time slicing is enabled
(`CONFIG_TIMESLICING=y`, `CONFIG_TIMESLICE_SIZE=20`), so the scheduler
preempts one worker every 20 ms and runs the other. When that preemption lands
between the load and the store, the following happens:

1. `worker_a` loads `counter = N` into a register and is preempted.
2. `worker_b` runs for its time slice and stores many increments, for example
   up to `N + k`.
3. `worker_a` resumes, adds 1 to its stale `N`, and stores `N + 1`. This
   overwrites all `k` of `worker_b`'s updates.

Each overwrite like this loses updates, so the final value ends up below
2,000,000. The exact number lost changes from run to run because it depends on
where each preemption falls.

### Why the protected version is correct (and slower)

With `k_mutex_lock()` / `k_mutex_unlock()` around `counter++`, only one thread
can be inside the read-modify-write at a time. If a worker is preempted while
holding the mutex, the other worker blocks on `k_mutex_lock()` until the
mutex is released. It can never read a stale value, so no updates are lost and
the result is exactly 2,000,000.

The cost is 2,000,000 lock/unlock pairs, each a kernel call, which is about
40 µs per iteration instead of about 150 ns. Contention makes it worse. Once
one worker blocks on the mutex, each unlock hands the mutex to the waiting
thread. The releasing thread then blocks on its next `k_mutex_lock()`, so the
threads keep ping-ponging with a context switch per increment (a lock convoy).
Locking around a single increment is too fine-grained. For a plain counter,
`atomic_inc()` (from `<zephyr/sys/atomic.h>`) would be correct and much
cheaper.
