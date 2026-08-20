#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(demo, LOG_LEVEL_DBG);

#define STACK_SIZE 1024

#define PRIO_COOP  (-1)
#define PRIO_HIGH    3
#define PRIO_MED     5
#define PRIO_LOW     7

void coop_fn(void *p1, void *p2, void *p3)
{
    LOG_INF("[COOP] starting - will run 5 steps without yielding");

    for (int i = 0; i < 5; i++) {
        k_busy_wait(40000);   
        LOG_INF("[COOP] step %d/5 - still holding CPU  tick=%u",
                i + 1, k_uptime_get_32());
    }

    LOG_INF("[COOP] yielding");
    k_yield();

    LOG_INF("[COOP] done");
}

void high_fn(void *p1, void *p2, void *p3)
{
    uint32_t step = 0;
    LOG_INF("[HIGH] started");

    while (1) {
        LOG_INF("[HIGH] step %d  tick=%u", step++, k_uptime_get_32());
        k_msleep(100);
    }
}

void med_fn(void *p1, void *p2, void *p3)
{
    uint32_t step = 0;
    LOG_INF("[MED] started");

    while (1) {
        LOG_INF("[MED] step %d  tick=%u", step++, k_uptime_get_32());
        k_msleep(200);
    }
}

void low_fn(void *p1, void *p2, void *p3)
{
    uint32_t step = 0;
    LOG_INF("[LOW] started");

    while (1) {
        LOG_INF("[LOW] step %d  tick=%u", step++, k_uptime_get_32());
        k_msleep(300);
    }
}

K_THREAD_DEFINE(t_coop, STACK_SIZE, coop_fn, NULL, NULL, NULL, PRIO_COOP, 0, 0);
K_THREAD_DEFINE(t_high, STACK_SIZE, high_fn, NULL, NULL, NULL, PRIO_HIGH, 0, 0);
K_THREAD_DEFINE(t_med,  STACK_SIZE, med_fn,  NULL, NULL, NULL, PRIO_MED,  0, 0);
K_THREAD_DEFINE(t_low,  STACK_SIZE, low_fn,  NULL, NULL, NULL, PRIO_LOW,  0, 0);

int main(void)
{
    LOG_INF("====== L1 Task 1 ======");
    LOG_INF("COOP prio=%d (cooperative),  HIGH prio=%d, MED prio=%d, LOW prio=%d",
            PRIO_COOP, PRIO_HIGH, PRIO_MED, PRIO_LOW);
    return 0;
}
