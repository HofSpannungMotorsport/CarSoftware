
#ifndef MYBUFFER_H
#define MYBUFFER_H

#include <stdint.h>
#include <string.h>

/** A templated software ring buffer
 *
 * Example:
 * @code
 *  #include "mbed.h"
 *  #include "MyBuffer.h"
 *
 *  MyBuffer <char> buf;
 *
 *  int main()
 *  {
 *      buf = 'a';
 *      buf.put('b');
 *      char *head = buf.head();
 *      puts(head);
 *
 *      char whats_in_there[2] = {0};
 *      int pos = 0;
 *
 *      while(buf.available())
 *      {   
 *          whats_in_there[pos++] = buf;
 *      }
 *      printf("%c %c\n", whats_in_there[0], whats_in_there[1]);
 *      buf.clear();
 *      error("done\n\n\n");
 *  }
 * @endcode
 */

template <typename T>
class MyBuffer
{
private:
    T   *_buf;
    volatile uint32_t   _wloc;
    volatile uint32_t   _rloc;
    uint32_t            _size;

public:
    /** Create a Buffer and allocate memory for it
     *  @param size The size of the buffer
     */
    MyBuffer(uint32_t size = 0x100);
    
    /** Get the size of the ring buffer
     * @return the size of the ring buffer
     */
    uint32_t getSize();
    
    /** Destry a Buffer and release it's allocated memory
     */
    ~MyBuffer();
    
    /** Add a data element into the buffer
     *  @param data Something to add to the buffer
     */
    void put(T data);
    
    /** Remove a data element from the buffer
     *  @return Pull the oldest element from the buffer
     */
    T get(void);
    
    /** Get the address to the head of the buffer
     *  @return The address of element 0 in the buffer
     */
    T *head(void);
    
    /** Reset the buffer to 0. Useful if using head() to parse packeted data
     */
    void clear(void);
    
    /** Determine if anything is readable in the buffer
     *  @return 1 if something can be read, 0 otherwise
     */
    uint32_t available(void);
    
    /** Overloaded operator for writing to the buffer
     *  @param data Something to put in the buffer
     *  @return
     */
    MyBuffer &operator= (T data)
    {
        put(data);
        return *this;
    }
    
    /** Overloaded operator for reading from the buffer
     *  @return Pull the oldest element from the buffer 
     */  
    operator int(void)
    {
        return get();
    }
    
    uint32_t peek(char c);

    template <class T>
    MyBuffer<T>::MyBuffer(uint32_t size)
    {
        _buf = new T [size];
        _size = size;
        clear();
        
        return;
    }

    template <class T>
    MyBuffer<T>::~MyBuffer()
    {
        delete [] _buf;
        
        return;
    }

    template <class T>
    uint32_t MyBuffer<T>::getSize() 
    { 
        return this->_size; 
    }

    template <class T>
    void MyBuffer<T>::clear(void)
    {
        _wloc = 0;
        _rloc = 0;
        memset(_buf, 0, _size);
        
        return;
    }

    template <class T>
    uint32_t MyBuffer<T>::peek(char c)
    {
        return 1;
    }

    // make the linker aware of some possible types
    template class MyBuffer<uint8_t>;
    template class MyBuffer<int8_t>;
    template class MyBuffer<uint16_t>;
    template class MyBuffer<int16_t>;
    template class MyBuffer<uint32_t>;
    template class MyBuffer<int32_t>;
    template class MyBuffer<uint64_t>;
    template class MyBuffer<int64_t>;
    template class MyBuffer<char>;
    template class MyBuffer<wchar_t>;

};

template <class T>
inline void MyBuffer<T>::put(T data)
{
    _buf[_wloc++] = data;
    _wloc %= (_size-1);
    
    return;
}

template <class T>
inline T MyBuffer<T>::get(void)
{
    T data_pos = _buf[_rloc++];
    _rloc %= (_size-1);
    
    return data_pos;
}

template <class T>
inline T *MyBuffer<T>::head(void)
{
    T *data_pos = &_buf[0];
    
    return data_pos;
}

template <class T>
inline uint32_t MyBuffer<T>::available(void)
{
    return (_wloc == _rloc) ? 0 : 1;
}

#endif

