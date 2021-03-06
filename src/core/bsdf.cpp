#include <objects/bsdf.h>
#include <objects/texture.h>

NORI_NAMESPACE_BEGIN

void BSDF::addChild(NoriObject *obj) {
    if (obj->getClassType() != ETexture)
        throw NoriException("BSDF::addChild(<%s>) is not supported!",
                            classTypeName(obj->getClassType()));

    Texture *texture = static_cast<Texture *>(obj);
    ETextureUse use = texture->getTextureUse();

    if (m_textures.find(use) != m_textures.end())
        throw NoriException(
            "BSDF: tried to register multiple Texture instances!");

    if (use == EUnknownUse)
        throw NoriException(
            "BSDF: tried to register Texture instance with unkonwn use!");

    m_textures[use] = texture;
}

BSDF::~BSDF() {
    for (auto it : m_textures) {
        if (it.second) delete it.second;
    }
}

NORI_NAMESPACE_END