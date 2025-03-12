#pragma once

#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>

#include "BumperGraphRenderer.hpp"
#include "bumper_graph.h"
#include "bumper_grid.h"
#include "Shader.hpp"
#include "sphere_mesh.h"

class Camera;

class Renderer final : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
public:
	explicit Renderer(QWidget *parent = nullptr);
	~Renderer() override;

	void animate(float alpha, float beta);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

	void keyPressEvent(QKeyEvent *event) override;

	private slots:
		void updateScene();

private:
	QTimer timer;
	SM::SphereMesh* sm {};
	SM::Graph::BumperGraph* bg {};
	BumperGraphRenderer* bgRenderer {};
	Camera *camera;

	Shader* sphereShader{};
	Shader* bumperShader{};

	bool m_leftButtonPressed;
	bool m_rightButtonPressed;
	QPoint m_lastMousePos;

	// TODO: Change this path
	const char* smFilePath = "/Users/davidepaollilo/Desktop/Workspace/C++/SMClothPhysicsSim/assets/gorillaAnim.sm";

	bool freeze = false;

	void useShader(const Shader* shdr) const;
};