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


DmxOutput dmxOutput1;
DmxOutput dmxOutput2;
DmxInput dmxInput1;
DmxInput dmxInput2;
#define UNIVERSE_LENGTH 512
#define INPUT_TIMEOUT_MS 3000
#define OUTPUT_TIMEOUT_MS 3000

uint8_t dmxDataA1[UNIVERSE_LENGTH + 1];

uint8_t dmxInputBuffer1[DMXINPUT_BUFFER_SIZE(512)];
uint8_t dmxInputBuffer2[DMXINPUT_BUFFER_SIZE(512)];

uint32_t timer_counter;
bool ledValue;
bool ledAValue;
bool ledBValue;

int led_mod_value;
int ledA_mod_value;
int ledB_mod_value;

clock_t lastOutputFrame1;
clock_t lastOutputFrame2;
clock_t lastInputFrame1;
clock_t lastInputFrame2;

bool outputActive1;
bool outputActive2;
bool inputActive1;
bool inputActive2;


bool repeating_timer_callback(struct repeating_timer *t)
{
    // Make sure we're not in the middle of sending
    while (dmxOutput1.busy());

    // Populate buffer from input 1
    uint8_t value = gpio_get(INPUT_PIN1) ? 0 : 255;

    memset(&dmxDataA1[1], value, 512);

    // Send DMX
    dmxOutput1.write(dmxDataA1, UNIVERSE_LENGTH + 1);

    ledA_mod_value = 2;

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

    printf("Init!");

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

    // Start DMX outputs
    dmxOutput1.begin(DMXOUT1_PIN, pio1);
    dmxOutput2.begin(DMXOUT2_PIN, pio1);

    // Start DMX inputs
    dmxInput1.begin(DMXIN1_PIN, 512);
    dmxInput2.begin(DMXIN2_PIN, 512);

    // Clear buffer
    memset(dmxDataA1, 0, sizeof(dmxDataA1));

    // Start send timer
    struct repeating_timer timer;
    add_repeating_timer_ms(-25, repeating_timer_callback, NULL, &timer);

    // Set initial configuration
    inputActive1 = false;
    outputActive1 = true;
    inputActive2 = false;
    outputActive2 = false;

    gpio_put(DMXENA1_PIN, outputActive1);
    gpio_put(DMXENA2_PIN, outputActive2);

    while (1)
    {
        tight_loop_contents();
    }    
}
