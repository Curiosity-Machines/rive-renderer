/*
 * Copyright 2022 Rive
 */

#include "rive/pls/gl/pls_render_context_gl_impl.hpp"

#include "shaders/constants.glsl"

#include <stdio.h>

#ifdef RIVE_WEBGL
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include "../out/obj/generated/glsl.exports.h"

namespace rive::pls
{
#ifndef GL_WEBGL_shader_pixel_local_storage

// WEBGL_shader_pixel_local_storage bindings aren't in mainline emcsripten yet. Don't implement this
// interface if we don't have bindings.
std::unique_ptr<PLSRenderContextGLImpl::PLSImpl> PLSRenderContextGLImpl::MakePLSImplWebGL()
{
    return nullptr;
}

#else

class PLSRenderContextGLImpl::PLSImplWebGL : public PLSRenderContextGLImpl::PLSImpl
{
    rcp<PLSRenderTargetGL> wrapGLRenderTarget(GLuint framebufferID,
                                              size_t width,
                                              size_t height,
                                              const PlatformFeatures&) override
    {
        // WEBGL_shader_pixel_local_storage can't load or store to framebuffers.
        return nullptr;
    }

    rcp<PLSRenderTargetGL> makeOffscreenRenderTarget(
        size_t width,
        size_t height,
        const PlatformFeatures& platformFeatures) override
    {
        auto renderTarget = rcp(new PLSRenderTargetGL(width, height, platformFeatures));
        renderTarget->allocateCoverageBackingTextures();
        glFramebufferTexturePixelLocalStorageWEBGL(FRAMEBUFFER_PLANE_IDX,
                                                   renderTarget->m_offscreenTextureID,
                                                   0,
                                                   0);
        glFramebufferTexturePixelLocalStorageWEBGL(COVERAGE_PLANE_IDX,
                                                   renderTarget->m_coverageTextureID,
                                                   0,
                                                   0);
        glFramebufferTexturePixelLocalStorageWEBGL(ORIGINAL_DST_COLOR_PLANE_IDX,
                                                   renderTarget->m_originalDstColorTextureID,
                                                   0,
                                                   0);
        glFramebufferTexturePixelLocalStorageWEBGL(CLIP_PLANE_IDX,
                                                   renderTarget->m_clipTextureID,
                                                   0,
                                                   0);
        renderTarget->createSideFramebuffer();
        return renderTarget;
    }

    void activatePixelLocalStorage(PLSRenderContextGLImpl*,
                                   const PLSRenderContext::FlushDescriptor& desc) override
    {
        auto renderTarget = static_cast<const PLSRenderTargetGL*>(desc.renderTarget);
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->drawFramebufferID());

        if (desc.loadAction == LoadAction::clear)
        {
            float clearColor4f[4];
            UnpackColorToRGBA32F(desc.clearColor, clearColor4f);
            glFramebufferPixelLocalClearValuefvWEBGL(FRAMEBUFFER_PLANE_IDX, clearColor4f);
        }

        GLenum loadOps[4] = {(GLenum)(desc.loadAction == LoadAction::clear ? GL_LOAD_OP_CLEAR_WEBGL
                                                                           : GL_LOAD_OP_LOAD_WEBGL),
                             GL_LOAD_OP_ZERO_WEBGL,
                             GL_DONT_CARE,
                             (GLenum)(desc.needsClipBuffer ? GL_LOAD_OP_ZERO_WEBGL : GL_DONT_CARE)};

        glBeginPixelLocalStorageWEBGL(4, loadOps);
    }

    void deactivatePixelLocalStorage(PLSRenderContextGLImpl*) override
    {
        constexpr static GLenum kStoreOps[4] = {GL_STORE_OP_STORE_WEBGL,
                                                GL_DONT_CARE,
                                                GL_DONT_CARE,
                                                GL_DONT_CARE};
        glEndPixelLocalStorageWEBGL(4, kStoreOps);
    }

    const char* shaderDefineName() const override { return GLSL_PLS_IMPL_WEBGL; }
};

std::unique_ptr<PLSRenderContextGLImpl::PLSImpl> PLSRenderContextGLImpl::MakePLSImplWebGL()
{
    return std::make_unique<PLSImplWebGL>();
}
#endif
} // namespace rive::pls
