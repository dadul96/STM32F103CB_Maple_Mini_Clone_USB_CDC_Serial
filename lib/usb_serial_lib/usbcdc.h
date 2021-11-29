#if !defined(__USBCDC_H__)
#define __USBCDC_H__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/stm32/f1/nvic.h>

#define CDC_DATA_PACKET_SIZE    (32U)
#define CDC_COMM_PACKET_SIZE    (16U)
#define USB_IRQ                 NVIC_USB_LP_CAN_RX0_IRQ
#define USB_ISR                 usb_lp_can_rx0_isr
#define IRQ_PRI_USB             (255U)

extern usbd_device *usb_device;

void usbd_cdcacm_init(void);
uint8_t usbd_get_host_connected_flag(void);

#endif /* __USBCDC_H__ */
