/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/object.h>
#include <nori/mesh.h>

NORI_NAMESPACE_BEGIN

struct EmitterQueryRecord {
    /// Sampled point on emitter
    Point3f s;
    /// Intersection point (emitted ray and mesh)
    Point3f p;
    /// Direction from s to p (emitter local frame)
    Vector3f w;
    /// Pointer to the associated mesh
    const Mesh *mesh;

    EmitterQueryRecord(const Point3f &p, const Mesh *mesh) 
        : p(p), mesh(mesh){ }
};

class Emitter : public NoriObject {
public:
    /** Compute the 'le' term in rendering equation
     * the radiance it emits to -eRec.wi
     * */
    virtual Color3f eval(const EmitterQueryRecord &eRec) const = 0;

    /** Compute the probability of sampling eRec.p
     * measured by solid angle
     * */
    virtual float pdf(const EmitterQueryRecord &eRec) const = 0;

    /** Sample the 'le' term term in rendering equation
     * */
    virtual Color3f sample(EmitterQueryRecord &eRec, const Point3f &sample) const = 0;

    /**
     * \brief Return the type of object (i.e. Mesh/Emitter/etc.) 
     * provided by this instance
     * */
    EClassType getClassType() const { return EEmitter; }
};

NORI_NAMESPACE_END
