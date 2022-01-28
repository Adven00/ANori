#pragma once

#include <nori/object.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Abstract 2D texture
 */
class Texture2D : public NoriObject {
public:
    /**
     * \brief Return the texture value at \c uv     
     */
    virtual Color3f eval(const Point2f &uv) const = 0;

    /**
     * \brief Return a bitmap representation of the texture
     */
    virtual const Bitmap *getBitmap() const = 0;

    /**
     * \brief Return the type of object (i.e. Mesh/Camera/etc.) 
     * provided by this instance
     * */
    EClassType getClassType() const { return ETexture2D; }

protected:
    Point2f m_uvOffset;
    Vector2i m_uvScale;

    Point2f uvToTextureCoord(const Point2f &uv) const {
        return Point2f(
            uv.x() * m_uvScale.x() + m_uvOffset.x(),
            uv.y() * m_uvScale.y() + m_uvOffset.y()
        );
    }
};

NORI_NAMESPACE_END