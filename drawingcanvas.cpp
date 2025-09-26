#include "drawingcanvas.h"
#include <QPainter>       // A classe principal para todo o desenho 2D no Qt.
#include <QColorDialog>   // Para abrir o seletor de cores.
#include <QMessageBox>    // Para exibir caixas de diálogo de erro/aviso.
#include <algorithm>      // Para usar std::sort e std::remove_if, algoritmos da biblioteca padrão C++.

// Construtor: inicializa as variáveis de estado com valores padrão.
DrawingCanvas::DrawingCanvas(QWidget *parent)
    : QWidget(parent),
      fillColor(Qt::blue),
      lineColor(Qt::black),
      lineThickness(2),
      polygonDrawn(false)
{}

// O coração da renderização no Qt. É chamado quando update() é invocado.
void DrawingCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event); // Indica ao compilador que não usaremos o parâmetro 'event'.
    QPainter painter(this); // Cria um objeto QPainter para desenhar NESTE widget.
    painter.setRenderHint(QPainter::Antialiasing); // Deixa as linhas mais suaves.

    // O modelo de pintura do Qt é baseado em estado. A cada 'paintEvent',
    // redesenhamos TUDO do zero com base nas nossas variáveis de estado (vértices, cores, etc.).

    // 1. Pinta o fundo de branco.
    painter.fillRect(rect(), Qt::white);

    // 2. Desenha o resultado do preenchimento.
    // Se o vetor 'filledScanlines' não estiver vazio, percorre-o e desenha cada linha.
    if (!filledScanlines.isEmpty()) {
        QPen fillPen(fillColor, 1); // Pen para o preenchimento, sempre com 1 pixel de espessura.
        painter.setPen(fillPen);
        painter.drawLines(filledScanlines); // Método otimizado para desenhar múltiplas linhas.
    }

    // 3. Desenha o contorno do polígono.
    if (vertices.size() > 0) {
        QPen linePen(lineColor, lineThickness); // Pen para o contorno, com cor e espessura definidas pelo usuário.
        linePen.setCapStyle(Qt::RoundCap);     // Deixa as pontas das linhas arredondadas.
        linePen.setJoinStyle(Qt::RoundJoin);    // Deixa as junções entre as linhas arredondadas.
        painter.setPen(linePen);

        // Desenha as arestas entre os vértices já inseridos.
        for (int i = 0; i < vertices.size() - 1; ++i) {
            painter.drawLine(vertices[i], vertices[i+1]);
        }
        // Se o polígono foi fechado, desenha a última aresta (do último ao primeiro vértice).
        if (polygonDrawn) {
            painter.drawLine(vertices.last(), vertices.first());
        }
    }

    // 4. Desenha os vértices como pequenos círculos para melhor visualização.
    painter.setBrush(lineColor); // Pincel para preencher os círculos.
    painter.setPen(Qt::NoPen);   // Sem contorno para os círculos.
    for(const QPoint& v : vertices) {
        painter.drawEllipse(v, 3, 3);
    }
}

// Captura o clique do mouse para adicionar um novo vértice.
void DrawingCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (polygonDrawn) {
            QMessageBox::information(this, "Aviso", "Polígono já foi preenchido. Limpe a tela para desenhar um novo.");
            return;
        }
        vertices.append(event->pos()); // Adiciona o ponto (coordenada do mouse) à lista de vértices.
        update(); // IMPORTANTE: Solicita que o Qt chame o paintEvent() para redesenhar o widget com o novo vértice.
    }
}

// Slots para manipulação da interface
void DrawingCanvas::setFillColor()
{
    QColor color = QColorDialog::getColor(fillColor, this, "Escolha a cor de preenchimento");
    if (color.isValid()) { // O usuário pode cancelar a caixa de diálogo.
        fillColor = color;
    }
}

void DrawingCanvas::setLineColor()
{
    QColor color = QColorDialog::getColor(lineColor, this, "Escolha a cor da linha");
    if (color.isValid()) {
        lineColor = color;
        update(); // Redesenha o contorno com a nova cor.
    }
}

void DrawingCanvas::setLineThickness(int thickness)
{
    lineThickness = thickness;
    update(); // Redesenha o contorno com a nova espessura.
}

void DrawingCanvas::clearCanvas()
{
    vertices.clear();
    filledScanlines.clear();
    polygonDrawn = false;
    update(); // Redesenha o widget, que agora estará vazio.
}

