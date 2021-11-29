#include "usb_serial.h"

char input_buff[CDC_DATA_PACKET_SIZE];

/* The USB_Serial_autodetect()-function is used to re-enumerate the USB device on the PC,
so that after flashing, the device does not need to be physically reconnected. Additionally, 
UBS D+ line (D23 = GPIOA, GPIO12) needs to be connected via a 1k5 Ohm pullup resistor to 
VCC (3.0 - 3.6V) to be compliant with the USB 2.0 full-speed electrical specification. */
static void USB_Serial_autodetect(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);

    /* set GPIOA, GPIO12 (USB D+ line) to output push-pull low (0V) and wait for 250ms: */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
    gpio_clear(GPIOA, GPIO12);
    delay_ms(250);

    /* set to alternate function (USB): */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO12); /* USB D+ line */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO11); /* USB D- line */
    delay_ms(250);
}

/* The USB_Serial_wait_for_host()-function is used to wait until the host starts a serial monitor. */
static void USB_Serial_wait_for_host(void)
{
    while (!usbd_get_host_connected_flag())
    {
        usbd_ep_write_packet(usb_device, 0x82, (char)('\0'), 1);
        __asm__("nop");
    }
    delay_ms(250);
}

void USB_Serial_init(void)
{
    USB_Serial_autodetect();
    usbd_cdcacm_init();
    USB_Serial_wait_for_host();
}

void USB_Serial_write(char *buff)
{
    uint8_t len = 0;
    uint16_t sent_len = 0;

    while (*buff != '\0')
    {
        len++;
        buff++;
        if (len == CDC_DATA_PACKET_SIZE - 1)
        {
            do
            {
                sent_len += usbd_ep_write_packet(usb_device, 0x82, (void *)buff - len, len);
            } while (sent_len != len);
            len = 0;
            sent_len = 0;
        }
    }

    if (len > 0)
    {
        do
        {
            sent_len += usbd_ep_write_packet(usb_device, 0x82, (void *)buff - len, len);
        } while (sent_len != len);
    }
}

void USB_Serial_write_u32(uint32_t number)
{
    char buff[8 * sizeof(number) + 2]; /* 8-bit (char) for each digit + '\n' + '\0' */
    char *str = &buff[sizeof(buff) - 1];
    *str = '\0';
    *--str = '\n';

    do
    {
        char c = number % 10;
        number /= 10;
        *--str = c + '0';
    } while (number);

    USB_Serial_write(str);
}

void USB_Serial_write_i32(int32_t number)
{
    uint8_t neg_sign = 0;
    char buff[8 * sizeof(number) + 3]; /* 8-bit (char) for each digit + '\n' + '\0' + sign symbol */
    char *str = &buff[sizeof(buff) - 1];
    *str = '\0';
    *--str = '\n';

    if (number < 0)
    {
        number = -number;
        neg_sign = 1;
    }

    do
    {
        char c = number % 10;
        number /= 10;
        *--str = c + '0';
    } while (number);

    if (neg_sign == 1)
    {
        *--str = '-';
    }

    USB_Serial_write(str);
}

void USB_Serial_write_float(float number)
{
    uint8_t neg_sign = 0;
    uint32_t int_part = 0;
    float digits_part = 0.0;
    char buff[8 * sizeof(number) + 4]; /* 8-bit (char) for each digit + '\n' + '\0' + sign symbol + '.' */
    char *str = &buff[sizeof(buff) - 1];
    *str = '\0';
    *--str = '\n';

    if (number < 0.0)
    {
        number = -number;
        neg_sign = 1;
    }

    double rounding = 0.5;
    for (uint8_t i = 0; i < FLOAT_PRECISION; i++)
    {
        rounding /= 10.0;
    }
    number += rounding;

    int_part = (uint32_t)number;
    digits_part = number - (float)int_part;

    str = str - FLOAT_PRECISION;
    for (uint8_t i = 0; i < FLOAT_PRECISION; i++)
    {
        digits_part *= 10.0;
        uint32_t remainder = digits_part;
        char c = remainder % 10;
        remainder /= 10;
        *str++ = c + '0';
        digits_part -= remainder;
    }
    str = str - FLOAT_PRECISION;

    *--str = '.';

    do
    {
        char c = int_part % 10;
        int_part /= 10;
        *--str = c + '0';
    } while (int_part);

    if (neg_sign == 1)
    {
        *--str = '-';
    }

    USB_Serial_write(str);
}

char* USB_Serial_read()
{
    int len = 0;

    do
    {
        len = usbd_ep_read_packet(usb_device, 0x01, input_buff, CDC_DATA_PACKET_SIZE);
    } while (len == 0);

    return input_buff;
}

uint32_t USB_Serial_read_u32()
{
    uint32_t decimal_factor = 1;
    uint32_t number = 0;
    uint8_t len = 0;
    char *input;

    input = USB_Serial_read();

    while ((*input >= '0') && (*input <= '9'))
    {
        input++;
        len++;
    }

    for (uint8_t i = 0; i < len; i++)
    {
        number += (*--input - '0') * decimal_factor;
        decimal_factor *= 10;
    }

    return number;
}

int32_t USB_Serial_read_i32()
{
    int32_t decimal_factor = 1;
    int32_t number = 0;
    uint8_t len = 0;
    uint8_t neg_sign = 0;
    char *input;

    input = USB_Serial_read();

    if (*input == '-')
    {
        input++;
        neg_sign = 1;
    }

    while ((*input >= '0') && (*input <= '9'))
    {
        input++;
        len++;
    }

    for (uint8_t i = 0; i < len; i++)
    {
        number += (*--input - '0') * decimal_factor;
        decimal_factor *= 10;
    }

    if (neg_sign == 1)
    {
        number *= -1;
    }

    return number;
}

float USB_Serial_read_float()
{
    float number = 0.0;
    float remainder_decimal_factor = 1.0;
    int32_t int_decimal_factor = 1;
    int32_t int_part = 0;
    uint8_t len_int_part = 0;
    uint8_t len_remainder_part = 0;
    uint8_t neg_sign = 0;
    char *input;

    input = USB_Serial_read();

    if (*input == '-')
    {
        input++;
        neg_sign = 1;
    }

    while ((*input >= '0') && (*input <= '9'))
    {
        input++;
        len_int_part++;
    }

    if (*input == '.')
    {
        input++;

        while ((*input >= '0') && (*input <= '9'))
        {
            input++;
            len_remainder_part++;
            remainder_decimal_factor /= 10.0;
        }

        for (uint8_t i = 0; i < len_remainder_part; i++)
        {
            number += (float)(*--input - '0') * remainder_decimal_factor;
            remainder_decimal_factor *= 10.0;
        }

        input--;
    }

    for (uint8_t i = 0; i < len_int_part; i++)
    {
        int_part += (*--input - '0') * int_decimal_factor;
        int_decimal_factor *= 10;
    }

    number += (float)int_part;

    if (neg_sign == 1)
    {
        number *= -1.0;
    }

    return number;
}
