#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class DrawingCanvas;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    DrawingCanvas *canvas;
};
#endif // MAINWINDOW_H