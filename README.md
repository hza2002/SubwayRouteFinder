<div style="display: flex; justify-content: center;">
  <img src="assets/icon-readme.png" style="max-width: 200px; height: auto;">
</div>
<br>
<h1 style="text-align: center;">Subway Route Finder</h1>

This system, developed using the **Qt** framework, offers a convenient and fast **Chongqing** subway route query function. It empowers users to effortlessly access subway lines, station information, and obtain optimal route planning.

![Preview](assets/preview.jpg)

# Program functionality

To achieve efficient data management and query functionality, we have selected appropriate data structures and algorithms such as graph representation and traversal, shortest path algorithms, and more.

![Features](assets/features.jpeg)

# Build

1. Clone the repository using Git.
2. Open the subway.app project file in Qt (The project is developed using Qt version 6.5.1).
3. Simply click on the "Build" button to generate the application on your system.

# Update subway information

The program utilizes Python for data crawling. The web crawler program is located in `data/crawler.py`. Execute the Python script to crawl and collect all subway line data.

The program reads `subway.txt` located in the project's root directory.

To update the data, modify the crawled data file to match the required format expected by the program.

The program expects the data to be in a specific format. Please ensure that the crawled data is edited to meet the required format specified by the program.

Required Data Format(Do not copy the comments, otherwise an error will occur):
```cpp
id: 1 // Subway line id, should be a unique number
name: 轨道交通1号线 // Subway Line Name
colour: #E70012 // The color used when drawing this subway line on the app
totalStations: 2 // The number of all stations on the subway line
1 朝天门 106.59387953085015,29.572352258313543 // Station information in this format
2 小什字 106.5900804816662,29.566137891130445 // id, name, latitude and longitude
... // More stations
```