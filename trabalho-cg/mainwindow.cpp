#include "mainwindow.h"
#include "drawingcanvas.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Preenchimento de Polígono (C++/Qt)");

    // Widget central e layout principal
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // Canvas à esquerda
    canvas = new DrawingCanvas(this);
    canvas->setMinimumSize(800, 600);
    mainLayout->addWidget(canvas);

    // Painel de controle à direita
    QWidget *controlPanel = new QWidget(this);
    controlPanel->setFixedWidth(220);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    
    // Botões e controles
    QPushButton *fillButton = new QPushButton("Preencher Polígono", this);
    QPushButton *fillColorButton = new QPushButton("Cor de Preenchimento", this);
    QPushButton *lineColorButton = new QPushButton("Cor da Linha", this);
    QPushButton *clearButton = new QPushButton("Limpar Tela", this);
    
    QLabel *thicknessLabel = new QLabel("Espessura da Linha:", this);
    QSlider *thicknessSlider = new QSlider(Qt::Horizontal, this);
    thicknessSlider->setRange(1, 10);
    thicknessSlider->setValue(2);

    controlLayout->addWidget(new QLabel("Controles", this));
    controlLayout->addWidget(fillButton);
    controlLayout->addWidget(fillColorButton);
    controlLayout->addWidget(lineColorButton);
    controlLayout->addWidget(thicknessLabel);
    controlLayout->addWidget(thicknessSlider);
    controlLayout->addStretch();
    controlLayout->addWidget(clearButton);
    
    mainLayout->addWidget(controlPanel);
    setCentralWidget(centralWidget);

    // Conexões (sinais e slots)
    connect(fillButton, &QPushButton::clicked, canvas, &DrawingCanvas::fillPolygon);
    connect(clearButton, &QPushButton::clicked, canvas, &DrawingCanvas::clearCanvas);
    connect(fillColorButton, &QPushButton::clicked, canvas, &DrawingCanvas::setFillColor);
    connect(lineColorButton, &QPushButton::clicked, canvas, &DrawingCanvas::setLineColor);
    connect(thicknessSlider, &QSlider::valueChanged, canvas, &DrawingCanvas::setLineThickness);
}

MainWindow::~MainWindow()
{
}