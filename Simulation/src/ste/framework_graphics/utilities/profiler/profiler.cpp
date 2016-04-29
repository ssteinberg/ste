
#include "stdafx.hpp"
#include "profiler.hpp"

#include <fstream>

using namespace StE::Graphics;

profiler::~profiler() {
	std::ofstream f(log_path);
	f << R"(<html> <head> <script type="text/javascript" src="https://www.google.com/jsapi"></script> <script type="text/javascript">google.load('visualization', '1',{packages: ['controls']}); google.setOnLoadCallback(drawVisualization); function drawVisualization(){var dashboard=new google.visualization.Dashboard( document.getElementById('dashboard')); var control=new google.visualization.ControlWrapper({'controlType': 'ChartRangeFilter', 'containerId': 'control', 'options':{'filterColumnIndex': 1, 'ui':{'chartType': 'LineChart', 'chartOptions':{'width': 1700px, 'height': 70, 'chartArea':{width: '80%', height: '80%'}, 'hAxis':{'baselineColor': 'none'}}, 'chartView':{'columns': [1, 2]}}},}); var chart=new google.visualization.ChartWrapper({'chartType': 'Timeline', 'containerId': 'chart', 'options':{'width': 1700px, 'height': 1000, 'chartArea':{width: '80%', height: '80%'}, 'backgroundColor': '#ffd'}, 'view':{'columns': [0, 1, 2]}}); var data=new google.visualization.DataTable(); data.addColumn({type: 'string', id: 'TestAction'}); data.addColumn({type: 'date', id: 'Start'}); data.addColumn({type: 'date', id: 'End'}); data.addRows([)" << std::endl;

	for (auto &e : entries)
		f << "[ '" << e.name << "', new Date(" << std::to_string(e.start) << "), new Date(" << std::to_string(e.end) << ") ]," << std::endl;

	f << R"(]); dashboard.bind(control, chart); dashboard.draw(data);}</script> </head> <body style="font-family: Arial;border: 0 none;"> <div id="dashboard" style="width:1700px;overflow:scroll;"> <div id="chart" style="position: relative; width: 1700px; height: 1100px;"></div><div id="control"></div></div><div id="junk_div" style="display: none;"></div></body></html>)" << std::endl;
}
