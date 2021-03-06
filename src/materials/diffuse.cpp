/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <objects/bsdf.h>
#include <tools/frame.h>
#include <core/warp.h>
#include <objects/texture.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Diffuse / Lambertian BRDF model
 */
class Diffuse : public BSDF {
public:
    Diffuse(const PropertyList &propList) { }

    /// Evaluate the BRDF model
    Color3f eval(const BSDFQueryRecord &bRec) const {
        /* This is a smooth BRDF -- return zero if the measure
           is wrong, or when queried for illumination on the backside */
        if (bRec.measure != ESolidAngle
            || Frame::cosTheta(bRec.wi) <= 0
            || Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);

        /* The BRDF is simply the albedo / pi */
        return m_textures.at(EAlbedo)->eval(bRec.its.uv) * INV_PI;
    }

    /// Compute the density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {
        /* This is a smooth BRDF -- return zero if the measure
           is wrong, or when queried for illumination on the backside */
        if (bRec.measure != ESolidAngle
            || Frame::cosTheta(bRec.wi) <= 0
            || Frame::cosTheta(bRec.wo) <= 0)
            return 0.0f;

        /* Importance sampling density wrt. solid angles:
           cos(theta) / pi.

           Note that the directions in 'bRec' are in local coordinates,
           so Frame::cosTheta() actually just returns the 'z' component.
        */
        return INV_PI * Frame::cosTheta(bRec.wo);
    }

    /// Draw a a sample from the BRDF model
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
        if (Frame::cosTheta(bRec.wi) <= 0)
            return Color3f(0.0f);

        bRec.measure = ESolidAngle;

        /* Warp a uniformly distributed sample on [0,1]^2
           to a direction on a cosine-weighted hemisphere */
        bRec.wo = Warp::squareToCosineHemisphere(sample);

        /* Relative index of refraction: no change */
        bRec.eta = 1.0f;

        /* eval() / pdf() * cos(theta) = albedo. There
           is no need to call these functions. */
        return m_textures.at(EAlbedo)->eval(bRec.its.uv);
    }

    bool isDiffuse() const {
        return true;
    }

    void activate() { 
        if (m_textures.find(EAlbedo) == m_textures.end()) {
            /* If no texture was assigned, instantiate a solid albedo texture */
            PropertyList pl;
            pl.setColor("value", Color3f(0.5f));
            pl.setString("use", "albedo");
            m_textures[EAlbedo] = static_cast<Texture *>(
                NoriObjectFactory::createInstance("solid", pl));
        }
    }

    /// Return a human-readable summary
    std::string toString() const {
        std::string textures;
        for (auto it : m_textures) {
            textures += std::string("  ") + indent(it.second->toString(), 2);
            if (it.second != (--m_textures.end())->second)
                textures += ",";
            textures += "\n";
        }

        return tfm::format(
            "Diffuse[\n"
            "  textures = {\n"
            "  %s  }\n"
            "]", 
            indent(textures)
        );
    }

    EClassType getClassType() const { return EBSDF; }
};

NORI_REGISTER_CLASS(Diffuse, "diffuse");
NORI_NAMESPACE_END
