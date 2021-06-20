const char* indexHtml = "<html>\n"
"<head>\n"
"    <script>\n"
"        window.onload = function () {\n"
"            var brewData = {target: [], output: []};\n"
"            var steamData = {target: [], output: []};\n"
"            var data = {\n"
"                datasets: [{\n"
"                    type: \'line\',\n"
"                    label: \'Temperature\',\n"
"                    data: [],\n"
"                    borderColor: \"#FF6384\",\n"
"                    backgroundColor: \"#FF6384\",\n"
"                    yAxisID: \'y\',\n"
"                }, {\n"
"                    type: \'bar\',\n"
"                    label: \'Output\',\n"
"                    data: brewData.output,\n"
"                    borderColor: \"#63FFDD\",\n"
"                    backgroundColor: \"#63FFDD\",\n"
"                    yAxisID: \'y1\',\n"
"                }],\n"
"            };\n"
"\n"
"            const options = {\n"
"                responsive: true,\n"
"                animations: false,\n"
"                interaction: {\n"
"                    mode: \'nearest\',\n"
"                    axis: \'x\',\n"
"                    intersect: false\n"
"                },\n"
"                plugins: {\n"
"                    decimation: {\n"
"                        enabled: \'true\',\n"
"                        algorithm: \'lttb\',\n"
"                        samples: 10,\n"
"                    },\n"
"                    title: {\n"
"                        display: true,\n"
"                        text: \"Brew Temperature\",\n"
"                    },\n"
"                },\n"
"                scales: {\n"
"                    x: {\n"
"                        type: \'timeseries\',\n"
"                        time: {\n"
"                            unit: \'second\',\n"
"                        },\n"
"                        ticks: {\n"
"                            source: \'data\',\n"
"                            autoSkip: true,\n"
"                            maxRotation: 0,\n"
"                            align: \'start\',\n"
"                        },\n"
"                        title: {\n"
"                            display: true,\n"
"                            text: \"Time\",\n"
"                        },\n"
"                        max: 60,\n"
"                    },\n"
"                    y: {\n"
"                        suggestedMax: 100,\n"
"                        title: {\n"
"                            display: true,\n"
"                            text: \"Temperature\",\n"
"                        },\n"
"                        position: \'left\',\n"
"                    },\n"
"                    y1: {\n"
"                        suggestedMax: 1000,\n"
"                        title: {\n"
"                            display: true,\n"
"                            text: \"Output\",\n"
"                        },\n"
"                        position: \"right\",\n"
"                    },\n"
"                },\n"
"            }\n"
"\n"
"            const actions = [{\n"
"                name: \"Display Brew Temps\",\n"
"                handler() {\n"
"                    chart.options.scales.y.suggestedMax = 100;\n"
"                    chart.options.plugins.title.text = \"Brew Temperature\";\n"
"                    chart.update();\n"
"                },\n"
"            }, {\n"
"                name: \"Display Steam Temps\",\n"
"                handler() {\n"
"                    chart.options.scales.y.suggestedMax = 145;\n"
"                    chart.options.plugins.title.text = \"Steam Temperature\";\n"
"                    chart.update();\n"
"                }\n"
"            }];\n"
"\n"
"            const ctx = document.getElementById(\'tempChart\').getContext(\'2d\');\n"
"            var chart = new Chart(ctx, {\n"
"                data: data,\n"
"                options: options,\n"
"            });\n"
"\n"
"            const container = document.getElementById(\'tempContainer\');\n"
"            actions.forEach(function(action){\n"
"                const button = document.createElement(\'button\');\n"
"                button.innerHTML = action.name;\n"
"                button.onclick = action.handler;\n"
"                container.appendChild(button);\n"
"            });\n"
"\n"
"            var xhttp = new XMLHttpRequest();\n"
"            xhttp.onreadystatechange = function(){\n"
"                if (this.readyState == 4 && this.status == 200) {\n"
"                    var tempData = JSON.parse(this.response);\n"
"                    var time = Date.now();\n"
"                    chart.data.datasets[0].data.push({x: time, y: tempData.temperature});\n"
"                    brewData.target.push({x: time, y: tempData.brew.target});\n"
"                    brewData.output.push({x: time, y: tempData.brew.output});\n"
"                    steamData.target.push({x: time, y: tempData.steam.target});\n"
"                    steamData.output.push({x: time, y: tempData.steam.output});\n"
"\n"
"                    if(chart.data.datasets[0].data.length > 300) {\n"
"                        chart.data.datasets[0].data.shift();\n"
"                        brewData.target.shift();\n"
"                        steamData.target.shift();\n"
"                        brewData.output.shift();\n"
"                        steamData.output.shift();\n"
"                    }\n"
"                    chart.update();\n"
"                    pollServer(this);\n"
"                }\n"
"            }\n"
"            function pollServer(xhttp) {\n"
"                xhttp.open(\"GET\", \"/get_temps\", true);\n"
"                xhttp.send();\n"
"            }\n"
"            pollServer(xhttp);\n"
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