#include "Window.hpp"
#include "Renderer.hpp"

Window::Window(QWidget *parent)
    : QMainWindow(parent)
{
    renderer = new Renderer(this);
    setCentralWidget(renderer);
    setWindowTitle("SM RT Renderer");
    resize(800, 600);
}

Window::~Window()
{
}
