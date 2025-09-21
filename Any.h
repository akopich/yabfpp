#pragma once
#include <array>
#include <functional>

namespace detail {
template <std::size_t Size>
class StaticStorage {
    private:
        using DeletePtr = void(*)(void*);
        using MovePtr = void(*)(void*, void*);
        using MoveAssignPtr = void(*)(void*, void*);
    public:
        template <typename T> requires(sizeof(T) <= Size)
        StaticStorage(T&& t) : 
            deletePtr(+[](void* ptr) { static_cast<T*>(ptr)->~T(); }), 
            movePtr(+[](void*ptr, void* dst) { new(dst) T(std::move(*static_cast<T*>(ptr))); }),
            moveAssignPtr(+[](void* ptr, void* dst) {   
                        *static_cast<T*>(dst) = std::move(*static_cast<T*>(ptr));
                    } ) {
            new(ptr()) T(std::forward<T>(t));
        }
        StaticStorage(StaticStorage&) = delete;
        StaticStorage& operator=(StaticStorage&) = delete;
        StaticStorage(StaticStorage&& other): deletePtr(other.deletePtr), 
                movePtr(other.movePtr), moveAssignPtr(other.moveAssignPtr) {
            std::invoke(movePtr, other.ptr(), ptr());
        }
        StaticStorage& operator=(StaticStorage&& other) {
            this->~StaticStorage();
            deletePtr = other.deletePtr;
            movePtr = other.movePtr;
            moveAssignPtr = other.moveAssignPtr;
            std::invoke(moveAssignPtr, other.ptr(), ptr());
            return *this;
        }
        ~StaticStorage() {
            std::invoke(deletePtr, ptr());
        }
    private:
        alignas(void*) std::array<char, Size> storage;

        void* ptr() {
            return &storage.front();
        }
        DeletePtr deletePtr;
        MovePtr movePtr;
        MoveAssignPtr moveAssignPtr;
};
}
