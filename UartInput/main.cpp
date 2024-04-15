#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "pico/async_context_threadsafe_background.h"
#include "pico/stdio_usb.h"
#include "DmxOutput.h"
#include "DmxInput.h"
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "uart_rx.pio.h"
#include "uart_tx.pio.h"

//#define PICO2_HARDWARE_REV 10
//#define PICO2_HARDWARE_REV 11
#define PICO2_HARDWARE_REV 12

#define SERIAL_BAUD 9600

#if PICO2_HARDWARE_REV == 10
// Hardware revision 1.0 used pins 6 and 7 for the expansion port
#define PIO_RX_PIN 7
#define PIO_TX_PIN 6
#elif PICO2_HARDWARE_REV == 11
// Hardware revision 1.1 used pins 21 and 22 for the expansion port
#define PIO_RX_PIN 22
#define PIO_TX_PIN 21
#else
// Hardware revision 1.2 and later used pins 4 and 5 for the expansion port
#define PIO_RX_PIN 5
#define PIO_TX_PIN 4
#endif
#define FIFO_SIZE 64

#define MAX_STRING_LENGTH 50


#define UNIVERSE_LENGTH 512
DmxOutput dmxOutput;
uint8_t dmxData[UNIVERSE_LENGTH + 1];

static PIO pio;
static uint sm_rx;
static uint sm_tx;
static int8_t pio_irq;
static queue_t fifo;
static uint offset_rx;
static uint offset_tx;
static bool work_done;


const uint DMXOUT1_PIN = 6;
const uint DMXIN1_PIN = 7;
const uint DMXENA1_PIN = 3;

const uint DMXOUT2_PIN = 0;
const uint DMXIN2_PIN = 2;
const uint DMXENA2_PIN = 1;

const uint OUTPUT_PIN1 = 15;
const uint OUTPUT_PIN2 = 16;
const uint OUTPUT_PIN3 = 17;
const uint OUTPUT_PIN4 = 18;
const uint LED_PIN_DMXPORTA = 24;
const uint LED_PIN_DMXPORTB = 23;
const uint LED_PIN_INDICATOR = 25;
const uint INPUT_PIN1 = 8;
const uint INPUT_PIN2 = 9;
const uint INPUT_PIN3 = 10;
const uint INPUT_PIN4 = 11;


uint32_t timer_counter;
bool ledValue;
bool ledAValue;
bool ledBValue;

int led_mod_value;
int ledA_mod_value;
int ledB_mod_value;

char receivedString[MAX_STRING_LENGTH + 1]; // +1 for null terminator
int writeIndex = 0;


void processString(const char *str) {
    // Your implementation for processing the received string
    // This function will be called when a complete string is received
    // You can replace or implement your logic here
    printf("Received: %s\n", str);

    int len = strlen(str);

    if (len >= 2)
    {
        if (str[0] == '.')
        {
            switch (str[1])
            {
                case 'X':
                    memset(dmxData, 0, sizeof(dmxData));
                break;

                case 'A':
                    memset(&dmxData[1], 255, sizeof(dmxData) - 1);
                break;
            }
        }
    }
}

static void async_worker_func(async_context_t *async_context, async_when_pending_worker_t *worker);

// An async context is notified by the irq to "do some work"
static async_context_threadsafe_background_t async_context;
static async_when_pending_worker_t worker = { .do_work = async_worker_func };

static void core1_main() {
    while(1) {
        int key = getchar();

        if (key != PICO_ERROR_TIMEOUT)
        {
            char c = (char)key;

            if (!queue_try_add(&fifo, &c)) {
                panic("fifo full");
            }

            // Tell the async worker that there are some characters waiting for us
            async_context_set_work_pending(&async_context.core, &worker);
        }
    }
}

// IRQ called when the pio fifo is not empty, i.e. there are some characters on the uart
// This needs to run as quickly as possible or else you will lose characters (in particular don't printf!)
static void pio_irq_func(void) {
    while(!pio_sm_is_rx_fifo_empty(pio, sm_rx)) {
        char c = uart_rx_program_getc(pio, sm_rx);
        if (!queue_try_add(&fifo, &c)) {
            panic("fifo full");
        }
    }
    // Tell the async worker that there are some characters waiting for us
    async_context_set_work_pending(&async_context.core, &worker);
}

