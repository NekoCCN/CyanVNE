//
// Created by unirz on 2025/7/16.
//

#ifndef COMPONENTEMPLACER_H
#define COMPONENTEMPLACER_H

#include <entt/entt.hpp>

namespace cyanvne::parser::ecs
{
    class IComponentEmplacer
    {
    public:
        virtual ~IComponentEmplacer() = default;
        virtual void emplace_to(entt::registry& registry, entt::entity entity) const = 0;
    };

    template<typename T>
    class ComponentEmplacer : public IComponentEmplacer
    {
    private:
        T component_data_;
    public:
        explicit ComponentEmplacer(T data) : component_data_(std::move(data))
        {  }

        void emplace_to(entt::registry& registry, entt::entity entity) const override
        {
            registry.emplace<T>(entity, component_data_);
        }
    };
}

#endif //COMPONENTEMPLACER_H
