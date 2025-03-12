#pragma once

#include <QMainWindow>

class Renderer;

class Window final : public QMainWindow
{
	Q_OBJECT
public:
	explicit Window(QWidget *parent = nullptr);
	~Window() override;

private:
	Renderer *renderer;
};