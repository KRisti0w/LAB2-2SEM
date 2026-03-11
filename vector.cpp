#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <memory>

template<typename T>
class Vector {
private:
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void reallocate(size_t new_capacity) {
        T* new_data = static_cast<T*>(operator new[](new_capacity * sizeof(T)));
        for (size_t i = 0; i < size_; ++i) {
            new (new_data + i) T(std::move(data_[i]));
            data_[i].~T();
        }
        operator delete[](data_);
        data_ = new_data;
        capacity_ = new_capacity;
    }

    void reserve_grow() {
        if (size_ == capacity_)
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
    }

public:
    Vector() = default;

    explicit Vector(size_t count, const T& value = T()) {
        reserve(count);
        for (size_t i = 0; i < count; ++i) push_back(value);
    }

    template<typename Iterator>
    Vector(Iterator first, Iterator last) { while (first != last) push_back(*first++); }

    Vector(const Vector& other) : Vector(other.begin(), other.end()) {}

    Vector(Vector&& other) noexcept
            : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = other.capacity_ = 0;
    }

    ~Vector() { clear(); operator delete[](data_); }

    Vector& operator=(const Vector& other) {
        return *this = Vector(other);
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            clear();
            operator delete[](data_);
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = other.capacity_ = 0;
        }
        return *this;
    }

    T& at(size_t pos) {
        if (pos >= size_) throw std::out_of_range("Vector::at");
        return data_[pos];
    }
    const T& at(size_t pos) const {
        if (pos >= size_) throw std::out_of_range("Vector::at");
        return data_[pos];
    }

    T& operator[](size_t pos) { return data_[pos]; }
    const T& operator[](size_t pos) const { return data_[pos]; }

    T& front() { return at(0); }
    const T& front() const { return at(0); }
    T& back() { return at(size_-1); }
    const T& back() const { return at(size_-1); }

    T* data() { return data_; }
    const T* data() const { return data_; }

    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() { return data_; }
    iterator end() { return data_ + size_; }
    const_iterator begin() const { return data_; }
    const_iterator end() const { return data_ + size_; }
    const_iterator cbegin() const { return data_; }
    const_iterator cend() const { return data_ + size_; }

    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }

    void reserve(size_t new_capacity) {
        if (new_capacity > capacity_) reallocate(new_capacity);
    }

    void shrink_to_fit() {
        if (capacity_ > size_) reallocate(size_);
    }

    void clear() {
        for (size_t i = 0; i < size_; ++i) data_[i].~T();
        size_ = 0;
    }

    void push_back(const T& value) { reserve_grow(); new (data_ + size_++) T(value); }
    void push_back(T&& value) { reserve_grow(); new (data_ + size_++) T(std::move(value)); }

    void pop_back() {
        if (empty()) throw std::out_of_range("Vector::pop_back");
        data_[--size_].~T();
    }

    iterator insert(iterator pos, const T& value) {
        size_t index = pos - begin();
        reserve_grow();
        for (size_t i = size_; i > index; --i) {
            new (data_ + i) T(std::move(data_[i-1]));
            data_[i-1].~T();
        }
        new (data_ + index) T(value);
        ++size_;
        return begin() + index;
    }

    iterator insert(iterator pos, T&& value) {
        size_t index = pos - begin();
        reserve_grow();
        for (size_t i = size_; i > index; --i) {
            new (data_ + i) T(std::move(data_[i-1]));
            data_[i-1].~T();
        }
        new (data_ + index) T(std::move(value));
        ++size_;
        return begin() + index;
    }

    iterator erase(iterator pos) {
        size_t index = pos - begin();
        for (size_t i = index + 1; i < size_; ++i)
            data_[i-1] = std::move(data_[i]);
        data_[--size_].~T();
        return begin() + index;
    }

    iterator erase(iterator first, iterator last) {
        size_t start = first - begin();
        size_t count = last - first;
        for (size_t i = last - begin(); i < size_; ++i)
            data_[start + (i - (last - begin()))] = std::move(data_[i]);
        for (size_t i = size_ - count; i < size_; ++i) data_[i].~T();
        size_ -= count;
        return begin() + start;
    }

    void resize(size_t new_size, const T& value = T()) {
        if (new_size < size_) {
            for (size_t i = new_size; i < size_; ++i) data_[i].~T();
        } else if (new_size > size_) {
            reserve(new_size);
            for (size_t i = size_; i < new_size; ++i) new (data_ + i) T(value);
        }
        size_ = new_size;
    }

    void swap(Vector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    bool operator==(const Vector& other) const {
        return size_ == other.size_ && std::equal(begin(), end(), other.begin());
    }

    bool operator!=(const Vector& other) const { return !(*this == other); }
};