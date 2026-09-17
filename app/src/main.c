#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/task_wdt/task_wdt.h>

LOG_MODULE_REGISTER(l4_task1, LOG_LEVEL_DBG);

#define STACK_SIZE                  2048

#define SENSOR_COUNT                50
#define SENSOR_PERIOD_MS            100
#define SENSOR_TASK_WDT_TIMEOUT_MS  150

#define LOGGER_PERIOD_MS            500

#define MONITOR_PERIOD_MS           100

#define QUEUE_SIZE                  20

/* ================================================================== */
/*  Shared channel message                                            */
/* ================================================================== */

struct sensor_data {
    int32_t temperature_mc;
    uint32_t timestamp_ms;
    uint8_t seq;
};

/* ================================================================== */
/*  Queue                                                           */
/* ================================================================== */

K_MSGQ_DEFINE(sensor_msgq, sizeof(struct sensor_data), QUEUE_SIZE, 4);

/* ================================================================== */
/*  Producer - Sensor                                                 */
/* ================================================================== */

static void sensor_thread_wdt_cb(int channel_id, void *user_data){
    LOG_ERR("[TASK-WDT] queue is full!");
}

static void sensor_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "sensor");

    int wdt_chan = task_wdt_add(
        SENSOR_TASK_WDT_TIMEOUT_MS,
        sensor_thread_wdt_cb,
        NULL
    );

    for (int i = 0; i < SENSOR_COUNT; i++) {
        struct sensor_data data = {
            .temperature_mc = 24000 + (i * 350),
            .timestamp_ms = k_uptime_get_32(),
            .seq = (uint8_t)i,
        };

        LOG_INF("[SENSOR] publish seq=%u temp=%d mC",
                data.seq,
                data.temperature_mc);

        int ret = k_msgq_put(&sensor_msgq, &data, K_MSEC(100));
        if (ret != 0) {
            LOG_WRN("[SENSOR] publish failed ret=%d", ret);
        }

        task_wdt_feed(wdt_chan);
        k_msleep(SENSOR_PERIOD_MS);
    }

    LOG_INF("[SENSOR] done");
}

/* ================================================================== */
/*  Consumer - logger                                                 */
/* ================================================================== */

static void logger_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "logger");

    int received = 0;

    while (received < SENSOR_COUNT) {
        struct sensor_data msg;

        int ret = k_msgq_get(&sensor_msgq, &msg, K_MSEC(1500));
        if (ret != 0) {
            LOG_WRN("[LOGGER-MSG] timeout ret=%d", ret);
            break;
        }

        received++;

        LOG_INF("[LOGGER-MSG] thread=%s seq=%u temp=%d latency=%ums",
                k_thread_name_get(k_current_get()),
                msg.seq,
                msg.temperature_mc,
                k_uptime_get_32() - msg.timestamp_ms);

        k_msleep(LOGGER_PERIOD_MS);
    }

    LOG_INF("[LOGGER-MSG] done received=%d", received);
}

/* ================================================================== */
/*  Health check monitor                                              */
/* ================================================================== */

static void queue_monitor_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    while (1) {

        uint32_t used = k_msgq_num_used_get(&sensor_msgq);
        
        int full_percent = (used * 100) / QUEUE_SIZE;

        if(full_percent == 75){
            LOG_WRN("[MONITOR] queue is %d%% full", full_percent);
        }

        k_msleep(MONITOR_PERIOD_MS);
    }
}

/* ================================================================== */
/*  Threads                                                           */
/* ================================================================== */

K_THREAD_DEFINE(sensor_thread, STACK_SIZE, sensor_thread_fn,
                NULL, NULL, NULL, 5, 0, 0);

K_THREAD_DEFINE(logger_thread, STACK_SIZE, logger_thread_fn,
                NULL, NULL, NULL, 6, 0, 0);

K_THREAD_DEFINE(monitor_thread, STACK_SIZE, queue_monitor_thread,
                NULL, NULL, NULL, 6, 0, 0);                

/* ================================================================== */
/*  Main                                                              */
/* ================================================================== */

int main(void)
{

    if (task_wdt_init(NULL)) {
        LOG_ERR("task_wdt_init() failed");
        return 0;
    }

    LOG_INF("=== L4 Task1: Zbus Pub-Sub ===");
    LOG_INF("sensor publishes every %dms", SENSOR_PERIOD_MS);
    LOG_INF("display listener runs in publisher context");
    LOG_INF("logger uses message subscriber copies, loggs every %dms", LOGGER_PERIOD_MS);

    return 0;
}