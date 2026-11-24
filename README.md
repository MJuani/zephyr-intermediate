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

## L6 Task 1: Latency Investigation and Tracing

### Guarantee

**Control work must start within `DEADLINE_MS` = 10 ms of the timer expiry
that created the event.** This is the 10 ms value the starter specifies, and
both the failing and the passing run below use it.

### Workload and detector (identical in both runs)

| Element | Value |
|---|---|
| Timer | `event_timer`, first expiry at 500 ms, then every `EVENT_PERIOD_MS` = 250 ms |
| Timer ISR | stamps `ready_ms`, puts the event in `control_queue`, gives `maintenance_start`, and emits trace event `event_triggered(seq, ready_ms)` |
| `control` | waits on `control_queue`, measures latency, counts misses, and emits trace event `event_processed(seq, latency)` |
| `maintenance` | waits on `maintenance_start`, then runs `k_busy_wait(MAINTENANCE_LOAD_US)` = 45 ms |
| Detector | `latency_ms > DEADLINE_MS` counts as a miss. Every event logs `seq`, `latency`, `deadline` and the running `misses` count, plus a rate-limited `deadline_miss` warning. |
| Platform | `native_sim`, 100 ticks per second (10 ms resolution), CTF tracing via the POSIX backend |

The **only** difference between the two runs is the thread priorities:

| Run | `CONTROL_PRIORITY` | `MAINTENANCE_PRIORITY` | Result |
|---|---|---|---|
| Original (failing) | 7 | 4 | 20/20 misses, latency 40 ms |
| Corrected (passing) | 4 | 7 | 0/20 misses, latency 0 ms |

In Zephyr, a **lower number means a higher priority**.

### What the measured latency covers

`ready_ms` is stamped in the timer ISR (`k_uptime_get_32()`). `control` takes
a second timestamp **immediately after `k_msgq_get()` returns, before it
logs anything**. The latency therefore covers:

- the time from the ISR to the moment `control` is scheduled, and
- any time `control` spent ready but waiting for higher-priority threads.

That is exactly the "start within 10 ms" guarantee. It does not include
`control`'s own processing or logging time.

`k_uptime_get_32()` has one-tick resolution (10 ms on `native_sim`), so the
reported value is rounded down to a multiple of 10 ms. The failing run reports
**40 ms**, while the trace (with finer timestamps) shows the real delay is
**45 ms**, which is exactly `MAINTENANCE_LOAD_US`. Either way it is far above
the 10 ms deadline.

### Latency output: original priorities (failing)

