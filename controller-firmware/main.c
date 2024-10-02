#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define CMD_BUFFER_SIZE 100

int main() {
    stdio_init_all();
    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(27);

    gpio_init(10);
    gpio_set_dir(10, GPIO_OUT);
    gpio_init(11);
    gpio_set_dir(11, GPIO_OUT);
    gpio_init(12);
    gpio_set_dir(12, GPIO_OUT);
    gpio_init(13);
    gpio_set_dir(13, GPIO_OUT);

    int deadzone = 100;

    char cmd_buf[CMD_BUFFER_SIZE];
    char incoming_byte;
    int cmd_len = 0;

    float target_azimuth;
    float target_elevation;
    int stop_azimuth = 0;
    int stop_elevation = 0;

    while (1) {
        // Read serial commands
        while ((incoming_byte = getchar()) != EOF) {
            if (incoming_byte == '\n' || incoming_byte == '\r' || incoming_byte == ' ' || incoming_byte == '\0') {
                // EasyComm II commands are minimum 2 chars, less than that is incomplete command
                if (cmd_len < 2)
                    break;

                cmd_buf[cmd_len] = '\0';
                
                // Commands are AZ, EL, ML, MR, MU, MD, SA, SE, VE
                if (cmd_buf[0] == 'A' && cmd_buf[1] == 'Z') {
                    stop_azimuth = 0;
                    target_azimuth = atof(cmd_buf[2]);
                }
                else if (cmd_buf[0] == 'E' && cmd_buf[1] == 'L') {
                    stop_elevation = 0;
                    target_elevation = atof(cmd_buf[2]);
                }
                else if (cmd_buf[0] == 'S') {
                    if (cmd_buf[1] == 'A') {
                        stop_azimuth = 1;
                    }
                    else if (cmd_buf[1] == 'E') {
                        stop_elevation = 1;
                    }
                }

                cmd_len = 0;
            }

            cmd_buf[cmd_len] = incoming_byte;
            cmd_len++;

            // Command is too large
            if (cmd_len >= CMD_BUFFER_SIZE) {
                break;
            }
        }

        // Rotator control loop
        

        adc_select_input(0);
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t result = adc_read();
        // printf("Y-Value - Raw value: %d, voltage: %f V\n", result, result * conversion_factor);
        if (result < 2048 - deadzone) {
            gpio_put(10, true);
        }
        else
        {
            gpio_put(10, false);
        }
        if (result > 2048 + deadzone)
        {
            gpio_put(11, true);
        }
        else
        {
            gpio_put(11, false);
        }
        adc_select_input(1);
        result = adc_read();
        if (result < 2048 - deadzone) {
            gpio_put(12, true);
        }
        else
        {
            gpio_put(12, false);
        }
        if (result > 2048 + deadzone)
        {
            gpio_put(13, true);
        }
        else
        {
            gpio_put(13, false);
        }
        // printf("X-Value - Raw value: %d, voltage: %f V\n", result, result * conversion_factor);
        // printf("\n");
        sleep_ms(10);
    }
};