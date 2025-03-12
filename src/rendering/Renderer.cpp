#include "Renderer.hpp"
#include "Camera.hpp"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

#include "bumper_grid.h"
#include "glm/gtc/type_ptr.hpp"

Renderer::Renderer(QWidget *parent)
    : QOpenGLWidget(parent),
      m_leftButtonPressed(false),
      m_rightButtonPressed(false)
{
    setFocusPolicy(Qt::StrongFocus);

    camera = new Camera();
    camera->setFocus({0.0f, 0.0f, 0.0f});
    camera->setDistance(2.0f);
    camera->setPerspective(false);

    connect(&timer, &QTimer::timeout, this, &Renderer::updateScene);
    timer.start(16); // 16 ms
}

Renderer::~Renderer()
{
    delete camera;
}

void Renderer::useShader(const Shader* shdr) const
{
    const float aspect = static_cast<float>(width()) / static_cast<float>(width());

    constexpr glm::mat4 m {1.0f};

    shdr->use();
    shdr->setMat4("model", m);
    shdr->setMat4("view", camera->viewMatrix());
    shdr->setMat4("projection", camera->projectionMatrix(aspect));
    shdr->setVec3("material.ambient", {1.0f, 0.0f, 0.0f});
    shdr->setVec3("material.diffuse", {0.9f, 0.9f, 0.9f});
    shdr->setVec3("material.specular", {1.0f, 1.0f, 1.0f});
    shdr->setFloat("material.shininess", 1);

    shdr->setVec3("light.position", {-1.f, 1.f, 0.f});
    shdr->setVec3("light.ambient", {.5f, .5f, .5f});
    shdr->setVec3("light.diffuse", {0.3f, 0.3f, 0.3f});
    shdr->setVec3("light.specular", {0.3f, 0.3f, 0.3f});
    shdr->release();
}

void Renderer::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    sm = new SM::SphereMesh();
    bg = new SM::Graph::BumperGraph();

    sm->loadFromFile(smFilePath);
    bg->constructFrom(*sm);
    sm->inflate(-0.075f);

    sphereShader = new Shader("shaders/impostor.vert", "shaders/impostor.frag");
    bumperShader = new Shader("shaders/bumper.vert", "shaders/bumper.frag");

    bgRenderer = new BumperGraphRenderer(bg);
    bgRenderer->setSphereShader(sphereShader);
    bgRenderer->setBumperShader(bumperShader);

    camera->setFocus(bgRenderer->getCentroid());

    bumperShader->bindAttribute("aPos", 0);
    bumperShader->bindAttribute("aNormal", 1);
    sphereShader->bindAttribute("aPos", 0);
}

void Renderer::resizeGL(const int w, int h)
{
    if (h == 0) h = 1;

    const float aspect = static_cast<float>(w) / static_cast<float>(h);

    const glm::mat4& proj = camera->projectionMatrix(aspect);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(proj));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Renderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(width()) / static_cast<float>(height());
    const auto& proj = camera->projectionMatrix(aspect);
    const auto& view = camera->viewMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(value_ptr(proj));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(value_ptr(view));

    useShader(sphereShader);
    useShader(bumperShader);
    bgRenderer->render();
}

void Renderer::updateScene()
{
    if (freeze) return;

    update();
}

void Renderer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_leftButtonPressed = true;
    else if (event->button() == Qt::RightButton)
        m_rightButtonPressed = true;

    m_lastMousePos = event->pos();
    event->accept();
}

void Renderer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_leftButtonPressed = false;
    else if (event->button() == Qt::RightButton)
        m_rightButtonPressed = false;

    event->accept();
}

void Renderer::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint currentPos = event->pos();
    const QPoint delta = currentPos - m_lastMousePos;
    m_lastMousePos = currentPos;

    if (m_leftButtonPressed)
    {
        camera->rotate(static_cast<float>(-delta.x()), static_cast<float>(delta.y()));
        update();
    }
    else if (m_rightButtonPressed)
    {
        const float factor = 0.5f;
        animate(delta.x() * factor, delta.y() * factor);
        update();
    }

    event->accept();
}

void Renderer::wheelEvent(QWheelEvent *event)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    float delta = static_cast<float>(event->angleDelta().y()) / 120.0f;
#else
    float delta = static_cast<float>(event->delta()) / 120.0f;
#endif
    camera->zoom(-delta);
    update();

    event->accept();
}

void Renderer::animate(const float alpha, const float beta)
{
    bg->setPose(alpha, beta);
    bg->applyPose();
    bgRenderer->update();
    update();
}

void Renderer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        window()->close();
    }
    else if (event->key() == Qt::Key_F)
    {
        freeze = !freeze;
        update();
    }
    else if (event->key() == Qt::Key_Right) animate(0.5f, 0.0f);
    else if (event->key() == Qt::Key_Left) animate(-0.5f, 0.0f);
    else if (event->key() == Qt::Key_Down) animate(0.0f, 0.5f);
    else if (event->key() == Qt::Key_Up) animate(0.0f, -0.5f);

    QOpenGLWidget::keyPressEvent(event);
}