```
*** Booting Zephyr OS build v4.4.0 ***
[00:00:00.000,000] <inf> homework: === L6 Homework: Runtime Investigation ===
[00:00:00.000,000] <inf> homework: Control work must start within 10 ms
[00:00:00.000,000] <inf> homework: Inspect, measure, trace, explain, and correct the delay
[00:00:00.545,000] <inf> homework: [CONTROL] seq=0 latency=40ms deadline=10ms misses=1
[00:00:00.795,000] <inf> homework: [CONTROL] seq=1 latency=40ms deadline=10ms misses=2
[00:00:01.045,000] <inf> homework: [CONTROL] seq=2 latency=40ms deadline=10ms misses=3
[00:00:01.045,000] <wrn> homework: Skipped 2 messages
[00:00:01.045,000] <wrn> homework: [CONTROL] deadline_miss count=3 seq=2 latency=40ms
[00:00:01.295,000] <inf> homework: [CONTROL] seq=3 latency=40ms deadline=10ms misses=4
[00:00:01.545,000] <inf> homework: [CONTROL] seq=4 latency=40ms deadline=10ms misses=5
[00:00:01.795,000] <inf> homework: [CONTROL] seq=5 latency=40ms deadline=10ms misses=6
[00:00:02.045,000] <inf> homework: [CONTROL] seq=6 latency=40ms deadline=10ms misses=7
[00:00:02.045,000] <wrn> homework: Skipped 3 messages
[00:00:02.045,000] <wrn> homework: [CONTROL] deadline_miss count=7 seq=6 latency=40ms
[00:00:02.295,000] <inf> homework: [CONTROL] seq=7 latency=40ms deadline=10ms misses=8
[00:00:02.545,000] <inf> homework: [CONTROL] seq=8 latency=40ms deadline=10ms misses=9
[00:00:02.795,000] <inf> homework: [CONTROL] seq=9 latency=40ms deadline=10ms misses=10
[00:00:03.045,000] <inf> homework: [CONTROL] seq=10 latency=40ms deadline=10ms misses=11
[00:00:03.045,000] <wrn> homework: Skipped 3 messages
[00:00:03.045,000] <wrn> homework: [CONTROL] deadline_miss count=11 seq=10 latency=40ms
[00:00:03.295,000] <inf> homework: [CONTROL] seq=11 latency=40ms deadline=10ms misses=12
[00:00:03.545,000] <inf> homework: [CONTROL] seq=12 latency=40ms deadline=10ms misses=13
[00:00:03.795,000] <inf> homework: [CONTROL] seq=13 latency=40ms deadline=10ms misses=14
[00:00:04.045,000] <inf> homework: [CONTROL] seq=14 latency=40ms deadline=10ms misses=15
[00:00:04.045,000] <wrn> homework: Skipped 3 messages
[00:00:04.045,000] <wrn> homework: [CONTROL] deadline_miss count=15 seq=14 latency=40ms
[00:00:04.295,000] <inf> homework: [CONTROL] seq=15 latency=40ms deadline=10ms misses=16
[00:00:04.545,000] <inf> homework: [CONTROL] seq=16 latency=40ms deadline=10ms misses=17
[00:00:04.795,000] <inf> homework: [CONTROL] seq=17 latency=40ms deadline=10ms misses=18
[00:00:05.045,000] <inf> homework: [CONTROL] seq=18 latency=40ms deadline=10ms misses=19
[00:00:05.045,000] <wrn> homework: Skipped 3 messages
[00:00:05.045,000] <wrn> homework: [CONTROL] deadline_miss count=19 seq=18 latency=40ms
[00:00:05.295,000] <inf> homework: [CONTROL] seq=19 latency=40ms deadline=10ms misses=20
```

Every sequence misses: `seq=0..19`, `latency=40ms`, `misses=20` after 20
events. The `deadline_miss` warning is rate-limited (`CONFIG_LOG_RATELIMIT_INTERVAL_MS=1000`),
so only one warning per second is printed and the rest appear as
`Skipped N messages`. The per-event `[CONTROL] seq=... misses=...` line still
records every sequence.

### Latency output: corrected priorities (passing)

```
*** Booting Zephyr OS build v4.4.0 ***
[00:00:00.000,000] <inf> homework: === L6 Homework: Runtime Investigation ===
[00:00:00.000,000] <inf> homework: Control work must start within 10 ms
[00:00:00.000,000] <inf> homework: Inspect, measure, trace, explain, and correct the delay
[00:00:00.500,000] <inf> homework: [CONTROL] seq=0 latency=0ms deadline=10ms misses=0
[00:00:00.750,000] <inf> homework: [CONTROL] seq=1 latency=0ms deadline=10ms misses=0
[00:00:01.000,000] <inf> homework: [CONTROL] seq=2 latency=0ms deadline=10ms misses=0
[00:00:01.250,000] <inf> homework: [CONTROL] seq=3 latency=0ms deadline=10ms misses=0
[00:00:01.500,000] <inf> homework: [CONTROL] seq=4 latency=0ms deadline=10ms misses=0
[00:00:01.750,000] <inf> homework: [CONTROL] seq=5 latency=0ms deadline=10ms misses=0
[00:00:02.000,000] <inf> homework: [CONTROL] seq=6 latency=0ms deadline=10ms misses=0
[00:00:02.250,000] <inf> homework: [CONTROL] seq=7 latency=0ms deadline=10ms misses=0
[00:00:02.500,000] <inf> homework: [CONTROL] seq=8 latency=0ms deadline=10ms misses=0
[00:00:02.750,000] <inf> homework: [CONTROL] seq=9 latency=0ms deadline=10ms misses=0
[00:00:03.000,000] <inf> homework: [CONTROL] seq=10 latency=0ms deadline=10ms misses=0
[00:00:03.250,000] <inf> homework: [CONTROL] seq=11 latency=0ms deadline=10ms misses=0
[00:00:03.500,000] <inf> homework: [CONTROL] seq=12 latency=0ms deadline=10ms misses=0
[00:00:03.750,000] <inf> homework: [CONTROL] seq=13 latency=0ms deadline=10ms misses=0
[00:00:04.000,000] <inf> homework: [CONTROL] seq=14 latency=0ms deadline=10ms misses=0
[00:00:04.250,000] <inf> homework: [CONTROL] seq=15 latency=0ms deadline=10ms misses=0
[00:00:04.500,000] <inf> homework: [CONTROL] seq=16 latency=0ms deadline=10ms misses=0
[00:00:04.750,000] <inf> homework: [CONTROL] seq=17 latency=0ms deadline=10ms misses=0
[00:00:05.000,000] <inf> homework: [CONTROL] seq=18 latency=0ms deadline=10ms misses=0
[00:00:05.250,000] <inf> homework: [CONTROL] seq=19 latency=0ms deadline=10ms misses=0
```

