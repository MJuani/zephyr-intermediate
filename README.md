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

## L5 Task 1: Bounded Queue and Task Watchdog

### Setup

| Element | Value |
|---|---|
| Producer (`sensor_thread_fn`) | 50 samples, one every `SENSOR_PERIOD_MS` = 100 ms, `k_msgq_put()` with a 100 ms timeout |
| Consumer (`logger_thread_fn`) | one sample every `LOGGER_PERIOD_MS` = 200 ms, `k_msgq_get()` with a 500 ms timeout |
| Queue (`sensor_msgq`) | `QUEUE_SIZE` = 20 messages |
| Monitor (`queue_monitor_thread`) | checks occupancy every 100 ms and warns when `used * 100 / QUEUE_SIZE >= 75` (15 of 20 messages or more) |
| Task watchdog | one channel owned by the **logger**, `LOGGER_TASK_WDT_TIMEOUT_MS` = 1000 ms |
| Stuck consumer | one `k_msleep(5000)` in the logger after it receives its 10th message |

#### Why the watchdog belongs to the consumer

The task watchdog channel is registered in `logger_thread_fn`. The logger
feeds it only **after** it has taken a message from `sensor_msgq` and processed
it, so the watchdog measures **consumer progress**.

The timeout (1000 ms) is longer than one normal logger iteration (about
210 ms), so the watchdog stays quiet while the logger runs normally. The
`k_msgq_get()` timeout (500 ms) is shorter than the watchdog timeout, so an
empty queue doesn't trigger the watchdog. When the logger exits, it calls
`task_wdt_delete()` to release its channel, so the watchdog doesn't fire after
the work is done.

### Log

