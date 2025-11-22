//
// Created by unirz on 2025/8/24.
//

#include "MeshBatchRenderer.h"
#include "Runtime/Components/Components.h"
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <vector>

#include <bgfx/embedded_shader.h>
#include <variant>

#include "Shaders/original_sprite/bin/glsl/vs_sprite.glsl.bin.h"
#include "Shaders/original_sprite/bin/glsl/fs_sprite.glsl.bin.h"
#include "Shaders/original_sprite/bin/essl/vs_sprite.glsl.bin.h"
#include "Shaders/original_sprite/bin/essl/fs_sprite.glsl.bin.h"
#include "Shaders/original_sprite/bin/spirv/vs_sprite.glsl.bin.h"
#include "Shaders/original_sprite/bin/spirv/fs_sprite.glsl.bin.h"

#if defined(_WIN32)
#include "Shaders/original_sprite/bin/dx11/vs_sprite.glsl.bin.h"
#include "Shaders/original_sprite/bin/dx11/fs_sprite.glsl.bin.h"
#include "Resources/UnifiedCacheManager/UnifiedCacheManager.h"

#endif

// Since bgfx does not support Metal on Windows, we exclude these headers on non-Apple platforms
// #if __APPLE__
// #include "Shaders/original_sprite/bin/mtl/vs_sprite.glsl.bin.h"
// #include "Shaders/original_sprite/bin/mtl/fs_sprite.glsl.bin.h"
// #endif

static const bgfx::EmbeddedShader s_embeddedShaders[] =
        {
                BGFX_EMBEDDED_SHADER(vs_sprite),
                BGFX_EMBEDDED_SHADER(fs_sprite),
                BGFX_EMBEDDED_SHADER_END()
        };

namespace cyanvne::runtime
{
    bgfx::VertexLayout MeshBatchRenderer::PosTexColorVertex::ms_layout;

    void MeshBatchRenderer::PosTexColorVertex::init()
    {
        ms_layout
                .begin()
                .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
                .end();
    }

    MeshBatchRenderer::MeshBatchRenderer()
            : m_program(BGFX_INVALID_HANDLE),
              m_texColorUniform(BGFX_INVALID_HANDLE)
    {
    }

    MeshBatchRenderer::~MeshBatchRenderer()
    {
        if (bgfx::isValid(m_texColorUniform))
        {
            bgfx::destroy(m_texColorUniform);
        }
        if (bgfx::isValid(m_program))
        {
            bgfx::destroy(m_program);
        }
    }

    void MeshBatchRenderer::init()
    {
        PosTexColorVertex::init();

        m_texColorUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

        bgfx::RendererType::Enum type = bgfx::getRendererType();

        bgfx::ShaderHandle vs = bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_sprite");
        bgfx::ShaderHandle fs = bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_sprite");

        m_program = bgfx::createProgram(vs, fs, true);

        if (!bgfx::isValid(m_program))
        {
            core::GlobalLogger::getCoreLogger()->critical("MeshBatchRenderer: Failed to create shader program!");
        }
    }

    void MeshBatchRenderer::beginFrame()
    {
        m_vertices.clear();
        m_indices.clear();
        m_batches.clear();
    }

    void MeshBatchRenderer::submitSprite(bgfx::TextureHandle texture,
                                         const glm::vec3& pos,
                                         const glm::vec2& size,
                                         const glm::vec4& uv_rect,
                                         uint32_t color)
    {
        if (!bgfx::isValid(texture))
        {
            return;
        }

        // Check if a new batch is needed (texture change or buffer overflow)
        bool needsNewBatch = m_batches.empty() ||
                             m_batches.back().texture.idx != texture.idx ||
                             m_vertices.size() + 4 > MAX_VERTICES;

        if (needsNewBatch)
        {
            BatchInfo newBatch;
            newBatch.texture = texture;
            newBatch.startIndex = static_cast<uint32_t>(m_indices.size());
            newBatch.numIndices = 0;
            m_batches.push_back(newBatch);
        }

        BatchInfo& currentBatch = m_batches.back();
        uint16_t startVtx = static_cast<uint16_t>(m_vertices.size());

        float u0 = uv_rect.x;
        float v0 = uv_rect.y;
        float u1 = uv_rect.x + uv_rect.z; // x + w
        float v1 = uv_rect.y + uv_rect.w; // y + h

        // 0: TL, 1: TR, 2: BR, 3: BL
        m_vertices.push_back({pos.x,          pos.y,          pos.z, u0, v0, color}); // 0
        m_vertices.push_back({pos.x + size.x, pos.y,          pos.z, u1, v0, color}); // 1
        m_vertices.push_back({pos.x + size.x, pos.y + size.y, pos.z, u1, v1, color}); // 2
        m_vertices.push_back({pos.x,          pos.y + size.y, pos.z, u0, v1, color}); // 3

        m_indices.push_back(startVtx + 0);
        m_indices.push_back(startVtx + 3);
        m_indices.push_back(startVtx + 2);

        m_indices.push_back(startVtx + 0);
        m_indices.push_back(startVtx + 2);
        m_indices.push_back(startVtx + 1);

        currentBatch.numIndices += 6;
    }

    void MeshBatchRenderer::flush(core::RenderLayer layer)
    {
        if (m_vertices.empty() || m_batches.empty())
        {
            return;
        }

        uint32_t numVertices = static_cast<uint32_t>(m_vertices.size());
        uint32_t numIndices = static_cast<uint32_t>(m_indices.size());

        if (bgfx::getAvailTransientVertexBuffer(numVertices, PosTexColorVertex::ms_layout) < numVertices ||
            bgfx::getAvailTransientIndexBuffer(numIndices) < numIndices)
        {
            core::GlobalLogger::getCoreLogger()->warn("MeshBatchRenderer: Transient buffer overflow!");
            return;
        }

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;

        bgfx::allocTransientVertexBuffer(&tvb, numVertices, PosTexColorVertex::ms_layout);
        bgfx::allocTransientIndexBuffer(&tib, numIndices);

        std::memcpy(tvb.data, m_vertices.data(), numVertices * sizeof(PosTexColorVertex));
        std::memcpy(tib.data, m_indices.data(), numIndices * sizeof(uint16_t));

        for (const auto& batch : m_batches)
        {
            if (batch.numIndices == 0) continue;

            uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
                             BGFX_STATE_DEPTH_TEST_LESS |
                             BGFX_STATE_BLEND_ALPHA;

            bgfx::setState(state);

            bgfx::setTexture(0, m_texColorUniform, batch.texture);
            bgfx::setVertexBuffer(0, &tvb);
            bgfx::setIndexBuffer(&tib, batch.startIndex, batch.numIndices);

            bgfx::submit(static_cast<bgfx::ViewId>(layer), m_program);
        }
    }

    void MeshBatchRenderer::endFrame(core::RenderLayer layer)
    {
        flush(layer);
    }
}