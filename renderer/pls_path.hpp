/*
 * Copyright 2022 Rive
 */

#pragma once

#include "rive/math/raw_path.hpp"
#include "rive/renderer.hpp"

namespace rive::pls
{
// RenderPath implementation for Rive's pixel local storage renderer.
class PLSPath : public lite_rtti_override<RenderPath, PLSPath>
{
public:
    PLSPath() = default;
    PLSPath(FillRule fillRule, RawPath& rawPath);

    void rewind() override;
    void fillRule(FillRule rule) override { m_fillRule = rule; }

    void moveTo(float x, float y) override;
    void lineTo(float x, float y) override;
    void cubicTo(float ox, float oy, float ix, float iy, float x, float y) override;
    void close() override;

    void addPath(CommandPath* p, const Mat2D& m) override { addRenderPath(p->renderPath(), m); }
    void addRenderPath(RenderPath* path, const Mat2D& matrix) override;

    const RawPath& getRawPath() const { return m_rawPath; }
    FillRule getFillRule() const { return m_fillRule; }

    const AABB& getBounds() const;
    uint64_t getRawPathMutationID() const;

#ifdef DEBUG
    // Allows ref holders to guarantee the rawPath doesn't mutate during a specific time.
    void lockRawPathMutations() const { ++m_rawPathMutationLockCount; }
    void unlockRawPathMutations() const
    {
        assert(m_rawPathMutationLockCount > 0);
        --m_rawPathMutationLockCount;
    }
#endif

private:
    FillRule m_fillRule = FillRule::nonZero;
    RawPath m_rawPath;
    mutable AABB m_bounds;
    mutable uint64_t m_rawPathMutationID;

    enum Dirt
    {
        kPathBoundsDirt = 1,
        kRawPathMutationIDDirt = 4,
        kAllDirt = ~0,
    };

    mutable uint32_t m_dirt = kAllDirt;
    RIVE_DEBUG_CODE(mutable int m_rawPathMutationLockCount = 0;)
};
} // namespace rive::pls
