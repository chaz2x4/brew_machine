const char* indexHtml = "<html>\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\" >\n"
"    <title>Gaggia - Homebrew Server</title>\n"
"    <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css\" integrity=\"sha256-djO3wMl9GeaC/u6K+ic4Uj/LKhRUSlUFcsruzS7v5ms=\" crossorigin=\"anonymous\">\n"
"    <style>\n"
"        button {\n"
"            touch-action: manipulation;\n"
"        }\n"
"    </style>\n"
"    <script>\n"
"        window.onload = function () {\n"
"            let brewTarget;\n"
"            let steamTarget;\n"
"            let tempOffset;\n"
"            let cycleTime = 1500;\n"
"            let mode = \'brew\';\n"
"            let tempData = [];\n"
"            let targetData = {};\n"
"            let outputData = {};\n"
"            let labels = [];\n"
"            let data = {\n"
"                labels: labels,\n"
"                datasets: [{\n"
"                    type: \'line\',\n"
"                    label: \'Temperature\',\n"
"                    data: [],\n"
"                    borderColor: \'#4285F4\',\n"
"                    borderWidth: 2,\n"
"                    backgroundColor: function (context) {\n"
"                        if(context.dataset.data.length) {\n"
"                            let pointTemp = context.dataset.data[context.dataIndex];\n"
"                            let targetTemp = targetData[mode][context.dataIndex];\n"
"                            let acceptableError = (mode == \'brew\') ? 0.5 : 1.0;\n"
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
"                    legend: {\n"
"                        display: false,\n"
"                    },\n"
"                    tooltip: {\n"
"                        displayColors: false,\n"
"                    },\n"
"                },\n"
"                scales: {\n"
"                    x: {\n"
"                        title: {\n"
"                            display: true,\n"
"                            text: \"Time\",\n"
"                        },\n"
"                        ticks: {\n"
"                            callback: (value, index) => {\n"
"                                return index % 10 === 0 ? value : \'\';\n"
"                            },\n"
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
"            moment.relativeTimeThreshold(\'s\', 200);\n"
"            moment.relativeTimeThreshold(\'ss\', 1);\n"
"            moment.updateLocale(\'en\', {\n"
"                relativeTime: {\n"
"                    \'s\': \'a second\',\n"
"                    \'m\': \'a minute\',\n"
"                    \'mm\': \'%d minutes\'\n"
"                }\n"
"            })\n"
"\n"
"            const actions = [{\n"
"                name: \"Brew\",\n"
"                id: \"brew\",\n"
"                handler() {\n"
"                    chart.options.plugins.title.text = \"Brew Temperature\";\n"
"                    chart.data.datasets[1].data = targetData.brew;\n"
"                    chart.data.datasets[2].data = outputData.brew;\n"
"                    mode = \'brew\';\n"
"                    switchTab(mode);\n"
"                    updateElements(mode);\n"
"                    chart.render();\n"
"                },\n"
"            }, {\n"
"                name: \"Steam\",\n"
"                id: \"steam\",\n"
"                handler() {\n"
"                    chart.options.plugins.title.text = \"Steam Temperature\";\n"
"                    chart.data.datasets[1].data = targetData.steam;\n"
"                    chart.data.datasets[2].data = outputData.steam;\n"
"                    mode = \'steam\';\n"
"                    switchTab(mode);\n"
"                    updateElements(mode);\n"
"                    chart.render();\n"
"                }\n"
"            }, {\n"
"                name: \"Advanced\",\n"
"                id: \"advanced\",\n"
"                handler() {\n"
"                    switchTab(\"advanced\");\n"
"                }\n"
"            }];\n"
"            \n"
"            function updateElements(mode){\n"
"                let targetTemp = (mode == \"brew\") ? brewTarget : steamTarget;\n"
"\n"
"                let targetLabel = document.getElementById(\"target_label\");\n"
"                targetLabel.innerHTML = \"Target: \" + targetTemp + \" C\";\n"
"\n"
"                let offsetLabel = document.getElementById(\"offset_label\");\n"
"                offsetLabel.innerHTML = \"Offset: \" + tempOffset + \" C\";\n"
"\n"
"                chart.options.scales.y1.suggestedMin = targetTemp % 5 === 0 ? Math.floor(targetTemp / 5 - 1) * 5 : Math.floor(targetTemp / 5) * 5;\n"
"                chart.options.scales.y1.suggestedMax = targetTemp % 5 === 0 ? Math.ceil(targetTemp / 5 + 1) * 5 : Math.ceil(targetTemp / 5) * 5;\n"
"\n"
"                chart.update();\n"
"            }\n"
"\n"
"            function switchTab(mode) {\n"
"                if(mode == \'brew\') {\n"
"                    brewSwitch.classList.add(\"active\");\n"
"                    steamSwitch.classList.remove(\"active\");\n"
"                    advancedSwitch.classList.remove(\"active\");\n"
"                    brewCard.classList.add(\"show\");\n"
"                    advancedCard.classList.remove(\"show\");\n"
"                    brewCard.classList.add(\"active\");\n"
"                    advancedCard.classList.remove(\"active\");\n"
"                }\n"
"                else if(mode == \'steam\'){\n"
"                    steamSwitch.classList.add(\"active\");\n"
"                    brewSwitch.classList.remove(\"active\");\n"
"                    advancedSwitch.classList.remove(\"active\");\n"
"                    brewCard.classList.add(\"show\");\n"
"                    advancedCard.classList.remove(\"show\");\n"
"                    brewCard.classList.add(\"active\");\n"
"                    advancedCard.classList.remove(\"active\");\n"
"                }\n"
"                else {\n"
"                    advancedSwitch.classList.add(\"active\");\n"
"                    steamSwitch.classList.remove(\"active\");\n"
"                    brewSwitch.classList.remove(\"active\");\n"
"                    advancedCard.classList.add(\"show\");\n"
"                    brewCard.classList.remove(\"show\");\n"
"                    advancedCard.classList.add(\"active\");\n"
"                    brewCard.classList.remove(\"active\");\n"
"                }\n"
"            }\n"
"\n"
"            function incrementTarget(){\n"
"                let maxTemp = (mode == \"brew\") ? 115 : 160;\n"
"                if(mode == \"brew\") brewTarget = (brewTarget < maxTemp) ? brewTarget + 0.5 : maxTemp;\n"
"                else steamTarget = (steamTarget < maxTemp) ? steamTarget + 1 : maxTemp;\n"
"                callServer.post(\"/increment_target?mode=\" + mode);\n"
"                updateElements(mode);\n"
"            }\n"
"\n"
"            function decrementTarget(){\n"
"                let minTemp = (mode == \"brew\") ? 85 : 140;\n"
"                if(mode == \"brew\") brewTarget = (brewTarget > minTemp) ? brewTarget - 0.5 : minTemp;\n"
"                else steamTarget = (steamTarget > minTemp) ? steamTarget - 1 : minTemp;\n"
"                callServer.post(\"/decrement_target?mode=\" + mode);\n"
"                updateElements(mode);\n"
"            }\n"
"\n"
"            function incrementOffset(){\n"
"                if(tempOffset < 15) tempOffset += 0.5;\n"
"                callServer.post(\"/increment_target?mode=offset\");\n"
"                updateElements(mode);\n"
"            }\n"
"\n"
"            function decrementOffset(){\n"
"                if(tempOffset > -15) tempOffset -= 0.5;\n"
"                callServer.post(\"/decrement_target?mode=offset\");\n"
"                updateElements(mode);\n"
"            }\n"
"\n"
"            let isTuningsOpen = false;\n"
"            function changeTunings() {\n"
"                let button = this;\n"
"                if(isTuningsOpen) {\n"
"                    button.classList.remove(\'active\');\n"
"                    button.innerHTML = \'<span class=\"spinner-border spinner-border-sm\" role=\"status\" aria-hidden=\"true\"></span> Tuning...\';\n"
"                    button.disabled = true;\n"
"                    isTuningsOpen = false;\n"
"                    isTuned = false;\n"
"                    let kp = document.getElementById(\"kp\");\n"
"                    let ki = document.getElementById(\"ki\");\n"
"                    let kd = document.getElementById(\"kd\");\n"
"                    let tunings = { \"kp\" : kp.value, \"ki\": ki.value, \"kd\": kd.value };\n"
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
"            let isTuned = true;\n"
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
"            let chart = new Chart(ctx, {\n"
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
"            let callServer = (function() {\n"
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
"                    let requestedLabels = [];\n"
"                    let requestedTempData = [];\n"
"                    let requestedBrewOutput = [];\n"
"                    let requestedSteamOutput = [];\n"
"                    let requestedBrewTargets = [];\n"
"                    let requestedSteamTargets = [];\n"
"                    \n"
"                    let sortedOutputs = response.sort((a, b) => a.time - b.time);\n"
"                    let lastOutput = sortedOutputs[sortedOutputs.length - 1];\n"
"                    if(brewTarget == null) [brewTarget, steamTarget, tempOffset] = [lastOutput.targets.brew, lastOutput.targets.steam, lastOutput.targets.offset];\n"
"                    \n"
"                    sortedOutputs.forEach((output, index) => {             \n"
"                        requestedLabels.push(moment(output.time * 1000).fromNow());\n"
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
"                    updateElements(mode);\n"
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
"                            let input = document.getElementById(name);\n"
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
"\n"
"            let steamSwitch = document.getElementById(\"switch_steam\");\n"
"            let brewSwitch = document.getElementById(\"switch_brew\");\n"
"            let advancedSwitch = document.getElementById(\"switch_advanced\");\n"
"            let brewCard = document.getElementById(\"brew_card\");\n"
"            let advancedCard = document.getElementById(\"advanced_card\");\n"
"            let tuningButton = document.getElementById(\"tuning_button\");\n"
"            \n"
"\n"
"            document.getElementById(\"targetplus\").onclick = incrementTarget;\n"
"            document.getElementById(\"targetminus\").onclick = decrementTarget;\n"
"            document.getElementById(\"offsetplus\").onclick = incrementOffset;\n"
"            document.getElementById(\"offsetminus\").onclick = decrementOffset;\n"
"            tuningButton.onclick = changeTunings;\n"
"        }\n"
"    </script>\n"
"</head>\n"
"<body>\n"
"    <div class=\"container-sm p-2\">\n"
"        <div class=\"row row-cols-1\" id=\"chartContainer\">\n"
"            <div class=\"w-100 d-block d-sm-none\">\n"
"                <div class=\"alert alert-info\"> Rotate screen for chart! </div>\n"
"            </div>\n"
"            <div class=\"col-sm d-none d-sm-flex\">\n"
"                <canvas id=\"tempChart\"></canvas>\n"
"            </div>\n"
"            <div class=\"col col-md-2 my-sm-5\">\n"
"                <div class=\"card\">\n"
"                    <div class=\"card-header\" id=\"tempContainer\">\n"
"                        <ul class=\"nav nav-pills nav-justified card-header-pills\">\n"
"                            <li class=\"nav-item\">\n"
"                                <a class=\"nav-link active\" id=\"switch_brew\" href=\"#brew_card\" data-toggle=\"pill\">Brew</a>\n"
"                              </li>\n"
"                              <li class=\"nav-item\">\n"
"                                <a class=\"nav-link\" id=\"switch_steam\" href=\"#brew_card\" data-toggle=\"pill\">Steam</a>\n"
"                              </li>\n"
"                              <li class=\"nav-item\">\n"
"                                  <a class=\"nav-link\" id=\"switch_advanced\" href=\"#advanced_card\" data-toggle=\"pill\"> Advanced </a>\n"
"                              </li>\n"
"                        </ul>\n"
"                    </div>\n"
"                    <div class=\"card-body tab-content\">\n"
"                        <div class=\"tab-pane fade show active\" id=\"brew_card\">\n"
"                            <div> \n"
"                                <h6>Temperature:</h6>\n"
"                                <div class=\"col-2 col-sm-1\"> \n"
"                                    <label class=\"form-label\" id=\"target_label\"> Target: 100 C</label>\n"
"                                    <div class=\"btn-group\">\n"
"                                        <button class=\"btn btn-outline-danger\" id=\"targetplus\"> + </button>\n"
"                                        <button class=\"btn btn-outline-secondary\" id=\"targetminus\"> - </button>\n"
"                                    </div>\n"
"                                </div>\n"
"                            </div>\n"
"                            <div> \n"
"                                <h6>Pressure:</h6>\n"
"                            </div>\n"
"                            <div> \n"
"                                <h6>Time:</h6>\n"
"                            </div>\n"
"                        </div>\n"
"                        <div class=\"tab-pane fade\" id=\"advanced_card\">\n"
"                            <div class=\"col-2 col-sm-1\"> \n"
"                                <label class=\"form-label\" id=\"offset_label\"> Offset: -8 C</label>\n"
"                                <div class=\"btn-group\">\n"
"                                    <button class=\"btn btn-outline-danger\" id=\"offsetplus\"> + </button>\n"
"                                    <button class=\"btn btn-outline-secondary\" id=\"offsetminus\"> - </button>\n"
"                                </div>\n"
"                            </div>\n"
"                            <div id=\"tunerContainer\">\n"
"                                <div class=\"my-2\">\n"
"                                    <button class=\"btn btn-secondary\" type=\"button\" id=\"tuning_button\" data-bs-toggle=\"collapse\" data-bs-target=\"#tunings_tab\">\n"
"                                        Set Tunings:\n"
"                                    </button>\n"
"                                </div>\n"
"                                <div class=\"collapse\" id=\"tunings_tab\">\n"
"                                    <form class=\"accordion-body\" id=\"tunings-form\">\n"
"                                        <div class=\"row row-cols-1 row-cols-sm-3 row-cols-md-1\">\n"
"                                            <div class=\"col\">\n"
"                                                <div class=\"tuning-button input-group\">\n"
"                                                    <span for=\"kp\" class=\"input-group-text\"> Kp: </span>\n"
"                                                    <input class=\"tuning-input form-control\" type=\"text\" id=\"kp\" value=\"\" placeholder=\"Proportional\" min=\"0\">\n"
"                                                </div>\n"
"                                            </div>\n"
"                                            <div class=\"col\">\n"
"                                                <div class=\"tuning-button input-group\">\n"
"                                                    <span for=\"kp\" class=\"input-group-text\"> Ki: </span>\n"
"                                                    <input class=\"tuning-input form-control\" type=\"text\" id=\"ki\" value=\"\" placeholder=\"Integral\" min=\"0\">\n"
"                                                </div>\n"
"                                            </div>\n"
"                                            <div class=\"col\">\n"
"                                                <div class=\"tuning-button input-group\">\n"
"                                                    <span for=\"kp\" class=\"input-group-text\"> Kd: </span>\n"
"                                                    <input class=\"tuning-input form-control\" type=\"text\" id=\"kd\" value=\"\" placeholder=\"Derivative\" min=\"0\">\n"
"                                                </div>\n"
"                                            </div>\n"
"                                        </div>\n"
"                                    </form>\n"
"                                </div>\n"
"                            </div>\n"
"                        </div>\n"
"                    </div>\n"
"                </div>\n"
"            </div>\n"
"        </div>\n"
"    </div>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/moment@2.29.1/moment.js\" integrity=\"sha256-8AdWdyRXkrETyAGla9NmgkYVlqw4MOHR6sJJmtFGAYQ=\" crossorigin=\"anonymous\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.min.js\" integrity=\"sha256-cMPWkL3FzjuaFSfEYESYmjF25hCIL6mfRSPnW8OVvM4=\" crossorigin=\"anonymous\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/chart.js@3.7.0/dist/chart.min.js\" integrity=\"sha256-Y26AMvaIfrZ1EQU49pf6H4QzVTrOI8m9wQYKkftBt4s=\" crossorigin=\"anonymous\"></script>\n"
"    <script src=\"https://cdn.jsdelivr.net/npm/chartjs-adapter-moment@1.0.0/dist/chartjs-adapter-moment.min.js\" integrity=\"sha256-sFB24K2xU2EOgWAtb3ySAGjhMqcUvUJGmwjDcTQa04k=\" crossorigin=\"anonymous\"></script>\n"
"</body>\n"
"</html>";