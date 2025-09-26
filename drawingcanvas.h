#ifndef DRAWINGCANVAS_H
#define DRAWINGCANVAS_H

#include <QWidget>
#include <QVector>
#include <QPoint>
#include <QColor>
#include <QMouseEvent>
#include <QMap>  //Usaremos QMap para a Edge Table (ET)


// --- Estrutura de Dados para Arestas ---
// Esta estrutura armazena todas as informações necessárias sobre uma aresta
// para o algoritmo de scanline.
struct EdgeInfo {
    int ymax;         // A coordenada y máxima da aresta. Usada para saber quando removê-la da AET.
    double x_current; // A coordenada x da intersecção da aresta com a linha de varredura ATUAL. Este valor é atualizado a cada linha.
    double inv_slope; // O inverso do coeficiente angular (1/m ou dx/dy). Usado para atualizar 'x_current' incrementalmente.
};

class DrawingCanvas : public QWidget
{
    Q_OBJECT // Habilita o uso de sinais e slots na classe

public:
    // Construtor padrão do widget
    explicit DrawingCanvas(QWidget *parent = nullptr);

protected:
    // --- Eventos do Qt ---
    // Este método é chamado automaticamente pelo Qt sempre que o widget precisa ser redesenhado.
    // TODA a lógica de desenho deve estar aqui dentro.
    void paintEvent(QPaintEvent *event) override;
    
    // Este método é chamado automaticamente quando um botão do mouse é pressionado sobre o widget.
    void mousePressEvent(QMouseEvent *event) override;

// --- Slots Públicos ---
// Slots são funções que podem ser conectadas a sinais (ex: clique de um botão).
// Eles são a forma como a MainWindow se comunica com o DrawingCanvas.
public slots:
    void setFillColor();
    void setLineColor();
    void setLineThickness(int thickness);
    void fillPolygon();
    void clearCanvas();

private:
    // Método auxiliar para construir a Tabela de Arestas (ET) a partir dos vértices.
    void buildEdgeTable(QMap<int, QVector<EdgeInfo>>& edgeTable);

    // --- Variáveis de Estado ---
    // Armazena a lista de vértices inseridos pelo usuário.
    QVector<QPoint> vertices;
    
    // Armazena o resultado do algoritmo de preenchimento: uma lista de linhas horizontais a serem desenhadas.
    QVector<QLine> filledScanlines;
    
    // Cores e espessura para o desenho.
    QColor fillColor;
    QColor lineColor;
    int lineThickness;
    
    // Flag para controlar o estado do polígono (aberto ou fechado/preenchido).
    bool polygonDrawn;
};

#endif // DRAWINGCANVAS_H