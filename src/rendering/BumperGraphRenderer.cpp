//
// Created by Davide Paollilo on 28/02/25.
//

#include "BumperGraphRenderer.hpp"

#include <QOpenGLFunctions>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace SM;
using namespace SM::Graph;

BumperGraphRenderer::BumperGraphRenderer(const BumperGraph* bumper_graph)
{
    bg = bumper_graph;
    update();
}

void BumperGraphRenderer::setSphereShader(Shader *shdr)
{
    sphereShader = shdr;
}

void BumperGraphRenderer::setBumperShader(Shader *shdr)
{
    bumperShader = shdr;
}

glm::vec3 BumperGraphRenderer::getCentroid() const
{
    glm::vec3 tmp(0.0f, 0.0f, 0.0f);
    for (auto& sphere : bg->sphere)
        tmp += sphere.center;
    return tmp / static_cast<float>(bg->sphere.size());
}

void BumperGraphRenderer::render()
{
    renderSpheres();

    m_VAO.bind();
    bumperShader->use();

    for (auto &[indexOffset, indexCount, color] : m_subMeshes)
    {
        bumperShader->setVec3("material.ambient",  color);
        bumperShader->setVec3("material.diffuse",  color);
        bumperShader->setVec3("material.specular", glm::vec3(0.1f, 0.1f, 0.1f));
        bumperShader->setFloat("material.shininess", 32.0f);

        const size_t offsetBytes = indexOffset * sizeof(unsigned int);
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(indexCount),
                       GL_UNSIGNED_INT,
                       reinterpret_cast<void*>(offsetBytes));
    }

    m_VAO.release();
    bumperShader->release();
}

void BumperGraphRenderer::update()
{
    m_vertices.clear();
    m_indices.clear();
    m_subMeshes.clear();

    SubMesh prysSub;
    prysSub.indexOffset = m_indices.size();
    prysSub.color = glm::vec3(0.8f, 0.5f, 0.3f);

    for (int i = 0; i < bg->bumper.size(); i++)
        if (bg->bumper[i].shapeType == Bumper::PRYSMOID)
            buildPrysmoidGeometry(i, prysSub.color);

    prysSub.indexCount = m_indices.size() - prysSub.indexOffset;
    m_subMeshes.push_back(prysSub);

    SubMesh quadSub;
    quadSub.indexOffset = m_indices.size();
    quadSub.color = glm::vec3(0.8f, 0.3f, 0.5f);

    for (int i = 0; i < bg->bumper.size(); i++)
        if (bg->bumper[i].shapeType == Bumper::QUAD)
            buildQuadGeometry(i, quadSub.color);

    quadSub.indexCount = m_indices.size() - quadSub.indexOffset;
    m_subMeshes.push_back(quadSub);

    SubMesh capsSub;
    capsSub.indexOffset = m_indices.size();
    capsSub.color = glm::vec3(0.0f, 0.0f, 0.75f);

    for (int i = 0; i < bg->bumper.size(); i++)
        if (bg->bumper[i].shapeType == Bumper::CAPSULOID)
            buildCapsuloidGeometry(i, capsSub.color);

    capsSub.indexCount = m_indices.size() - capsSub.indexOffset;
    m_subMeshes.push_back(capsSub);

    uploadGeometryToGPU();
}

void BumperGraphRenderer::renderSpheres() const
{
    for (auto& sphere : bg->sphere)
        renderSphere(sphere.center, sphere.radius, glm::vec3(1.0f, 0.0f, 0.0f));
}

void BumperGraphRenderer::renderSphere(const glm::vec3 &center, const float radius, const glm::vec3 &color) const
{
    static QOpenGLVertexArrayObject VAO;
    static QOpenGLBuffer VBO(QOpenGLBuffer::VertexBuffer);
    static QOpenGLBuffer EBO(QOpenGLBuffer::IndexBuffer);
    static std::vector<float> vertices;
    static std::vector<unsigned int> indices;
    static int vertexCount = 0;

    if (!VAO.isCreated()) {
        vertices = {
            -1.0f,  1.0f,
             1.0f,  1.0f,
             1.0f, -1.0f,
            -1.0f, -1.0f
        };

        indices = {
            0, 1, 2,
            2, 3, 0
        };

        vertexCount = static_cast<int>(vertices.size());

        VAO.create();
        VAO.bind();

        VBO.create();
        VBO.bind();
        VBO.allocate(vertices.data(), vertices.size() * sizeof(float));

        EBO.create();
        EBO.bind();
        EBO.allocate(indices.data(), indices.size() * sizeof(unsigned int));

        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
        f->glEnableVertexAttribArray(0);

        VAO.release();
    }

    sphereShader->use();
    sphereShader->setVec3("center", center);
    sphereShader->setVec3("material.ambient", color);
    sphereShader->setVec3("material.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
    sphereShader->setVec3("material.specular", glm::vec3(0.0f, 0.0f, 0.0f));
    sphereShader->setFloat("material.shininess", 0.0f);
    sphereShader->setFloat("radius", radius);

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    VAO.bind();
    glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);
    VAO.release();

    sphereShader->release();
}

