#pragma once

#include <objects/object.h>

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
    virtual const Bitmap *getBitmap() { return m_bitmap; };

    /// Return texture use
    ETextureUse getTextureUse() const { return m_use; }

    /// Return texture use accroding to string
    static ETextureUse getTextureUse(std::string use);

    /// Turn a texture use into a human-readable string
    static std::string textureUseName(ETextureUse use) {
        switch (use) {
            case EAlbedo:     return "albedo";
            case ERadiance:   return "radiance";
            case EMetalness:  return "metalness";
            case ERoughness:  return "roughness";
            case EGlossiness: return "glossiness";
            case EUnknownUse: return "<unknown>";
            default         : return "<unknown>";
        }
    }

    /// Return class type
    EClassType getClassType() const { return ETexture; }

    ~Texture();

protected:
    Bitmap *m_bitmap = nullptr;
    ETextureUse m_use;
};

NORI_NAMESPACE_END