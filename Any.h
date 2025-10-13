#pragma once
#include "TypeTag.h"
#include <array>
#include <functional>
#include <utility>
#include <memory>
#include <type_traits>

namespace detail {

template <typename S, typename T>
using RetainConstPtr = std::conditional_t<std::is_const_v<std::remove_reference_t<S>>, const T*, T*>;

struct MemManagerTwoPtrs {
    private:
        using DeletePtr = void(*)(void*);
        using MovePtr = void(*)(void*, void*);
    public:
        void del(void* p) const {
            std::invoke(deletePtr, p);
        }

        void move(void* src, void* dst) const {
            std::invoke(movePtr, src, dst);
        }

        DeletePtr deletePtr;
        MovePtr movePtr;
};

template <typename T>
constexpr inline auto delStatic = [](void* ptr) {
    static_cast<T*>(ptr)->~T(); 
};

template <typename T>
constexpr inline auto movStatic = [](void* src, void* dst) {
    new(dst) T(std::move(*static_cast<T*>(src))); 
    static_cast<T*>(src)->~T(); 
};

template <typename Del, typename Mov>
consteval auto mkMemManagerTwoPtrsFromLambdas(Del, Mov) {
    return MemManagerTwoPtrs{
            Del{},
            Mov{},
    };
}

constexpr inline auto mkMemManagerTwoPtrs = []<typename T>(TypeTag<T>) consteval {
    return mkMemManagerTwoPtrsFromLambdas(delStatic<T>, movStatic<T>);
};

template <typename T>
constexpr inline auto delDynamic = [](void* ptr) {
    delete *static_cast<T**>(ptr);
};

template <typename T>
constexpr inline auto getDynamic = [](void* ptr) {
    return *static_cast<void**>(ptr);
};

template <typename T>
constexpr inline auto movDynamic = [](void* src, void* dst) {
    *static_cast<T**>(dst) = *static_cast<T**>(src); 
    *static_cast<T**>(src) = nullptr;
};


constexpr inline auto mkMemManagerTwoPtrsDynamic = []<typename T>(TypeTag<T>) consteval {
    return MemManagerTwoPtrs {
            delDynamic<T>,
            movDynamic<T>,
    };
};

enum Op {
    DEL, MOV
};

struct MemManagerOnePtr {
    private :
        using Ptr = void(*)(Op, void*, void*);
    public: 

        void del(void* p) const {
            std::invoke(ptr, DEL, p, nullptr);
        }

        void move(void* src, void* dst) const {
            std::invoke(ptr, MOV, src, dst);
        }

        Ptr ptr;
};


template <typename Del, typename Mov>
consteval auto mkMemManagerOnePtrFromLambdas(Del, Mov) {
    return MemManagerOnePtr {
        +[]( Op op, void* ptr, void* dst) -> void {
            switch (op) {
                case DEL: 
                    Del{}(ptr);
                    break;
                case MOV:
                    Mov{}(ptr, dst);
            }
        }
    };
}

constexpr inline auto mkMemManagerOnePtr = []<typename T>(TypeTag<T>) consteval {
    return mkMemManagerOnePtrFromLambdas(delStatic<T>, movStatic<T>);
};

constexpr inline auto mkMemManagerOnePtrDynamic = []<typename T>(TypeTag<T>) consteval {
    return mkMemManagerOnePtrFromLambdas(delDynamic<T>, movDynamic<T>);
};

template <typename MemManager, auto mmStaticMaker, auto mmDynamicMaker, std::size_t Size> requires(Size >= sizeof(void*) )
class StaticStorage {
    private: 
        template <typename T> 
        inline static constexpr bool kIsBig = sizeof(T) > Size || alignof(T) > alignof(void*);
    public:
        template <typename T> 
        StaticStorage(T&& t) : StaticStorage(std::in_place_type<T>, std::forward<T>(t)) {}

        template <typename T, typename ... Args>
        StaticStorage(std::in_place_type_t<T>, Args... args) {
            if constexpr (kIsBig<T>) {
                static constinit auto mm = mmDynamicMaker(kTypeTag<T>);
                this->mm = &mm;
               auto* obj = new T(std::forward<Args>(args)...);
               *static_cast<void**>(ptr()) = obj;
            } else {
                static constinit auto mm = mmStaticMaker(kTypeTag<T>);
                this->mm = &mm;
                new(ptr()) T(std::forward<Args>(args)...);
            }
        }
        StaticStorage(const StaticStorage&) = delete;
        StaticStorage& operator=(StaticStorage&) = delete;
        StaticStorage(StaticStorage&& other): mm(other.mm) {
            mm->move(other.ptr(), ptr());
            other.mm = nullptr;
        }
        StaticStorage& operator=(StaticStorage&& other) {
            this->~StaticStorage();
            mm = other.mm;
            mm->move(other.ptr(), ptr());
            other.mm = nullptr;
            return *this;
        }
        ~StaticStorage() {
            if (mm != nullptr)
                mm->del(ptr());
        }

        template <typename T, typename Self>
        decltype(auto) get(this Self&& self) {
            auto p = const_cast<void*>(std::forward<Self>(self).ptr());
            if constexpr (kIsBig<T>) {
                p = *static_cast<void**>(p);
            } 
            return *static_cast<RetainConstPtr<Self, T>>(p);
        }
    private:
        alignas(void*) std::array<char, Size> storage;
        MemManager* mm;

        template <typename Self>
        decltype(auto) ptr(this Self&& self) {
            return static_cast<RetainConstPtr<Self, void>>(&std::forward<Self>(self).storage.front());
        }
};

struct Deleter {
    private:
        using DeletePtr = void(*)(void*);
    public:
        constexpr Deleter(): deletePtr(nullptr) {}

        template <typename T>
        Deleter(TypeTag<T>) :
            deletePtr(+[](void* ptr) { delete static_cast<T*>(ptr); }) {}

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

    DynamicStorage(const DynamicStorage&) = delete;
    DynamicStorage& operator=(const DynamicStorage&) = delete;

    template <typename T>
    DynamicStorage(T&& t): storage{new T(std::forward<T>(t)), Deleter{kTypeTag<T>}} {}

    DynamicStorage& operator=(DynamicStorage&& t) {
        storage = std::move(t.storage);
        return *this;
    }

    DynamicStorage(DynamicStorage&& t): storage(std::move(t.storage)) { }

    template <typename T, typename ... Args>
    DynamicStorage(std::in_place_type_t<T>, Args&& ... args): storage{new T(std::forward<Args>(args)...), Deleter{kTypeTag<T>}} {}

    ~DynamicStorage() = default; 

    template <typename T, typename Self>
    decltype(auto) get(this Self&& self) {
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
using AnyTwoPtrs = detail::StaticStorage<detail::MemManagerTwoPtrs, detail::mkMemManagerTwoPtrs, detail::mkMemManagerTwoPtrsDynamic, Size>;
