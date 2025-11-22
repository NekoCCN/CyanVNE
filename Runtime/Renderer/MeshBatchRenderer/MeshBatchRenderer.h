#pragma once

#include <bgfx/bgfx.h>
#include <vector>
#include <glm/glm.hpp>
#include "Core/ViewID/ViewID.h"

namespace cyanvne::runtime
{
    class MeshBatchRenderer
    {
    public:
        // Matches layout in varying.def.glsl
        struct PosTexColorVertex
        {
            float x, y, z;    // a_position
            float u, v;       // a_texcoord0
            uint32_t rgba;    // a_color0 (normalized)

            static void init();
            static bgfx::VertexLayout ms_layout;
        };

        MeshBatchRenderer();
        ~MeshBatchRenderer();

        MeshBatchRenderer(const MeshBatchRenderer&) = delete;
        MeshBatchRenderer& operator=(const MeshBatchRenderer&) = delete;

        // Initialize shader program and uniforms
        void init();

        // Clears internal buffers for a new frame
        void beginFrame();

        /**
         * @brief Submits a sprite to the batch.
         * Automatically handles batch splitting on texture change.
         * @param texture Texture handle
         * @param pos World position (x, y, z)
         * @param size Sprite size (width, height)
         * @param uv_rect UV coordinates (x, y, w, h), default is full texture
         * @param color Tint color (Hex ABGR), default is white
         */
        void submitSprite(bgfx::TextureHandle texture,
                          const glm::vec3& pos,
                          const glm::vec2& size,
                          const glm::vec4& uv_rect = {0.0f, 0.0f, 1.0f, 1.0f},
                          uint32_t color = 0xffffffff);

        // Flushes all batches to the GPU
        void endFrame(core::RenderLayer layer);

    private:
        void flush(core::RenderLayer layer);

        bgfx::ProgramHandle m_program;
        bgfx::UniformHandle m_texColorUniform;

        struct BatchInfo
        {
            bgfx::TextureHandle texture;
            uint32_t startIndex;
            uint32_t numIndices;
        };

        std::vector<PosTexColorVertex> m_vertices;
        std::vector<uint16_t> m_indices;
        std::vector<BatchInfo> m_batches;

        // 16-bit index limit
        static constexpr uint32_t MAX_VERTICES = 65536;
    };
}