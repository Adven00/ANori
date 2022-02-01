#include <nori/texture.h>
#include <nori/bitmap.h>

NORI_NAMESPACE_BEGIN

class SolidTexture : public Texture {
public:
    SolidTexture(const PropertyList &propList) : Texture(propList) {
        m_color = propList.getColor("albedo", Color3f(0.5f));
    }

    Color3f eval(const Point2f &uv) const {
        return m_color;
    }

    const Bitmap *getBitmap() {
        m_bitmap = new Bitmap(Vector2i(50, 50));

        for (int i = 0; i < m_bitmap->rows(); ++i)
            for (int j = 0; j < m_bitmap->cols(); ++j)
                m_bitmap->coeffRef(i, j) = m_color;

        return m_bitmap;
    }

    /// Return a human-readable summary
    std::string toString() const {
        return tfm::format(
            "SolidTexture[color=%s]", 
            m_color.toString()
        );
    }

private:
    Color3f m_color;
};

NORI_REGISTER_CLASS(SolidTexture, "solid");
NORI_NAMESPACE_END