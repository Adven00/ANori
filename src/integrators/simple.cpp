#include <objects/integrator.h>
#include <objects/scene.h>

NORI_NAMESPACE_BEGIN

class PointLightIntegrator : public Integrator {
public:
    PointLightIntegrator(const PropertyList &props) {
        m_position = props.getPoint("position", Point3f(-20.f, 40.f, 20.f));
        m_energy = props.getColor("energy", Color3f(3.76e4));
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        Intersection its;
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        Vector3f wi = (m_position - its.p).normalized();
        Ray3f shadowRay = Ray3f(its.p, wi);
        if (scene->rayIntersect(shadowRay))
            return Color3f(0.0f);

        auto cosTheta = Frame::cosTheta(its.shFrame.toLocal(wi));
        float attenuation = std::max(0.f, cosTheta) / std::pow((m_position - its.p).norm(), 2.0f);
        Color3f color = m_energy * INV_PI * INV_FOURPI;

        return color * attenuation;
    }

    std::string toString() const {
        return tfm::format(
            "  PointLightIntegrator[\n"
            "  position = %s,\n"
            "  energy = %s\n"
            "]",
            m_position.toString(),
            m_energy.toString()
        );
    }
private:
    Point3f m_position;
    Color3f m_energy;
};

NORI_REGISTER_CLASS(PointLightIntegrator, "simple");
NORI_NAMESPACE_END