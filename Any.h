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

template <typename T>
constexpr inline auto delStatic = [](void* ptr) {
    static_cast<T*>(ptr)->~T(); 
    return nullptr;
};

template <typename T>
constexpr inline auto movStatic = [](void* src, void* dst) {
    new(dst) T(std::move(*static_cast<T*>(src))); 
    return nullptr;
};

template <typename T>
constexpr inline auto getStatic = [](void* ptr) {
    return ptr;
};


template <typename F> 
struct FunArgs : FunArgs<decltype(&F::operator())> {};

template <typename C, typename ... Args, typename R>
struct FunArgs<R(C::*)(Args...) const> {
    using ArgsT = std::tuple<Args...>;
 };

template <typename F, typename ... Args>
constexpr auto mkReturn(std::tuple<Args...> ) {
    return +[](Args ... args) -> void {
        std::invoke(F{}, std::forward<Args>(args)...);
    };
}

template<typename T>
constexpr inline auto noReturn = mkReturn<T>(typename FunArgs<T>::ArgsT{});

template <typename Del, typename Mov, typename Get>
consteval auto mkMemManagerThreePtrsFromLambdas(Del, Mov, Get) {
    return MemManagerThreePtrs{
            noReturn<Del>,
            noReturn<Mov>,
            Get{}
    };
}

constexpr inline auto mkMemManagerThreePtrs = []<typename T>(TypeTag<T>) consteval {
    return mkMemManagerThreePtrsFromLambdas(delStatic<T>, movStatic<T>, getStatic<T>);
};

template <typename T>
constexpr inline auto delDynamic = [](void* ptr) {
    delete *static_cast<T**>(ptr);
    return nullptr;
};

template <typename T>
constexpr inline auto getDynamic = [](void* ptr) {
    return *static_cast<void**>(ptr);
};

template <typename T>
constexpr inline auto movDynamic = [](void* src, void* dst) {
    *static_cast<T**>(dst) = *static_cast<T**>(src); 
    *static_cast<T**>(src) = nullptr;
    return nullptr;
};


constexpr inline auto mkMemManagerThreePtrsDynamic = []<typename T>(TypeTag<T>) consteval {
    return MemManagerThreePtrs {
            noReturn<decltype(delDynamic<T>)>,
            noReturn<decltype(movDynamic<T>)>,
            getDynamic<T>
    };
};

enum Op {
    GET, DEL, MOV
};

struct MemManagerOnePtr {
    private :
        using Ptr = void*(*)(Op, void*, void*);
    public: 

        void del(void* p) const {
            std::invoke(ptr, DEL, p, nullptr);
        }

        void move(void* src, void* dst) const {
            std::invoke(ptr, MOV, src, dst);
        }

        void* get(void* p) const {
            return std::invoke(ptr, GET, p, nullptr);
        }

        Ptr ptr;
};


template <typename Del, typename Mov, typename Get>
consteval auto mkMemManagerOnePtrFromLambdas(Del, Mov, Get) {
    return MemManagerOnePtr {
        +[]( Op op, void* ptr, void* dst) -> void* {
            switch (op) {
                case GET:
                    return Get{}(ptr);
                case DEL: 
                    return Del{}(ptr);
                case MOV:
                    return Mov{}(ptr, dst);
            }
        }
    };
}

constexpr inline auto mkMemManagerOnePtr = []<typename T>(TypeTag<T>) consteval {
    return mkMemManagerOnePtrFromLambdas(delStatic<T>, movStatic<T>, getStatic<T>);
};

constexpr inline auto mkMemManagerOnePtrDynamic = []<typename T>(TypeTag<T>) consteval {
    return mkMemManagerOnePtrFromLambdas(delDynamic<T>, movDynamic<T>, getDynamic<T>);
};

template <typename MemManager, auto mmStaticMaker, auto mmDynamicMaker, std::size_t Size> requires(Size >= sizeof(void*) )
class StaticStorage {
    public:
        template <typename T, bool IsBig = (sizeof(T) > Size)> 
        StaticStorage(T&& t) : StaticStorage(std::in_place_type<T>, std::forward<T>(t)) {}

        template <typename T, typename ... Args, bool IsBig = (sizeof(T) > Size)>
        StaticStorage(std::in_place_type_t<T>, Args... args) {
            if constexpr (IsBig) {
                static constinit auto mm = mmDynamicMaker(kTypeTag<T>);
                this->mm = mm;
               auto* obj = new T(std::forward<Args>(args)...);
               *static_cast<void**>(ptr()) = obj;
            } else {
                static constinit auto mm = mmStaticMaker(kTypeTag<T>);
                this->mm = mm;
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
        MemManager mm;

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