// Process characters
static void async_worker_func(async_context_t *async_context, async_when_pending_worker_t *worker)
{
    work_done = true;
    while (!queue_is_empty(&fifo))
    {
        char c;
        if (!queue_try_remove(&fifo, &c))
        {
            panic("fifo empty");
        }

        if (c == '\r')
            // Ignore
            continue;

        if (c == '\n')
        {
            receivedString[writeIndex] = '\0'; // Null-terminate the string
            processString(receivedString);     // Call the function with the received string
            writeIndex = 0;                    // Reset index for the next string
        }
        else if (writeIndex < MAX_STRING_LENGTH)
        {
            receivedString[writeIndex++] = c; // Add character to the string
        }
    }
}

// Find a free pio and state machine and load the program into it.
// Returns false if this fails
static bool init_pio(const pio_program_t *program, PIO *pio_hw, uint *sm, uint *offset) {
    // Find a free pio
    *pio_hw = pio1;
    if (!pio_can_add_program(*pio_hw, program)) {
        *pio_hw = pio0;
        if (!pio_can_add_program(*pio_hw, program)) {
            *offset = -1;
            return false;
        }
    }
    *offset = pio_add_program(*pio_hw, program);
    // Find a state machine
    *sm = (int8_t)pio_claim_unused_sm(*pio_hw, false);
    if (*sm < 0) {
        return false;
    }
    return true;
}