glm::vec3 BumperGraphRenderer::computeUpperPlaneNormal(const Sphere &sa, const Sphere &sb, const Sphere &sc, const int direction)
{
    glm::vec3 a = sa.center;
    glm::vec3 b = sb.center;
    glm::vec3 c = sc.center;

    const float sign = static_cast<float>(direction);

    glm::vec3 n = sign * glm::normalize(glm::cross(b - a, c - a));
    const glm::vec3 startN = n;

    for (int i = 0; i < 1000; i++){
        a = sa.center + n * sa.radius;
        b = sb.center + n * sb.radius;
        c = sc.center + n * sc.radius;

        glm::vec3 new_n = glm::normalize(sign * glm::cross(b - a, c - a));
        if (glm::dot(n, new_n) >= 0.999f)
            if (glm::dot(startN, new_n) < 0)
                return new_n;
        n = new_n;
    }

    return n;
}

void BumperGraphRenderer::buildPrysmoidGeometry(const int index, const glm::vec3 &color)
{
    const auto &bp = std::get<BumperPrysmoid>(bg->bumper[index].bumper);

    const Sphere &s0 = bg->sphere[bp.sphereIndex[0]];
    const Sphere &s1 = bg->sphere[bp.sphereIndex[1]];
    const Sphere &s2 = bg->sphere[bp.sphereIndex[2]];

    glm::vec3 C1 = s0.center;
    glm::vec3 C2 = s1.center;
    glm::vec3 C3 = s2.center;

    const float R1 = s0.radius;
    const float R2 = s1.radius;
    const float R3 = s2.radius;

    glm::vec3 nTop    = computeUpperPlaneNormal(s0, s1, s2,  1);
    glm::vec3 nBottom = computeUpperPlaneNormal(s0, s1, s2, -1);

    glm::vec3 V1_top    = C1 + nTop * R1;
    glm::vec3 V2_top    = C2 + nTop * R2;
    glm::vec3 V3_top    = C3 + nTop * R3;
    glm::vec3 V1_bottom = C1 + nBottom * R1;
    glm::vec3 V2_bottom = C2 + nBottom * R2;
    glm::vec3 V3_bottom = C3 + nBottom * R3;

    appendTriangle(V1_top, V2_top, V3_top, nTop, nTop, nTop);
    appendTriangle(V1_bottom, V2_bottom, V3_bottom, nBottom, nBottom, nBottom);

    buildCapsuleBetweenSpheres(bp.sphereIndex[0], bp.sphereIndex[1], color);
    buildCapsuleBetweenSpheres(bp.sphereIndex[1], bp.sphereIndex[2], color);
    buildCapsuleBetweenSpheres(bp.sphereIndex[2], bp.sphereIndex[0], color);
}

void BumperGraphRenderer::appendTriangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
    const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3)
{
    const unsigned int startIndex = static_cast<unsigned int>(m_vertices.size());

    m_vertices.push_back(Vertex{ p1, n1 });
    m_vertices.push_back(Vertex{ p2, n2 });
    m_vertices.push_back(Vertex{ p3, n3 });

    m_indices.push_back(startIndex + 0);
    m_indices.push_back(startIndex + 1);
    m_indices.push_back(startIndex + 2);
}

void BumperGraphRenderer::buildQuadGeometry(const int index, const glm::vec3 &color)
{
    const auto &bq = std::get<BumperQuad>(bg->bumper[index].bumper);

    const Sphere &s0 = bg->sphere[bq.sphereIndex[0]];
    const Sphere &s1 = bg->sphere[bq.sphereIndex[1]];
    const Sphere &s2 = bg->sphere[bq.sphereIndex[2]];
    const Sphere &s3 = bg->sphere[bq.sphereIndex[3]];

    glm::vec3 C1 = s0.center;
    glm::vec3 C2 = s1.center;
    glm::vec3 C3 = s2.center;
    glm::vec3 C4 = s3.center;

    const float R1 = s0.radius;
    const float R2 = s1.radius;
    const float R3 = s2.radius;
    const float R4 = s3.radius;

    glm::vec3 nTop    = computeUpperPlaneNormal(s0, s1, s2,  1);
    glm::vec3 nBottom = computeUpperPlaneNormal(s0, s1, s2, -1);

    glm::vec3 V1_top = C1 + nTop * R1;
    glm::vec3 V2_top = C2 + nTop * R2;
    glm::vec3 V3_top = C3 + nTop * R3;
    glm::vec3 V4_top = C4 + nTop * R4;

    glm::vec3 V1_bottom = C1 + nBottom * R1;
    glm::vec3 V2_bottom = C2 + nBottom * R2;
    glm::vec3 V3_bottom = C3 + nBottom * R3;
    glm::vec3 V4_bottom = C4 + nBottom * R4;

    appendTriangle(V1_top, V2_top, V3_top, nTop, nTop, nTop);
    appendTriangle(V3_top, V4_top, V1_top, nTop, nTop, nTop);

    appendTriangle(V1_bottom, V2_bottom, V3_bottom, nBottom, nBottom, nBottom);
    appendTriangle(V3_bottom, V4_bottom, V1_bottom, nBottom, nBottom, nBottom);

    buildCapsuleBetweenSpheres(bq.sphereIndex[0], bq.sphereIndex[1], color);
    buildCapsuleBetweenSpheres(bq.sphereIndex[1], bq.sphereIndex[2], color);
    buildCapsuleBetweenSpheres(bq.sphereIndex[2], bq.sphereIndex[3], color);
    buildCapsuleBetweenSpheres(bq.sphereIndex[3], bq.sphereIndex[0], color);
}

