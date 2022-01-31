#include <nori/tfilter.h>
#include <nori/bitmap.h>

NORI_NAMESPACE_BEGIN

class BilinearFilter : public TextureFilter {
public:
    BilinearFilter(const PropertyList &propList) {
        /* No parameters this time */
    }

    Color3f eval(const Point2f &uv, const Vector2i &scale, const Point2f &offest, const Bitmap *m) const {
        Vector2f p(uv.x() * scale.x() + offest.x(), uv.y() * scale.y() + offest.y());
        Vector2f t(p.x() - floorf(p.x()), p.y() - floorf(p.y()));

        /* UV warp method is repeat */
        int x = (int)(p.x()) % m->cols();
        int y = (int)(p.y()) % m->rows();
        x = x < 0 ? int(m->cols()) + x : x;
        y = y < 0 ? int(m->rows()) + y : y;
        x = (x == m->cols() - 1) ? 0 : x;
        y = (y == m->rows() - 1) ? 0 : y;

        Color3f cb = lerp<Color3f>(t.x(), m->coeff(x, y), m->coeff(x + 1, y));
        Color3f ct = lerp<Color3f>(t.x(), m->coeff(x, y + 1), m->coeff(x + 1, y + 1));
        Color3f c = lerp<Color3f>(t.y(), cb, ct);
        return c;
    }

    std::string toString() const {
        return "BilinearFilter[]";
    }
};

class NearestFilter : public TextureFilter {
public:
    NearestFilter(const PropertyList &propList) {
        /* No parameters this time */
    }

    Color3f eval(const Point2f &uv, const Vector2i &scale, const Point2f &offest, const Bitmap *m) const {
        Vector2f p(uv.x() * scale.x() + offest.x(), uv.y() * scale.y() + offest.y());
        int x = (int)(p.x()) % m->cols();
        int y = (int)(p.y()) % m->rows();
        x = x < 0 ? int(m->cols()) + x : x;
        y = y < 0 ? int(m->rows()) + y : y;

        return m->coeff(x, y);
    }

    std::string toString() const {
        return "NearestFilter[]";
    }
};

NORI_REGISTER_CLASS(BilinearFilter, "bilinear")
NORI_REGISTER_CLASS(NearestFilter, "nearest")

NORI_NAMESPACE_END