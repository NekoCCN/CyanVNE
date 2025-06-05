#pragma once
#include <Runtime/GameStateManager/GameStateManager.h>

namespace cyanvne
{
	namespace runtime
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

			virtual void init(GameStateManager& manager) = 0;
			virtual void update(GameStateManager& manager, float deltaTime) = 0;

			virtual void pause(GameStateManager& manager)
			{  }

            virtual void resume(GameStateManager& manager)
			{  }

			virtual void render(GameStateManager& manager) = 0;
			virtual void shutdown(GameStateManager& manager) = 0;

            virtual ~IGameState() = default;
		};
	}
}