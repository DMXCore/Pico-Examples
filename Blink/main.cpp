#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "DmxOutput.h"
#include "DmxInput.h"
#include <time.h>
#include <string.h>
#include <ctype.h>


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

int led_mod_value;


bool repeating_timer_callback(struct repeating_timer *t)
{
    timer_counter++;
    led_mod_value = 10;

    if (timer_counter % (led_mod_value - 1) == 0)
    {
        ledValue = !ledValue;
        gpio_put(LED_PIN_INDICATOR, ledValue);
        gpio_put(LED_PIN_DMXPORTA, ledValue);
        gpio_put(LED_PIN_DMXPORTB, ledValue);

        gpio_put(OUTPUT_PIN1, !ledValue);
        gpio_put(OUTPUT_PIN2, !ledValue);
        gpio_put(OUTPUT_PIN3, !ledValue);
        gpio_put(OUTPUT_PIN4, !ledValue);
    }

    return true;
}

int main()
{
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

    // Start send timer
    struct repeating_timer timer;
    add_repeating_timer_ms(-25, repeating_timer_callback, NULL, &timer);

    while (1)
    {
        tight_loop_contents();
    }    
}
