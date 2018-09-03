#pragma once

#include <cstddef>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

#include "intrusive_ptr.hpp"


namespace std {

    template <class T>
    class weak_intrusive_ptr {
    public:
        using pointer = T*;
        using const_pointer = const T*;
        using element_type = T;
        using reference = T&;
        using const_reference = const T&;

        static constexpr bool has_weak_ptr_semantics = true;

        constexpr weak_intrusive_ptr() noexcept : ptr_(nullptr) {

        }

        weak_intrusive_ptr(pointer raw_ptr, bool add_ref = true) noexcept {
            set_ptr(raw_ptr, add_ref);
        }

        weak_intrusive_ptr(weak_intrusive_ptr&& other) noexcept
                : ptr_(other.detach()) {
        }

        weak_intrusive_ptr(const weak_intrusive_ptr& other) noexcept {
            set_ptr(other.get(), true);
        }

        template <class Y>
        weak_intrusive_ptr(weak_intrusive_ptr<Y> other) noexcept
                : ptr_(other.detach()) {
            static_assert(std::is_convertible<Y*, T*>::value, "Y* is not assignable to T*");
        }

        ~weak_intrusive_ptr() {
            if (ptr_) {
                intrusive_ptr_release_weak(ptr_);
            }
        }

        void swap(weak_intrusive_ptr& other) noexcept {
            std::swap(ptr_, other.ptr_);
        }
        pointer detach() noexcept {
            auto result = ptr_;
            if (result) {
                ptr_ = nullptr;
            }
            return result;
        }

        pointer release() noexcept {
            return detach();
        }

        void reset(pointer new_value = nullptr, bool add_ref = true) {
            auto old = ptr_;
            set_ptr(new_value, add_ref);
            if (old) {
                intrusive_ptr_release_weak(old);
            }
        }

        weak_intrusive_ptr& operator=(pointer ptr) noexcept {
            reset(ptr);
            return *this;
        }

        weak_intrusive_ptr& operator=(weak_intrusive_ptr other) noexcept {
            swap(other);
            return *this;
        }

        pointer get() const noexcept {
            return ptr_;
        }

        pointer operator->() const noexcept {
            return ptr_;
        }

        reference operator*() const noexcept {
            return *ptr_;
        }

        bool operator!() const noexcept {
            return !ptr_;
        }

        explicit operator bool() const noexcept {
            return static_cast<bool>(ptr_);
        }


        intrusive_ptr<T> lock() const noexcept {
            if (!ptr_ || !intrusive_ptr_upgrade_weak(ptr_)) {
                return nullptr;
            }

            return {ptr_, false};
        }


        pointer get_locked() const noexcept {
            if (!ptr_ || !intrusive_ptr_upgrade_weak(ptr_)) {
                return nullptr;
            }
            return ptr_;
        }

    private:
        void set_ptr(pointer raw_ptr, bool add_ref) noexcept {
            ptr_ = raw_ptr;
            if (raw_ptr && add_ref) {
                intrusive_ptr_add_weak_ref(raw_ptr);
            }
        }

        pointer ptr_;
    };

    template <class X, typename Y>
    bool operator==(const weak_intrusive_ptr<X>& lhs, const weak_intrusive_ptr<Y>& rhs) {
        return lhs.get() == rhs.get();
    }


    template <class X, typename Y>
    bool operator!=(const weak_intrusive_ptr<X>& lhs, const weak_intrusive_ptr<Y>& rhs) {
        return !(lhs == rhs);
    }

}

