#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 初始化主窗口
    ui->setupUi(this);
    this->setWindowTitle("重庆地铁线路查询系统");

    // 初始化路线显示窗口
    ui->routeTextBrowser->setTextBackgroundColor(Qt::white);
    ui->routeTextBrowser->setTextColor(Qt::black);
    ui->routeTextBrowser->setStyleSheet("background-color:white;");

    // 初始化地图显示窗口
    myView = new Graphics_view_zoom(ui->graphicsView);
    myView->set_modifiers(Qt::NoModifier);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    scene = new QGraphicsScene;
    scene->setSceneRect(-LINE_INFO_WIDTH, 0, SCENE_WIDTH, SCENE_HEIGHT);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setBackgroundBrush(Qt::white);

    // 初始化后台地铁线路图
    subwayGraph = new Graph;
    bool flag = subwayGraph->readFileData(":/data/subway.txt");
    if (!flag) {
        QMessageBox box;
        box.setWindowTitle("Error reading subway line data");
        box.setText("读取地铁线路数据错误!");
        box.addButton("确定", QMessageBox::AcceptRole);
        if (box.exec() == QMessageBox::Accepted) {
            box.close();
        }
    }

    // 连接信号和槽函数
    connect(ui->startLineComboBox,
            &QComboBox::currentTextChanged,
            this,
            &MainWindow::transferStartLineChanged);
    connect(ui->dstLineComboBox,
            &QComboBox::currentTextChanged,
            this,
            &MainWindow::transferDstLineChanged);
    connect(ui->allLineButton, &QPushButton::clicked, this, &MainWindow::showMap);
    connect(ui->transferButton, &QPushButton::clicked, this, &MainWindow::transferQuery);

    updateTranserQueryInfo(); // 更新换乘选择信息
    showMap();                // 显示总地铁线路图
}

MainWindow::~MainWindow()
{
    delete ui;
    delete myView;
    delete scene;
    delete subwayGraph;
}

// 将站点的经纬度地理坐标转为视图坐标
QPointF MainWindow::transferCoord(QPointF coord)
{
    QPointF minCoord = subwayGraph->getMinCoord();
    QPointF maxCoord = subwayGraph->getMaxCoord();
    double x = (coord.x() - minCoord.x()) / (maxCoord.x() - minCoord.x()) * NET_WIDTH + MARGIN;
    double y = (maxCoord.y() - coord.y()) / (maxCoord.y() - minCoord.y()) * NET_HEIGHT + MARGIN;
    return QPointF(x, y);
}

// 计算线路混合颜色
QColor MainWindow::getLinesColor(const QList<int> &linesList)
{
    QColor color1 = QColor(255, 255, 255), color2;
    for (int i = 0; i < linesList.size(); ++i) {
        color2 = subwayGraph->getLineColor(linesList[i]);
        color1.setRed(color1.red() * color2.red() / 255);
        color1.setGreen(color1.green() * color2.green() / 255);
        color1.setBlue(color1.blue() * color2.blue() / 255);
    }
    return color1;
}

// 获得线路表的名字集
QString MainWindow::getLinesName(const QList<int> &linesList)
{
    QString str = "\t";
    for (int i = 0; i < linesList.size(); ++i) {
        str += " " + subwayGraph->getLineName(linesList[i]);
    }
    return str;
}

// 绘制网络图的边
void MainWindow::drawEdges(const QList<Edge> &edgesList)
{
    for (int i = 0; i < edgesList.size(); ++i) {
        int s1 = edgesList[i].first;
        int s2 = edgesList[i].second;
        QList<int> linesList = subwayGraph->getCommonLines(s1, s2);
        QColor color = getLinesColor(linesList);
        QString tip = "途经：" + subwayGraph->getStationName(s1) + "--"
                      + subwayGraph->getStationName(s2) + "\n线路：" + getLinesName(linesList);
        QPointF s1Pos = transferCoord(subwayGraph->getStationCoord(s1));
        QPointF s2Pos = transferCoord(subwayGraph->getStationCoord(s2));

        QGraphicsLineItem *edgeItem = new QGraphicsLineItem;
        edgeItem->setPen(QPen(color, EDGE_PEN_WIDTH));
        edgeItem->setCursor(Qt::PointingHandCursor);
        edgeItem->setToolTip(tip);
        edgeItem->setPos(s1Pos);
        edgeItem->setLine(0, 0, s2Pos.x() - s1Pos.x(), s2Pos.y() - s1Pos.y());
        scene->addItem(edgeItem);
    }
}

