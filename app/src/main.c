#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/tracing/tracing.h>

LOG_MODULE_REGISTER(homework, LOG_LEVEL_INF);

#define STACK_SIZE            2048
#define CONTROL_PRIORITY         4
#define MAINTENANCE_PRIORITY     7
#define EVENT_PERIOD_MS        250
#define MAINTENANCE_LOAD_US  45000

/*
 * Guarantee: control work starts within DEADLINE_MS (10 ms) of the timer
 * expiry that created the event.
 *
 * Root cause of the original misses (CONTROL_PRIORITY 7, MAINTENANCE_PRIORITY 4):
 * the timer ISR makes both threads ready at the same instant. In Zephyr a
 * lower number is a higher priority, so maintenance (4) ran first and spent
 * MAINTENANCE_LOAD_US (45 ms) in k_busy_wait() before control (7) could run.
 * Every event started ~45 ms late, regardless of how little work control does.
 *
 * Fix: control gets the higher priority (4) and maintenance the lower one (7).
 * When both become ready, control runs immediately and maintenance does its
 * 45 ms afterwards; maintenance has no short deadline, so the delay is harmless.
 * See README.md (L6 Task 1) for logs, trace excerpts and thread lists.
 */
#define DEADLINE_MS             10

struct control_event {
    uint32_t seq;
    uint32_t ready_ms;
};

K_MSGQ_DEFINE(control_queue, sizeof(struct control_event), 4, 4);
K_SEM_DEFINE(maintenance_start, 0, 1);

/* ================================================================== */
/*  Timer expiry: creates one control event                           */
/* ================================================================== */

static void event_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    static uint32_t seq;
    struct control_event event = {
        .seq = seq++,
        .ready_ms = k_uptime_get_32(),
    };

    /* Timer expiry runs in interrupt context, so never wait here. */
    int ret = k_msgq_put(&control_queue, &event, K_NO_WAIT);

    if (ret != 0) {
        return;
    }

    /* Both threads become ready when the timer interrupt returns. */
    k_sem_give(&maintenance_start);

    /* TODO: Add an application trace event for this sequence. */
    sys_trace_named_event("event_triggered", event.seq, event.ready_ms);
}

K_TIMER_DEFINE(event_timer, event_timer_expiry, NULL);

/* ================================================================== */
/*  Control thread                                                   */
/* ================================================================== */

static void control_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    uint32_t deadline_misses = 0;

    while (true) {
        struct control_event event;
        int ret = k_msgq_get(&control_queue, &event, K_FOREVER);

        if (ret != 0) {
            LOG_ERR("[CONTROL] receive failed: %d", ret);
            continue;
        }

        /*
         * Latency = time from the timer expiry (ready_ms, taken in the ISR)
         * until control starts its work for this event. It is measured
         * immediately after k_msgq_get() returns, before any logging, so it
         * covers ISR -> control wake-up plus any time control spent waiting
         * for higher-priority threads. Resolution is one system tick.
         */
        uint32_t latency_ms = k_uptime_get_32() - event.ready_ms;
        bool missed = latency_ms > DEADLINE_MS;

        if (missed) {
            deadline_misses++;
        }

        /* One line per event: seq, latency and running miss count. */
        LOG_INF("[CONTROL] seq=%u latency=%ums deadline=%ums misses=%u",
                event.seq, latency_ms, DEADLINE_MS, deadline_misses);

        if (missed) {
            /* Rate-limited so a persistent fault does not flood the log. */
            LOG_WRN_RATELIMIT("[CONTROL] deadline_miss count=%u seq=%u latency=%ums",
                              deadline_misses, event.seq, latency_ms);
        }

        /* TODO: Add an application trace event for completion. */
        sys_trace_named_event("event_processed", event.seq, latency_ms);        
    }
}

/* ================================================================== */
/*  Background maintenance thread                                    */
/* ================================================================== */

static void maintenance_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    while (true) {
        k_sem_take(&maintenance_start, K_FOREVER);

        /* This work is important, but it has no short deadline. */
        k_busy_wait(MAINTENANCE_LOAD_US);
    }
}

K_THREAD_DEFINE(control, STACK_SIZE, control_fn,
                NULL, NULL, NULL, CONTROL_PRIORITY, 0, 0);

K_THREAD_DEFINE(maintenance, STACK_SIZE, maintenance_fn,
                NULL, NULL, NULL, MAINTENANCE_PRIORITY, 0, 0);

int main(void)
{
    LOG_INF("=== L6 Homework: Runtime Investigation ===");
    LOG_INF("Control work must start within 10 ms");
    LOG_INF("Inspect, measure, trace, explain, and correct the delay");

    k_timer_start(&event_timer, K_MSEC(500), K_MSEC(EVENT_PERIOD_MS));

    return 0;
}