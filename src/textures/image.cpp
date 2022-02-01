#include <nori/texture.h>
#include <nori/tfilter.h>
#include <nori/bitmap.h>
#include <filesystem/resolver.h>
#include <fstream>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

NORI_NAMESPACE_BEGIN

class ImageTexture : public Texture {
public:
    typedef unsigned char uchar;

    ImageTexture(const PropertyList &propList) : Texture(propList) {
        filesystem::path filename =
            getFileResolver()->resolve(propList.getString("filename"));

        /* Supported format */
        std::string ext = filename.extension();
        std::vector<std::string> support = {"png", "jpg", "tga", "bmp", "psd"};
        if (std::find(support.begin(), support.end(), ext) == support.end())
            throw NoriException("unexpected texture format \"%s\"", ext);

        std::ifstream is(filename.str());
        if (is.fail())
            throw NoriException("unable to open \"%s\"!", filename);

        int x, y, n;
        uchar *rgb8 = stbi_load(filename.str().c_str(), &x, &y, &n, 3);
        m_filename = filename.str();
        m_bitmap = new Bitmap(Point2i(x, y));

        auto dst = rgb8;
        for (int i = 0; i < m_bitmap->rows(); ++i) {
            for (int j = 0; j < m_bitmap->cols(); ++j) {
                float r = (float) dst[0] / 255.f;
                float g = (float) dst[1] / 255.f;
                float b = (float) dst[2] / 255.f;
                m_bitmap->coeffRef(i, j) = Color3f(r, g, b).toLinearRGB();
                dst += 3;
            }        
        }
        stbi_image_free(rgb8);
    }

    Color3f eval(const Point2f &uv) const {
        int x = int(m_bitmap->cols()), y = int(m_bitmap->rows());
        return m_tfilter->eval(uv, Vector2i(x, y), Point2f(-0.5f), m_bitmap);
    }

    const Bitmap *getBitmap() {
        return m_bitmap;
    }

    void addChild(NoriObject *obj) {
        switch (obj->getClassType()) {
            case ETextureFilter:
                if (m_tfilter)
                    throw NoriException("Texture: tried to register multiple texture filters!");
                m_tfilter = static_cast<TextureFilter *>(obj);
                break;

            default:
                throw NoriException("Texture::addChild(<%s>) is not supported!",
                    classTypeName(obj->getClassType()));
        }
    }
    
    void activate() {
        if (!m_tfilter)
            m_tfilter = static_cast<TextureFilter *>(
                NoriObjectFactory::createInstance("bilinear", PropertyList()));
    }

    std::string toString() const {
        return tfm::format("ImageTexture[filename=\"%s\"]", m_filename);
    }

private:
    std::string    m_filename;
    TextureFilter *m_tfilter = nullptr;
};

NORI_REGISTER_CLASS(ImageTexture, "image");
NORI_NAMESPACE_END