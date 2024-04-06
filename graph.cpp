#include "graph.h"

// 从文件读取数据
bool Graph::readFileData(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    QTextStream in(&file);
    while (!in.atEnd()) {
        Line line;
        QString id, name, colour, totalStations, color;
        bool ok;
        int total;
        int lvIndex, svIndex1, svIndex2;

        in >> id >> line.id;
        in >> name >> line.name;
        in >> colour >> color;
        line.color.setRgba(color.remove(0, 1).toUInt(&ok, 16));
        in >> totalStations >> total;

        if (linesHash.count(line.name)) {
            lvIndex = linesHash[line.name];
        } else {
            lvIndex = linesHash[line.name] = lines.size();
            lines.push_back(line);
        }

        Station station;
        QString longlat;
        QStringList strList;
        for (int i = 0; !in.atEnd() && i < total; ++i) {
            in >> station.id >> station.name >> longlat;
            strList = longlat.split(',');
            station.longitude = strList.first().toDouble();
            station.latitude = strList.last().toDouble();

            if (stationsHash.count(station.name)) {
                svIndex2 = stationsHash[station.name];
            } else {
                svIndex2 = stationsHash[station.name] = stations.size();
                stations.push_back(station);
            }
            
            stations[svIndex2].belongLines.insert(lvIndex);
            lines[lvIndex].stationsSet.insert(svIndex2);

            if (i) {
                lines[lvIndex].edges.insert(Edge(svIndex1, svIndex2));
                lines[lvIndex].edges.insert(Edge(svIndex2, svIndex1));
                insertEdge(svIndex1, svIndex2);
            }
            svIndex1 = svIndex2;
        }

        bool flag = id == "id:" && name == "name:" && colour == "colour:"
                    && totalStations == "totalStations:" && ok && !in.atEnd();

        if (flag == false) {
            file.close();
            clearData();
            return false;
        }
        in.readLine();
    }
    file.close();

    updateMinMaxLongiLati();

    return true;
}

// dijkstra获取最少时间的线路
bool Graph::queryTransferMinTime(int start,
                                 int end,
                                 QList<int> &stationsList,
                                 QList<Edge> &edgesList)
{
    const long long INF = 999999999;
    stationsList.clear();
    edgesList.clear();

    if (start == end) {
        stationsList.push_back(start);
        stationsList.push_back(end);
        return true;
    }

    createGraph();

    std::vector<int> path(stations.size(), -1);
    std::vector<double> dist(stations.size(), INF);

    dist[start] = 0;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    pq.push(Node(start, 0));
    while (!pq.empty()) {
        Node top = pq.top();
        pq.pop();
        if (top.stationID == end) {
            break;
        }
        for (int i = 0; i < graph[top.stationID].size(); ++i) {
            Node &adjNode = graph[top.stationID][i];
            if (top.distance + adjNode.distance < dist[adjNode.stationID]) {
                path[adjNode.stationID] = top.stationID;
                dist[adjNode.stationID] = top.distance + adjNode.distance;
                pq.push(Node(adjNode.stationID, dist[adjNode.stationID]));
            }
        }
    }

    if (path[end] == -1) {
        return false;
    }

    int p = end;
    while (path[p] != -1) {
        stationsList.push_front(p);
        edgesList.push_front(Edge(path[p], p));
        p = path[p];
    }
    stationsList.push_front(start);

    return true;
}

// bfs获取最少换乘的线路
bool Graph::queryTransferMinTransfer(int start,
                                     int end,
                                     QList<int> &stationsList,
                                     QList<Edge> &edgesList)
{
    stationsList.clear();
    edgesList.clear();
    if (start == end) {
        stationsList.push_back(start);
        stationsList.push_back(end);
        return true;
    }
    std::vector<bool> linesVisted(lines.size(), false);
    std::vector<int> path(stations.size(), -1);
    path[start] = -2;
    std::queue<int> q;
    q.push(start);
    while (!q.empty()) {
        int top = q.front();
        q.pop();
        for (auto l = stations[top].belongLines.begin(); l != stations[top].belongLines.end(); l++) {
            if (!linesVisted[*l]) {
                linesVisted[*l] = true;
                for (auto s = lines[*l].stationsSet.begin(); s != lines[*l].stationsSet.end(); s++) {
                    if (path[*s] == -1) {
                        path[*s] = top;
                        q.push(*s);
                    }
                }
            }
        }
    }
    if (path[end] == -1) {
        return false;
    } else {
        int p = end;
        while (path[p] != -2) {
            stationsList.push_front(p);
            edgesList.push_front(Edge(path[p], p));
            p = path[p];
        }
        stationsList.push_front(start);
        return true;
    }
}

