#pragma once

#include <nori/object.h>

NORI_NAMESPACE_BEGIN

/**
 * Texture filtering or texture smoothing is the method 
 * used to determine the texture for a texture mapped pixel
 */
class TextureFilter : public NoriObject {
public:
    /// Evaluate the filter function
    virtual Color3f eval(const Point2f &uv, const Vector2i &scale, const Point2f &offest, const Bitmap *m) const = 0;

    /**
     * \brief Return the type of object (i.e. Mesh/Camera/etc.) 
     * provided by this instance
     * */
    EClassType getClassType() const { return ETextureFilter; }
};

NORI_NAMESPACE_END
