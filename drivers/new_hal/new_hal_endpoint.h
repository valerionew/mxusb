#ifndef NEW_HAL_ENDPOINT_H
#define	NEW_HAL_ENDPOINT_H

#ifndef MXUSB_LIBRARY
#error "This is header is private, it can be used only within mxusb."
#error "If your code depends on a private header, it IS broken."
#endif //MXUSB_LIBRARY

#include "usb.h"
#include "device_state.h"
#include "endpoint_base_impl.h"
#include "drivers/new_hal/new_hal_config.h"

#ifdef MIOSIX_DEFINE_BOARD_NAME

namespace mxusb {

/**
 * \internal
 * Implemenation class for Endpoint facade class.
 * It contains all what should be accessible from within the mxgui library,
 * but not accessible by user code.
 */
class EndpointImpl : public EndpointBaseImpl
{
public:
    /**
     * Allows to access an endpoint.
     * \param epNum Endpoint number, must be in range 1<=epNum<maxNumEndpoints()
     * or behaviour is undefined.
     * \return the endpoint class that allows to read/write from that endpoint
     */
    static EndpointImpl *get(unsigned char epNum)
    {
        if(epNum==0 || epNum>=NUM_ENDPOINTS) return &invalidEp;
        return &endpoints[epNum-1];
    }

    /**
     * Allows to access an endpoint within IRQ or with interrupts disabled.
     * \param epNum Endpoint number, must be in range 1<=epNum<maxNumEndpoints()
     * or behaviour is undefined.
     * \return the endpoint class that allows to read/write from that endpoint
     */
    static EndpointImpl *IRQget(unsigned char epNum) { return get(epNum); }

    /**
     * Deconfigure all endpoints
     */
    static void IRQdeconfigureAll();

    /**
     * Configure all endpoints
     * \param desc configuration descriptor
     */
    static void IRQconfigureAll(const unsigned char *desc);

    /**
     * Deconfigure this endpoint.
     * \param epNum the number of this endpoint, used to ser data.epNumber
     */
    void IRQdeconfigure(int epNum) override;

    bool write(const unsigned char *data, int size, int& written) override;

    bool read(unsigned char *data, int& readBytes) override;

private:
    EndpointImpl(const EndpointImpl&);
    EndpointImpl& operator= (const EndpointImpl&);

    EndpointImpl() {}

    /**
     * Called by IRQconfigure() to set up an Interrupt endpoint
     * \param desc Must be the descriptor of an Interrupt endpoint
     */
    void IRQconfigureInterruptEndpoint(const unsigned char *desc) override;

    /**
     * Called by IRQconfigure() to set up an Bulk endpoint
     * \param desc Must be the descriptor of a Bulk endpoint
     */
    void IRQconfigureBulkEndpoint(const unsigned char *desc) override;

    static EndpointImpl endpoints[NUM_ENDPOINTS-1];
    static EndpointImpl invalidEp; //Invalid endpoint, always disabled
};

}

#endif //MIOSIX_DEFINE_BOARD_NAME

#endif //NEW_HAL_ENDPOINT_H