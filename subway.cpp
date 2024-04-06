#include "subway.h"

double Station::minLongitude = 200;
double Station::minLatitude = 200;
double Station::maxLongitude = 0;
double Station::maxLatitude = 0;

// 角度转弧度
double rad(double d)
{
    return d * M_PI / 180;
}

// 构造函数
Station::Station(QString name, double longitude, double latitude, QList<int> linesList)
    : name(name)
    , longitude(longitude)
    , latitude(latitude)
{
    for (const int &item : linesList) {
        belongLines.insert(item);
    };
}

// 站点间距离
double Station::distance(Station other)
{
    const double EARTH_RADIUS = 6378.137;
    double radLat1 = rad(latitude);
    double radLat2 = rad(other.latitude);
    double a = radLat1 - radLat2;
    double b = rad(longitude) - rad(other.longitude);
    double dis = 2
                 * asin(sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2)));
    dis *= EARTH_RADIUS;
    return dis;
}