// 绘制网络图的站点节点
void MainWindow::drawStations(const QList<int> &stationsList)
{
    for (auto s = stationsList.begin(); s != stationsList.end(); s++) {
        QString name = subwayGraph->getStationName(*s);
        QList<int> linesList = subwayGraph->getStationLinesInfo(*s);
        QColor color = getLinesColor(linesList);
        QPointF coord = transferCoord(subwayGraph->getStationCoord(*s));
        QString tip = "站名：" + name + "\n途径线路：" + getLinesName(linesList);

        QGraphicsEllipseItem *stationItem = new QGraphicsEllipseItem;
        stationItem->setRect(-NODE_HALF_WIDTH,
                             -NODE_HALF_WIDTH,
                             NODE_HALF_WIDTH << 1,
                             NODE_HALF_WIDTH << 1);
        stationItem->setPos(coord);
        stationItem->setPen(color);
        stationItem->setCursor(Qt::PointingHandCursor);
        stationItem->setToolTip(tip);

        if (linesList.size() <= 1) {
            stationItem->setBrush(QColor(QRgb(0xffffff)));
        }

        scene->addItem(stationItem);

        QGraphicsTextItem *textItem = new QGraphicsTextItem;
        textItem->setDefaultTextColor("0xffffff");
        textItem->setPlainText(name);
        textItem->setFont(QFont("", 4, 1));
        textItem->setPos(coord.x(), coord.y() - NODE_HALF_WIDTH * 2);
        scene->addItem(textItem);
    }
}

// -------------------槽函数-------------------

// 换乘出发线路改变槽函数
void MainWindow::transferStartLineChanged(QString lineName)
{
    ui->startStationComboBox->clear();
    int lineHash = subwayGraph->getLineHash(lineName);
    if (lineHash != -1) {
        QList<QString> stationsList = subwayGraph->getLineStationsList(lineHash);
        for (auto &a : stationsList) {
            ui->startStationComboBox->addItem(a);
        }
    }
}

// 换乘目的线路改变槽函数
void MainWindow::transferDstLineChanged(QString lineName)
{
    ui->dstStationComboBox->clear();
    int lineHash = subwayGraph->getLineHash(lineName);
    if (lineHash != -1) {
        QList<QString> stationsList = subwayGraph->getLineStationsList(lineHash);
        for (auto &a : stationsList) {
            ui->dstStationComboBox->addItem(a);
        }
    }
}

// 更新换乘选择信息
void MainWindow::updateTranserQueryInfo()
{
    ui->startLineComboBox->clear();
    ui->dstLineComboBox->clear();
    QList<QString> linesList = subwayGraph->getLinesNameList();
    for (auto &a : linesList) {
        ui->startLineComboBox->addItem(a);
        ui->dstLineComboBox->addItem(a);
    }
    transferStartLineChanged(ui->startLineComboBox->itemText(0));
    transferDstLineChanged(ui->dstLineComboBox->itemText(0));
}

// 动作查看所有线路图槽函数
void MainWindow::showMap()
{
    scene->clear();
    ui->routeTextBrowser->clear();
    QList<int> stationsList;
    QList<Edge> edgesList;
    subwayGraph->getGraph(stationsList, edgesList);
    drawEdges(edgesList);
    drawStations(stationsList);
}

// 换乘查询槽函数
void MainWindow::transferQuery()
{
    int s1 = subwayGraph->getStationHash(ui->startStationComboBox->currentText());
    int s2 = subwayGraph->getStationHash(ui->dstStationComboBox->currentText());
    bool way = ui->minTransferButton->isChecked();
    if (s1 == -1 || s2 == -1) {
        QMessageBox box;
        box.setWindowTitle("换乘查询");
        box.setText("请选择有站点的线路");
        box.addButton("确定", QMessageBox::AcceptRole);
        if (box.exec() == QMessageBox::Accepted) {
            box.close();
        }
    } else {
        QList<int> stationsList;
        QList<Edge> edgesList;
        bool flag{true};
        flag = way ? subwayGraph->queryTransferMinTransfer(s1, s2, stationsList, edgesList)
                   : subwayGraph->queryTransferMinTime(s1, s2, stationsList, edgesList);
        if (flag) {
            scene->clear();
            drawEdges(edgesList);
            drawStations(stationsList);
            QString text;
            for (int i = 0; i < stationsList.size(); ++i) {
                if (i) {
                    text += "\n  ↓\n";
                }
                text += subwayGraph->getStationName(stationsList[i]);
                QString linesStr = getLinesName(subwayGraph->getStationLinesInfo(stationsList[i]));
                text += linesStr;
            }
            ui->routeTextBrowser->clear();
            ui->routeTextBrowser->setText(text);
        } else {
            QMessageBox box;
            box.setWindowTitle("查询失败");
            box.setText("该起始和终止站点暂时无法到达");
            box.addButton("确定", QMessageBox::AcceptRole);
            if (box.exec() == QMessageBox::Accepted) {
                box.close();
            }
        }
    }
}
