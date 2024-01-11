#include <stdio.h>
#include "pico/stdlib.h"


const uint LED_PIN_INDICATOR = 25;

volatile char receivedChar;

void received_char_callback(void* param)
{
    gpio_put(LED_PIN_INDICATOR, !gpio_get(LED_PIN_INDICATOR));

    int key = getchar_timeout_us(0); // get any pending key press but don't wait
    if (key > 0)
    {
        // We never get here
        receivedChar = (char)key;
    }
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

    stdio_set_chars_available_callback(received_char_callback, NULL);

    // Send to console
    printf("Start!\n");

    while (1) {
        if (receivedChar != 0)
            printf("Last char: %c\n", receivedChar);

        sleep_ms(500);
    }
}