```
*** Booting Zephyr OS build v4.4.0 ***
[00:00:00.000,000] <inf> l5_task1: === L5 Task1 ===
[00:00:00.000,000] <inf> l5_task1: sensor publishes every 100ms
[00:00:00.000,000] <inf> l5_task1: logger loggs every 200ms
[00:00:00.000,000] <inf> l5_task1: [SENSOR] publish seq=0 temp=24000 mC
[00:00:00.000,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=0 temp=24000 latency=0ms
[00:00:00.110,000] <inf> l5_task1: [SENSOR] publish seq=1 temp=24350 mC
[00:00:00.210,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=1 temp=24350 latency=100ms
[00:00:00.220,000] <inf> l5_task1: [SENSOR] publish seq=2 temp=24700 mC
[00:00:00.330,000] <inf> l5_task1: [SENSOR] publish seq=3 temp=25050 mC
[00:00:00.420,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=2 temp=24700 latency=200ms
[00:00:00.440,000] <inf> l5_task1: [SENSOR] publish seq=4 temp=25400 mC
[00:00:00.550,000] <inf> l5_task1: [SENSOR] publish seq=5 temp=25750 mC
[00:00:00.630,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=3 temp=25050 latency=300ms
[00:00:00.660,000] <inf> l5_task1: [SENSOR] publish seq=6 temp=26100 mC
[00:00:00.770,000] <inf> l5_task1: [SENSOR] publish seq=7 temp=26450 mC
[00:00:00.840,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=4 temp=25400 latency=400ms
[00:00:00.880,000] <inf> l5_task1: [SENSOR] publish seq=8 temp=26800 mC
[00:00:00.990,000] <inf> l5_task1: [SENSOR] publish seq=9 temp=27150 mC
[00:00:01.050,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=5 temp=25750 latency=500ms
[00:00:01.100,000] <inf> l5_task1: [SENSOR] publish seq=10 temp=27500 mC
[00:00:01.210,000] <inf> l5_task1: [SENSOR] publish seq=11 temp=27850 mC
[00:00:01.260,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=6 temp=26100 latency=600ms
[00:00:01.320,000] <inf> l5_task1: [SENSOR] publish seq=12 temp=28200 mC
[00:00:01.430,000] <inf> l5_task1: [SENSOR] publish seq=13 temp=28550 mC
[00:00:01.470,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=7 temp=26450 latency=700ms
[00:00:01.540,000] <inf> l5_task1: [SENSOR] publish seq=14 temp=28900 mC
[00:00:01.650,000] <inf> l5_task1: [SENSOR] publish seq=15 temp=29250 mC
[00:00:01.680,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=8 temp=26800 latency=800ms
[00:00:01.760,000] <inf> l5_task1: [SENSOR] publish seq=16 temp=29600 mC
[00:00:01.870,000] <inf> l5_task1: [SENSOR] publish seq=17 temp=29950 mC
[00:00:01.890,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=9 temp=27150 latency=900ms
[00:00:01.980,000] <inf> l5_task1: [SENSOR] publish seq=18 temp=30300 mC
[00:00:02.090,000] <inf> l5_task1: [SENSOR] publish seq=19 temp=30650 mC
[00:00:02.200,000] <inf> l5_task1: [SENSOR] publish seq=20 temp=31000 mC
[00:00:02.310,000] <inf> l5_task1: [SENSOR] publish seq=21 temp=31350 mC
[00:00:02.420,000] <inf> l5_task1: [SENSOR] publish seq=22 temp=31700 mC
[00:00:02.530,000] <inf> l5_task1: [SENSOR] publish seq=23 temp=32050 mC
[00:00:02.640,000] <inf> l5_task1: [SENSOR] publish seq=24 temp=32400 mC
[00:00:02.640,000] <wrn> l5_task1: [MONITOR] queue is 75% full
[00:00:02.690,000] <err> l5_task1: [LOGGER-TASK-WDT] Process stuck!
[00:00:02.750,000] <inf> l5_task1: [SENSOR] publish seq=25 temp=32750 mC
[00:00:02.750,000] <wrn> l5_task1: [MONITOR] queue is 80% full
[00:00:02.860,000] <inf> l5_task1: [SENSOR] publish seq=26 temp=33100 mC
[00:00:02.860,000] <wrn> l5_task1: [MONITOR] queue is 85% full
[00:00:02.970,000] <inf> l5_task1: [SENSOR] publish seq=27 temp=33450 mC
[00:00:02.970,000] <wrn> l5_task1: [MONITOR] queue is 90% full
[00:00:03.080,000] <inf> l5_task1: [SENSOR] publish seq=28 temp=33800 mC
[00:00:03.080,000] <wrn> l5_task1: [MONITOR] queue is 95% full
[00:00:03.190,000] <inf> l5_task1: [SENSOR] publish seq=29 temp=34150 mC
[00:00:03.190,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:03.300,000] <inf> l5_task1: [SENSOR] publish seq=30 temp=34500 mC
[00:00:03.300,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:03.410,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:03.410,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:03.520,000] <inf> l5_task1: [SENSOR] publish seq=31 temp=34850 mC
[00:00:03.520,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:03.630,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:03.630,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:03.740,000] <inf> l5_task1: [SENSOR] publish seq=32 temp=35200 mC
[00:00:03.740,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:03.850,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:03.850,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:03.960,000] <inf> l5_task1: [SENSOR] publish seq=33 temp=35550 mC
[00:00:03.960,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:04.070,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:04.070,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:04.180,000] <inf> l5_task1: [SENSOR] publish seq=34 temp=35900 mC
[00:00:04.180,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:04.290,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:04.290,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:04.400,000] <inf> l5_task1: [SENSOR] publish seq=35 temp=36250 mC
[00:00:04.400,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:04.510,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:04.510,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:04.620,000] <inf> l5_task1: [SENSOR] publish seq=36 temp=36600 mC
[00:00:04.620,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:04.730,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:04.730,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:04.840,000] <inf> l5_task1: [SENSOR] publish seq=37 temp=36950 mC
[00:00:04.840,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:04.950,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:04.950,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:05.060,000] <inf> l5_task1: [SENSOR] publish seq=38 temp=37300 mC
[00:00:05.060,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:05.170,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:05.170,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:05.280,000] <inf> l5_task1: [SENSOR] publish seq=39 temp=37650 mC
[00:00:05.280,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:05.390,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:05.390,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:05.500,000] <inf> l5_task1: [SENSOR] publish seq=40 temp=38000 mC
[00:00:05.500,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:05.610,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:05.610,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:05.720,000] <inf> l5_task1: [SENSOR] publish seq=41 temp=38350 mC
[00:00:05.720,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:05.830,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:05.830,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:05.940,000] <inf> l5_task1: [SENSOR] publish seq=42 temp=38700 mC
[00:00:05.940,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:06.050,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:06.050,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:06.160,000] <inf> l5_task1: [SENSOR] publish seq=43 temp=39050 mC
[00:00:06.160,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:06.270,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:06.270,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:06.380,000] <inf> l5_task1: [SENSOR] publish seq=44 temp=39400 mC
[00:00:06.380,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:06.490,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:06.490,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:06.600,000] <inf> l5_task1: [SENSOR] publish seq=45 temp=39750 mC
[00:00:06.600,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:06.710,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:06.710,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:06.820,000] <inf> l5_task1: [SENSOR] publish seq=46 temp=40100 mC
[00:00:06.820,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:06.930,000] <wrn> l5_task1: [SENSOR] publish failed ret=-11
[00:00:06.930,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:07.040,000] <inf> l5_task1: [SENSOR] publish seq=47 temp=40450 mC
[00:00:07.040,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:07.110,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=10 temp=27500 latency=6010ms
[00:00:07.150,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:07.220,000] <inf> l5_task1: [SENSOR] publish seq=48 temp=40800 mC
[00:00:07.260,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:07.320,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=11 temp=27850 latency=6110ms
[00:00:07.370,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:07.430,000] <inf> l5_task1: [SENSOR] publish seq=49 temp=41150 mC
[00:00:07.480,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:07.530,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=12 temp=28200 latency=6210ms
[00:00:07.590,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:07.640,000] <inf> l5_task1: [SENSOR] done
[00:00:07.700,000] <wrn> l5_task1: [MONITOR] queue is 100% full
[00:00:07.740,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=13 temp=28550 latency=6310ms
[00:00:07.810,000] <wrn> l5_task1: [MONITOR] queue is 95% full
[00:00:07.920,000] <wrn> l5_task1: [MONITOR] queue is 95% full
[00:00:07.950,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=14 temp=28900 latency=6410ms
[00:00:08.030,000] <wrn> l5_task1: [MONITOR] queue is 90% full
[00:00:08.140,000] <wrn> l5_task1: [MONITOR] queue is 90% full
[00:00:08.160,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=15 temp=29250 latency=6510ms
[00:00:08.250,000] <wrn> l5_task1: [MONITOR] queue is 85% full
[00:00:08.360,000] <wrn> l5_task1: [MONITOR] queue is 85% full
[00:00:08.370,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=16 temp=29600 latency=6610ms
[00:00:08.470,000] <wrn> l5_task1: [MONITOR] queue is 80% full
[00:00:08.580,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=17 temp=29950 latency=6710ms
[00:00:08.580,000] <wrn> l5_task1: [MONITOR] queue is 75% full
[00:00:08.690,000] <wrn> l5_task1: [MONITOR] queue is 75% full
[00:00:08.790,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=18 temp=30300 latency=6810ms
[00:00:09.000,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=19 temp=30650 latency=6910ms
[00:00:09.210,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=20 temp=31000 latency=7010ms
[00:00:09.420,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=21 temp=31350 latency=7110ms
[00:00:09.630,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=22 temp=31700 latency=7210ms
[00:00:09.840,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=23 temp=32050 latency=7310ms
[00:00:10.050,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=24 temp=32400 latency=7410ms
[00:00:10.260,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=25 temp=32750 latency=7510ms
[00:00:10.470,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=26 temp=33100 latency=7610ms
[00:00:10.680,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=27 temp=33450 latency=7710ms
[00:00:10.890,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=28 temp=33800 latency=7810ms
[00:00:11.100,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=29 temp=34150 latency=7910ms
[00:00:11.310,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=47 temp=40450 latency=4270ms
[00:00:11.520,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=48 temp=40800 latency=4300ms
[00:00:11.730,000] <inf> l5_task1: [LOGGER-MSG] thread=logger seq=49 temp=41150 latency=4300ms
[00:00:12.450,000] <wrn> l5_task1: [LOGGER-MSG] timeout ret=-11
[00:00:12.450,000] <inf> l5_task1: [LOGGER-MSG] done received=33
```

