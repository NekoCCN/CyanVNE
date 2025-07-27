#pragma once

#define M_PI       3.14159265358979323846
#include "Runtime/Components/Components.h"
#include <entt/entt.hpp>
#include <SDL3/SDL.h>

namespace cyanvne
{
    namespace platform
    { class WindowContext;
        class EventBus; }

    namespace resources
    { class ICacheResourcesManager; }

    namespace runtime
    {
        class GameStateManager;

        namespace ecs
        { class Scene; }
    }
}

namespace cyanvne::audio
{ class AudioManager; }

namespace cyanvne::ecs
{
    struct CommandPacket;
    using CommandQueue = std::deque<CommandPacket>;
}

namespace cyanvne::ecs::systems
{
    void HierarchySystem(entt::registry& registry);

    void FinalRectSystem(entt::registry& registry,
                         const platform::WindowContext& window,
                         const resources::ICacheResourcesManager& cache_manager);

    void RenderSystem(entt::registry& registry,
                      const platform::WindowContext& window,
                      const resources::ICacheResourcesManager& cache_manager);

    void SetParent(entt::registry& registry, entt::entity child, entt::entity parent);

    void AnimationSystem(entt::registry& registry, float delta_time);

    void TweenSystem(entt::registry& registry, float delta_time);

    void CommandSystem(entt::registry& registry, runtime::ecs::Scene& scene, runtime::GameStateManager& gsm, platform::EventBus& bus);

    void AudioSystem(entt::registry& registry, audio::AudioManager& audio_manager);

    class InteractionSystem
    {
    private:
        std::shared_ptr<entt::registry> registry_;
        std::shared_ptr<cyanvne::ecs::CommandQueue> command_queue_;
        void pushCommands(const std::vector<std::shared_ptr<commands::ICommand>>& commands, entt::entity source) const;

    public:
        InteractionSystem(const std::shared_ptr<entt::registry>& registry,
                          platform::EventBus& bus,
                          const std::shared_ptr<cyanvne::ecs::CommandQueue>& command_queue);
        bool processInput(const SDL_Event& event) const;
    };

    namespace Easing
    {
        inline float Linear(float t) { return t; }
        inline float EaseInQuad(float t) { return t * t; }
        inline float EaseOutQuad(float t) { return t * (2.0f - t); }
        inline float EaseInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; }
        inline float EaseOutSine(float t) { return sin(t * (float)M_PI / 2.0f); }
    }
}