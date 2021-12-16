/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/accel.h>
#include <Eigen/Geometry>

NORI_NAMESPACE_BEGIN

void Accel::addMesh(Mesh *mesh) {
    if (m_mesh)
        throw NoriException("Accel: only a single mesh is supported!");
    m_mesh = mesh;
    m_bbox = m_mesh->getBoundingBox();
}

void Accel::build() {
    
    uint32_t count = m_mesh->getTriangleCount();
    std::vector<uint32_t> triangles;
    for (uint32_t i = 0; i < count; ++i)
        triangles.push_back(i);
    
    m_bvh_tree = buildBvhTree(triangles, 0, count - 1);
}

Accel::BvhNode *Accel::buildBvhTree(std::vector<uint32_t> &triangles, uint32_t begin, uint32_t end) {
    BvhNode *parent = new BvhNode;
    BoundingBox3f bbox = m_mesh->getBoundingBox(triangles[begin]);
    for (uint32_t i = begin; i <= end; ++i)
        bbox.expandBy(m_mesh->getBoundingBox(triangles[i]));
    parent->bbox = bbox;

    if (end - begin  < 10) {
        for (uint32_t i = begin; i <= end; ++i)
            parent->triangle_list.push_back(triangles[i]);
        return parent;
    }

    int axis = bbox.getLargestAxis();
    uint32_t mid = (begin + end) / 2;
    std::sort(
        triangles.begin() + begin,
        triangles.begin() + end + 1,
        [&](uint32_t a, uint32_t b) {
            return m_mesh->getCentroid(a)[axis] < m_mesh->getCentroid(b)[axis];
        }
    );
    
    parent->lchild = buildBvhTree(triangles, begin, mid);
    parent->rchild = buildBvhTree(triangles, mid + 1, end);
    return parent;
}

void Accel::traverseBvhTree(Ray3f &ray, Intersection &its, bool &intersected, BvhNode *node, bool shadowRay) const {
    if (!node || !node->bbox.rayIntersect(ray))
        return;
    
    if (!node->lchild && !node->rchild) {
        float u, v, t;
        for (auto idx : node->triangle_list) {
            if (m_mesh->rayIntersect(idx, ray, u, v, t) && t < its.t) {
                if (shadowRay) {
                    intersected = true;
                    return;
                }
                ray.maxt = its.t = t;
                its.uv = Point2f(u, v);
                its.mesh = m_mesh;
                its.f = idx;
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

    traverseBvhTree(ray, its, intersected, m_bvh_tree, shadowRay);

    if (intersected && !shadowRay) {
        /* At this point, we now know that there is an intersection,
           and we know the triangle index of the closest such intersection.
           The following computes a number of additional properties which
           characterize the intersection (normals, texture coordinates, etc..)
        */

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