// 清空数据
void Graph::clearData()
{
    stations.clear();
    lines.clear();
    stationsHash.clear();
    linesHash.clear();
    edges.clear();
    graph.clear();
}

// 插入一条边
bool Graph::insertEdge(int n1, int n2)
{
    if (edges.contains(Edge(n1, n2)) || edges.contains(Edge(n2, n1))) {
        return false;
    }
    edges.insert(Edge(n1, n2));
    return true;
}

// 生成图结构
void Graph::createGraph()
{
    graph.clear();
    graph = QVector<QVector<Node>>(stations.size(), QVector<Node>());
    for (auto e = edges.begin(); e != edges.end(); e++) {
        double dist = stations[e->first].distance(stations[e->second]);
        graph[e->first].push_back(Node(e->second, dist));
        graph[e->second].push_back(Node(e->first, dist));
    }
}

// 获取线路颜色
QColor Graph::getLineColor(int l)
{
    return lines[l].color;
}

// 获取线路名
QString Graph::getLineName(int l)
{
    return lines[l].name;
}

// 获取线路hash值
int Graph::getLineHash(QString lineName)
{
    return linesHash.contains(lineName) ? linesHash[lineName] : -1;
}

// 获取线路集合hash值
QList<int> Graph::getLinesHash(QList<QString> linesList)
{
    QList<int> hashList;
    for (auto &a : linesList) {
        hashList.push_back(getLineHash(a));
    }
    return hashList;
}

// 获取线路名集合
QList<QString> Graph::getLinesNameList()
{
    QList<QString> linesNameList;
    for (auto l = lines.begin(); l != lines.end(); l++) {
        linesNameList.push_back(l->name);
    }
    return linesNameList;
}

// 获取线路的所有包含站点
QList<QString> Graph::getLineStationsList(int l)
{
    QList<QString> stationsList;
    for (auto s = lines[l].stationsSet.begin(); s != lines[l].stationsSet.end(); s++) {
        stationsList.push_back(stations[*s].name);
    }
    return stationsList;
}

// 更新边界经纬度
void Graph::updateMinMaxLongiLati()
{
    double minLongitude = 200, minLatitude = 200;
    double maxLongitude = 0, maxLatitude = 0;
    for (auto &s : stations) {
        minLongitude = std::min(minLongitude, s.longitude);
        minLatitude = std::min(minLatitude, s.latitude);
        maxLongitude = std::max(maxLongitude, s.longitude);
        maxLatitude = std::max(maxLatitude, s.latitude);
    }
    Station::minLongitude = minLongitude;
    Station::minLatitude = minLatitude;
    Station::maxLongitude = maxLongitude;
    Station::maxLatitude = maxLatitude;
}

// 获取站点最小坐标
QPointF Graph::getMinCoord()
{
    return QPointF(Station::minLongitude, Station::minLatitude);
}

// 获取站点最大坐标
QPointF Graph::getMaxCoord()
{
    return QPointF(Station::maxLongitude, Station::maxLatitude);
}

// 获取两个站点的公共所属线路
QList<int> Graph::getCommonLines(int s1, int s2)
{
    QList<int> linesList;
    for (auto s = stations[s1].belongLines.begin(); s != stations[s1].belongLines.end(); ++s) {
        if (stations[s2].belongLines.contains(*s))
            linesList.push_back(*s);
    }
    return linesList;
}

// 获取站点名
QString Graph::getStationName(int s)
{
    return stations[s].name;
}

// 获取站点地理坐标
QPointF Graph::getStationCoord(int s)
{
    return QPointF(stations[s].longitude, stations[s].latitude);
}

// 获取站点所属线路信息
QList<int> Graph::getStationLinesInfo(int s)
{
    QList<int> returnList;
    for (auto i = stations[s].belongLines.begin(); i != stations[s].belongLines.end(); ++i) {
        returnList.push_back(*i);
    }
    return returnList;
}

// 获取站点id
int Graph::getStationHash(QString stationName)
{
    return stationsHash.contains(stationName) ? stationsHash[stationName] : -1;
}

// 获取站点集合id
QList<QString> Graph::getStationsNameList()
{
    QList<QString> list;
    for (auto &s : stations) {
        list.push_back(s.name);
    }
    return list;
}

// 获取网络结构，用于显示
void Graph::getGraph(QList<int> &stationsList, QList<Edge> &edgesList)
{
    stationsList.clear();
    for (int i = 0; i < stations.size(); ++i) {
        stationsList.push_back(i);
    }
    for (auto e = edges.begin(); e != edges.end(); e++) {
        edgesList.push_back(*e);
    }
}
