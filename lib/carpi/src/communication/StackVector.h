#ifndef VECTOR_H
#define VECTOR_H

/*
    Attention! Use with caution!

    This is only called Vector for compatibility reasons. The storage is fixed and only the interface is similar.
*/

namespace stack {

template <class T>
using iterator = T*;

template<class T, unsigned int maxSize, typename counter_type_t = unsigned int>
class vector : private NonCopyable<vector<T, maxSize, counter_type_t>> {
    public:
        vector() {}

        // Destructor
        ~vector() {
            clear();
        }

        // Element access
        T& at(counter_type_t pos) {
            if (pos >= _currentElementCount)
                return *_data;
            
            return _data[pos];
        }

        T& operator[](counter_type_t pos) {
            return at(pos);
        }

        T& front() {
            return *_data;
        }

        T& back() {
            if (empty()) {
                return *_data;
            }

            return begin[_currentElementCount - 1];
        }

        T* data() {
            if (empty()) {
                return nullptr;
            }
            
            return _data;
        }

        // Iterators
        iterator<T> begin() {
            return _data;
        }

        iterator<T> end() {
            return _data + _currentElementCount;
        }

        // Capacity
        bool empty() {
            return _currentElementCount == 0;
        }

        counter_type_t size() {
            return _currentElementCount;
        }
        
        counter_type_t capacity() {
            return maxSize;
        }

        // Modifiers
        void clear() {
            _currentElementCount = 0;
        }

        iterator<T> erase(iterator<T> pos) {
            if (pos >= end()) return pos;

            counter_type_t index = pos - _data;

            pos->~T();

            for (counter_type_t i = index; i < _currentElementCount - 1; ++i) { // -1 because 1 element will be deleted and otherwise it will fail by out of bound by 1
                _data[i] = _data[i+1];
            }

            --_currentElementCount;

            return pos;
        }

        void push_back(T& value) {
            // Check if there is still free memory
            if (maxSize == _currentElementCount) {
                #ifdef MESSAGE_REPORT
                    printf("[stack::vector]@push_back: Vector full but still used for storing more\n");
                #endif
                return;
            }

            // Copy element
            _data[_currentElementCount++] = value;
        }

        void pop_back() {
            --_currentElementCount;
        }
    
    private:
        counter_type_t _currentElementCount = 0;
        T _data[maxSize];
};

}; // namespace std

#endif // VECTOR_H