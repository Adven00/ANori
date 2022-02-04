/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <core/accel.h>
#include <Eigen/Geometry>

NORI_NAMESPACE_BEGIN

void Accel::addMesh(Mesh *mesh) {
    m_meshes.push_back(mesh);
    m_bbox.expandBy(mesh->getBoundingBox());
}

void Accel::build() {
    std::vector<TriInfo> tris;
    for (auto mesh : m_meshes) {
        uint32_t count = mesh->getTriangleCount();

        for (uint32_t i = 0; i < count; ++i)
            tris.push_back(TriInfo(i, mesh));
    }
    
    m_bvhTree = buildBvhTree(tris, 0, uint32_t(tris.size() - 1));
}

Accel::BvhNode *Accel::buildBvhTree(std::vector<TriInfo> &tris, uint32_t begin, uint32_t end) {

    /* Build bbox for every node */
    BvhNode *parent = new BvhNode;
    BoundingBox3f bbox;
    for (uint32_t i = begin; i <= end; ++i)
        bbox.expandBy(tris[i].mesh->getBoundingBox(tris[i].f));
    parent->bbox = bbox;

    /* Fill the triangle list of leaf node */
    if (end - begin  < 10) {
        for (uint32_t i = begin; i <= end; ++i)
            parent->tri_list.push_back(tris[i]);
        return parent;
    }

    /* Split the node accroding to the position of middle triangle. 
       Ensure numbers of triangles are the same in both child nodes */
    int axis = bbox.getLargestAxis();
    uint32_t mid = (begin + end) / 2;
    std::sort(
        tris.begin() + begin,
        tris.begin() + end + 1,
        [&](TriInfo a, TriInfo b) {
            return a.mesh->getCentroid(a.f)[axis] < b.mesh->getCentroid(b.f)[axis];
        }
    );
    
    parent->lchild = buildBvhTree(tris, begin, mid);
    parent->rchild = buildBvhTree(tris, mid + 1, end);
    return parent;
}

void Accel::traverseBvhTree(Ray3f &ray, Intersection &its, bool &intersected, BvhNode *node, bool shadowRay) const {
    if (!node || !node->bbox.rayIntersect(ray))
        return;
    
    if (!node->lchild && !node->rchild) {
        float u, v, t;
        for (auto ti : node->tri_list) {
            if (ti.mesh->rayIntersect(ti.f, ray, u, v, t) && t < its.t) {
                /* For shadow ray we don't need to record */
                if (shadowRay) {
                    intersected = true;
                    return;
                }
                /* Record the intersection */
                ray.maxt = its.t = t;
                its.uv = Point2f(u, v);
                its.mesh = ti.mesh;
                its.f = ti.f;
                intersected = true;
            }
        }
    }
    traverseBvhTree(ray, its, intersected, node->lchild, shadowRay);
    traverseBvhTree(ray, its, intersected, node->rchild, shadowRay);
    return;
}

bool Accel::rayIntersect(const Ray3f &ray_, Intersection &its, bool shadowRay) const {
    bool intersected = false;        // Was an intersection found so far?
    its.f = (uint32_t) - 1;          // Triangle index of the closest intersection
    its.t = std::numeric_limits<float>::infinity();

    Ray3f ray(ray_); /// Make a copy of the ray (we will need to update its '.maxt' value)

    traverseBvhTree(ray, its, intersected, m_bvhTree, shadowRay);

    if (intersected && !shadowRay) {
        /* At this point, we now know that there is an intersection,
           and we know the triangle index of the closest such intersection.
           The following computes a number of additional properties which
           characterize the intersection (normals, texture coordinates, etc..) */

        /* Find the barycentric coordinates */
        Vector3f bary;
        bary << 1 - its.uv.sum(), its.uv;

        /* References to all relevant mesh buffers */
        const Mesh *mesh   = its.mesh;
        const MatrixXf &V  = mesh->getVertexPositions();
        const MatrixXf &N  = mesh->getVertexNormals();
        const MatrixXf &UV = mesh->getVertexTexCoords();
        const MatrixXu &F  = mesh->getIndices();

        /* Vertex indices of the triangle */
        uint32_t idx0 = F(0, its.f), idx1 = F(1, its.f), idx2 = F(2, its.f);

        Point3f p0 = V.col(idx0), p1 = V.col(idx1), p2 = V.col(idx2);

        /* Compute the intersection positon accurately
           using barycentric coordinates */
        its.p = bary.x() * p0 + bary.y() * p1 + bary.z() * p2;

        /* Compute proper texture coordinates if provided by the mesh */
        if (UV.size() > 0)
            its.uv = bary.x() * UV.col(idx0) +
                bary.y() * UV.col(idx1) +
                bary.z() * UV.col(idx2);
        else 
            its.uv = Point2f(0.f, 0.f);

        /* Compute the geometry frame */
        its.geoFrame = Frame((p1 - p0).cross(p2 - p0).normalized());

        if (N.size() > 0) {
            /* Compute the shading frame. Note that for simplicity,
               the current implementation doesn't attempt to provide
               tangents that are continuous across the surface. That
               means that this code will need to be modified to be able
               use anisotropic BRDFs, which need tangent continuity */

            its.shFrame = Frame(
                (bary.x() * N.col(idx0) +
                 bary.y() * N.col(idx1) +
                 bary.z() * N.col(idx2)).normalized());
        } else {
            its.shFrame = its.geoFrame;
        }
    }

    return intersected;
}

NORI_NAMESPACE_END

