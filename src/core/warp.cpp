/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/warp.h>
#include <nori/vector.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

Point2f Warp::squareToUniformSquare(const Point2f &sample) {
    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
    return ((sample.array() >= 0.f).all() && (sample.array() <= 1.f).all()) ? 1.f : 0.f;
}

Point2f Warp::squareToTent(const Point2f &sample) {
    auto tentCdfInverse = [](float t) {
        return t < 0.5f ? sqrt(2.f * t) - 1.f : 1.f - sqrt(2.f - 2.f * t);
    };
    return Point2f(tentCdfInverse(sample.x()), tentCdfInverse(sample.y()));
}

float Warp::squareToTentPdf(const Point2f &p) {
    auto tentPdf = [](float t) {
        return t >= -1.f && t <= 1.f ? 1.f - abs(t) : 0.f;
    };
    return tentPdf(p.x()) * tentPdf(p.y());
}

Point2f Warp::squareToUniformDisk(const Point2f &sample) {
    float radius = sqrt(sample.x());
    float theta = 2.f * M_PI * sample.y();
    return Point2f(radius * cos(theta), radius * sin(theta));
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
    return std::sqrt(p.x() * p.x() + p.y() * p.y()) <= 1.f ? INV_PI : 0.f;
}

Vector3f Warp::squareToUniformSphere(const Point2f &sample) {
    float phi = sample.x() * M_PI * 2;
    float theta = acos(1 - 2 * sample.y());
    return Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

float Warp::squareToUniformSpherePdf(const Vector3f &v) {
    return INV_FOURPI;
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {
    float phi = sample.x() * M_PI * 2.f;
    float theta = acos(1 - sample.y());
    return Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
    return v.z() < 0.f ? 0.f : INV_TWOPI;
}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
    Point2f p = squareToUniformDisk(sample);
    float z = sqrt(1 - p.x() * p.x() - p.y() * p.y());
    return Vector3f(p.x(), p.y(), z);
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
    return v.z() < 0 ? 0 : v.z() * INV_PI;
}

Vector3f Warp::squareToBeckmann(const Point2f &sample, float alpha) {
    float phi = sample.x() * M_PI * 2.f;
    float theta = atan(abs(alpha) * sqrt(-log(1 - sample.y())));
    return Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

float Warp::squareToBeckmannPdf(const Vector3f &m, float alpha) {
    if (m.z() <= 0)
        return 0;
    
    float alpha2 = alpha * alpha;
    float tan2 = (m.x() * m.x() + m.y() * m.y()) / (m.z() * m.z());
    float cos3 = m.z() * m.z() * m.z();
    return INV_PI * exp(-tan2 / alpha2) / (alpha2 * cos3);
}

NORI_NAMESPACE_END
