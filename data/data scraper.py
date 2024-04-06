from datetime import datetime
import bs4
import pandas as pd
import requests
from retrying import retry
import random


# 坐标转换函数，高德坐标转换为百度坐标
@retry(stop_max_attempt_number=100)
def LocTransform(loc):
    old_loc = loc  # 经度,纬度
    key = "GgUZ8eHC7RrrOQZkWr5R575tfEFfwo7c"  # 百度Api
    url = "https://api.map.baidu.com/geoconv/v1/?coords={}&from=3&to=5&ak={}".format(
        old_loc, key)  # 高德to百度
    re = requests.get(url)
    js = re.json()
    lng = js["result"][0]["x"]
    lat = js["result"][0]["y"]
    new_loc = str(lng) + "," + str(lat)
    return new_loc


# 随机生成十六进制颜色代码
def generate_random_color():
    # 生成三个随机整数作为RGB值
    r = random.randint(0, 255)
    g = random.randint(0, 255)
    b = random.randint(0, 255)
    # 将RGB值转换为十六进制，并格式化为颜色代码
    color_code = "#{:02x}{:02x}{:02x}".format(r, g, b)
    return color_code


# 获取高德地铁数据
def gaode_subway():
    # 获取城市列表
    url = "http://map.amap.com/subway/index.html"
    res = requests.get(url)
    res.encoding = res.apparent_encoding
    soup = bs4.BeautifulSoup(res.text, "html.parser")

    name_dict = []
    # 获取显示出的城市列表
    for soup_a in soup.find("div", class_="city-list fl").find_all("a"):
        city_name_py = soup_a["cityname"]
        city_id = soup_a["id"]
        city_name_ch = soup_a.get_text()
        name_dict.append({
            "name_py": city_name_py,
            "id": city_id,
            "name_ch": city_name_ch
        })
    # 获取未显示出来的城市列表
    for soup_a in soup.find("div", class_="more-city-list").find_all("a"):
        city_name_py = soup_a["cityname"]
        city_id = soup_a["id"]
        city_name_ch = soup_a.get_text()
        name_dict.append({
            "name_py": city_name_py,
            "id": city_id,
            "name_ch": city_name_ch
        })
    df_name = pd.DataFrame(name_dict)

    # 通过id筛选出需要的城市，5000为重庆id，注意id为字符串
    id_list = ["5000"]  # 保留的id
    df_name = df_name[df_name["id"].isin(id_list)]

    # 将城市列表构造为明细数据URL
    url_list = []

    for tup in zip(df_name["id"], df_name["name_py"]):
        now = datetime.now()
        timeStamp = int(now.timestamp() * 1000)
        url = "http://map.amap.com/service/subway?_{}&srhdata={}_drw_{}.json".format(
            timeStamp, tup[0], tup[1])
        url_list.append(url)

    # 抓取各个城市地铁数据
    subway_LineStation_list = []

    for city_url in url_list:
        r = requests.get(city_url)
        if r.status_code == 200:
            data_json = r.json()
            city = data_json["s"].replace("地铁", "")  # 城市名称
            d1 = data_json["l"]
            for i in d1:
                kn = i["kn"]  # 线路名称
                line_id = i["ls"]  # 线路id
                st = i["st"]
                n = 0  # 序号
                for i2 in st:
                    n += 1
                    name = i2["n"]  # 站点名称
                    station_sid = i2["sid"]  # 站点id
                    sl = i2["sl"]  # 经纬度
                    if i2["su"] == "1":
                        station_loc_baidu = LocTransform(sl)
                        subway_LineStation_list.append([
                            city, kn, n, name, station_loc_baidu, line_id,
                            station_sid
                        ])
                        # 城市 线路名称 序号 站点名称 百度经纬坐标 路线id 站点id
                        print(city, kn, n, name, station_loc_baidu, line_id,
                              station_sid)

    # 最终的数据就是subway_LineStation_list
    # 格式化输出到文件：
    filename = "subway.txt"
    with open(filename, 'w') as file:
        for line in subway_LineStation_list:
            city, line_name, sequence, station_name, coordinates, line_id, station_id = line
            file.write(f"{sequence} {station_name} {coordinates}\n")


gaode_subway()