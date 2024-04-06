#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "graph.h"
#include "graphics_view_zoom.h"

#include <QColorDialog>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QTextStream>

#define LINE_INFO_WIDTH 0                                      // 预留边框用于信息展示
#define MARGIN 30                                              // 视图左边距
#define NET_WIDTH 1000                                         // 网络图最大宽度
#define NET_HEIGHT 1000                                        // 网络图最大高度
#define SCENE_WIDTH (LINE_INFO_WIDTH + MARGIN * 2 + NET_WIDTH) // 视图宽度
#define SCENE_HEIGHT (MARGIN * 2 + NET_HEIGHT)                 // 视图高度

#define EDGE_PEN_WIDTH 1  //线路边宽
#define NODE_HALF_WIDTH 2 //节点大小

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

    // 换乘出发线路改变槽函数
    void transferStartLineChanged(QString lineName);
    // 换乘目的线路改变槽函数
    void transferDstLineChanged(QString lineNames);
    // 更新换乘选择信息
    void updateTranserQueryInfo();
    // 动作查看所有线路图槽函数
    void showMap();
    // 换乘查询槽函数
    void transferQuery();

protected:
    Ui::MainWindow *ui;         // 主窗口UI
    Graphics_view_zoom *myView; // 自定义视图，用于鼠标缩放
    QGraphicsScene *scene;      // 场景
    Graph *subwayGraph;         // 后端图

    // 站点经纬度转为视图坐标
    QPointF transferCoord(QPointF coord);
    // 计算线路混合颜色
    QColor getLinesColor(const QList<int> &linesList);
    // 获得线路表的名字集
    QString getLinesName(const QList<int> &linesList);
    // 绘制网络图的边
    void drawEdges(const QList<Edge> &edgesList);
    // 绘制网络图的站点节点
    void drawStations(const QList<int> &stationsList);
};

#endif // MAINWINDOW_H
