#ifndef COMPONENTSYSTEM_H
#define COMPONENTSYSTEM_H

#include <memory>
#include <typeindex>
#include <tuple>
#include <functional>
#include <unordered_map>

class ComponentSystem {
public:
    template<typename T, typename... Args>
    void configure(Args&&... args) {
        auto factory = [captured_args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            return std::apply([](auto&&... args) {
                return std::make_unique<T>(std::forward<decltype(args)>(args)...);
            }, std::move(captured_args));
        };

        factories_[typeid(T)] = [factory = std::move(factory)]() mutable {
            return factory();
        };
    }

    template<typename TBase, typename TConcrete>
    void configure_derived() {
        factories_[typeid(TBase)] = [] {
            return std::unique_ptr<void>(static_cast<TBase*>(new TConcrete()));
        };
    }

    template<typename T>
    std::unique_ptr<T> create() {
        auto it = factories_.find(typeid(T));
        if(it == factories_.end()) {
            throw std::runtime_error("Component type not registered");
        }

        auto ptr = it->second();
        return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
    }

private:
    std::unordered_map<std::type_index,
                       std::function<std::unique_ptr<void>()>> factories_;
};

#endif // COMPONENTSYSTEM_H
