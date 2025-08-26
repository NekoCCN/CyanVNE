#include "GameStateManager.h"
#include "Runtime/Systems/Systems.h"

namespace cyanvne::runtime
{
    GameStateManager::GameStateManager(std::shared_ptr<platform::WindowContext> window_ctx,
                                       std::shared_ptr<platform::EventBus> event_bus,
                                       std::shared_ptr<resources::UnifiedCacheManager> cache_manager,
                                       std::shared_ptr<platform::concurrency::UnifiedConcurrencyManager> concurrency_manager,
                                       std::shared_ptr<audio::AudioManager<audio::SoloudAudioEngine>> audio_manager)
            : window_context_(std::move(window_ctx)),
              event_bus_(std::move(event_bus)),
              cache_manager_(std::move(cache_manager)),
              concurrency_manager_(std::move(concurrency_manager)),
              audio_manager_(std::move(audio_manager))
    {
        state_stack_.reserve(10);
    }

    GameStateManager::~GameStateManager()
    {
        while (!state_stack_.empty())
        {
            state_stack_.back()->shutdown(shared_from_this());
            state_stack_.pop_back();
        }
    }

    void GameStateManager::quit()
    {
        running_ = false;
    }

    void GameStateManager::pushState(std::unique_ptr<IGameState> new_state)
    {
        if (!new_state) return;

        if (!state_stack_.empty())
        {
            state_stack_.back()->pause(shared_from_this());
        }

        new_state->init(shared_from_this());
        state_stack_.push_back(std::move(new_state));
    }

    void GameStateManager::popState()
    {
        if (!state_stack_.empty())
        {
            state_stack_.back()->shutdown(shared_from_this());
            state_stack_.pop_back();

            if (!state_stack_.empty())
            {
                state_stack_.back()->resume(shared_from_this());
            }
        }
    }

    void GameStateManager::changeState(std::unique_ptr<IGameState> new_state)
    {
        while (!state_stack_.empty())
        {
            state_stack_.back()->shutdown(shared_from_this());
            state_stack_.pop_back();
        }
        pushState(std::move(new_state));
    }

    void GameStateManager::handleEvents()
    {
        if (!state_stack_.empty())
        {
            state_stack_.back()->handle_events(shared_from_this());
        }
    }

    void GameStateManager::update(float delta_time)
    {
        if (audio_manager_) {
            audio_manager_->update();
        }

        if (!state_stack_.empty())
        {
            state_stack_.back()->update(shared_from_this(), delta_time);
        }
    }

    void GameStateManager::render()
    {
        for (const auto& state : state_stack_)
        {
            state->render(shared_from_this());
        }
    }
}