bool repeating_timer_callback(struct repeating_timer *t)
{
    // Make sure we're not in the middle of sending
    while (dmxOutput.busy());
    // Send DMX
    dmxOutput.write(dmxData, UNIVERSE_LENGTH + 1);

    ledB_mod_value = 2;

    timer_counter++;
    led_mod_value = 40;

    if (led_mod_value == 0)
    {
        gpio_put(LED_PIN_INDICATOR, 0);
    }
    else if (led_mod_value == 1)
    {
        gpio_put(LED_PIN_INDICATOR, 1);
    }
    else if (timer_counter % (led_mod_value - 1) == 0)
    {
        ledValue = !ledValue;
        gpio_put(LED_PIN_INDICATOR, ledValue);
    }

    if (ledA_mod_value == 0)
    {
        gpio_put(LED_PIN_DMXPORTA, 0);
    }
    else if (ledA_mod_value == 1)
    {
        gpio_put(LED_PIN_DMXPORTA, 1);
    }
    else if (timer_counter % (ledA_mod_value - 1) == 0)
    {
        ledAValue = !ledAValue;
        gpio_put(LED_PIN_DMXPORTA, ledAValue);
    }

    if (ledB_mod_value == 0)
    {
        gpio_put(LED_PIN_DMXPORTB, 0);
    }
    else if (ledB_mod_value == 1)
    {
        gpio_put(LED_PIN_DMXPORTB, 1);
    }
    else if (timer_counter % (ledB_mod_value - 1) == 0)
    {
        ledBValue = !ledBValue;
        gpio_put(LED_PIN_DMXPORTB, ledBValue);
    }

    return true;
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
    gpio_init(INPUT_PIN1);
    gpio_init(INPUT_PIN2);
    gpio_init(INPUT_PIN3);
    gpio_init(INPUT_PIN4);
    gpio_init(OUTPUT_PIN1);
    gpio_init(OUTPUT_PIN2);
    gpio_init(OUTPUT_PIN3);
    gpio_init(OUTPUT_PIN4);
    gpio_init(LED_PIN_DMXPORTA);
    gpio_init(LED_PIN_DMXPORTB);
    gpio_init(LED_PIN_INDICATOR);

    gpio_set_dir(INPUT_PIN1, GPIO_IN);
    gpio_set_dir(INPUT_PIN2, GPIO_IN);
    gpio_set_dir(INPUT_PIN3, GPIO_IN);
    gpio_set_dir(INPUT_PIN4, GPIO_IN);
    gpio_set_dir(OUTPUT_PIN1, GPIO_OUT);
    gpio_set_dir(OUTPUT_PIN2, GPIO_OUT);
    gpio_set_dir(OUTPUT_PIN3, GPIO_OUT);
    gpio_set_dir(OUTPUT_PIN4, GPIO_OUT);
    gpio_set_dir(LED_PIN_DMXPORTA, GPIO_OUT);
    gpio_set_dir(LED_PIN_DMXPORTB, GPIO_OUT);
    gpio_set_dir(LED_PIN_INDICATOR, GPIO_OUT);

    // Set initial state
    gpio_put(OUTPUT_PIN1, 0);
    gpio_put(OUTPUT_PIN2, 0);
    gpio_put(OUTPUT_PIN3, 0);
    gpio_put(OUTPUT_PIN4, 0);
    gpio_put(LED_PIN_DMXPORTA, 0);
    gpio_put(LED_PIN_DMXPORTB, 0);
    gpio_put(LED_PIN_INDICATOR, 0);

    // Init DMX pins
    gpio_init(DMXENA1_PIN);
    gpio_set_dir(DMXENA1_PIN, GPIO_OUT);
    gpio_init(DMXENA2_PIN);
    gpio_set_dir(DMXENA2_PIN, GPIO_OUT);

    //HW1.0: Turn on DMX port A output so the Receive is disabled
    gpio_put(DMXENA1_PIN, true);

    // create a queue so the irq can save the data somewhere
    queue_init(&fifo, 1, FIFO_SIZE);

    // Setup an async context and worker to perform work when needed
    if (!async_context_threadsafe_background_init_with_defaults(&async_context)) {
        panic("failed to setup context");
    }
    async_context_add_when_pending_worker(&async_context.core, &worker);

    // Set up the state machine we're going to use to receive them.
    // In real code you need to find a free pio and state machine in case pio resources are used elsewhere
    if (!init_pio(&uart_rx_program, &pio, &sm_rx, &offset_rx)) {
        panic("failed to setup pio for RX");
    }
    uart_rx_program_init(pio, sm_rx, offset_rx, PIO_RX_PIN, SERIAL_BAUD);

    if (!init_pio(&uart_tx_program, &pio, &sm_tx, &offset_tx)) {
        panic("failed to setup pio for TX");
    }
    uart_tx_program_init(pio, sm_tx, offset_tx, PIO_TX_PIN, SERIAL_BAUD);

    // Find a free irq
    static_assert(PIO0_IRQ_1 == PIO0_IRQ_0 + 1 && PIO1_IRQ_1 == PIO1_IRQ_0 + 1, "");
    pio_irq = (pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0;
    if (irq_get_exclusive_handler(pio_irq)) {
        pio_irq++;
        if (irq_get_exclusive_handler(pio_irq)) {
            panic("All IRQs are in use");
        }
    }

    // Enable interrupt
    irq_add_shared_handler(pio_irq, pio_irq_func, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); // Add a shared IRQ handler
    irq_set_enabled(pio_irq, true); // Enable the IRQ
    const uint irq_index = pio_irq - ((pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0); // Get index of the IRQ
    pio_set_irqn_source_enabled(pio, irq_index, (pio_interrupt_source)(pis_sm0_rx_fifo_not_empty + sm_rx), true); // Set pio to tell us when the FIFO is NOT empty

    // Start DMX outputs
    dmxOutput.begin(DMXOUT2_PIN, pio1);
    gpio_put(DMXENA2_PIN, true);

    // Clear buffer
    memset(dmxData, 0, sizeof(dmxData));

    multicore_launch_core1(core1_main);

    // Start timer
    struct repeating_timer timer;
    add_repeating_timer_ms(-25, repeating_timer_callback, NULL, &timer);

    // Send to console
    printf("Start!\n");

    // Send string to UART
    uart_tx_program_puts(pio, sm_tx, "Hello World!\n");

    while (1) {
        // Note that we could just sleep here as we're using "threadsafe_background" that uses a low priority interrupt
        // But if we changed to use a "polling" context that wouldn't work. The following works for both types of context.
        // When using "threadsafe_background" the poll does nothing. This loop is just preventing main from exiting!
        work_done = false;
        async_context_poll(&async_context.core);
        async_context_wait_for_work_ms(&async_context.core, 2000);

        tight_loop_contents();
    }
}
