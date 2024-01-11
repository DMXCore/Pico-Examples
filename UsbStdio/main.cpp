#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/async_context_threadsafe_background.h"


const uint LED_PIN_INDICATOR = 25;

static bool work_done;

static void async_worker_func(async_context_t *async_context, async_when_pending_worker_t *worker);

// An async context is notified by the irq to "do some work"
static async_context_threadsafe_background_t async_context;
static async_when_pending_worker_t worker = { .do_work = async_worker_func };


static void async_worker_func(async_context_t *async_context, async_when_pending_worker_t *worker)
{
    while(1)
    {
        int key = getchar_timeout_us(0);
        
        if (key == PICO_ERROR_TIMEOUT)
            break;

        printf("Key: %d\n", key);

        gpio_put(LED_PIN_INDICATOR, !gpio_get(LED_PIN_INDICATOR));
    }
}

void received_char_callback(void* param)
{
    // Tell the async worker that there are some characters waiting for us
    async_context_set_work_pending(&async_context.core, &worker);
}

int main()
{
    stdio_init_all();

    // There isn't a strict rule that you must always call sleep_ms()
    // after stdio_init_all(). However, in some cases, it can be a helpful
    // precautionary measure to ensure that the UART has properly 
    // initialized and is ready to transmit data without any issues.
    sleep_ms(2000);

    printf("--==Init==--\n");

    // Init all inputs and outputs
    gpio_init(LED_PIN_INDICATOR);

    gpio_set_dir(LED_PIN_INDICATOR, GPIO_OUT);

    // Set initial state
    gpio_put(LED_PIN_INDICATOR, 0);

    // Setup an async context and worker to perform work when needed
    if (!async_context_threadsafe_background_init_with_defaults(&async_context)) {
        panic("failed to setup context");
    }
    async_context_add_when_pending_worker(&async_context.core, &worker);

    stdio_set_chars_available_callback(received_char_callback, NULL);

    // Send to console
    printf("Start!\n");

    while (1) {
        // Note that we could just sleep here as we're using "threadsafe_background" that uses a low priority interrupt
        // But if we changed to use a "polling" context that wouldn't work. The following works for both types of context.
        // When using "threadsafe_background" the poll does nothing. This loop is just preventing main from exiting!
        work_done = false;
        async_context_poll(&async_context.core);
        async_context_wait_for_work_ms(&async_context.core, 500);
    }
}
