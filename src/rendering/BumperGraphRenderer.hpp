#pragma once
#include "bumper_graph.h"
#include "Shader.hpp"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class BumperGraphRenderer
{
public:
	explicit BumperGraphRenderer(const SM::Graph::BumperGraph* bumper_graph);

	void setSphereShader(Shader* shdr);
	void setBumperShader(Shader* shdr);

	glm::vec3 getCentroid() const;

	void render();
	void update();

private:
	Shader* sphereShader;
	Shader* bumperShader;
	const SM::Graph::BumperGraph* bg;

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
	};
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;

	struct SubMesh {
		size_t indexOffset;
		size_t indexCount;
		glm::vec3 color;
	};
	std::vector<SubMesh> m_subMeshes;

	QOpenGLVertexArrayObject m_VAO;
	QOpenGLBuffer m_VBO { QOpenGLBuffer::VertexBuffer };
	QOpenGLBuffer m_EBO { QOpenGLBuffer::IndexBuffer };

	void renderSpheres() const;
	void renderSphere(const glm::vec3 &center, float radius, const glm::vec3 &color) const;

	static glm::vec3 computeUpperPlaneNormal(
		const SM::Sphere &sa,
		const SM::Sphere &sb,
		const SM::Sphere &sc,
		int direction);

	void buildPrysmoidGeometry(int index, const glm::vec3 &color);
	void buildQuadGeometry(int index, const glm::vec3 &color);
	void buildCapsuloidGeometry(int index, const glm::vec3 &color);
	void buildCapsuleBetweenSpheres(int sphereIndex1, int sphereIndex2,
									const glm::vec3& color);

	void appendTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
						const glm::vec3& n1, const glm::vec3& n2, const glm::vec3& n3);
	void uploadGeometryToGPU();
};