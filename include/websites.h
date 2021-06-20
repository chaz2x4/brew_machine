const char* indexHtml = 
"<html>\n"
"<head>\n"
"    <script>\n"
"        window.onload = function () {\n"
"            var NUM_POINTS = 1000;\n"
"            var start = 0;\n"
"            var pointData = [];\n"
"            for (let i = 0; i < NUM_POINTS; ++i) {\n"
"                const max = Math.random() < 0.001 ? 100 : 20;\n"
"                pointData.push({x: start + (i * 30000), y: Math.floor(Math.random() * max) + 1});\n"
"            }\n"
"\n"
"            var tempOutput = pointData;\n"
"            var data = {\n"
"                datasets: [{\n"
"                    label: 'Brew Temperature',\n"
"                    data: tempOutput,\n"
"                }],\n"
"            };\n"
"\n"
"            const options = {\n"
"                interaction: {\n"
"                    mode: 'nearest',\n"
"                    axis: 'x',\n"
"                    intersect: false\n"
"                },\n"
"                plugins: {\n"
"                    decimation: {\n"
"                        enabled: 'true',\n"
"                        algorithm: 'lttb',\n"
"                        samples: 500\n"
"                    }\n"
"                },\n"
"                scales: {\n"
"                    x: {\n"
"                        type: 'timeseries',\n"
"                        time: {\n"
"                            unit: 'second',\n"
"                        },\n"
"                        ticks: {\n"
"                            source: 'data',\n"
"                        },\n"
"                    },\n"
"                },\n"
"            }\n"
"\n"
"            const actions = [{\n"
"                name: \"Display Brew Temps\",\n"
"                handler() {\n"
"                    chart.data.datasets.forEach( dataset => {\n"
"                        dataset.data = [];\n"
"                    });\n"
"                    chart.update();\n"
"                },\n"
"            }, {\n"
"                name: \"Display Steam Temps\",\n"
"                handler() {\n"
"                    chart.data.datasets.forEach( dataset => {\n"
"                        dataset.data = pointData;\n"
"                    });\n"
"                    chart.update();\n"
"                }\n"
"            }];\n"
"\n"
"            const ctx = document.getElementById('tempChart').getContext('2d');\n"
"            var chart = new Chart(ctx, {\n"
"                type: 'line',\n"
"                data: data,\n"
"                options: options,\n"
"            });\n"
"\n"
"            const container = document.getElementById('tempContainer');\n"
"            actions.forEach(function(action){\n"
"                const button = document.createElement('button');\n"
"                button.innerHTML = action.name;\n"
"                button.onclick = action.handler;\n"
"                container.appendChild(button);\n"
"            });\n"
"\n"
"            var xhttp = new XMLHttpRequest();\n"
"            xhttp.onreadystatechange = function(){\n"
"                if (this.readyState == 4 && this.status == 200) {\n"
"                    console.log(this);\n"
"                }\n"
"            }\n"
"            xhttp.open(\"GET\", \"http://gaggia.local/get_temps\", false);\n"
"            xhttp.send();\n"
"        }\n"
"    </script>\n"
"</head>\n"
"<body>\n"
"    <div id=\"tempContainer\" class=\"chart-container\" style=\"position:relative; height:40vh; width:120vh; margin:auto;\">\n"
"        <canvas id=\"tempChart\"></canvas>\n"
"    </div>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/chart.js@3.3.2\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/moment@2.29.1\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/chartjs-adapter-moment@0.1.1\"></script>\n"
"</body>\n"
"</html>";