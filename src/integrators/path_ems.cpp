#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <nori/integrator.h>
#include <nori/sampler.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN


/// Emitter sampling
class PathEmsIntegrator : public Integrator {
public:
    PathEmsIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        Intersection its;
        Ray3f nextRay(ray);
        Color3f result(0.f);
        Color3f throughOutput(1.f);

        /* Using this to record whether the ray comes from specular material 
           We use 0-1 weight to avoid double counting, so when it's false,  
           indirect light will not be sampled */
        bool previousIsSpecular = true;

        /* Used for russian roulette heuristic */
        float RR = 1.f;
        int depth = 0;
        float etaProduct = 1.f;

        while (sampler->next1D() < RR) {
            if (!scene->rayIntersect(nextRay, its))
                break;

            /* Sampling indirect light */
            if (its.mesh->isEmitter()) {
                EmitterQueryRecord rec(nextRay.o, its.mesh);
                rec.w = its.toLocal(-nextRay.d.normalized());

                if (previousIsSpecular)
                    result += its.mesh->getEmitter()->eval(rec) * throughOutput;
            }

            /* Read this loop from here */
            if (its.mesh->getBSDF()->isDiffuse()) {
                /* Sampling the light */
                auto light = scene->sampleLight(sampler->next1D());
                EmitterQueryRecord eRec(its, light);
                Color3f color = light->getEmitter()->sample(eRec, sampler->next3D());

                /* If succeed to sample light */
                Ray3f shadowRay(eRec.p, its.toWorld(eRec.wo), Epsilon, (eRec.s - eRec.p).norm() - Epsilon);
                if (!scene->rayIntersect(shadowRay)) {
                    /* Compute BSDF */
                    BSDFQueryRecord bRecDirect(its.toLocal(-nextRay.d), eRec.wo, ESolidAngle, its);
                    Color3f f = its.mesh->getBSDF()->eval(bRecDirect);
                
                    float pdf = 1 / (float)(scene->getLights().size());
                    result += color * f * throughOutput / pdf;
                }
                previousIsSpecular = false;
            } else {
                previousIsSpecular = true;
            }

            /* Sampling the next ray accroding to BRDF */
            BSDFQueryRecord bRec(its.toLocal(-nextRay.d), its);
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
        return "PathEmsIntegrator[]";
    }
};


NORI_REGISTER_CLASS(PathEmsIntegrator, "path_ems");
NORI_NAMESPACE_END