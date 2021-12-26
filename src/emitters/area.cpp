#include <nori/emitter.h>
#include <nori/sampler.h>

NORI_NAMESPACE_BEGIN

class AreaLight : public Emitter {
private:
    Color3f m_radiance;

public:
    AreaLight(const PropertyList &propList) {
        m_radiance = propList.getColor("radiance");
    }

    Color3f eval(const EmitterQueryRecord &eRec) const {
        return Frame::cosTheta(eRec.w) > 0.f ? m_radiance : 0.0f;
    }

    Color3f sample(EmitterQueryRecord &eRec, const Point3f &sample) const {
        auto [s, n] = eRec.mesh->getSampleResult(sample);
        eRec.s = s;
        eRec.w = Frame(n).toLocal(eRec.p - eRec.s).normalized();

        return eval(eRec) / pdf(eRec);
    }

    float pdf(const EmitterQueryRecord &eRec) const {
        float cosTheta = std::abs(Frame::cosTheta(eRec.w));
        
        float squaredNorm = (eRec.p - eRec.s).squaredNorm();
        return eRec.mesh->getSamplePdf() * squaredNorm / cosTheta;;
    }

    std::string toString() const {
        return tfm::format(
            "AreaLight[\n"
            "  radiance = %s\n"
            "]",
            m_radiance.toString()
        );
    }
};

NORI_REGISTER_CLASS(AreaLight, "area")
NORI_NAMESPACE_END