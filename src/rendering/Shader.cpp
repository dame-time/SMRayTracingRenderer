#include "Shader.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

Shader::Shader(const QString &vertexPath, const QString &fragmentPath) : m_program(new QOpenGLShaderProgram)
{
    if (!load(vertexPath, fragmentPath)) qDebug() << "Shaders not loaded correctly!";
}

Shader::~Shader()
{
    delete m_program;
}

QString Shader::resolvePath(const QString &relativePath)
{
    const auto basePath = QString(PROJECT_SOURCE_DIR);
    const QDir dir(basePath);
    return dir.absoluteFilePath(relativePath);
}

bool Shader::load(const QString &vertexPath, const QString &fragmentPath) const
{
    const QString vertexFullPath = resolvePath(vertexPath);
    const QString fragmentFullPath = resolvePath(fragmentPath);

    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexFullPath))
    {
        qDebug() << "Vertex shader compile error:" << m_program->log();
        return false;
    }

    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentFullPath))
    {
        qDebug() << "Fragment shader compile error:" << m_program->log();
        return false;
    }

    if (!m_program->link())
    {
        qDebug() << "Shader link error:" << m_program->log();
        return false;
    }

    return true;
}

void Shader::bindAttribute(const std::string& name, const unsigned int location) const
{
    if (m_program)
        m_program->bindAttributeLocation(name.c_str(), location);
}

void Shader::use() const
{
    m_program->bind();
}

void Shader::release() const
{
    m_program->release();
}

int Shader::uniformLocation(const QString &name) const
{
    return m_program->uniformLocation(name);
}

void Shader::setFloat(const QString &name, const float value) const
{
    if (!m_program->bind())
    {
        qDebug() << "Shader bind error in setFloat()";
        return;
    }
    m_program->setUniformValue(name.toUtf8().constData(), value);
}

void Shader::setVec3(const QString &name, const glm::vec3& value) const
{
    if (!m_program->bind())
    {
        qDebug() << "Shader bind error in setVec3()";
        return;
    }

    QVector3D qValue(value.x, value.y, value.z);
    m_program->setUniformValue(name.toUtf8().constData(), qValue);
}

void Shader::setMat4(const QString &name, const glm::mat4& value) const
{
    if (!m_program->bind())
    {
        qDebug() << "Shader bind error in setMat4()";
        return;
    }

    QMatrix4x4 qMat;
    qMat.setRow(0, QVector4D(value[0][0], value[1][0], value[2][0], value[3][0]));
    qMat.setRow(1, QVector4D(value[0][1], value[1][1], value[2][1], value[3][1]));
    qMat.setRow(2, QVector4D(value[0][2], value[1][2], value[2][2], value[3][2]));
    qMat.setRow(3, QVector4D(value[0][3], value[1][3], value[2][3], value[3][3]));

    m_program->setUniformValue(name.toUtf8().constData(), qMat);
}

QOpenGLShaderProgram* Shader::program() const
{
    return m_program;
}
