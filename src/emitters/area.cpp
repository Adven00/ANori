#include <objects/emitter.h>
#include <objects/texture.h>

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
        eRec.w = Frame(n).toLocal((eRec.p - eRec.s).normalized());
        eRec.wo = eRec.its.toLocal((eRec.s - eRec.p).normalized());
        eRec.measure = ESolidAngle;

        float cosTheta1 = std::max(Frame::cosTheta(eRec.wo), 0.f);
        float cosTheta2 = std::max(Frame::cosTheta(eRec.w), 0.f);
        float squaredNorm = (eRec.p - eRec.s).squaredNorm();
        float pdf = eRec.mesh->getSamplePdf();

        return m_radiance * cosTheta1 * cosTheta2 / (squaredNorm * pdf);
    }

    float pdf(const EmitterQueryRecord &eRec) const {
        float cosTheta = Frame::cosTheta(eRec.w);
        if (cosTheta <= 0.f)
            return 0.f;
        
        float squaredNorm = (eRec.p - eRec.s).squaredNorm();
        return eRec.mesh->getSamplePdf() * squaredNorm / cosTheta;
    }

     void activate() { 
        if (m_textures.find(ERadiance) == m_textures.end()) {
            /* If no texture was assigned, instantiate a solid radiance texture */
            PropertyList pl;
            pl.setColor("value", Color3f(40.f));
            pl.setString("use", "radiance");
            m_textures[ERadiance] = static_cast<Texture *>(
                NoriObjectFactory::createInstance("solid", pl));
        }
    }

    std::string toString() const {
        std::string textures;
        for (auto it : m_textures) {
            textures += std::string("  ") + indent(it.second->toString(), 2);
            if (it.second != (--m_textures.end())->second)
                textures += ",";
            textures += "\n";
        }

        return tfm::format(
            "AreaLight[\n"
            "  textures = {\n"
            "  %s  }\n"
            "]", 
            indent(textures)
        );
    }
};

NORI_REGISTER_CLASS(AreaLight, "area")
NORI_NAMESPACE_END