/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <objects/bsdf.h>
#include <tools/frame.h>
#include <core/warp.h>

NORI_NAMESPACE_BEGIN

class Microfacet : public BSDF {
public:
    Microfacet(const PropertyList &propList) {
        /* RMS surface roughness */
        m_alpha = propList.getFloat("alpha", 0.1f);

        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);

        /* Albedo of the diffuse base material (a.k.a "kd") */
        m_kd = propList.getColor("kd", Color3f(0.5f));

        /* To ensure energy conservation, we must scale the 
           specular component by 1-kd. 

           While that is not a particularly realistic model of what 
           happens in reality, this will greatly simplify the 
           implementation. Please see the course staff if you're 
           interested in implementing a more realistic version 
           of this BRDF. */
        m_ks = 1 - m_kd.maxCoeff();
    }

    /// Evaluate the BRDF for the given pair of directions
    Color3f eval(const BSDFQueryRecord &bRec) const {
        /* Diffuse part */
    	Color3f diffuse = m_kd * INV_PI;

        /* Specular part
           d: density of noraml distribution
           f: fresnel reflection coefficient
           g: geo shadowing term */ 
        Vector3f wh = (bRec.wi + bRec.wo).normalized();
        float d = Warp::squareToBeckmannPdf(wh, m_alpha);
        float f = fresnel(wh.dot(bRec.wi), m_extIOR, m_intIOR);

        float cosThetaI = Frame::cosTheta(bRec.wi);
        float cosThetaO = Frame::cosTheta(bRec.wo);
        float cosThetaH = Frame::cosTheta(wh);

        auto g1 = [&](const Vector3f &wv, const Vector3f &wh) {
            if (wv.dot(wh) / Frame::cosTheta(wv) <= 0.f) {
                return 0.f;
            } else {
                float b = 1.0f / (m_alpha * Frame::tanTheta(wv));
                return b >= 1.6f ? 1 :
                    (3.535f * b + 2.181f * b * b) / (1.f + 2.276f * b + 2.577f * b * b);
            }
        };

        float g = g1(bRec.wi, wh) * g1(bRec.wo, wh);
        float specular = d * f * g / (4.f * cosThetaH * cosThetaI * cosThetaO);

        return diffuse + m_ks * specular;
    }

    /// Evaluate the sampling density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {
    	if (bRec.wo.z() <= 0)
            return 0;

        Vector3f wh = (bRec.wi + bRec.wo).normalized();
        float d = Warp::squareToBeckmannPdf(wh, m_alpha);
        float j = 1 / (4.0f * wh.dot(bRec.wo));
        return m_ks * d * j + (1 - m_ks) * Frame::cosTheta(bRec.wo) * INV_PI;
    }

    /// Sample the BRDF
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample_) const {
        if (Frame::cosTheta(bRec.wi) <= 0) 
            return Color3f(0.0f);

        if (sample_.x() > m_ks) {
            Point2f sample((sample_.x() - m_ks) / (1.f - m_ks), sample_.y());
            bRec.wo = Warp::squareToCosineHemisphere(sample);

        } else {
            Point2f sample(sample_.x() / m_ks, sample_.y());
            Vector3f wh = Warp::squareToBeckmann(sample, m_alpha);
            bRec.wo = ((2.0f * wh.dot(bRec.wi) * wh) - bRec.wi).normalized();
        }

        if (bRec.wo.z() <= 0.f)
            return Color3f(0.0f);

        /* Note that result is multiplied by the cosine factor from the reflection equation */
        return eval(bRec) * Frame::cosTheta(bRec.wo) / pdf(bRec);
    }

    bool isDiffuse() const {
        /* While microfacet BRDFs are not perfectly diffuse, they can be
           handled by sampling techniques for diffuse/non-specular materials,
           hence we return true here */
        return true;
    }

    std::string toString() const {
        return tfm::format(
            "Microfacet[\n"
            "  alpha = %f,\n"
            "  intIOR = %f,\n"
            "  extIOR = %f,\n"
            "  kd = %s,\n"
            "  ks = %f\n"
            "]",
            m_alpha,
            m_intIOR,
            m_extIOR,
            m_kd.toString(),
            m_ks
        );
    }
private:
    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks;
    Color3f m_kd;
};

NORI_REGISTER_CLASS(Microfacet, "microfacet");
NORI_NAMESPACE_END
