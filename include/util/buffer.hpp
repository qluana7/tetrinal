#pragma once

#include <array>

/**
 * @brief Fixed-size circular buffer for storing history-like data.
 *
 * Implements a buffer with a constant capacity. When new data is added and the buffer is full,
 * the oldest entry is automatically removed. The buffer uses three indices:
 * - _M_start: points to the oldest element.
 * - _M_last: points to the most recently added element.
 * - _M_current: points to the currently accessed element.
 *
 * The current index can move forward and backward, but always stays within [start, last].
 * Adding a new value updates last, and if the buffer is full, also advances start.
 * Users can traverse the buffer using the current index.
 */
template <typename T, std::size_t N>
class buffer {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using const_reference = const T&;

    class iterator {
    public:
        using difference_type = buffer::difference_type;
        using value_type = buffer::value_type;
        using reference = buffer::reference;
        using pointer = buffer::pointer;
        using iterator_category = std::random_access_iterator_tag;

        iterator() = default;
        iterator(const iterator&) = default;

        iterator(buffer* buf, std::size_t index)
            : _M_buf(buf), _M_index(index) {}

        ~iterator() = default;

    private:
        buffer* _M_buf = nullptr;
        std::size_t _M_index = 0;

    public:
        iterator& operator=(const iterator&) = default;

        bool operator==(const iterator& other) const
        { return _M_index == other._M_index; }
        bool operator!=(const iterator& other) const
        { return !(*this == other); }
        bool operator<(const iterator& other) const
        { return _M_index < other._M_index; }
        bool operator<=(const iterator& other) const
        { return _M_index <= other._M_index; }
        bool operator>(const iterator& other) const
        { return _M_index > other._M_index; }
        bool operator>=(const iterator& other) const
        { return _M_index >= other._M_index; }

        iterator& operator++() { _M_index++; return *this; }
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        iterator& operator--() { _M_index--; return *this; }
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        iterator& operator+=(difference_type n) { _M_index += n; return *this; }
        iterator operator+(difference_type n) const {
            iterator tmp = *this;
            tmp += n;
            return tmp;
        }        
        friend iterator operator+(difference_type n, const iterator& it)
        { return it + n; }

        iterator& operator-=(difference_type n) { _M_index -= n; return *this; }
        iterator operator-(difference_type n) const {
            iterator tmp = *this;
            tmp -= n;
            return tmp;
        }
        difference_type operator-(const iterator& other) const
        { return _M_index - other._M_index; }

        reference operator*()
        { return _M_buf->_M_data[_M_index % _M_buf->capacity()]; }
        pointer operator->()
        { return &_M_buf->_M_data[_M_index % _M_buf->capacity()]; }
        reference operator[](difference_type n)
        { return _M_buf->_M_data[(_M_index + n) % _M_buf->capacity()]; }
    };

    class const_iterator {
    public:
        using difference_type = buffer::difference_type;
        using value_type = buffer::value_type;
        using reference = const buffer::reference;
        using pointer = const buffer::pointer;
        using iterator_category = std::random_access_iterator_tag;

        const_iterator() = default;
        const_iterator(const const_iterator&) = default;
        const_iterator(const iterator& __it) {
            _M_buf = __it._M_buf;
            _M_index = __it._M_index;
        }

        const_iterator(const buffer* buf, std::size_t index)
            : _M_buf(buf), _M_index(index) {}

        ~const_iterator() = default;

    private:
        const buffer* _M_buf = nullptr;
        std::size_t _M_index = 0;

    public:
        const_iterator& operator=(const const_iterator&) = default;

        bool operator==(const const_iterator& other) const
        { return _M_index == other._M_index; }
        bool operator!=(const const_iterator& other) const
        { return !(*this == other); }
        bool operator<(const const_iterator& other) const
        { return _M_index < other._M_index; }
        bool operator<=(const const_iterator& other) const
        { return _M_index <= other._M_index; }
        bool operator>(const const_iterator& other) const
        { return _M_index > other._M_index; }
        bool operator>=(const const_iterator& other) const
        { return _M_index >= other._M_index; }

        const_iterator& operator++() { _M_index++; return *this; }
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        const_iterator& operator--() { _M_index--; return *this; }
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        const_iterator& operator+=(difference_type n) { _M_index += n; return *this; }
        const_iterator operator+(difference_type n) const {
            const_iterator tmp = *this;
            tmp += n;
            return tmp;
        }        
        friend const_iterator operator+(difference_type n, const const_iterator& it)
        { return it + n; }

        const_iterator& operator-=(difference_type n) { _M_index -= n; return *this; }
        const_iterator operator-(difference_type n) const {
            const_iterator tmp = *this;
            tmp -= n;
            return tmp;
        }
        difference_type operator-(const const_iterator& other) const
        { return _M_index - other._M_index; }

        reference operator*()
        { return _M_buf->_M_data[_M_index % _M_buf->capacity()]; }
        pointer operator->()
        { return &_M_buf->_M_data[_M_index % _M_buf->capacity()]; }
        reference operator[](difference_type n)
        { return _M_buf->_M_data[(_M_index + n) % _M_buf->capacity()]; }
    };

public:
    constexpr buffer() = default;
    constexpr buffer(const buffer&) = default;
    constexpr buffer(buffer&&) = default;

