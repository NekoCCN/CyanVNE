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
#include "Core/ViewID/ViewID.h"
#include <variant>

namespace cyanvne::runtime
{
    struct MeshComponent
    {
        uint32_t layer_mask = 1;

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

    struct TransformComponent
    {
        glm::vec2 position = { 0.0f, 0.0f };
        glm::vec2 scale = { 1.0f, 1.0f };
        float rotation = 0.0f;
    };

    struct HierarchyComponent
    {
        entt::entity parent = entt::null;
        entt::entity first_child = entt::null;
        entt::entity prev_sibling = entt::null;
        entt::entity next_sibling = entt::null;
    };

    struct WorldTransformComponent
    {
        glm::mat4 transform{1.0f};
    };

    struct CameraComponent
    {
        core::RenderLayer render_layer = core::RenderLayer::World3D;

        glm::mat4 view_matrix{1.0f};

        glm::mat4 projection_matrix{1.0f};

        std::weak_ptr<platform::WindowContext> target_window;

        glm::vec4 viewport_rect = { 0.0f, 0.0f, 1.0f, 1.0f };

        uint32_t culling_mask = 0xFFFFFFFF;
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
