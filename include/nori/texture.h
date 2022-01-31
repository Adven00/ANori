#pragma once

#include <nori/object.h>
#include <nori/bitmap.h>
#include <map>

NORI_NAMESPACE_BEGIN

/**
 * \brief Abstract 2D texture
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

    Texture(const PropertyList &propList) { 
        std::string use = propList.getString("use", "diffuse");

        std::map<std::string, ETextureUse> textures;
        textures["diffuse"] = ETextureUse::EDiffuse;

        auto it = textures.find(use);
        if (it == textures.end()) 
            throw NoriException("Texture: unexpected texture use \"%s\"", use);
        m_use = it->second;
    }

    ~Texture() { 
        if (m_bitmap) delete m_bitmap; 
    }

protected:
    Bitmap *m_bitmap = nullptr;
    ETextureUse m_use;
};

NORI_NAMESPACE_END