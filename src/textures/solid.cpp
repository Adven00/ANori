#include <objects/texture.h>
#include <core/bitmap.h>

NORI_NAMESPACE_BEGIN

class SolidTexture : public Texture {
public:
    SolidTexture(const PropertyList &propList) {
        m_value = propList.getColor("value");
        
        std::string use = propList.getString("use");
        m_use = getTextureUse(use);

        m_bitmap = new Bitmap(Vector2i(50, 50));
        for (int i = 0; i < m_bitmap->rows(); ++i)
            for (int j = 0; j < m_bitmap->cols(); ++j)
                m_bitmap->coeffRef(i, j) = m_value;
    }

    Color3f eval(const Point2f &uv) const {
        return m_value;
    }

    /// Return a human-readable summary
    std::string toString() const {
        return tfm::format(
            "SolidTexture[value=%s, use=%s]", 
            m_value.toString(),
            textureUseName(m_use)
        );
    }

private:
    Color3f m_value;
};

NORI_REGISTER_CLASS(SolidTexture, "solid");
NORI_NAMESPACE_END