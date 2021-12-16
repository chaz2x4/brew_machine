const char* indexHtml = "<html >\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
"    <title>Gaggia - Homebrew Server</title>\n"
"    <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css\" integrity=\"sha256-djO3wMl9GeaC/u6K+ic4Uj/LKhRUSlUFcsruzS7v5ms=\" crossorigin=\"anonymous\">\n"
"    <script>\n"
"        window.onload = function () {\n"
"            var brewTarget;\n"
"            var steamTarget;\n"
"            var tempOffset;\n"
"            var cycleTime = 2000;\n"
"            var mode = \'brew\';\n"
"            var tempData = [];\n"
"            var brewData = [];\n"
"            var steamData = [];\n"
"            var labels = [];\n"
"            var data = {\n"
"                labels: labels,\n"
"                datasets: [{\n"
"                    type: \'line\',\n"
"                    label: \'Temperature\',\n"
"                    data: [],\n"
"                    backgroundColor: function (context) {\n"
"                        var pointTemp = context.dataset.data[context.dataIndex];\n"
"                        var targetTemp = (mode == \'brew\') ? brewTarget : steamTarget;\n"
"                        if(pointTemp > (targetTemp + 0.5)) return \'#FF6384\';\n"
"                        if(pointTemp < (targetTemp - 0.5)) return \'#331F2E\';\n"
"                        else return \'#63FF8F\';\n"
"                    },\n"
"                    tension: 0.4,\n"
"                    yAxisID: \'y\',\n"
"                }, {\n"
"                    type: \'bar\',\n"
"                    label: \'Output\',\n"
"                    data: brewData,\n"
"                    backgroundColor: \"#63FFDD\",\n"
"                    yAxisID: \'y1\',\n"
"                }],\n"
"            };\n"
"\n"
"            const options = {\n"
"                responsive: true,\n"
"                interaction: {\n"
"                    mode: \'nearest\',\n"
"                    axis: \'x\',\n"
"                    intersect: false\n"
"                },\n"
"                plugins: {\n"
"                    title: {\n"
"                        display: true,\n"
"                        text: \"Brew Temperature\",\n"
"                    },\n"
"                },\n"
"                scales: {\n"
"                    x: {\n"
"                        type: \'time\',\n"
"                        time: {\n"
"                            unit: \'second\',\n"
"                        },\n"
"                        title: {\n"
"                            display: true,\n"
"                            text: \"Time\",\n"
"                        },\n"
"                        spanGaps: true,\n"
"                    },\n"
"                    y: {\n"
"                        title: {\n"
"                            display: true,\n"
"                            text: \"Temperature\",\n"
"                        },\n"
"                        position: \'left\',\n"
"                        grid: {\n"
"                            color: function(context) {\n"
"                                if(context.tick.value == brewTarget) return \"#FF6384\";\n"
"                            },\n"
"                        },\n"
"                    },\n"
"                    y1: {\n"
"                        suggestedMax: cycleTime,\n"
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
"                name: \"Brew Temps\",\n"
"                id: \"brew\",\n"
"                handler() {\n"
"                    chart.options.scales.y.suggestedMin = brewTarget - 1;\n"
"                    chart.options.scales.y.suggestedMax = brewTarget + 1;\n"
"                    chart.options.plugins.title.text = \"Brew Temperature\";\n"
"                    chart.options.scales.y.grid.color = function(context) {\n"
"                        if(context.tick.value == brewTarget) return \"#FF6384\";\n"
"                    };\n"
"                    chart.data.datasets[1].data = brewData;\n"
"                    mode = \'brew\';\n"
"                    updateElements();\n"
"                    chart.render();\n"
"                    refreshTunings();\n"
"                },\n"
"            }, {\n"
"                name: \"Steam Temps\",\n"
"                id: \"steam\",\n"
"                handler() {\n"
"                    chart.options.scales.y.suggestedMin = steamTarget - 1;\n"
"                    chart.options.scales.y.suggestedMax = steamTarget + 1;\n"
"                    chart.options.plugins.title.text = \"Steam Temperature\";\n"
"                    chart.options.scales.y.grid.color = function(context) {\n"
"                        if(context.tick.value == steamTarget) return \"#FF6384\";\n"
"                    };\n"
"                    chart.data.datasets[1].data = steamData;\n"
"                    mode = \'steam\';\n"
"                    updateElements();\n"
"                    chart.render();\n"
"                    refreshTunings();\n"
"                }\n"
"            }];\n"
"            \n"
"            function updateElements(){\n"
"                var targetTemp = (mode == \"brew\") ? brewTarget : steamTarget;\n"
"\n"
"                var targetLabel = document.getElementById(\"target_label\");\n"
"                targetLabel.innerHTML = \"Target: \" + targetTemp + \" C\";\n"
"\n"
"                var switchButton = document.getElementById(\"switch_\" + mode);\n"
"                switchButton.classList.add(\"active\");\n"
"\n"
"                var inactiveSwitch = (mode == \"brew\") ? document.getElementById(\"switch_steam\") : document.getElementById(\"switch_brew\");\n"
"                inactiveSwitch.classList.remove(\"active\");\n"
"\n"
"                var offsetLabel = document.getElementById(\"offset_label\");\n"
"                offsetLabel.innerHTML = \"Offset: \" + tempOffset + \" C\";\n"
"\n"
"                chart.options.scales.y.suggestedMin = targetTemp - 1;\n"
"                chart.options.scales.y.suggestedMax = targetTemp + 1;\n"
"\n"
"                chart.update();\n"
"            }\n"
"\n"
"            function incrementTarget(){\n"
"                var maxTemp = (mode == \"brew\") ? 105 : 155;\n"
"                if(mode == \"brew\") brewTarget = (brewTarget < maxTemp) ? brewTarget + 0.5 : maxTemp;\n"
"                else steamTarget = (steamTarget < maxTemp) ? steamTarget + 1 : maxTemp;\n"
"                callServer.post(\"/increment_target\", mode);\n"
"                updateElements();\n"
"            }\n"
"\n"
"            function decrementTarget(){\n"
"                var minTemp = (mode == \"brew\") ? 85 : 145;\n"
"                if(mode == \"brew\") brewTarget = (brewTarget > minTemp) ? brewTarget - 0.5 : minTemp;\n"
"                else steamTarget = (steamTarget > minTemp) ? steamTarget - 1 : minTemp;\n"
"                callServer.post(\"/decrement_target\", mode);\n"
"                updateElements();\n"
"            }\n"
"\n"
"            function incrementOffset(){\n"
"                if(tempOffset < 15) tempOffset += 0.5;\n"
"                callServer.put(\"/set_offset\", tempOffset);\n"
"                updateElements();\n"
"            }\n"
"\n"
"            function decrementOffset(){\n"
"                if(tempOffset > -15) tempOffset -= 0.5;\n"
"                callServer.put(\"/set_offset\", tempOffset);\n"
"                updateElements();\n"
"            }\n"
"\n"
"            var tunings_open = false;\n"
"            function changeTunings() {\n"
"                var button = this;\n"
"                if(tunings_open) {\n"
"                    button.classList.remove(\'active\');\n"
"                    button.innerHTML = \"Setting Tunings...\";\n"
"                    button.disabled = true;\n"
"                    tunings_open = false;\n"
"                    var kp = document.getElementById(\"kp\");\n"
"                    var ki = document.getElementById(\"ki\");\n"
"                    var kd = document.getElementById(\"kd\");\n"
"                    var tunings = [mode, kp.value, ki.value, kd.value];\n"
"                    callServer.put(\"/set_tunings\", tunings , function(request){\n"
"                        if(request) {\n"
"                            if(request.status == 200){\n"
"                                button.disabled = false;\n"
"                                button.innerHTML = \"Set Tunings:\";\n"
"                            }\n"
"                        }\n"
"                    })\n"
"                }\n"
"                else {\n"
"                    button.classList.add(\'active\');\n"
"                    button.innerHTML = \"Submit\";\n"
"                    tunings_open = true;\n"
"                }\n"
"            }\n"
"\n"
"            const ctx = document.getElementById(\'tempChart\').getContext(\'2d\');\n"
"            var chart = new Chart(ctx, {\n"
"                data: data,\n"
"                options: options,\n"
"            });\n"
"\n"
"            const container = document.getElementById(\'tempContainer\');\n"
"            actions.forEach(function(action){\n"
"                const button = document.getElementById(\"switch_\" + action.id);\n"
"                button.onclick = action.handler;\n"
"            });\n"
"\n"
"            var callServer = (function() {\n"
"                function request(url, method, data, callback){\n"
"                    var xhttp = new XMLHttpRequest();\n"
"                    xhttp.onreadystatechange = function(){\n"
"                        if (this.readyState == 4 && this.status == 200) {\n"
"                            if(callback) callback(this);\n"
"                        }\n"
"                    }\n"
"                    xhttp.open(method, url, true);\n"
"                    xhttp.send(data);\n"
"                }\n"
"\n"
"                return {\n"
"                    get: function(url, callback){\n"
"                        request(url, \"GET\", null, callback);\n"
"                    },\n"
"                    post: function(url, data, callback){\n"
"                        request(url, \"POST\", data, callback);\n"
"                    },\n"
"                    put: function(url, data, callback) {\n"
"                        request(url, \"PUT\", data, callback);\n"
"                    }\n"
"                }\n"
"            })();\n"
"\n"
"            (function refreshTemps(){\n"
"                return callServer.get(\"/get_temps\", function(request){\n"
"                    var requestedData = JSON.parse(request.response);\n"
"                    var requestedLabels = [];\n"
"                    var requestedTempData = [];\n"
"                    var requestedBrewData = [];\n"
"                    var requestedSteamData = [];\n"
"\n"
"                    if(brewTarget == null) [brewTarget, steamTarget, tempOffset] = [requestedData.targets.brew, requestedData.targets.steam, requestedData.targets.offset];\n"
"\n"
"                    var sortedOutputs = requestedData.outputs.sort((a, b) => a.time - b.time);\n"
"\n"
"                    sortedOutputs.forEach(output => {\n"
"                        requestedLabels.push(output.time * 1000);\n"
"                        requestedTempData.push(output.temperature);\n"
"                        requestedBrewData.push(output.outputs.brew);\n"
"                        requestedSteamData.push(output.outputs.steam);\n"
"                    })\n"
"\n"
"                    chart.data.labels = requestedLabels;\n"
"                    chart.data.datasets[0].data = tempData = requestedTempData;\n"
"                    if(mode == \'brew\') { chart.data.datasets[1].data = brewData = requestedBrewData }\n"
"                    else { chart.data.datasets[1].data = steamData = requestedSteamData }        \n"
"\n"
"                    updateElements();\n"
"                    refreshTemps();\n"
"                });\n"
"            })();\n"
"\n"
"            function refreshTunings() {\n"
"                callServer.get(\"/get_tunings/\" + mode, function(request){\n"
"                    var requestedData = JSON.parse(request.response);\n"
"                    Object.entries(requestedData).forEach(entry => {\n"
"                        const [name, value] = entry;\n"
"                        var input = document.getElementById(name);\n"
"                        input.value = value;\n"
"                    });\n"
"                });\n"
"            }\n"
"\n"
"            refreshTunings();\n"
"            document.getElementById(\"targetplus\").onclick = incrementTarget;\n"
"            document.getElementById(\"targetminus\").onclick = decrementTarget;\n"
"            document.getElementById(\"offsetplus\").onclick = incrementOffset;\n"
"            document.getElementById(\"offsetminus\").onclick = decrementOffset;\n"
"            document.getElementById(\"tuning_button\").onclick = changeTunings;\n"
"        }\n"
"    </script>\n"
"</head>\n"
"<body class=\"container-sm\">\n"
"    <div class=\"row p-2\" id=\"chartContainer\">\n"
"        <canvas id=\"tempChart\"></canvas>\n"
"    </div>\n"
"    <div class=\"row p-2 row-cols-2 row-cols-sm-5 align-items-center\" id=\"tempContainer\">\n"
"        <div class=\"col order-1\">\n"
"            <button class=\"btn btn-outline-primary active\" id=\"switch_brew\"> Water Temp </button>\n"
"        </div>\n"
"        <div class=\" col order-3 me-sm-auto\">\n"
"            <button class=\"btn btn-outline-primary\" id=\"switch_steam\"> Steam Temp </button>\n"
"        </div>\n"
"        <div class=\"col row row-cols-1 row-cols-xl-2 order-2 order-sm-3\"> \n"
"            <label class=\"form-label col\" id=\"target_label\"> Target: 100 C</label>\n"
"            <div class=\"col-2 btn-group\">\n"
"                <button class=\"btn btn-outline-danger\" id=\"targetplus\"> + </button>\n"
"                <button class=\"btn btn-outline-secondary\" id=\"targetminus\"> - </button>\n"
"            </div>\n"
"        </div>\n"
"        <div class=\"col row row-cols-1 row-cols-xl-2 order-4\"> \n"
"            <label class=\"form-label col\" id=\"offset_label\"> Offset: -8 C</label>\n"
"            <div class=\"col-2 btn-group\">\n"
"                <button class=\"btn btn-outline-danger\" id=\"offsetplus\"> + </button>\n"
"                <button class=\"btn btn-outline-secondary\" id=\"offsetminus\"> - </button>\n"
"            </div>\n"
"        </div>\n"
"    </div>\n"
"    <div class=\"row p-2\" id=\"tunerContainer\">\n"
"        <p>\n"
"            <button class=\"btn btn-outline-primary\" type=\"button\" id=\"tuning_button\" data-bs-toggle=\"collapse\" data-bs-target=\"#tunings_tab\">\n"
"                Set Tunings:\n"
"            </button>\n"
"        </p>\n"
"        <div class=\"collapse\" id=\"tunings_tab\">\n"
"            <form class=\"accordion-body\" id=\"tunings-form\">\n"
"                <div class=\"row\">\n"
"                    <div class=\"tuning-button input-group col\">\n"
"                        <span for=\"kp\" class=\"input-group-text\"> Kp: </span>\n"
"                        <input class=\"tuning-input form-control\" type=\"text\" id=\"kp\" value=\"\" placeholder=\"Proportional\" min=\"0\">\n"
"                        <button class=\"tuning-button btn btn-outline-secondary\" type=\"button\" id=\"kpplus\"> + </button>\n"
"                        <button class=\"tuning-button btn btn-outline-secondary\" type=\"button\" id=\"kpminus\"> - </button>\n"
"                    </div>\n"
"                    <div class=\"tuning-button input-group col\">\n"
"                        <span for=\"kp\" class=\"input-group-text\"> Ki: </span>\n"
"                        <input class=\"tuning-input form-control\" type=\"text\" id=\"ki\" value=\"\" placeholder=\"Integral\" min=\"0\">\n"
"                        <button class=\"tuning-button btn btn-outline-secondary\" type=\"button\" id=\"kiplus\"> + </button>\n"
"                        <button class=\"tuning-button btn btn-outline-secondary\" type=\"button\" id=\"kiminus\"> - </button>\n"
"                    </div>\n"
"                    <div class=\"tuning-button input-group col\">\n"
"                        <span for=\"kp\" class=\"input-group-text\"> Kd: </span>\n"
"                        <input class=\"tuning-input form-control\" type=\"text\" id=\"kd\" value=\"\" placeholder=\"Derivative\" min=\"0\">\n"
"                        <button class=\"tuning-button btn btn-outline-secondary\" type=\"button\" id=\"kdplus\"> + </button>\n"
"                        <button class=\"tuning-button btn btn-outline-secondary\" type=\"button\" id=\"kdminus\"> - </button>\n"
"                    </div>\n"
"                </div>\n"
"            </form>\n"
"        </div>\n"
"    </div>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/moment@2.29.1/moment.js\" integrity=\"sha256-8AdWdyRXkrETyAGla9NmgkYVlqw4MOHR6sJJmtFGAYQ=\" crossorigin=\"anonymous\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/chart.js@3.4.1/dist/chart.min.js\" integrity=\"sha256-GMN9UIJeUeOsn/Uq4xDheGItEeSpI5Hcfp/63GclDZk=\" crossorigin=\"anonymous\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/chartjs-adapter-moment@1.0.0/dist/chartjs-adapter-moment.min.js\" integrity=\"sha256-sFB24K2xU2EOgWAtb3ySAGjhMqcUvUJGmwjDcTQa04k=\" crossorigin=\"anonymous\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/js/bootstrap.min.js\" integrity=\"sha256-XDbijJp72GS2c+Ij234ZNJIyJ1Nv+9+HH1i28JuayMk=\" crossorigin=\"anonymous\"></script>\n"
"</body>\n"
"</html>";