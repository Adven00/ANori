#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <nori/integrator.h>
#include <nori/sampler.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN

/// Multi importance sampling
class PathMisIntegrator : public Integrator {
public:
    PathMisIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        Intersection its;
        Ray3f nextRay(ray);
        Color3f result(0.f);
        Color3f throughput(1.f);

        /* Used for russian roulette heuristic */
        float RR = 1.f;
        float etaProduct = 1.f;
        int depth = 0;

        /* Multi importance sampling */
        float wMats, wEms, pdfEms, pdfMats = 0.f;
        
        /* Using this to record whether the ray comes from specular material 
           which means emitter sampling has measure EDiscrect */
        bool previousIsSpecular = true;

        while (sampler->next1D() < RR) {
            if (!scene->rayIntersect(nextRay, its))
                break;

            /* Sampling indirect light */
            if (its.mesh->isEmitter()) {
                EmitterQueryRecord eRecMats(nextRay.o, its.p, its.mesh, ESolidAngle);
                eRecMats.w = its.toLocal(-nextRay.d.normalized());

                /* Refresh sampling weight */
                if (previousIsSpecular) {
                    wMats = 1.f;
                } else {
                    pdfEms = its.mesh->getEmitter()->pdf(eRecMats);
                    wMats = (pdfEms > 0.f && pdfMats > 0.f) ? weight(pdfMats, pdfEms) : 0.f;
                }
                result += its.mesh->getEmitter()->eval(eRecMats) * throughput * wMats;
            }

            /* Read this loop from here */
            if (its.mesh->getBSDF()->isDiffuse()) {
                /* Sampling the emitter */
                auto light = scene->sampleLight(sampler->next1D());
                EmitterQueryRecord eRecEms(its, light);
                Color3f color = light->getEmitter()->sample(eRecEms, sampler->next3D());

                /* If succeed to sample direct light */
                Ray3f shadowRay(eRecEms.p, its.toWorld(eRecEms.wo), Epsilon, (eRecEms.s - eRecEms.p).norm() - Epsilon);
                if (!scene->rayIntersect(shadowRay)) {
                    /* Compute BSDF */
                    BSDFQueryRecord bRecEms(its.toLocal(-nextRay.d), eRecEms.wo, ESolidAngle);
                    Color3f f = its.mesh->getBSDF()->eval(bRecEms);

                    /* Refresh sampling weight */
                    pdfMats = its.mesh->getBSDF()->pdf(bRecEms);
                    pdfEms = light->getEmitter()->pdf(eRecEms);
                    wEms = (pdfEms > 0.f && pdfMats > 0.f) ? weight(pdfEms, pdfMats) : 0.f;

                    float pdf = 1 / (float)(scene->getLights().size());
                    result += color * f * throughput / pdf * wEms;
                }
                previousIsSpecular = false;
            } else {
                previousIsSpecular = true;
            }

            /* Sampling the next ray accroding to BRDF */
            BSDFQueryRecord bRecMats(its.toLocal(-nextRay.d));
            Color3f color = its.mesh->getBSDF()->sample(bRecMats, sampler->next2D());
            nextRay = Ray3f(its.p, its.toWorld(bRecMats.wo));

            /* Fail to sample BSDF */
            if (color.isZero())
                break;

            /* Rfresh probability for BSDF sampling */
            pdfMats = its.mesh->getBSDF()->pdf(bRecMats);

            /* Russian roulette */
            etaProduct *= bRecMats.eta * bRecMats.eta;
            RR = ++depth > 4 ? std::min(0.99f, throughput.maxCoeff() * etaProduct) : 1.f;
            throughput *= color / RR;
        }

        return result;
    }

    std::string toString() const {
        return "PathMisIntegrator[]";
    }

    /* Weight using power heuristic */
    inline static float weight(float pdfA, float pdfB) {
        pdfA *= pdfA;
        pdfB *= pdfB;
        return pdfA / (pdfA + pdfB);
    }
};


NORI_REGISTER_CLASS(PathMisIntegrator, "path_mis");
NORI_NAMESPACE_END