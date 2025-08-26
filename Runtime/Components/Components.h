#pragma once

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <map>
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "Audio/IAudioEngine/IAudioEngine.h"
#include "Runtime/Renderer/MeshBatchRenderer/MeshBatchRenderer.h"
#include <variant>

namespace cyanvne::runtime
{
    struct MeshComponent
    {
        std::vector<MeshBatchRenderer::PosTexColorVertex> vertices;
        std::vector<uint32_t> indices;
    };

    struct PinnedTexture
    {
        resources::PinnedResourceHandle data_handle;
        bgfx::TextureHandle texture_handle = BGFX_INVALID_HANDLE;

        PinnedTexture() = default;
        PinnedTexture(const PinnedTexture&) = delete;
        PinnedTexture& operator=(const PinnedTexture&) = delete;
        PinnedTexture(PinnedTexture&& other) noexcept;
        PinnedTexture& operator=(PinnedTexture&& other) noexcept;
        ~PinnedTexture();
    };

    struct MaterialComponent
    {
        enum class LoadState
        {
            Unloaded,
            Loading,
            Loaded,
            Failed
        };

        std::string texture_atlas_alias;
        bool is_pinned = false;

        LoadState load_state = LoadState::Unloaded;

        using ResourceVariant = std::variant<std::monostate,
                                                PinnedTexture,
                                                resources::ResourceHandle<resources::TextureResource>>;
        ResourceVariant resource_handle;

        glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

        explicit MaterialComponent(std::string alias, bool pinned = false)
                : texture_atlas_alias(std::move(alias)),
                  is_pinned(pinned)
        {}
    };

    struct SpriteAnimationComponent
    {
        std::vector<SDL_FRect> frames;
        float frame_duration = 0.1f;
        float current_time = 0.0f;
        int current_frame = 0;
        bool is_playing = true;
        bool loop = true;
    };

    struct AudioSourceComponent
    {
        std::string resource_key;
        std::string bus_name = "SFX";
        float volume = 1.0f;
        bool loop = false;
        bool play_on_create = true;

        audio::VoiceHandle voice_handle = 0;
    };

    struct AudioListenerComponent {};
}