Same workload, same detector: `latency=0ms` for every sequence and
`misses=0`.

### CTF trace (decoded with Babeltrace)

The traces are committed as `data/fail/` (original priorities) and
`data/pass/` (corrected priorities). Each directory holds `channel0_0` plus
the CTF `metadata`. To decode them:

```bash
babeltrace2 data/fail
babeltrace2 data/pass
```

The excerpts below show the scheduler events around **seq=3**, filtered to
`thread_sched_ready`, `thread_switched_in/out`, `thread_busy_wait_*` and
`named_event`. The first column is the uptime in seconds. All other
sequences look the same.

**From `data/fail` (original priorities: control 7, maintenance 4):**

```
1.250000000  thread_sched_ready: { thread_id = 134629056, name = "control" }
1.250000000  thread_sched_ready: { thread_id = 134628896, name = "maintenance" }
1.250000000  named_event: { name = "event_triggered", arg0 = 3, arg1 = 1250 }
1.250000000  thread_switched_out: { thread_id = 134633216, name = "idle" }
1.250000000  thread_switched_in: { thread_id = 134628896, name = "maintenance" }
1.250000000  thread_busy_wait_enter: { usec_to_wait = 45000 }
1.295000000  thread_busy_wait_exit: { usec_to_wait = 45000 }
1.295000000  thread_switched_out: { thread_id = 134628896, name = "maintenance" }
1.295000000  thread_switched_in: { thread_id = 134629056, name = "control" }
1.295000000  named_event: { name = "event_processed", arg0 = 3, arg1 = 40 }
1.295000000  thread_switched_out: { thread_id = 134629056, name = "control" }
1.295000000  thread_switched_in: { thread_id = 134633216, name = "idle" }
```

How it shows the delay:

1. At **1.250 s** the timer ISR fires (`event_triggered`, seq 3), and both
   `control` and `maintenance` become ready at the same instant
   (`thread_sched_ready`).
2. The scheduler switches in **`maintenance`** first because it has the
   higher priority (4), and it immediately enters
   `thread_busy_wait_enter { 45000 us }`.
3. `control` is ready during this whole time but never runs. It only gets
   the CPU at **1.295 s**, right after `thread_busy_wait_exit`, when
   `maintenance` switches out.
4. The gap between `event_triggered` (1.250) and control's
   `thread_switched_in` (1.295) is the **45 ms delay**. `event_processed`
   reports `arg1 = 40`, the measured latency at 10 ms tick resolution.

**From `data/pass` (corrected priorities: control 4, maintenance 7):**

```
1.250000000  thread_sched_ready: { thread_id = 134629056, name = "control" }
1.250000000  thread_sched_ready: { thread_id = 134628896, name = "maintenance" }
1.250000000  named_event: { name = "event_triggered", arg0 = 3, arg1 = 1250 }
1.250000000  thread_switched_out: { thread_id = 134633216, name = "idle" }
1.250000000  thread_switched_in: { thread_id = 134629056, name = "control" }
1.250000000  named_event: { name = "event_processed", arg0 = 3, arg1 = 0 }
1.250000000  thread_switched_out: { thread_id = 134629056, name = "control" }
1.250000000  thread_switched_in: { thread_id = 134628896, name = "maintenance" }
1.250000000  thread_busy_wait_enter: { usec_to_wait = 45000 }
1.295000000  thread_busy_wait_exit: { usec_to_wait = 45000 }
1.295000000  thread_switched_out: { thread_id = 134628896, name = "maintenance" }
1.295000000  thread_switched_in: { thread_id = 134633216, name = "idle" }
```

