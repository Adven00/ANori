#include <nori/texture.h>
#include <nori/bitmap.h>
#include <map>

NORI_NAMESPACE_BEGIN

Texture::Texture(const PropertyList &propList) { 
    std::string use = propList.getString("use", "diffuse");

    std::map<std::string, ETextureUse> textures;
    textures["diffuse"] = EDiffuse;

    auto it = textures.find(use);
    if (it == textures.end()) 
        m_use = EUnknownUse;
    else
        m_use = it->second;
}

Texture::~Texture() { 
    if (m_bitmap) delete m_bitmap; 
}

NORI_NAMESPACE_END