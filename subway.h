#ifndef SUBWAY_H
#define SUBWAY_H

#include <QColor>
#include <QPair>
#include <QPointF>
#include <QSet>
#include <QString>
#include <QVector>
#include <cmath>

class Graph;
class QTextStream;

// 边
typedef QPair<int, int> Edge;

// 站点
class Station
{
private:
    int id;                     // 站点ID
    QString name;               // 站点名称
    double longitude, latitude; // 经纬度
    QSet<int> belongLines;      // 站点所属线路

    // 站点的边界位置
    static double minLongitude, minLatitude, maxLongitude, maxLatitude;

    // 站点间距离
    double distance(Station other);

public:
    // 构造函数
    Station() = default;
    Station(QString name, double longitude, double latitude, QList<int> linesList);

    // 声明友元
    friend class Graph;
    friend class QTextStream;
};

// 线路
class Line
{
private:
    int id;                // 线路ID
    QString name;          // 线路名称
    QColor color;          // 线路颜色
    QSet<int> stationsSet; // 线路包含站点
    QSet<Edge> edges;      // 线路站点连接关系

public:
    //构造函数
    Line() = default;
    Line(QString name, QColor color)
        : name(name)
        , color(color){};

    //声明友元
    friend class Graph;
    friend class QTextStream;
};

#endif // SUBWAY_H
