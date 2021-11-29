#include "usb_serial.h"

int main(void)
{
	/* enable LED (GPIOB, GPIO1) to show the entry of main: */
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
	gpio_set(GPIOB, GPIO1);

	/* initialize USB serial: */
	USB_Serial_init();

	/* disable LED to show that USB serial initialized successfully: */
	gpio_clear(GPIOB, GPIO1);

	/* test functions of USB serial write: */
	USB_Serial_write("Hello world!\n");

	char test[] = {"\nThis is a test to check if splitting into several tx-packages is working properly!\n"};
	USB_Serial_write(test);

	USB_Serial_write("\nTest of USB_Serial_write_u32:\n");
	USB_Serial_write_u32((uint32_t)123456);
	USB_Serial_write_u32(123456U);
	USB_Serial_write_u32(10U);
	USB_Serial_write_u32(1);
	USB_Serial_write_u32(-1);
	USB_Serial_write_u32((uint8_t)10);
	uint8_t test2 = 7;
	USB_Serial_write_u32(test2);
	USB_Serial_write_u32(7.77);

	USB_Serial_write("\nTest of USB_Serial_write_i32:\n");
	USB_Serial_write_i32((int32_t)123456);
	USB_Serial_write_i32((uint32_t)123456);
	USB_Serial_write_i32((int32_t)-123456);
	USB_Serial_write_i32(-123456);
	USB_Serial_write_i32(10U);
	USB_Serial_write_i32(1);
	USB_Serial_write_i32(-1);
	USB_Serial_write_i32((int8_t)-10);
	int8_t test3 = -7;
	USB_Serial_write_i32(test3);
	USB_Serial_write_i32(7.77);

	USB_Serial_write("\nTest of USB_Serial_write_float:\n");
	USB_Serial_write_float((float)1.1111111111);
	USB_Serial_write_float((float)-1.1111111111);
	USB_Serial_write_float(2.1111111111);
	USB_Serial_write_float(-2.1111111111);
	USB_Serial_write_float((int32_t)-123456);
	USB_Serial_write_float((uint32_t)123456);
	float test4 = -7.1111111111;
	USB_Serial_write_float(test4);
	USB_Serial_write_float((float)1.123456789);
	USB_Serial_write_float((float)-1.987654321);

	/* test functions of USB serial read: */
	USB_Serial_write("\nEnter your name: ");
	char *input1;
	input1 = USB_Serial_read();
	USB_Serial_write("\nYou have entered: ");
	USB_Serial_write(input1);

	USB_Serial_write("\nEnter a unsigned number: ");
	uint32_t input_number_1 = USB_Serial_read_u32();
	USB_Serial_write("\nYou have entered: ");
	USB_Serial_write_u32(input_number_1);

	USB_Serial_write("\nEnter a signed number: ");
	int32_t input_number_2 = USB_Serial_read_i32();
	USB_Serial_write("\nYou have entered: ");
	USB_Serial_write_i32(input_number_2);

	USB_Serial_write("\nEnter a float number: ");
	float input_number_3 = USB_Serial_read_float();
	USB_Serial_write("\nYou have entered: ");
	USB_Serial_write_float(input_number_3);

	/* blink LED to indicate that everything finished: */
	USB_Serial_write("\nTest finished -> start blinking...");
	while (1)
	{
		gpio_set(GPIOB, GPIO1);
		delay_ms(300);
		gpio_clear(GPIOB, GPIO1);
		delay_ms(300);
	}

	return 0;
}
