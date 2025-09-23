#pragma once
#include <array>
#include <functional>
#include <utility>
#include <memory>
#include <type_traits>
#include <print>

namespace detail {

template <typename S, typename T>
using RetainConstPtr = std::conditional_t<std::is_const_v<std::remove_reference_t<S>>, const T*, T*>;

struct Deleter {
    private:
        using DeletePtr = void(*)(void*);
    public:
        constexpr Deleter(): deletePtr(nullptr) {}

        template <typename T>
        Deleter(std::in_place_type_t<T>) :
            deletePtr(+[](void* ptr) { static_cast<T*>(ptr)->~T(); }) {}

        void del(void* p) const {
            std::invoke(deletePtr, p);
        }

        void operator()(void* p) const {
            del(p);
        }

        DeletePtr deletePtr;
};

struct MemManagerTwoPtrs : Deleter {
    private:
        using MovePtr = void(*)(void*, void*);
    public:
        template <typename T>
        MemManagerTwoPtrs(std::in_place_type_t<T>) :
            Deleter(std::in_place_type<T>), 
            movePtr(+[](void* ptr, void* dst) { new(dst) T(std::move(*static_cast<T*>(ptr))); }) {}

        void move(void* src, void* dst) const {
            std::invoke(movePtr, src, dst);
        }

        MovePtr movePtr;
};

struct MemManagerOnePtr {
    private :
        enum Op {
            DEL, MOV
        };
        using Ptr = void(*)(void*, void*, Op);
    public: 
        template <typename T>
        MemManagerOnePtr(std::in_place_type_t<T>):
            ptr(+[](void* ptr, void* dst, Op op) {
               switch (op) {
                 case DEL: 
                    static_cast<T*>(ptr)->~T();
                    break;
                 case MOV:
                    new(dst) T(std::move(*static_cast<T*>(ptr))); 
             }
             })
        {}

        void del(void* p) const {
            std::invoke(ptr, p, nullptr, DEL);
        }

        void move(void* src, void* dst) const {
            std::invoke(ptr, src, dst, MOV);
        }

        Ptr ptr;
};

template <typename MemManager, std::size_t Size>
class StaticStorage {
    public:
        template <typename T> requires(sizeof(T) <= Size)
        StaticStorage(T&& t) :  
            mm(std::in_place_type<T>) {
            new(ptr()) T(std::forward<T>(t));
        }
        template <typename T, typename ... Args> requires(sizeof(T) <= Size)
        StaticStorage(std::in_place_type_t<T> tag, Args... args) :  
            mm(tag) {
            new(ptr()) T(std::forward<Args>(args)...);
        }
        StaticStorage(StaticStorage&) = delete;
        StaticStorage& operator=(StaticStorage&) = delete;
        StaticStorage(StaticStorage&& other): mm(other.mm) {
            mm.move(other.ptr(), ptr());
        }
        StaticStorage& operator=(StaticStorage&& other) {
            this->~StaticStorage();
            mm = other.mm;
            mm.move(other.ptr(), ptr());
            return *this;
        }
        ~StaticStorage() {
            mm.del(ptr());
        }

        template <typename T, typename Self>
        decltype(auto) get(this Self&& self) {
            return *static_cast<RetainConstPtr<Self, T>>(std::forward<Self>(self).ptr());
        }
    private:
        alignas(void*) std::array<char, Size> storage;

        template <typename Self>
        decltype(auto) ptr(this Self&& self) {
            return static_cast<RetainConstPtr<Self, void>>(&std::forward<Self>(self).storage.front());
        }

        MemManager mm;
};

class DynamicStorage {
private:
    std::unique_ptr<void, Deleter> storage;
public:
    constexpr DynamicStorage(): storage(nullptr) {}

    template <typename T>
    DynamicStorage(T&& t): storage{new T(std::forward<T>(t)), Deleter{std::in_place_type<T>}} {}

    template <typename T, typename ... Args>
    DynamicStorage(std::in_place_type_t<T> tag, Args&& ... args): storage{new T(std::forward<Args>(args)...), Deleter{tag}} {}

    template <typename T, typename Self>
    auto& get(this Self&& self) {
        return *static_cast<RetainConstPtr<Self, T>>(std::forward<Self>(self).storage.get());
    }
};

template <typename T, typename Storage>
decltype(auto) any_cast(Storage&& s) {
    return std::forward<Storage>(s).template get<T>();
}

}

