#ifndef GRAPH_H
#define GRAPH_H

#include <QDebug>
#include <QFile>
#include <QHash>
#include <QPoint>
#include <QString>
#include <QTextStream>
#include <QVector>
#include <algorithm>
#include <queue>

#include "subway.h"

// 节点
class Node
{
public:
    int stationID;   // 邻接点ID
    double distance; // 两点距离

    // 构造函数
    Node(){};
    Node(int s, double dist)
        : stationID(s)
        , distance(dist){};

    // Node小顶堆大小比较，重载 >
    bool operator>(const Node &n) const { return this->distance > n.distance; }
};

// 图
class Graph
{
private:
    QVector<Station> stations;        // 存储所有站点
    QVector<Line> lines;              // 存储所有线路
    QHash<QString, int> stationsHash; // 站点名->存储id
    QHash<QString, int> linesHash;    // 线路名->存储id
    QSet<Edge> edges;                 // 所有边
    QVector<QVector<Node>> graph;     // 网络图

    // 清空数据
    void clearData();
    // 插入一条边
    bool insertEdge(int s1, int s2);
    // 更新边界经纬度
    void updateMinMaxLongiLati();
    // 生成图结构
    void createGraph();

public:
    // 获取线路名
    QString getLineName(int l);
    // 获取线路颜色
    QColor getLineColor(int l);
    // 获取线路hash值
    int getLineHash(QString lineName);
    // 获取线路集合hash值
    QList<int> getLinesHash(QList<QString> linesList);
    // 获取线路名集合
    QList<QString> getLinesNameList();
    // 获取线路的所有包含站点
    QList<QString> getLineStationsList(int l);
    // 获取站点名
    QString getStationName(int s);
    // 获取站点地理坐标
    QPointF getStationCoord(int s);
    // 获取最小坐标
    QPointF getMinCoord();
    // 获取最大坐标
    QPointF getMaxCoord();
    // 获取站点所属线路信息
    QList<int> getStationLinesInfo(int s);
    // 获取两个站点的公共所属线路
    QList<int> getCommonLines(int s1, int s2);
    // 获取站点id
    int getStationHash(QString stationName);
    // 获取站点集合id
    QList<QString> getStationsNameList();
    // 获取网络结构，用于前端显示
    void getGraph(QList<int> &stationsList, QList<Edge> &edgesList);
    // 获取最少时间的线路
    bool queryTransferMinTime(int s1, int s2, QList<int> &stationsList, QList<Edge> &edgesList);
    // 获取最少换乘的线路
    bool queryTransferMinTransfer(int s1, int s2, QList<int> &stationsList,
                                  QList<Edge> &edgesList);
    // 从文件读取数据
    bool readFileData(QString fileName);
};

#endif // GRAPH_H
