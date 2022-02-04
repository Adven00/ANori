#include <objects/texture.h>
#include <core/bitmap.h>
#include <map>

NORI_NAMESPACE_BEGIN

ETextureUse Texture::getTextureUse(std::string use) {
    std::map<std::string, ETextureUse> textures;
    textures["albedo"] = EAlbedo;
    textures["radiance"] = ERadiance;

    auto it = textures.find(use);
    if (it == textures.end()) 
        return EUnknownUse;
    else
        return it->second;
}



Texture::~Texture() { 
    if (m_bitmap) delete m_bitmap; 
}

NORI_NAMESPACE_END