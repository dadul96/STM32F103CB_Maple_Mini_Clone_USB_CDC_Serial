#if !defined(__USB_SERIAL_H__)
#define __USB_SERIAL_H__

#include "usbcdc.h"
#include "delay.h"

#define FLOAT_PRECISION (6U)

void USB_Serial_init(void);
void USB_Serial_write(char *buff);
void USB_Serial_write_u32(uint32_t buff);
void USB_Serial_write_i32(int32_t buff);
void USB_Serial_write_float(float buff);
char *USB_Serial_read(void);
uint32_t USB_Serial_read_u32(void);
int32_t USB_Serial_read_i32(void);
float USB_Serial_read_float(void);

#endif /* __USB_SERIAL_H__ */
