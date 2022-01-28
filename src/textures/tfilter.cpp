#include <nori/tfilter.h>
#include <nori/bitmap.h>

NORI_NAMESPACE_BEGIN

class BilinearFilter : public TextureFilter {
public:
    BilinearFilter(const PropertyList &propList) {
        /* No parameters this time */
    }

    Color3f eval(const Point2f &p, const Bitmap *m) const {
        Vector2f t(p.x() - ceilf(p.x()), p.y() - ceilf(p.y()));
        int x = (int)(p.x()) % m->cols();
        int y = (int)(p.y()) % m->rows();
        x = x < 0 ? int(m->cols()) + x : x;
        y = y < 0 ? int(m->rows()) + y : y;

        Color3f cb = lerp<Color3f>(0.1f, m->coeff(x, y), m->coeff(x + 1, y));
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

    Color3f eval(const Point2f &p, const Bitmap *m) const {
        Vector2f t(p.x() - ceilf(p.x()), p.y() - ceilf(p.y()));
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