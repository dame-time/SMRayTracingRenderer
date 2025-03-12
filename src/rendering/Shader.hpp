#pragma once

#include <QOpenGLShaderProgram>
#include <QString>

#include "glm/glm.hpp"

/**
 * @brief The Shader class wraps QOpenGLShaderProgram and provides a convenient
 *        interface to load shaders from files using relative paths.
 */
class Shader
{
public:
	Shader(const QString &vertexPath, const QString &fragmentPath);
	~Shader();

	void use() const;
	void release() const;

	int uniformLocation(const QString &name) const;

	void bindAttribute(const std::string& name, const unsigned int location) const;
	void setFloat(const QString &name, float value) const;
	void setVec3(const QString &name, const glm::vec3 &value) const;
	void setMat4(const QString &name, const glm::mat4 &value) const;

	QOpenGLShaderProgram* program() const;

private:
	QOpenGLShaderProgram *m_program;

	bool load(const QString &vertexPath, const QString &fragmentPath) const;

	static QString resolvePath(const QString &relativePath);
};