    /**
     * @brief Constructs a buffer with the given data and initial state.
     * 
     * The buffer is initialized with the provided data, and the current,
     * start, last, and size indices are set accordingly.
     * 
     * @param data The initial data to fill the buffer.
     * @param current The index of the current element (default is 0).
     * @param start The index of the start element (default is 0).
     * @param last The index of the last element (default is 0).
     */
    constexpr buffer(
        const std::array<T, N>& __data, 
        std::size_t __current = 0,
        std::size_t __start = 0,
        std::size_t __last = 0
    ) noexcept 
    : _M_data(__data), _M_current(__current), _M_start(__start),
    _M_last(__last) {}

private:
    std::array<T, N> _M_data;
    size_type _M_current = -1; // -1 indicates no current element
    size_type _M_start = 0;
    size_type _M_last = 0;

public:
    /**
     * @brief Adds a new value to the buffer.
     * 
     * If the buffer is full, the oldest value is removed.
     * If current is last, last advances to the next position.
     * 
     * All iterators (including the `end()` iterator) and
     * all references to the elements are invalidated.
     * 
     * @param value The value to be added to the buffer.
     */
    void push(const T& value) {
        _M_current++;
        _M_last = _M_current;

        _M_data[_M_current % N] = value;

        if (_M_current == _M_last && size() == N) _M_start++;
    }

    /**
     * @brief Adds a new value to the buffer.
     * 
     * If the buffer is full, the oldest value is removed.
     * If current is last, last advances to the next position.
     * 
     * All iterators (including the `end()` iterator) and
     * all references to the elements are invalidated.
     * 
     * @param value The value to be added to the buffer.
     */
    void push(T&& value) {
        _M_current++;
        _M_last = _M_current;

        _M_data[_M_current % N] = std::move(value);

        if (_M_current == _M_last && size() == N) _M_start++;
    }

    /**
     * @brief Removes the last value from the buffer.
     * 
     * All iterators (including the `end()` iterator) and
     * all references to the elements are invalidated.
     *
     * @note
     * This method may be unnecessary,
     * because the buffer is designed to automatically remove
     * the oldest value when full.
     * It is provided for completeness.
     */
    void pop() {
        if (empty()) return;
        
        _M_data[_M_last % N] = T(); // Clear the last element
        if (is_last()) _M_current--;
        _M_last--;
    }

    /**
     * @brief Moves the current index to the next position.
     * 
     * No iterators or references are invalidated.
     * 
     * @return Whether the current index was moved successfully.
     */
    bool next() {
        if (is_last()) return false;
        _M_current++;
        return true;
    }

    /**
     * @brief Moves the current index to the previous position.
     * 
     * No iterators or references are invalidated. 
     *
     * @return Whether the current index was moved successfully.
     */
    bool prev() {
        if (is_first()) return false;
        _M_current--;
        return true;
    }

    /**
     * @brief Clears the buffer.
     * 
     * All iterators (including the `end()` iterator) and
     * all references to the elements are invalidated.
     * 
     * @note
     * This method does not deallocate memory,
     * it only resets the internal state of the buffer.
     * It is equivalent to calling `clear()` on the underlying array.
     */
    void clear() {
        _M_current = -1;
        _M_start = 0;
        _M_last = 0;
    }

    /**
     * @brief Check current index indicates the first element.
     * 
     * @note
     * If this method return true, `prev()` will return false.
     */
    constexpr bool is_first() const noexcept
    { return _M_current == _M_start; }

    /**
     * @brief Check current index indicates the last element.
     * 
     * @note
     * If this method return true, `next()` will return false.
     */
    constexpr bool is_last() const noexcept
    { return _M_current == _M_last; }

    /**
     * @brief Returns a reference to the current element.
     */
    reference current() { return _M_data[_M_current % N]; }

    /**
     * @brief Returns a const reference to the current element.
     */
    const_reference current() const { return _M_data[_M_current % N]; }

    /**
     * @brief Returns the capacity of the buffer.
     */
    constexpr size_type capacity() const noexcept { return N; }
    /**
     * @brief Returns the number of elements in the buffer.
     */
    constexpr size_type size() const noexcept { return _M_last - _M_start; }
    /**
     * @brief Returns true if the buffer is empty.
     */
    constexpr bool empty() const noexcept { return size() == 0; }

    iterator begin() { return iterator(this, _M_start); }
    iterator end() { return iterator(this, _M_last + 1); }
    const_iterator begin() const { return const_iterator(this, _M_start); }
    const_iterator end() const { return const_iterator(this, _M_last + 1); }
    const_iterator cbegin() const { return const_iterator(this, _M_start); }
    const_iterator cend() const { return const_iterator(this, _M_last + 1); }

#ifdef DEBUG
    size_type current_index() const noexcept { return _M_current; }
    size_type start_index() const noexcept { return _M_start; }
    size_type last_index() const noexcept { return _M_last; }
    const std::array<T, N>& data() const noexcept { return _M_data; }
#endif
};