void BumperGraphRenderer::buildCapsuleBetweenSpheres(const int sphereIndex1, const int sphereIndex2,
                                                     const glm::vec3 &color)
{
    const Sphere &s0 = bg->sphere[sphereIndex1];
    const Sphere &s1 = bg->sphere[sphereIndex2];

    glm::vec3 v0 = s0.center;
    glm::vec3 v1 = s1.center;
    float r0 = s0.radius;
    float r1 = s1.radius;

    if (r1 < r0) {
        std::swap(v0, v1);
        std::swap(r0, r1);
    }

    constexpr int SEGMENTS = 32;
    glm::vec3 d = v0 - v1;
    float dLength = glm::length(d);

    float l = std::sqrt(glm::dot(d, d) - (r1 - r0) * (r1 - r0));

    float r0Bis = r0 * (l / dLength);
    float r1Bis = r1 * (l / dLength);

    float d0 = (r1 - r0) * (r0 / dLength);
    float d1 = (r1 - r0) * (r1 / dLength);

    glm::vec3 v0Bis = v0 + glm::normalize(d) * d0;
    glm::vec3 v1Bis = v1 + glm::normalize(d) * d1;

    glm::vec3 arbitrary = (std::abs(d.x) < 0.99f ? glm::vec3(1.0f, 0.0f, 0.0f)
                                                  : glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 right = glm::normalize(glm::cross(d, arbitrary));
    glm::vec3 up    = glm::normalize(glm::cross(right, d));

    std::vector<glm::vec3> circle1(SEGMENTS), circle2(SEGMENTS);
    std::vector<glm::vec3> normal1(SEGMENTS), normal2(SEGMENTS);

    for (int i = 0; i < SEGMENTS; ++i) {
        float theta = 2.0f * glm::pi<float>() * i / SEGMENTS;
        glm::vec3 offset1 = (right * std::cos(theta) + up * std::sin(theta)) * r0Bis;
        circle1[i] = v0Bis + offset1;
        normal1[i] = glm::normalize(offset1);

        glm::vec3 offset2 = (right * std::cos(theta) + up * std::sin(theta)) * r1Bis;
        circle2[i] = v1Bis + offset2;
        normal2[i] = glm::normalize(offset2);
    }

    for (int i = 0; i < SEGMENTS; ++i) {
        int next = (i + 1) % SEGMENTS;

        glm::vec3 p1 = circle1[i];
        glm::vec3 p2 = circle2[i];
        glm::vec3 p3 = circle1[next];
        glm::vec3 p4 = circle2[next];

        glm::vec3 n1 = normal1[i];
        glm::vec3 n2 = normal2[i];
        glm::vec3 n3 = normal1[next];
        glm::vec3 n4 = normal2[next];

        appendTriangle(p1, p2, p3, n1, n2, n3);
        appendTriangle(p2, p4, p3, n2, n4, n3);
    }
}

void BumperGraphRenderer::buildCapsuloidGeometry(const int index, const glm::vec3 &color)
{
    const auto &caps = std::get<BumperCapsuloid>(bg->bumper[index].bumper);
    buildCapsuleBetweenSpheres(caps.sphereIndex[0], caps.sphereIndex[1], color);
}

void BumperGraphRenderer::uploadGeometryToGPU()
{
    if (!m_VAO.isCreated()) {
        m_VAO.create();
    }
    m_VAO.bind();

    if (!m_VBO.isCreated()) {
        m_VBO.create();
    }
    m_VBO.bind();
    m_VBO.allocate(m_vertices.data(),
                   static_cast<int>(m_vertices.size() * sizeof(Vertex)));

    if (!m_EBO.isCreated()) {
        m_EBO.create();
    }
    m_EBO.bind();
    m_EBO.allocate(m_indices.data(),
                   static_cast<int>(m_indices.size() * sizeof(unsigned int)));

    QOpenGLFunctions f;
    f.initializeOpenGLFunctions();

    f.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                            reinterpret_cast<void*>(offsetof(Vertex, position)));
    f.glEnableVertexAttribArray(0);

    f.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                            reinterpret_cast<void*>(offsetof(Vertex, normal)));
    f.glEnableVertexAttribArray(1);

    m_VAO.release();
    m_VBO.release();
    m_EBO.release();
}