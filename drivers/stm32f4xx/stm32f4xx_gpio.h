#ifndef STM32F4XX_GPIO_H
#define	STM32F4XX_GPIO_H

#ifdef _BOARD_STM32F4DISCOVERY

#ifdef _MIOSIX
#include "interfaces/gpio.h"
#else //_MIOSIX
#include "libraries/gpio.h"
#endif //_MIOSIX

#include "usb_gpio.h"

namespace mxusb {

/**
 * This class provides support for configuring USB GPIO pins, in particular
 * the pin connected to a 1.5K pullup to signal to the host that a device is
 * present. It might need to be adapted to match the circuit schematic of
 * the board used, since the GPIO that can be used for this purpose is not
 * unique.
 */
class USBgpioImpl : public USBgpio
{
private:
    /*
     * Note: change these to reflect the hardware set up.
     */
    #ifdef _MIOSIX
    typedef miosix::Gpio<GPIOA_BASE,8> sof;        //USB SOF
    typedef miosix::Gpio<GPIOA_BASE,9> vbus;        //USB vbus
    typedef miosix::Gpio<GPIOA_BASE,10> id;        //USB ID
    typedef miosix::Gpio<GPIOA_BASE,11> dm;         //USB d-
    typedef miosix::Gpio<GPIOA_BASE,12> dp;         //USB d+
    // typedef miosix::Gpio<GPIOB_BASE,14> disconnect; //USB disconnect //FIXME -> seems it doesn't exist for this board
    #else //_MIOSIX
    typedef miosix::Gpio<GPIOA_BASE,9> vbus;        //USB vbus
    typedef Gpio<GPIOA_BASE,11> dm;         //USB d-
    typedef Gpio<GPIOA_BASE,12> dp;         //USB d+
    // typedef Gpio<GPIOB_BASE,14> disconnect; //USB disconnect
    #endif //_MIOSIX

public:
    /**
     * \internal
     * Initializes USB related GPIOs
     */
    void init()
    {
        #ifdef _MIOSIX
        using namespace miosix;
        #endif //_MIOSIX

        // Enable port A of GPIO. It includes leds, vbus, dp and dm.
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

        // //alternate function
        // GPIOA->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1
        //                     | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1;
        // GPIOA->MODER &= ~(GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0 | GPIO_MODER_MODER10_0
        //                     | GPIO_MODER_MODER11_0 | GPIO_MODER_MODER12_0);

        // GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_11 | GPIO_OTYPER_OT_12);
        // GPIOA->OTYPER |= GPIO_OTYPER_OT_9 | GPIO_OTYPER_OT_10;

        // GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9 | GPIO_PUPDR_PUPDR11 | GPIO_PUPDR_PUPDR12);
        // GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;
        // GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10_1;

        // GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9 | GPIO_OSPEEDER_OSPEEDR10
        //                     | GPIO_OSPEEDER_OSPEEDR11 | GPIO_OSPEEDER_OSPEEDR12;

        // GPIOA->AFR[1] = 0x000AAAAA;

        // sof::mode(Mode::ALTERNATE);
        // sof::speed(Speed::_100MHz);

        dm::mode(Mode::ALTERNATE);
        dm::speed(Speed::_100MHz);

        dp::mode(Mode::ALTERNATE);
        dp::speed(Speed::_100MHz);


        // vbus::mode(Mode::ALTERNATE_OD);
        // vbus::speed(Speed::_100MHz);

        // id::mode(Mode::ALTERNATE_OD);
        // id::speed(Speed::_100MHz);

        // sof::alternateFunction(10);
        dm::alternateFunction(10);
        dp::alternateFunction(10);
        // vbus::alternateFunction(10);
        // id::alternateFunction(10);
    }

    /**
     * \internal
     * Enable 1.5K pullup resistor, to signal to the host that a devce has
     * been connected.
     */
    void enablePullup()
    {
        // disconnect::low();
    }

    /**
     * \internal
     * Disable the 1.5K pullup resistor, to signal to the host that the device
     * has been disconnected.
     */
    void disablePullup()
    {
        // disconnect::high();
    }
};

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY

#endif //STM32F4XX_GPIO_H