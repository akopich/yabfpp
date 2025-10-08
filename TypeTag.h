#pragma once

namespace detail {

template <typename T>
class TypeTag{
    using Type = T;
};

template <typename T>
inline constexpr TypeTag<T> kTypeTag{};

}
