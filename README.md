# STM32F103CB_Maple_Mini_Clone_USB_CDC_Serial
Boilerplate for USB CDC VCOM serial communication with STM32F103CB based Maple Mini Clone.

### Folder Structure
```
--STM32F103CB_Maple_Mini_Clone_USB_CDC_Serial
  |
  |--lib
  |  |
  |  |--usb_serial_lib
  |     |
  |     |- delay.c
  |     |- delay.h
  |     |- usb_serial.c
  |     |- usb_serial.h
  |     |- usbcdc.c
  |     |- usbcdc.h
  |
  |--src
  |   |
  |   |- main.c
  |
  |- platformio.ini
```

### Code Dependency
```
            [main.c]
               |
               |
         [usb_serial.*]
               |
               |
      +------------------+
      |                  |
      |                  |
  [usbcdc.*]         [delay.*]
```

### Built With
* [PlatformIO IDE](https://platformio.org/platformio-ide) - IDE used for programming
* [libopencm3](https://github.com/libopencm3/libopencm3) - Open-Source lowlevel hardware library for ARM Cortex-M microcontrollers

### Author
**Daniel Duller** - [dadul96](https://github.com/dadul96)

### License
This project is licensed under the GPL-3.0 License - see the [LICENSE](LICENSE) file for details

### References/Documentation/Inspiration
* [libopencm3 USB CDCACM example](https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f1/stm32-maple/usb_cdcacm/cdcacm.c)
* [satoshinm/pill_serial](https://github.com/satoshinm/pill_serial)
* [Missing USB D+ pullup workaround](https://hackaday.io/project/19799-lifetime-fails-project/log/160352-missing-external-usb-d-pullup-on-stm32-powered-pcb/discussion-163951)
* [LeafLabs Maple Mini Documentation](http://docs.leaflabs.com/static.leaflabs.com/pub/leaflabs/maple-docs/0.0.12/hardware/maple-mini.html)
* [stm32-base.org: Baite Maple Mini Clone Documentation](https://stm32-base.org/boards/STM32F103C8T6-Baite-Maple-Mini-Clone)
* [beyondlogic.org: USB Descriptors](https://beyondlogic.org/usbnutshell/usb5.shtml)
* [ST AN4879 USB hardware and PCB guidelines](https://www.st.com/content/ccc/resource/technical/document/application_note/group0/0b/10/63/76/87/7a/47/4b/DM00296349/files/DM00296349.pdf/jcr:content/translations/en.DM00296349.pdf)
