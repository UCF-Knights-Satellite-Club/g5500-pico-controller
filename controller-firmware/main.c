#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define CMD_BUFFER_SIZE 20

#define ADC_AZIMUTH 0
#define ADC_ELEVATION 1
#define ROTATE_LEFT 10
#define ROTATE_RIGHT 11
#define ROTATE_UP 12
#define ROTATE_DOWN 13

int get_command(char* buf);
void parse_command(const char* buf, const int buf_len);
void move_towards_target();
float get_value(int pin);
void clear_buffer(char* buf);
void update_current_position();

struct data {
    float target_azimuth;
    float target_elevation;
    float current_azimuth;
    float current_elevation;
};

struct data myData;

int main() {
    myData.current_azimuth = 180;
    myData.current_elevation = 45;
    stdio_init_all();
    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(27);

    gpio_init(ROTATE_LEFT);
    gpio_set_dir(ROTATE_LEFT, GPIO_OUT);
    gpio_init(ROTATE_RIGHT);
    gpio_set_dir(ROTATE_RIGHT, GPIO_OUT);
    gpio_init(ROTATE_UP);
    gpio_set_dir(ROTATE_UP, GPIO_OUT);
    gpio_init(ROTATE_DOWN);
    gpio_set_dir(ROTATE_DOWN, GPIO_OUT);

    char cmd_buf[CMD_BUFFER_SIZE];
    char incoming_byte;
    int cmd_len = 0;
    while (1) {
        // Read serial commands
        cmd_len = get_command(cmd_buf);
        parse_command(cmd_buf, cmd_len);

        // Rotator control loop
        move_towards_target();
        update_current_position();
        clear_buffer(cmd_buf);
    }
}

void move_towards_target() {
    if (myData.current_azimuth < myData.target_azimuth) {
        gpio_put(ROTATE_RIGHT, false);
        gpio_put(ROTATE_LEFT, true);
    }
    else if (myData.current_azimuth > myData.target_azimuth) {
        gpio_put(ROTATE_LEFT, false);
        gpio_put(ROTATE_RIGHT, true);
    }
    else {
        gpio_put(ROTATE_LEFT, false);
        gpio_put(ROTATE_RIGHT, false);
    }

    if (myData.current_elevation < myData.target_elevation) {
        gpio_put(ROTATE_DOWN, false);
        gpio_put(ROTATE_UP, true);
    }
    else if (myData.current_elevation > myData.target_elevation) {
        gpio_put(ROTATE_UP, false);
        gpio_put(ROTATE_DOWN, true);
    }
    else {
        gpio_put(ROTATE_UP, false);
        gpio_put(ROTATE_DOWN, false);
    }
}

int get_command(char* buf) {
    int len = 0;
    char incoming_byte;
    while ((incoming_byte = getchar()) != EOF)
    {
        if (incoming_byte == '\n' || incoming_byte == '\r' || incoming_byte == ' ' || incoming_byte == '\0')
        {
            // EasyComm II commands are minimum 2 chars, less than that is incomplete command
            if (len < 2)
                break;

            buf[len] = '\0';
            break;
        }
        buf[len] = incoming_byte;
        len++;

        // Command is too large
        if (len >= CMD_BUFFER_SIZE)
        {
            break;
        }
    }

    return len;
}

void parse_command(const char* buf, const int buf_len) {
    // Commands are AZ, EL, ML, MR, MU, MD, SA, SE, VE
    if (buf[0] == 'A' && buf[1] == 'Z')
    {
        if (buf_len > 2) {
            myData.target_azimuth = atof(buf + 2);
        }
        else {
            printf("%.1f\n", myData.current_azimuth);
            fflush(stdout);
        }
    }
    else if (buf[0] == 'E' && buf[1] == 'L')
    {
        if (buf_len > 2) {
            myData.target_elevation = atof(buf + 2);
        }
        else {
            printf("%.1f\n", myData.current_elevation);
            fflush(stdout);
        }
    }
    else if (buf[0] == 'S')
    {
        if (buf[1] == 'A')
        {
            gpio_put(ROTATE_LEFT, false);
            gpio_put(ROTATE_RIGHT, false);
        }
        else if (buf[1] == 'E')
        {
            gpio_put(ROTATE_UP, false);
            gpio_put(ROTATE_DOWN, false);
        }
    }
    return;
}

void update_current_position() {
    myData.current_azimuth = get_value(ADC_AZIMUTH);
    myData.current_elevation = get_value(ADC_ELEVATION);
}

float get_value(int pin) {
    adc_select_input(pin);
    // uint16_t result = adc_read();
    // return result;
    return adc_read();
}

void clear_buffer(char* buf) {
    for (int i = 0; i < CMD_BUFFER_SIZE; i++)
    {
        buf[i] = '\0';
    }
}