Here `control` is switched in **at the same timestamp** as
`event_triggered` (1.250 s) and finishes with `latency = 0`. Only then does
`maintenance` run its 45 ms busy-wait, and it still completes long before
the next event at 1.500 s.

### `kernel thread list` (kernel shell)

**Original priorities (failing):**

```
uart:~$ kernel thread list
Scheduler: 0 since last call
Threads:
 0x8064620 maintenance
	options: 0x0, priority: 4 timeout: 0
	state: pending, entry: 0x8049c52
	stack size 2048, unused 2024, usage 24 / 2048 (1 %)

 0x80646c0 control   
	options: 0x0, priority: 7 timeout: 0
	state: pending, entry: 0x8049c78
	stack size 2048, unused 2024, usage 24 / 2048 (1 %)

*0x8065080 shell_uart
	options: 0x0, priority: 14 timeout: 0
	state: queued, entry: 0x8051ee3
	stack size 2048, unused 2024, usage 24 / 2048 (1 %)

 0x8064780 logging   
	options: 0x0, priority: 14 timeout: 0
	state: pending, entry: 0x804cb8f
	stack size 768, unused 744, usage 24 / 768 (3 %)

 0x8065700 idle      
	options: 0x1, priority: 15 timeout: 0
	state: , entry: 0x80556bf
	stack size 256, unused 232, usage 24 / 256 (9 %)
```

**Corrected priorities (passing):**

```
uart:~$ kernel thread list
Scheduler: 0 since last call
Threads:
 0x8064620 maintenance
	options: 0x0, priority: 7 timeout: 0
	state: pending, entry: 0x8049c52
	stack size 2048, unused 2024, usage 24 / 2048 (1 %)

 0x80646c0 control   
	options: 0x0, priority: 4 timeout: 0
	state: pending, entry: 0x8049c78
	stack size 2048, unused 2024, usage 24 / 2048 (1 %)

*0x8065080 shell_uart
	options: 0x0, priority: 14 timeout: 0
	state: queued, entry: 0x8051ee3
	stack size 2048, unused 2024, usage 24 / 2048 (1 %)

 0x8064780 logging   
	options: 0x0, priority: 14 timeout: 0
	state: pending, entry: 0x804cb8f
	stack size 768, unused 744, usage 24 / 768 (3 %)

 0x8065700 idle      
	options: 0x1, priority: 15 timeout: 0
	state: , entry: 0x80556bf
	stack size 256, unused 232, usage 24 / 256 (9 %)
```

In both snapshots, `control` and `maintenance` are `pending`: they are
blocked on `control_queue` and `maintenance_start` waiting for the next timer
event. The only thing that changes is the `priority` field, which is exactly
the root cause.

### Root cause

The timer ISR makes **both** threads ready at the same moment: it puts the
event in `control_queue` and gives `maintenance_start`. When the ISR returns,
the scheduler runs the highest-priority ready thread. With the original
priorities that thread is `maintenance` (4), not `control` (7).

`maintenance` then runs `k_busy_wait(45 ms)`. Busy-waiting never blocks or
yields, so `control` cannot run until the whole 45 ms is over. Every event is
therefore delayed by about 45 ms, which is 4.5 times the 10 ms budget.

The problem is not how much work `control` does. It is that a thread with no
short deadline was given precedence over the one with a short deadline: a
**priority assignment error**.

### Why the priority change fixes it

With `CONTROL_PRIORITY` 4 and `MAINTENANCE_PRIORITY` 7, `control` is the
highest-priority ready thread after the ISR. It runs immediately (latency
0 ms at tick resolution), and `maintenance` does its 45 ms afterwards.

`maintenance` has no short deadline, and its work still fits easily inside
the 250 ms event period, so delaying it costs nothing. Even if `maintenance`
were already busy-waiting when an event arrives, `control` would **preempt**
it, because both are preemptible threads and control's priority is now
higher. The 10 ms guarantee therefore holds no matter what `maintenance` is
doing.

The general rule is to assign priorities by deadline: the thread with the
shortest deadline gets the highest priority.
