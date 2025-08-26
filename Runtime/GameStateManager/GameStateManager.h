#pragma once
#include <memory>
#include <vector>
#include "Platform/EventBus/EventBus.h"
#include "Platform/WindowContext/WindowContext.h"
#include "Runtime/Components/Components.h"
#include "Resources/UnifiedCacheManager/UnifiedCacheManager.h"
#include "Audio/AudioManager/AudioManager.h"
#include "Audio/SoloudAudioEngine/SoloudAudioEngine.h"

namespace cyanvne::runtime
{
    class GameStateManager;

    class IGameState
    {
    protected:
        IGameState() = default;
    public:
        IGameState(const IGameState&) = delete;
        IGameState& operator=(const IGameState&) = delete;
        IGameState(IGameState&&) = delete;
        IGameState& operator=(IGameState&&) = delete;

        virtual void init(std::shared_ptr<GameStateManager> manager) = 0;
        virtual void shutdown(std::shared_ptr<GameStateManager> manager) = 0;

        virtual void handle_events(std::shared_ptr<GameStateManager> manager) = 0;
        virtual void update(std::shared_ptr<GameStateManager> manager, float delta_time) = 0;
        virtual void render(std::shared_ptr<GameStateManager> manager) = 0;

        virtual void pause(std::shared_ptr<GameStateManager> manager)
        {  }
        virtual void resume(std::shared_ptr<GameStateManager> manager)
        {  }

        virtual ~IGameState() = default;
    };

    class GameStateManager : public std::enable_shared_from_this<GameStateManager>
    {
    private:
        std::vector<std::unique_ptr<IGameState>> state_stack_;

        std::shared_ptr<platform::WindowContext> window_context_;
        std::shared_ptr<platform::EventBus> event_bus_;
        std::shared_ptr<resources::UnifiedCacheManager> cache_manager_;
        std::shared_ptr<platform::concurrency::UnifiedConcurrencyManager> concurrency_manager_;
        std::shared_ptr<audio::AudioManager<audio::SoloudAudioEngine>> audio_manager_;

        bool running_ = true;

    public:
        GameStateManager(std::shared_ptr<platform::WindowContext> window_ctx,
                         std::shared_ptr<platform::EventBus> event_bus,
                         std::shared_ptr<resources::UnifiedCacheManager> cache_manager,
                         std::shared_ptr<platform::concurrency::UnifiedConcurrencyManager> concurrency_manager,
                         std::shared_ptr<audio::AudioManager<audio::SoloudAudioEngine>> audio_manager);

        ~GameStateManager();

        GameStateManager(const GameStateManager&) = delete;
        GameStateManager& operator=(const GameStateManager&) = delete;

        void pushState(std::unique_ptr<IGameState> new_state);
        void popState();
        void changeState(std::unique_ptr<IGameState> new_state);
        void quit();

        void handleEvents();
        void update(float delta_time);
        void render();

        bool isRunning() const { return running_; }
        bool hasStates() const { return !state_stack_.empty(); }

        std::shared_ptr<platform::WindowContext> getWindowContext()
        {
            return window_context_;
        }
        std::shared_ptr<platform::EventBus> getEventBus()
        {
            return event_bus_;
        }
        std::shared_ptr<resources::UnifiedCacheManager> getCacheManager()
        {
            return cache_manager_;
        }
        std::shared_ptr<platform::concurrency::UnifiedConcurrencyManager> getConcurrencyManager()
        {
            return concurrency_manager_;
        }
        std::shared_ptr<audio::AudioManager<audio::SoloudAudioEngine>> getAudioManager()
        {
            return audio_manager_;
        }
    };
}
