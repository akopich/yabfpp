#pragma once
#include <array>
#include <functional>
#include <utility>
#include <memory>
#include <type_traits>

namespace detail {

template <typename S, typename T>
using RetainConstPtr = std::conditional_t<std::is_const_v<std::remove_reference_t<S>>, const T*, T*>;

struct MemManagerThreePtrs {
    private:
        using DeletePtr = void(*)(void*);
        using MovePtr = void(*)(void*, void*);
        using GetPtr = void*(*)(void*);
    public:
        void del(void* p) const {
            std::invoke(deletePtr, p);
        }

        void move(void* src, void* dst) const {
            std::invoke(movePtr, src, dst);
        }

        void* get(void* p) const {
            return std::invoke(getPtr, p);
        }

        DeletePtr deletePtr;
        MovePtr movePtr;
        GetPtr getPtr;
};

constexpr inline auto mkMemManagerThreePtrs = []<typename T>(std::in_place_type_t<T>) {
    return MemManagerThreePtrs{
            [](void* ptr) { static_cast<T*>(ptr)->~T(); },
            [](void* ptr, void* dst) { new(dst) T(std::move(*static_cast<T*>(ptr))); },
            [](void* p) { return p; }
    };
};

constexpr inline auto mkMemManagerThreePtrsDynamic = []<typename T>(std::in_place_type_t<T>) {
    using Uniq = std::unique_ptr<T>;
    return MemManagerThreePtrs {
            [](void* ptr) { delete *static_cast<T**>(ptr); },
            [](void* ptr, void* dst) { 
                *static_cast<T**>(dst) = *static_cast<T**>(ptr); 
                *static_cast<T**>(ptr) = nullptr;
            },
            [](void* p) -> void* { return *static_cast<void**>(p); }
    };
};

enum Op {
    GET, DEL, MOV
};

struct MemManagerOnePtr {
    private :
        using Ptr = void*(*)(void*, void*, Op);
    public: 

        void del(void* p) const {
            std::invoke(ptr, p, nullptr, DEL);
        }

        void move(void* src, void* dst) const {
            std::invoke(ptr, src, dst, MOV);
        }

        void* get(void* p) const {
            return std::invoke(ptr, p, nullptr, GET);
        }

        Ptr ptr;
};

constexpr inline auto mkMemManagerOnePtr = []<typename T>(std::in_place_type_t<T>) {
    return MemManagerOnePtr{
        +[](void* ptr, void* dst, Op op) -> void* {
            switch (op) {
                case GET:
                    return ptr;
                case DEL: 
                    static_cast<T*>(ptr)->~T();
                    return nullptr;
                case MOV:
                    new(dst) T(std::move(*static_cast<T*>(ptr))); 
                    return nullptr;
            }
        }
    };
};

constexpr inline auto mkMemManagerOnePtrDynamic = []<typename T>(std::in_place_type_t<T>) {
    return MemManagerOnePtr{
        +[](void* ptr, void* dst, Op op) -> void* {
            switch (op) {
                case GET:
                    return *static_cast<void**>(ptr);
                case DEL: 
                    delete *static_cast<T**>(ptr);
                    return nullptr;
                case MOV:
                    *static_cast<T**>(dst) = *static_cast<T**>(ptr); 
                    *static_cast<T**>(ptr) = nullptr;
                    return nullptr;
            }
        }
    };
};

template <typename MemManager, auto mmMaker, auto mmDynamicMaker, std::size_t Size> requires(Size >= sizeof(void*) )
class StaticStorage {
    public:
        template <typename T, bool IsBig = (sizeof(T) > Size)> 
        StaticStorage(T&& t) : StaticStorage(std::in_place_type<T>, std::forward<T>(t)) {}

        template <typename T, typename ... Args, bool IsBig = (sizeof(T) > Size)>
        StaticStorage(std::in_place_type_t<T> tag, Args... args) :  
            mm(IsBig ? mmDynamicMaker(tag): mmMaker(tag)) {
            if constexpr (sizeof(T) > Size) {
               auto* obj = new T(std::forward<Args>(args)...);
               *static_cast<void**>(ptr()) = obj;
            } else {
                new(ptr()) T(std::forward<Args>(args)...);
            }
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
            return *static_cast<RetainConstPtr<Self, T>>(self.mm.get(
                      const_cast<void*>(std::forward<Self>(self).ptr())
                ));
        }
    private:
        alignas(void*) std::array<char, Size> storage;

        template <typename Self>
        decltype(auto) ptr(this Self&& self) {
            return static_cast<RetainConstPtr<Self, void>>(&std::forward<Self>(self).storage.front());
        }

        MemManager mm;
};

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

template <size_t Size>
using AnyOnePtr = detail::StaticStorage<detail::MemManagerOnePtr, detail::mkMemManagerOnePtr, detail::mkMemManagerOnePtrDynamic, Size>;


template <size_t Size>
using AnyThreePtrs = detail::StaticStorage<detail::MemManagerThreePtrs, detail::mkMemManagerThreePtrs, detail::mkMemManagerThreePtrsDynamic, Size>;
