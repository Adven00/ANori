/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <objects/bsdf.h>
#include <tools/frame.h>

NORI_NAMESPACE_BEGIN

/// Ideal dielectric BSDF
class Dielectric : public BSDF {
public:
    Dielectric(const PropertyList &propList) {
        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);
    }

    Color3f eval(const BSDFQueryRecord &) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return Color3f(0.0f);
    }

    float pdf(const BSDFQueryRecord &) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return 0.0f;
    }

    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
        float cosThetaI = Frame::cosTheta(bRec.wi);
        float f = fresnel(cosThetaI, m_extIOR, m_intIOR);
        bRec.eta = m_intIOR / m_extIOR;
        bRec.measure = EDiscrete;

        /* Select reflection */
        if (sample.x() < f) {
            bRec.wo = Vector3f(-bRec.wi.x(), -bRec.wi.y(), bRec.wi.z());
            bRec.eta = 1.f;

        /* Select refraction */
        } else {
            float eta = cosThetaI < 0 ? m_intIOR / m_extIOR : m_extIOR / m_intIOR;
            float sign = cosThetaI > 0 ? -1.f : 1.f;
            float cosThetaTSqr = 1 - (1 - cosThetaI * cosThetaI) * (eta * eta); 

            /* Total Internal Reflection */
            if (cosThetaTSqr <= 0) {
                bRec.wo = Vector3f(0.f);
            } else {
                /* Orthogonal decomposition of wo */
                Vector3f n(0.f, 0.f, 1.f);
                Vector3f woPerp = sign * sqrt(cosThetaTSqr) * n;
                Vector3f woPara = eta * (cosThetaI * n - bRec.wi);
                bRec.wo = woPerp + woPara;
            }
        }

        /* It means no energy loss */
        return Color3f(1.f);
    }

    std::string toString() const {
        return tfm::format(
            "Dielectric[\n"
            "  intIOR = %f,\n"
            "  extIOR = %f\n"
            "]",
            m_intIOR, 
            m_extIOR
        );
    }
private:
    float m_intIOR, m_extIOR;
};

NORI_REGISTER_CLASS(Dielectric, "dielectric");
NORI_NAMESPACE_END
