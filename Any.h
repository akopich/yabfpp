#pragma once
#include <array>
#include <functional>

namespace detail {
template <std::size_t Size>
class StaticStorage {
    private:
        using DeletePtr = void(*)(void*);
        using MovePtr = void(*)(void*, void*);
    public:
        template <typename T> requires(sizeof(T) <= Size)
        StaticStorage(T&& t) : 
            deletePtr(+[](void* ptr) { static_cast<T*>(ptr)->~T(); }), 
            movePtr(+[](void*ptr, void* dst) { new(dst) T(std::move(*static_cast<T*>(ptr))); }) {
            new(ptr()) T(std::forward<T>(t));
        }
        StaticStorage(StaticStorage&) = delete;
        StaticStorage& operator=(StaticStorage&) = delete;
        StaticStorage(StaticStorage&& other): deletePtr(other.deletePtr), movePtr(other.movePtr) {
            std::invoke(movePtr, other.ptr(), ptr());
        }
        StaticStorage& operator=(StaticStorage&& other) {
            this->~StaticStorage();
            deletePtr = other.deletePtr;
            movePtr = other.movePtr;
            std::invoke(movePtr, other.ptr(), ptr());
            return *this;
        }
        ~StaticStorage() {
            std::invoke(deletePtr, ptr());
        }

        template <typename T>
        T& get() {
            return *static_cast<T*>(ptr());
        }
        template <typename T>
        const T& get() const {
            return *static_cast<const T*>(ptr());
        }
    private:
        alignas(void*) std::array<char, Size> storage;

        void* ptr() {
            return &storage.front();
        }

        const void* ptr() const {
            return &storage.front();
        }
        DeletePtr deletePtr;
        MovePtr movePtr;
};
}