### Timeline

| Time | Event |
|---|---|
| 0.00 – 1.89 s | Normal operation. The logger takes seq 0 to 9 and feeds the watchdog after each one. |
| 1.68 s | **Last watchdog feed** before the stall (after seq 8) |
| 1.89 s | The logger takes seq 9 (its 10th message) and **stalls** in `k_msleep(5000)` |
| 1.89 – 3.19 s | The sensor keeps publishing, and with no `k_msgq_get()` calls the queue grows by one message about every 110 ms |
| 2.64 s | Monitor: **queue is 75% full** (15 of 20 messages) |
| 2.69 s | **`[LOGGER-TASK-WDT] Process stuck!`**, 1000 ms after the last feed |
| 3.19 s | The queue reaches **100%**. It holds seq 10 to 29. |
| 3.30 – 6.93 s | Every `k_msgq_put()` waits its 100 ms timeout and returns `-EAGAIN` (`ret=-11`). **Seq 30 to 46 are dropped (17 samples).** |
| 6.89 s | The stall ends, the logger feeds the watchdog again, and supervision continues normally |
| 7.11 s | The logger takes seq 10, which waited **6010 ms** in the queue |
| 7.04 → 7.22 s | Seq 47's put was waiting when the logger freed a slot, so it got in. Seq 48 and 49 also fit. |
| 7.64 – 11.73 s | The sensor is done and the logger drains the backlog. The monitor stops warning once the queue drops below 75% at 8.69 s. |
| 11.10 → 11.31 s | The logger jumps from **seq 29 to seq 47**, which is exactly the gap the dropped samples left |
| 12.45 s | `k_msgq_get()` times out with no more data. The logger calls `task_wdt_delete()` and exits with **`received=33`** (50 − 17 dropped). |

