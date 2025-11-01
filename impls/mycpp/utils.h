#pragma once
#include <memory>

template <typename T, typename Ptr>
constexpr auto dyn(const Ptr& ptr) {
    return std::dynamic_pointer_cast<T>(ptr);
}
