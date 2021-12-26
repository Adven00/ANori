#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <nori/integrator.h>
#include <nori/sampler.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN

class WhittedIntegrator : public Integrator {
public:
    WhittedIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        /* Find the surface that is visible in the requested direction */
        Intersection its;
        if (!scene->rayIntersect(ray, its)) {
            return Color3f(0.f);
        }

        /* Direct illumination from light to camera */
        Color3f Le(0.0f);
        if (its.mesh->isEmitter()) {
            EmitterQueryRecord rec(ray.o, its.mesh);
            rec.w = its.toLocal(-ray.d.normalized());
            Le = rec.mesh->getEmitter()->eval(rec);
        }

        /* Sampling the light source if diffuse */
        if (its.mesh->getBSDF()->isDiffuse()) {
            /* Sample a light from scene (uniform) */
            auto light = scene->getSampledEmitter(sampler->next1D());

            /* Direct illumination from light to mesh */
            EmitterQueryRecord eRec(its.p, light);
            Color3f Li = light->getEmitter()->sample(eRec, sampler->next3D());

            /* If there is occluder between light and mesh */
            Ray3f shadowRay(eRec.p, (eRec.s - eRec.p).normalized(), Epsilon, (eRec.s - eRec.p).norm() - Epsilon);
            Li = scene->rayIntersect(shadowRay) ? Color3f(0.f) : Li;           

            /* Compute BSDF of the surface */
            Vector3f wo = its.toLocal(eRec.s - eRec.p).normalized();
            BSDFQueryRecord bRec(its.toLocal(-ray.d), wo, ESolidAngle);
            Color3f bsdf = its.mesh->getBSDF()->eval(bRec);

            /* Compute cosine term and probability*/
            float cosTheta = std::max(Frame::cosTheta(wo), 0.f);
            float pdf = 1.0f / (float)scene->getEmitters().size();

            return Le + Li * bsdf * cosTheta / pdf;

        /* Otherwise change the ray direction */
        } else {
            BSDFQueryRecord bRec(its.toLocal(-ray.d));
            Color3f color = its.mesh->getBSDF()->sample(bRec, sampler->next2D());

            /* Using russian roulette to control the recursion depth */
            float RR = 0.95f;
            if (sampler->next1D() < RR && color.x() > 0.f) {
                Ray3f newRay(its.p, its.toWorld(bRec.wo));
                return Li(scene, sampler, newRay) / RR * color;
            } else {
                return Color3f(0.0f);
            }
        }
    }

    std::string toString() const {
        return "WhittedIntegrator[]";
    }
};


NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END