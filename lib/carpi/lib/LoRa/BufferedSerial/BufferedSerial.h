#ifndef BUFFEREDSERIAL_H
#define BUFFEREDSERIAL_H
 
#include "mbed.h"
#include "MyBuffer.h"
#include <stdarg.h>

#if (MBED_MAJOR_VERSION == 5) && (MBED_MINOR_VERSION >= 2)
#elif (MBED_MAJOR_VERSION == 2) && (MBED_PATCH_VERSION > 130)
#else
#error "BufferedSerial version 13 and newer requires use of Mbed OS 5.2.0 and newer or Mbed 2 version 130 and newer. Use BufferedSerial version 12 and older or upgrade the Mbed version."
#endif

/** A serial port (UART) for communication with other serial devices
 *
 * Can be used for Full Duplex communication, or Simplex by specifying
 * one pin as NC (Not Connected)
 *
 * Example:
 * @code
 *  #include "mbed.h"
 *  #include "BufferedSerial.h"
 *
 *  BufferedSerial pc(USBTX, USBRX);
 *
 *  int main()
 *  { 
 *      while(1)
 *      {
 *          Timer s;
 *        
 *          s.start();
 *          pc.printf("Hello World - buffered\n");
 *          int buffered_time = s.read_us();
 *          wait(0.1f); // give time for the buffer to empty
 *        
 *          s.reset();
 *          printf("Hello World - blocking\n");
 *          int polled_time = s.read_us();
 *          s.stop();
 *          wait(0.1f); // give time for the buffer to empty
 *        
 *          pc.printf("printf buffered took %d us\n", buffered_time);
 *          pc.printf("printf blocking took %d us\n", polled_time);
 *          wait(0.5f);
 *      }
 *  }
 * @endcode
 */

/**
 *  @class BufferedSerial
 *  @brief Software buffers and interrupt driven tx and rx for Serial
 */  
class BufferedSerial : public RawSerial 
{
private:
    MyBuffer <char> _rxbuf;
    MyBuffer <char> _txbuf;
    uint32_t      _buf_size;
    uint32_t      _tx_multiple;
 
    void rxIrq(void)
    {
        // read from the peripheral and make sure something is available
        if(serial_readable(&_serial)) {
            _rxbuf = serial_getc(&_serial); // if so load them into a buffer
        }

        return;
    }

    void txIrq(void)
    {
        // see if there is room in the hardware fifo and if something is in the software fifo
        while(serial_writable(&_serial)) {
            if(_txbuf.available()) {
                serial_putc(&_serial, (int)_txbuf.get());
            } else {
                // disable the TX interrupt when there is nothing left to send
                RawSerial::attach(NULL, RawSerial::TxIrq);
                break;
            }
        }

        return;
    }

    void prime(void)
    {
        // if already busy then the irq will pick this up
        if(serial_writable(&_serial)) {
            RawSerial::attach(NULL, RawSerial::TxIrq);    // make sure not to cause contention in the irq
            txIrq();                // only write to hardware in one place
            RawSerial::attach(callback(this, &txIrq), RawSerial::TxIrq);
        }

        return;
    }
    
public:
    /** Create a BufferedSerial port, connected to the specified transmit and receive pins
     *  @param tx Transmit pin
     *  @param rx Receive pin
     *  @param buf_size printf() buffer size
     *  @param tx_multiple amount of max printf() present in the internal ring buffer at one time
     *  @param name optional name
     *  @note Either tx or rx may be specified as NC if unused
     */
    BufferedSerial(PinName tx, PinName rx, uint32_t buf_size = 256, uint32_t tx_multiple = 4,const char* name=NULL);
    
    /** Destroy a BufferedSerial port
     */
    virtual ~BufferedSerial(void);
    
    /** Check on how many bytes are in the rx buffer
     *  @return 1 if something exists, 0 otherwise
     */
    virtual int readable(void);
    
    /** Check to see if the tx buffer has room
     *  @return 1 always has room and can overwrite previous content if too small / slow
     */
    virtual int writeable(void);
    
    /** Get a single byte from the BufferedSerial Port.
     *  Should check readable() before calling this.
     *  @return A byte that came in on the Serial Port
     */
    virtual int getc(void);
    
    /** Write a single byte to the BufferedSerial Port.
     *  @param c The byte to write to the Serial Port
     *  @return The byte that was written to the Serial Port Buffer
     */
    virtual int putc(int c);
    
    /** Write a string to the BufferedSerial Port. Must be NULL terminated
     *  @param s The string to write to the Serial Port
     *  @return The number of bytes written to the Serial Port Buffer
     */
    virtual int puts(const char *s);
    
    /** Write a formatted string to the BufferedSerial Port.
     *  @param format The string + format specifiers to write to the Serial Port
     *  @return The number of bytes written to the Serial Port Buffer
     */
    virtual int printf(const char* format, ...);
    
    /** Write data to the Buffered Serial Port
     *  @param s A pointer to data to send
     *  @param length The amount of data being pointed to
     *  @return The number of bytes written to the Serial Port Buffer
     */
    virtual ssize_t write(const void *s, std::size_t length);

    BufferedSerial(PinName tx, PinName rx, uint32_t buf_size, uint32_t tx_multiple, const char* name)
        : RawSerial(tx, rx) , _rxbuf(buf_size), _txbuf((uint32_t)(tx_multiple*buf_size))
    {
        RawSerial::attach(callback(this, &rxIrq), Serial::RxIrq);
        this->_buf_size = buf_size;
        this->_tx_multiple = tx_multiple;   
        return;
    }

    ~BufferedSerial(void)
    {
        RawSerial::attach(NULL, RawSerial::RxIrq);
        RawSerial::attach(NULL, RawSerial::TxIrq);

        return;
    }

    int readable(void)
    {
        return _rxbuf.available();  // note: look if things are in the buffer
    }

    int writeable(void)
    {
        return 1;   // buffer allows overwriting by design, always true
    }

    int getc(void)
    {
        return _rxbuf;
    }

    int putc(int c)
    {
        _txbuf = (char)c;
        prime();

        return c;
    }

    int puts(const char *s)
    {
        if (s != NULL) {
            const char* ptr = s;
        
            while(*(ptr) != 0) {
                _txbuf = *(ptr++);
            }
            _txbuf = '\n';  // done per puts definition
            prime();
        
            return (ptr - s) + 1;
        }
        return 0;
    }

    int printf(const char* format, ...)
    {
        char buffer[this->_buf_size];
        memset(buffer,0,this->_buf_size);
        int r = 0;

        va_list arg;
        va_start(arg, format);
        r = vsprintf(buffer, format, arg);
        // this may not hit the heap but should alert the user anyways
        if(r > this->_buf_size) {
            error("%s %d buffer overwrite (max_buf_size: %d exceeded: %d)!\r\n", __FILE__, __LINE__,this->_buf_size,r);
            va_end(arg);
            return 0;
        }
        va_end(arg);
        r = write(buffer, r);

        return r;
    }

    ssize_t write(const void *s, size_t length)
    {
        if (s != NULL && length > 0) {
            const char* ptr = (const char*)s;
            const char* end = ptr + length;
        
            while (ptr != end) {
                _txbuf = *(ptr++);
            }
            prime();
        
            return ptr - (const char*)s;
        }
        return 0;
    }

};

#endif
