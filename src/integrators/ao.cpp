#include <objects/integrator.h>
#include <objects/scene.h>
#include <core/warp.h>

NORI_NAMESPACE_BEGIN

class AmbientLightIntegrator : public Integrator {
public:
    AmbientLightIntegrator(const PropertyList &props) { }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        Intersection its;
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        Vector3f sampleDir = Warp::squareToCosineHemisphere(sampler->next2D());
        Vector3f outDir = its.toWorld(sampleDir).normalized();
        Ray3f shadowRay = Ray3f(its.p, outDir);
        int visiblity = scene->rayIntersect(shadowRay) ? 0 : 1;

        return Color3f(float(visiblity));
    }

    std::string toString() const {
        return "AmbientLightIntegrator[]";
    }
};

NORI_REGISTER_CLASS(AmbientLightIntegrator, "ao");
NORI_NAMESPACE_END