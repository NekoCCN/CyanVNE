#pragma once
#include <Runtime/IGameState/IGameState.h>
#include <Runtime/GameStateManager/GameStateManager.h>
#include <imgui.h>

namespace cyanvne
{
	namespace runtime
	{
		class GuiDebugState : public IGameState
		{
		private:
			bool status_ = true;
		public:
			GuiDebugState() = default;

			void init(GameStateManager& manager) override
			{
				status_ = true;
			}
			void update(GameStateManager& manager, float deltaTime) override
			{  }

			void pause(GameStateManager& manager) override
			{
				status_ = false;
			}

			void resume(GameStateManager& manager) override
			{
				status_ = true;
			}

			void render(GameStateManager& manager) override
			{
				ImGui::ShowDemoWindow();
				ImGui::ShowDebugLogWindow();
				ImGui::ShowFontSelector("Font");
				ImGui::ShowStyleEditor();
				ImGui::ShowMetricsWindow();
				ImGui::ShowUserGuide();
			}

			void shutdown(GameStateManager& manager) override
			{
				status_ = false;
			}

			~GuiDebugState() override = default;
		};
	}
}