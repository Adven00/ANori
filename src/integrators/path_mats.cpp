#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <nori/integrator.h>
#include <nori/sampler.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN

/// Material sampling
class PathMatsIntegrator : public Integrator {
public:
    PathMatsIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        Intersection its;
        Ray3f nextRay(ray);
        Color3f result(0.f);
        Color3f throughOutput(1.f);

        /* Used for russian roulette heuristic */
        float RR = 1.f;
        int depth = 0;
        float etaProduct = 1.f;

        while (sampler->next1D() < RR) {
            if (!scene->rayIntersect(nextRay, its))
                break;

            if (its.mesh->isEmitter()) {
                EmitterQueryRecord eRec(nextRay.o, its.mesh);
                eRec.w = its.toLocal(-nextRay.d.normalized());
                result += its.mesh->getEmitter()->eval(eRec) * throughOutput;
            }

            /* Sampling the next ray accroding to BSDF */
            BSDFQueryRecord bRec(its.toLocal(-nextRay.d));
            Color3f color = its.mesh->getBSDF()->sample(bRec, sampler->next2D());
            nextRay = Ray3f(its.p, its.toWorld(bRec.wo));

            /* Fail to sample BSDF */
            if (color.isZero())
                break;

            /* Russian roulette */
            etaProduct *= bRec.eta * bRec.eta;
            RR = ++depth > 4 ? std::min(0.99f, throughOutput.maxCoeff() * etaProduct) : 1.f;
            throughOutput *= color / RR;
        }

        return result;
    }

    std::string toString() const {
        return "PathMatsIntegrator[]";
    }
};


NORI_REGISTER_CLASS(PathMatsIntegrator, "path_mats");
NORI_NAMESPACE_END


//    Color3f Li1(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
//        /* Used for russian roulette heuristic */
//        static int recursionDepth = 1;
//
//        /* Find the surface that is visible in the requested direction */
//        Intersection its;
//        if (!scene->rayIntersect(ray, its))
//            return Color3f(0.f);
//
//        /* Direct illumination from light to camera */
//        Color3f le(0.0f);
//        if (its.mesh->isEmitter()) {
//            EmitterQueryRecord eRec(ray.o, its.mesh);
//            eRec.w = its.toLocal(-ray.d.normalized());
//            le = eRec.mesh->getEmitter()->eval(eRec);
//        }
//
//        if (!its.mesh->getBSDF())
//            return le;
//
//        /* Sampling according to the BSDF */
//        BSDFQueryRecord bRec(its.toLocal(-ray.d));
//        Color3f color = its.mesh->getBSDF()->sample(bRec, sampler->next2D());
//        Ray3f newRay(its.p, its.toWorld(bRec.wo));
//
//        /* Using russian roulette to control the recursion depth 
//           Note that cosine factor is multiplied in BSDF->sample() */
//        float RR = 0.8f;
//        if (sampler->next1D() < RR) {
//            return Li(scene, sampler, newRay) / RR * color + le;
//            recursionDepth++;
//        } else {
//            return Color3f(0.f);
//        }
//    }
//
//    std::string toString() const {
//        return "PathMatsIntegrator[]";
//    }