// componentsystem.h
#ifndef COMPONENTSYSTEM_H
#define COMPONENTSYSTEM_H

#include <memory>
#include <typeindex>
#include <functional>
#include <unordered_map>
#include <stdexcept>
#include <type_traits>

class ComponentSystem {
public:
    // 1) Register a service interface → concrete
    template<typename Interface, typename Implementation>
    void register_interface() {
        static_assert(std::is_base_of_v<Interface,Implementation>,
                      "Implementation must derive from Interface");
        factories_[typeid(Interface)] = []() -> std::shared_ptr<void> {
            // make_shared<Implementation>() gives shared_ptr<Implementation>,
            // then static_pointer_cast<void> erases to shared_ptr<void>.
            return std::static_pointer_cast<void>(
                std::make_shared<Implementation>()
                );
        };
    }

    // 2) Create an instance of the requested interface
    template<typename Interface>
    std::shared_ptr<Interface> create() {
        auto it = factories_.find(typeid(Interface));
        if (it == factories_.end())
            throw std::runtime_error("Component not registered: " +
                                     std::string(typeid(Interface).name()));
        // call the stored factory → shared_ptr<void>
        auto erased = it->second();
        // cast back to shared_ptr<Interface>
        return std::static_pointer_cast<Interface>(erased);
    }

private:
    // Map: type_index → factory returning shared_ptr<void>
    std::unordered_map<std::type_index,
                       std::function<std::shared_ptr<void>()>> factories_;
};

#endif // COMPONENTSYSTEM_H
