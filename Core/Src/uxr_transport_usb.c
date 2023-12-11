#include <stdbool.h>
#include "uxr_transport.h"
#include "usbd_cdc_if.h"
#include "usbd_cdc.h"
#include "cRingbuf.h"



#define USB_BUFFER_SIZE 4096
#define WRITE_TIMEOUT_MS 100U

#if 1

// Line coding: Rate: 8MBits/s; CharFormat: 1 Stop bit; Parity: None; Data: 8 bits
static uint8_t line_coding[7] = {0x00, 0x12, 0x7A, 0x00, 0x00, 0x00, 0x08};

volatile uint8_t storage_buffer[USB_BUFFER_SIZE] = {0};
volatile size_t it_head = 0;
volatile size_t it_tail = 0;
volatile bool g_write_complete = false;
bool initialized = false;
ringbuf_t usb_rx_ringbuf;

// --- USB CDC Handles ---
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;
extern USBD_HandleTypeDef hUsbDeviceFS;



// --- Reimplemented USB CDC callbacks ---
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len);

// Transmission completed callback
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
    (void) Buf;
    (void) Len;
    (void) epnum;

    g_write_complete = true;
//    osSemaphoreRelease(USBTxSemaphoreHandle);
    return USBD_OK;
}

// USB CDC requests callback
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
    switch(cmd)
    {
        case CDC_SET_LINE_CODING:
        memcpy(line_coding, pbuf, sizeof(line_coding));
        break;

        case CDC_GET_LINE_CODING:
        memcpy(pbuf, line_coding, sizeof(line_coding));
        break;

        case CDC_SEND_ENCAPSULATED_COMMAND:
        case CDC_GET_ENCAPSULATED_RESPONSE:
        case CDC_SET_COMM_FEATURE:
        case CDC_GET_COMM_FEATURE:
        case CDC_CLEAR_COMM_FEATURE:
        case CDC_SET_CONTROL_LINE_STATE:
        case CDC_SEND_BREAK:
        default:
            break;
    }

    return USBD_OK;
}

// Data received callback
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
    ringbuf_write_block(&usb_rx_ringbuf,Buf,*Len);
	USBD_CDC_ReceivePacket(&hUsbDeviceFS);

	return (USBD_OK);
}

bool my_custom_transport_open(struct uxrCustomTransport * transport){

    if (!initialized)
    {
        // USB is initialized on generated main code: Replace default callbacks here
        USBD_Interface_fops_FS.Control = CDC_Control_FS;
        USBD_Interface_fops_FS.Receive = CDC_Receive_FS;
        USBD_Interface_fops_FS.TransmitCplt = CDC_TransmitCplt_FS;
        initialized = true;
        ringbuf_init(&usb_rx_ringbuf,(void*)storage_buffer,USB_BUFFER_SIZE,RINGBUF_RULE_DISCARD);
    }

    return true;
}

bool my_custom_transport_close(struct uxrCustomTransport * transport)
{
    return true;
}

size_t my_custom_transport_write(struct uxrCustomTransport* transport, uint8_t * buf, size_t len, uint8_t * err)
{

	 /* Original Method*/
	 uint8_t ret = CDC_Transmit_FS(buf, len);
	 if (USBD_OK != ret)
	 {
	 	return 0;
	 }

     while(!g_write_complete){}

     size_t writed = g_write_complete ? len : 0;
     g_write_complete = false;

	 return writed;
}

size_t my_custom_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err)
{

    size_t readed=0;

    ringbuf_get_block(&usb_rx_ringbuf,buf,len,&readed);
    return readed;

}

#endif