Note: each `publish failed ret=-11` line belongs to the `publish seq=N` line
just above it, because the sensor logs the sample before it calls
`k_msgq_put()`.

### Queue buildup

1. **Before the stall, a small backlog already forms.** The logger needs about
   210 ms per message and the sensor about 110 ms, so latency climbs by about
   100 ms per message (0 to 900 ms for seq 0 to 9). When the stall begins,
   8 messages (seq 10 to 17) are already waiting.
2. **During the stall, nothing is consumed.** The sensor keeps publishing at
   its normal rate, so the queue grows by one message per producer period. It
   crosses the 75% warning threshold at 2.64 s and fills completely at
   3.19 s.
3. **When the queue is full, samples are lost.** The queue is bounded, so the
   producer cannot keep adding messages. `k_msgq_put()` blocks for its 100 ms
   timeout and then returns `-EAGAIN`. The sample is discarded, the sensor
   sleeps 100 ms, and the next sample meets the same full queue. That is why
   every other log line is a failure. The 17 dropped samples (seq 30 to 46)
   are the cost of the stall.
4. **After the stall, latency is high.** The messages already in the queue
   are old by the time the logger reads them: seq 10 waited 6.0 s, and
   latency keeps growing to 7.9 s for seq 29, because the logger drains more
   slowly than they were produced. The queue keeps the **oldest** data and
   rejects new data, so the logger processes stale samples and newer ones
   are lost.

The monitor shows the whole cycle: it warns from 75% up to 100% while the
logger is stuck, and the reported fill level then drops back through 95% … 75%
as the logger drains the queue.

### Watchdog callback

1. Each normal iteration ends with `task_wdt_feed()`, which sets the
   channel's deadline to `now + 1000 ms`. The last feed before the stall is
   at about 1.68 s, so the deadline is about 2.68 s.
2. The logger is blocked in `k_msleep(5000)` and misses that deadline. At
   **2.69 s** the task watchdog calls `logger_thread_wdt_cb()`, which logs
   `[LOGGER-TASK-WDT] Process stuck!`.
3. A callback is registered, so the task watchdog calls it **instead of
   rebooting** (it only calls `sys_reboot()` when no callback is given).
   `main()` calls `task_wdt_init(NULL)`, so no hardware watchdog backs it up,
   and the system keeps running.
4. The callback fires **once**, even though the stall lasts 5 s. After
   calling the callback, the task watchdog doesn't re-arm the channel. The
   channel is armed again by the next `task_wdt_feed()` at 6.89 s, and the
   logger feeds on time from then on, so the callback doesn't fire again.
5. The callback runs from the task watchdog's kernel timer, which means
   **interrupt context**. It must be short and must not block. A deferred
   `LOG_ERR` is fine.
6. At the end of the run, the logger calls `task_wdt_delete()` before
   exiting, so the watchdog doesn't report the finished thread as stuck.

The watchdog does not prevent data loss; the bounded queue and the drop
policy decide what is lost. What the watchdog adds is **detection**. It
reports that the consumer has stopped making progress. From the producer's
side you can't tell that, because all it sees is failed puts, which can also
happen during a short burst.
