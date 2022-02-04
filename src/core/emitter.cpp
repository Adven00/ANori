#include <objects/emitter.h>
#include <objects/texture.h>

NORI_NAMESPACE_BEGIN

void Emitter::addChild(NoriObject *obj) {
    if (obj->getClassType() != ETexture)
        throw NoriException("Emitter::addChild(<%s>) is not supported!",
                            classTypeName(obj->getClassType()));

    Texture *texture = static_cast<Texture *>(obj);
    ETextureUse use = texture->getTextureUse();

    if (m_textures.find(use) != m_textures.end())
        throw NoriException(
            "Emitter: tried to register multiple Texture instances!");

    if (use == EUnknownUse)
        throw NoriException(
            "Emitter: tried to register Texture instance with unkonwn use!");

    m_textures[use] = texture;
}

Emitter::~Emitter() {
    for (auto it : m_textures) {
        if (it.second) delete it.second;
    }
}

NORI_NAMESPACE_END