#pragma once

#include <nori/object.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Abstract 2D texture. Subclass type (image, solid, etc.) 
 * means where the texture infomation comes from, and ETextureUse
 * means where the texture wiil be used for (diffuse, environment, etc.)
 */
class Texture : public NoriObject {
public:
    /// Return the texture value at uv     
    virtual Color3f eval(const Point2f &uv) const = 0;

    /// Return a bitmap representation of the texture
    virtual const Bitmap *getBitmap() = 0;

    /// Return texture use
    ETextureUse getTextureUse() const { return m_use; }

    /// Return class type
    EClassType getClassType() const { return ETexture; }

    Texture(const PropertyList &propList);

    ~Texture();

protected:
    Bitmap *m_bitmap = nullptr;
    ETextureUse m_use;
};

NORI_NAMESPACE_END