// =================================================================================
// ||                                                                             ||
// ||                IMPLEMENTAÇÃO DO ALGORITMO DE SCANLINE (ET / AET)              ||
// ||                                                                             ||
// =================================================================================
void DrawingCanvas::fillPolygon()
{
    if (vertices.size() < 3) {
        QMessageBox::critical(this, "Erro", "São necessários pelo menos 3 vértices para formar um polígono.");
        return;
    }

    polygonDrawn = true;
    filledScanlines.clear(); // Limpa preenchimentos antigos.

    // --- Passo 1: Construir a Tabela de Arestas (ET) ---
    QMap<int, QVector<EdgeInfo>> edgeTable;
    buildEdgeTable(edgeTable);

    if (edgeTable.isEmpty()) { // Se não há arestas válidas (ex: todos os pontos na mesma linha)
        update();
        return;
    }

    // Define os limites da varredura.
    int y_min = edgeTable.firstKey();
    int y_max = 0;
    // Encontra o y_max global para saber onde parar a varredura.
    // É preciso percorrer a ET, pois a última chave (último y_min) não garante o maior y_max.
    for(const auto& edgeList : edgeTable) {
        for(const auto& edge : edgeList) {
            if(edge.ymax > y_max) y_max = edge.ymax;
        }
    }

    // A Tabela de Arestas Ativas (AET) começa vazia.
    QVector<EdgeInfo> activeEdgeTable;

    // --- Passo 2: O Loop Principal de Varredura ---
    // Itera para cada linha de pixel, de y_min até y_max.
    for (int y = y_min; y <= y_max; ++y) {

        // 2a: Mover arestas da ET para a AET se elas começam na linha 'y' atual.
        if (edgeTable.contains(y)) {
            activeEdgeTable.append(edgeTable[y]);
        }

        // 2b: Remover arestas da AET que já terminaram (cujo ymax é a linha atual).
        activeEdgeTable.erase(
            std::remove_if(activeEdgeTable.begin(), activeEdgeTable.end(),
                           [y](const EdgeInfo& edge) { return edge.ymax == y; }),
            activeEdgeTable.end());

        if (activeEdgeTable.isEmpty()) continue; // Se não há arestas ativas, pula para a próxima linha.

        // 2c: Ordenar a AET pela coordenada x atual. Este é o passo mais CRÍTICO.
        // A ordenação garante que podemos pegar as arestas em pares da esquerda para a direita.
        std::sort(activeEdgeTable.begin(), activeEdgeTable.end(),
                  [](const EdgeInfo& a, const EdgeInfo& b) {
                      return a.x_current < b.x_current;
                  });

        // 2d: Preencher os pixels entre os pares de intersecções da AET.
        // O loop avança de 2 em 2, formando pares (par 0-1, par 2-3, etc.),
        // implementando a regra de paridade (par-ímpar).
        for (int i = 0; i < activeEdgeTable.size(); i += 2) {
            if (i + 1 < activeEdgeTable.size()) {
                int x_start = static_cast<int>(round(activeEdgeTable[i].x_current));
                int x_end = static_cast<int>(round(activeEdgeTable[i+1].x_current));
                if(x_start < x_end) {
                    // Adiciona a linha horizontal ao nosso vetor de resultados.
                    filledScanlines.append(QLine(x_start, y, x_end, y));
                }
            }
        }

        // 2e: Atualizar a coordenada x de cada aresta na AET para a próxima linha.
        // Esta é a aplicação da "Coerência de Arestas".
        for (auto& edge : activeEdgeTable) {
            edge.x_current += edge.inv_slope;
        }
    }

    // Ao final do loop, solicita uma repintura para desenhar as 'filledScanlines'.
    update();
}

// Método auxiliar que percorre os vértices e constrói a Tabela de Arestas.
void DrawingCanvas::buildEdgeTable(QMap<int, QVector<EdgeInfo>>& edgeTable)
{
    for (int i = 0; i < vertices.size(); ++i) {
        const QPoint& p1 = vertices[i];
        const QPoint& p2 = vertices[(i + 1) % vertices.size()]; // O operador '%' garante que a última aresta conecte o último ao primeiro vértice.

        // Ignora arestas horizontais, pois elas não cruzam linhas de varredura.
        if (p1.y() == p2.y()) continue;

        // Determina y_min, y_max e o x correspondente a y_min.
        int y_min = std::min(p1.y(), p2.y());
        int y_max = std::max(p1.y(), p2.y());
        double x_at_ymin = (p1.y() < p2.y()) ? p1.x() : p2.x();

        // Calcula o inverso do coeficiente angular (dx/dy).
        double inv_slope = static_cast<double>(p2.x() - p1.x()) / (p2.y() - p1.y());

        // Adiciona a aresta na ET, usando 'y_min' como chave.
        edgeTable[y_min].append({y_max, x_at_ymin, inv_slope});
    }
}