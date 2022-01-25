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
        Color3f le(0.0f);
        if (its.mesh->isEmitter()) {
            EmitterQueryRecord rec(ray.o, its.mesh);
            rec.w = its.toLocal(-ray.d.normalized());
            le = rec.mesh->getEmitter()->eval(rec);
        }

        /* Sampling the light source if diffuse */
        if (its.mesh->getBSDF() && its.mesh->getBSDF()->isDiffuse()) {
            /* Sample a light from scene (uniform) */
            auto light = scene->sampleLight(sampler->next1D());

            /* Direct illumination from light to mesh */
            EmitterQueryRecord eRec(its, light);
            Color3f color = light->getEmitter()->sample(eRec, sampler->next3D());

            /* If there is occluder between light and mesh */
            Ray3f shadowRay(eRec.p, its.toWorld(eRec.wo), Epsilon, (eRec.s - eRec.p).norm() - Epsilon);
            color = scene->rayIntersect(shadowRay) ? Color3f(0.f) : color;           

            /* Compute BSDF of the surface */
            BSDFQueryRecord bRec(its.toLocal(-ray.d), eRec.wo, ESolidAngle);
            Color3f f = its.mesh->getBSDF()->eval(bRec);

            /* Compute light sampling probability*/
            float pdf = 1.0f / (float)scene->getLights().size();

            return le + color * f / pdf;

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