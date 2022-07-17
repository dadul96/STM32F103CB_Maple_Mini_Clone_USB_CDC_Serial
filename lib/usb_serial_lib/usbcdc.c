#include "usbcdc.h"

static const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,		/* USB 2.0 */
	.bDeviceClass = USB_CLASS_CDC,
	.bDeviceSubClass = USB_CDC_SUBCLASS_ACM,
	.bDeviceProtocol = USB_CDC_PROTOCOL_AT,
	.bMaxPacketSize0 = CDC_DATA_PACKET_SIZE,
	.idVendor = 0x0483,		/* STM */
	.idProduct = 0x5740,	/* Virtual COM Port */
	.bcdDevice = 0x0100,	/* Device Release Number: 1.0 */
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

static const struct usb_endpoint_descriptor comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,	/* bit[7]=1b -> direction: IN; bit[3:0]=011b -> endpoint no. 3 */
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = CDC_COMM_PACKET_SIZE,
	.bInterval = 255,			/* polling interval in frames (USB2.0: 1 frame = 1ms) -> 255ms */
}};

static const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,	/* bit[7]=0b -> direction: OUT; bit[3:0]=001b -> endpoint no. 1 */
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDC_DATA_PACKET_SIZE,
	.bInterval = 1,				/* polling interval in frames (USB2.0: 1 frame = 1ms) -> 1ms */
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,	/* bit[7]=1b -> direction: IN; bit[3:0]=010b -> endpoint no. 2 */
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDC_DATA_PACKET_SIZE,
	.bInterval = 1,				/* polling interval in frames (USB2.0: 1 frame = 1ms) -> 1ms */
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
}cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength = sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 },
};

static const struct usb_interface_descriptor comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = sizeof(comm_endp)/sizeof(comm_endp[0]),
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints =  sizeof(data_endp)/sizeof(data_endp[0]),
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp,
}};

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = data_iface,
}};

static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = USB_DT_CONFIGURATION_SIZE+(USB_DT_INTERFACE_SIZE*2)+(USB_DT_ENDPOINT_SIZE*3)+(sizeof(cdcacm_functional_descriptors)),
	.bNumInterfaces = sizeof(ifaces)/sizeof(ifaces[0]),
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,	/* BUS powered Device */
	.bMaxPower = 0xFA,		/* 0xFA -> 250d -> 250*2mA=500mA */

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"STM32 Maple Mini Clone (Baite)",
	"CDC-ACM Communication",
	"1.0.0",
};

usbd_device *usb_device;
static uint8_t usbd_control_buffer[128];
static volatile uint8_t host_connected_flag = 0;

static enum usbd_request_return_codes cdcacm_control_request_callback(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)usbd_dev;

	switch (req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
		/*
		 * This Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
		char local_buf[10];
		struct usb_cdc_notification *notif = (void *)local_buf;

		/* We echo signals back to host as notification. */
		notif->bmRequestType = 0xA1;
		notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
		notif->wValue = 0;
		notif->wIndex = 0;
		notif->wLength = 2;
		local_buf[8] = req->wValue & 3;
		local_buf[9] = 0;
		// usbd_ep_write_packet(usbd_dev, 0x83, buf, 10);
		return USBD_REQ_HANDLED;
		}
	case USB_CDC_REQ_SET_LINE_CODING:
		if (*len < sizeof(struct usb_cdc_line_coding))
			return USBD_REQ_NOTSUPP;
		return USBD_REQ_HANDLED;
	}
	return USBD_REQ_NOTSUPP;
}

static void cdcacm_data_tx_callback(usbd_device *usbd_dev, uint8_t ep)
{
	(void)usbd_dev;
	(void)ep;

	if (!host_connected_flag)
	{
		host_connected_flag = 1;
	}
}

static void cdcacm_set_config_callback(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)usbd_dev;
	(void)wValue;

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, CDC_DATA_PACKET_SIZE, ((void *)0));
	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, CDC_DATA_PACKET_SIZE, cdcacm_data_tx_callback);
	usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, CDC_COMM_PACKET_SIZE, ((void *)0));

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				cdcacm_control_request_callback);
}

void usbd_cdcacm_init(void)
{
	host_connected_flag = 0;
	usb_device = usbd_init(&st_usbfs_v1_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usb_device, cdcacm_set_config_callback);
	nvic_set_priority(USB_IRQ, IRQ_PRI_USB);
	nvic_enable_irq(USB_IRQ);
}

uint8_t usbd_get_host_connected_flag(void)
{
	return host_connected_flag;
}

void USB_ISR(void)
{
	usbd_poll(usb_device);
}
