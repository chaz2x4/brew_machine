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
"            var cycleTime = 1500;\n"
"            var mode = \'brew\';\n"
"            var tempData = [];\n"
"            var targetData = {};\n"
"            var outputData = {};\n"
"            var labels = [];\n"
"            var data = {\n"
"                labels: labels,\n"
"                datasets: [{\n"
"                    type: \'line\',\n"
"                    label: \'Temperature\',\n"
"                    data: [],\n"
"                    borderColor: \'#4285F4\',\n"
"                    borderWidth: 2,\n"
"                    backgroundColor: function (context) {\n"
"                        if(context.dataset.data.length) {\n"
"                            var pointTemp = context.dataset.data[context.dataIndex];\n"
"                            var targetTemp = targetData[mode][context.dataIndex];\n"
"                            var acceptableError = (mode == \'brew\') ? 0.5 : 1.0;\n"
"                            if(pointTemp > (targetTemp + acceptableError)) return \'#FF6384\';\n"
"                            if(pointTemp < (targetTemp - acceptableError)) return \'#331F2E\';\n"
"                            else return \'#63FF8F\';\n"
"                        }\n"
"                        else return \'#4285F4\';\n"
"                    },\n"
"                    elements: {\n"
"                        point: {\n"
"                            radius: 0.5,\n"
"                        },\n"
"                    },\n"
"                    tension: 0.1,\n"
"                    yAxisID: \'y1\',\n"
"                }, {\n"
"                    type: \'line\',\n"
"                    label: \'Target\',\n"
"                    data: targetData,\n"
"                    borderColor: \'#4285F4\',\n"
"                    borderWidth: 2,\n"
"                    borderDash: [5, 5],\n"
"                    yAxisID: \'y1\',\n"
"                    elements: {\n"
"                        point: {\n"
"                            radius: 0\n"
"                        }\n"
"                    },\n"
"                    animations: \'off\',\n"
"                }, {\n"
"                    type: \'bar\',\n"
"                    label: \'Output\',\n"
"                    data: outputData.brew,\n"
"                    backgroundColor: \'#63FFDD\',\n"
"                    yAxisID: \'y2\',\n"
"                }],\n"
"            };\n"
"\n"
"            const options = {\n"
"                responsive: true,\n"
"                interaction: {\n"
"                    mode: \'index\',\n"
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
"                    y1: {\n"
"                        title: {\n"
"                            display: true,\n"
"                            text: \"Temperature\",\n"
"                        },\n"
"                        position: \'left\',\n"
"                    },\n"
"                    y2: {\n"
"                        min: 0,\n"
"                        max: cycleTime,\n"
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
"                    chart.options.plugins.title.text = \"Brew Temperature\";\n"
"                    chart.data.datasets[1].data = targetData.brew;\n"
"                    chart.data.datasets[2].data = outputData.brew;\n"
"                    mode = \'brew\';\n"
"                    updateElements();\n"
"                    chart.render();\n"
"                },\n"
"            }, {\n"
"                name: \"Steam Temps\",\n"
"                id: \"steam\",\n"
"                handler() {\n"
"                    chart.options.plugins.title.text = \"Steam Temperature\";\n"
"                    chart.data.datasets[1].data = targetData.steam;\n"
"                    chart.data.datasets[2].data = outputData.steam;\n"
"                    mode = \'steam\';\n"
"                    updateElements();\n"
"                    chart.render();\n"
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
"                chart.options.scales.y1.suggestedMin = targetTemp - 0.5;\n"
"                chart.options.scales.y1.suggestedMax = targetTemp + 0.5;\n"
"\n"
"                chart.update();\n"
"            }\n"
"\n"
"            function incrementTarget(){\n"
"                var maxTemp = (mode == \"brew\") ? 115 : 160;\n"
"                if(mode == \"brew\") brewTarget = (brewTarget < maxTemp) ? brewTarget + 0.5 : maxTemp;\n"
"                else steamTarget = (steamTarget < maxTemp) ? steamTarget + 1 : maxTemp;\n"
"                callServer.post(\"/increment_target?mode=\" + mode);\n"
"                updateElements();\n"
"            }\n"
"\n"
"            function decrementTarget(){\n"
"                var minTemp = (mode == \"brew\") ? 85 : 140;\n"
"                if(mode == \"brew\") brewTarget = (brewTarget > minTemp) ? brewTarget - 0.5 : minTemp;\n"
"                else steamTarget = (steamTarget > minTemp) ? steamTarget - 1 : minTemp;\n"
"                callServer.post(\"/decrement_target?mode=\" + mode);\n"
"                updateElements();\n"
"            }\n"
"\n"
"            function incrementOffset(){\n"
"                if(tempOffset < 15) tempOffset += 0.5;\n"
"                callServer.post(\"/increment_target?mode=offset\");\n"
"                updateElements();\n"
"            }\n"
"\n"
"            function decrementOffset(){\n"
"                if(tempOffset > -15) tempOffset -= 0.5;\n"
"                callServer.post(\"/decrement_target?mode=offset\");\n"
"                updateElements();\n"
"            }\n"
"\n"
"            var isTuningsOpen = false;\n"
"            function changeTunings() {\n"
"                var button = this;\n"
"                if(isTuningsOpen) {\n"
"                    button.classList.remove(\'active\');\n"
"                    button.innerHTML = \"Setting Tunings...\";\n"
"                    button.disabled = true;\n"
"                    isTuningsOpen = false;\n"
"                    isTuned = false;\n"
"                    var kp = document.getElementById(\"kp\");\n"
"                    var ki = document.getElementById(\"ki\");\n"
"                    var kd = document.getElementById(\"kd\");\n"
"                    var tunings = { \"kp\" : kp.value, \"ki\": ki.value, \"kd\": kd.value };\n"
"                    callServer.put(\"/set_tunings\", JSON.stringify(tunings), function(){\n"
"                        isTuned = true;\n"
"                        button.disabled = false;\n"
"                        button.innerHTML = \"Set Tunings:\";\n"
"                    })\n"
"                }\n"
"                else{\n"
"                    isTuningsOpen = true;\n"
"                    changeTuningButtons();\n"
"                    refreshTunings();\n"
"                }\n"
"            }\n"
"\n"
"            var isTuned = true;\n"
"            function changeTuningButtons() {\n"
"                if(isTuned) {\n"
"                    tuningButton.disabled = false;\n"
"                    if(isTuningsOpen) {\n"
"                        tuningButton.innerHTML = \"Submit\";\n"
"                        tuningButton.classList.add(\'active\');\n"
"                    }\n"
"                    else {\n"
"                        tuningButton.innerHTML = \"Set Tunings:\";\n"
"                        tuningButton.classList.remove(\'active\');\n"
"                    }\n"
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
"                async function request(url, method, data){\n"
"                    const response = await fetch(url, {\n"
"                        method: method,\n"
"                        body: data,\n"
"                        headers: {\n"
"                            \'Content-Type\': \'application/json\'\n"
"                        },\n"
"                    });\n"
"                    return response;\n"
"                }\n"
"\n"
"                return {\n"
"                    get: function(url, callback){\n"
"                        request(url, \"GET\", null)\n"
"                            .then(response => response.json())\n"
"                            .then(responseData => callback(responseData));\n"
"                    },\n"
"                    post: function(url, data, callback){\n"
"                        request(url, \"POST\", data);\n"
"                    },\n"
"                    put: function(url, data, callback) {\n"
"                        request(url, \"PUT\", data)\n"
"                            .then(response => callback(response));\n"
"                    }\n"
"                }\n"
"            })();\n"
"\n"
"            setInterval(function(){\n"
"                callServer.get(\"/get_temps\", function(response){\n"
"                    var requestedLabels = [];\n"
"                    var requestedTempData = [];\n"
"                    var requestedBrewOutput = [];\n"
"                    var requestedSteamOutput = [];\n"
"                    var requestedBrewTargets = [];\n"
"                    var requestedSteamTargets = [];\n"
"                    \n"
"                    var sortedOutputs = response.sort((a, b) => a.time - b.time);\n"
"                    var lastOutput = sortedOutputs[sortedOutputs.length - 1];\n"
"                    if(brewTarget == null) [brewTarget, steamTarget, tempOffset] = [lastOutput.targets.brew, lastOutput.targets.steam, lastOutput.targets.offset];\n"
"                    \n"
"                    sortedOutputs.forEach(output => {                        \n"
"                        requestedLabels.push(output.time * 1000);\n"
"                        requestedTempData.push(output.temperature);\n"
"                        requestedBrewOutput.push(output.outputs.brew);\n"
"                        requestedSteamOutput.push(output.outputs.steam);\n"
"                        requestedBrewTargets.push(output.targets.brew);\n"
"                        requestedSteamTargets.push(output.targets.steam);\n"
"                    })\n"
"\n"
"                    chart.data.labels = requestedLabels;\n"
"                    chart.data.datasets[0].data = tempData = requestedTempData;\n"
"                    if(mode == \'brew\') { \n"
"                        chart.data.datasets[1].data = targetData.brew = requestedBrewTargets;\n"
"                        chart.data.datasets[2].data = outputData.brew = requestedBrewOutput;\n"
"                    }\n"
"                    else { \n"
"                        chart.data.datasets[1].data = targetData.steam = requestedSteamTargets;\n"
"                        chart.data.datasets[2].data = outputData.steam = requestedSteamOutput;\n"
"                    }        \n"
"\n"
"                    updateElements();\n"
"                    if(!isTuned && !isTuningsOpen) refreshTunings();\n"
"                });\n"
"            }, 1000);\n"
"\n"
"\n"
"            function refreshTunings(isFirstLoad) {\n"
"                callServer.get(\"/get_tunings\", function(response){\n"
"                    if(response) {                        \n"
"                        Object.entries(response).forEach(entry => {\n"
"                            const [name, value] = entry;\n"
"                            var input = document.getElementById(name);\n"
"                            input.value = value;\n"
"                        });\n"
"                        isTuned = true;\n"
"                        if(!isFirstLoad) changeTuningButtons();\n"
"                    }\n"
"                    else { \n"
"                        if(!isTuned) return;\n"
"                        isTuned = false;\n"
"                        changeTuningButtons();\n"
"                    }\n"
"                });\n"
"            }\n"
"\n"
"            refreshTunings(true);\n"
"            document.getElementById(\"targetplus\").onclick = incrementTarget;\n"
"            document.getElementById(\"targetminus\").onclick = decrementTarget;\n"
"            document.getElementById(\"offsetplus\").onclick = incrementOffset;\n"
"            document.getElementById(\"offsetminus\").onclick = decrementOffset;\n"
"            tuningButton = document.getElementById(\"tuning_button\");\n"
"            tuningButton.onclick = changeTunings;\n"
"        }\n"
"    </script>\n"
"</head>\n"
"<body class=\"container-sm\">\n"
"    <div class=\"row p-2\" id=\"chartContainer\">\n"
"        <div class=\"col-sm d-none d-sm-flex\">\n"
"            <canvas id=\"tempChart\"></canvas>\n"
"        </div>\n"
"        <div class=\"w-100 d-block d-sm-none\">\n"
"            <div class=\"alert alert-info\"> Rotate screen for chart! </div>\n"
"        </div>\n"
"        <div class=\"w-100 d-block d-md-none\"></div>\n"
"        <div class=\"card col-md-2 my-sm-5\">\n"
"            <div class=\"card-body\">\n"
"                <h5 class=\"card-title\">\n"
"                    Gaggia Classic:\n"
"                </h5>\n"
"                <div class=\"card-text\"> \n"
"                    <p> Temperature: </p>\n"
"                    <p> Pressure: </p>\n"
"                    <p> Time: </p>\n"
"                </div>\n"
"            </div>\n"
"        </div>\n"
"    </div>\n"
"    <div class=\"row p-2 row-cols-2 row-cols-sm-4 align-items-center\" id=\"tempContainer\">\n"
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
"            <button class=\"btn btn-outline-secondary\" type=\"button\" id=\"tuning_button\" data-bs-toggle=\"collapse\" data-bs-target=\"#tunings_tab\">\n"
"                Set Tunings:\n"
"            </button>\n"
"        </p>\n"
"        <div class=\"collapse\" id=\"tunings_tab\">\n"
"            <form class=\"accordion-body\" id=\"tunings-form\">\n"
"                <div class=\"row\">\n"
"                    <div class=\"tuning-button input-group col\">\n"
"                        <span for=\"kp\" class=\"input-group-text\"> Kp: </span>\n"
"                        <input class=\"tuning-input form-control\" type=\"text\" id=\"kp\" value=\"\" placeholder=\"Proportional\" min=\"0\">\n"
"                    </div>\n"
"                    <div class=\"tuning-button input-group col\">\n"
"                        <span for=\"kp\" class=\"input-group-text\"> Ki: </span>\n"
"                        <input class=\"tuning-input form-control\" type=\"text\" id=\"ki\" value=\"\" placeholder=\"Integral\" min=\"0\">\n"
"                    </div>\n"
"                    <div class=\"tuning-button input-group col\">\n"
"                        <span for=\"kp\" class=\"input-group-text\"> Kd: </span>\n"
"                        <input class=\"tuning-input form-control\" type=\"text\" id=\"kd\" value=\"\" placeholder=\"Derivative\" min=\"0\">\n"
"                    </div>\n"
"                </div>\n"
"            </form>\n"
"        </div>\n"
"    </div>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/moment@2.29.1/moment.js\" integrity=\"sha256-8AdWdyRXkrETyAGla9NmgkYVlqw4MOHR6sJJmtFGAYQ=\" crossorigin=\"anonymous\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.min.js\" integrity=\"sha256-cMPWkL3FzjuaFSfEYESYmjF25hCIL6mfRSPnW8OVvM4=\" crossorigin=\"anonymous\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/chart.js@3.7.0/dist/chart.min.js\" integrity=\"sha256-Y26AMvaIfrZ1EQU49pf6H4QzVTrOI8m9wQYKkftBt4s=\" crossorigin=\"anonymous\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/chartjs-adapter-moment@1.0.0/dist/chartjs-adapter-moment.min.js\" integrity=\"sha256-sFB24K2xU2EOgWAtb3ySAGjhMqcUvUJGmwjDcTQa04k=\" crossorigin=\"anonymous\"></script>\n"
"</body>\n"
"</html>";