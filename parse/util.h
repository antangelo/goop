#ifndef GOOP_UTIL_H
#define GOOP_UTIL_H

#include <cassert>
#include <concepts>
#include <memory>

namespace goop
{

//template<std::copy_constructible T>
template<typename T>
class box
{
    std::unique_ptr<T> inner;

    public:
    box &operator=(box &&other) {
        if (other.inner) {
            inner = std::make_unique<T>(*other.inner);
        } else {
            inner = nullptr;
        }
    }

    explicit operator bool() const noexcept {
        return inner;
    }

    T& operator*() const noexcept {
        return *inner;
    }

    T& operator->() const noexcept {
        return *inner;
    }

    box(T *ptr): inner{ptr} {}

    box(std::unique_ptr<T> ptr):
        inner{std::move(ptr)} {}

    box(const box<T> &other):
        inner{std::make_unique<T>(*other.inner)} {}

    box(box<T> &&other):
        inner{std::move(other.inner)} {}
};

template<std::copy_constructible T, class... Args>
box<T> make_box(Args &&... args)
{
    return box<T>(std::make_unique(args...));
}

}

#endif
