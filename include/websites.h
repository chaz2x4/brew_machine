const char* indexHtml = "<html>\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\" >\n"
"    <title>Gaggia - Homebrew Server</title>\n"
"    <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css\" integrity=\"sha256-YvdLHPgkqJ8DVUxjjnGVlMMJtNimJ6dYkowFFvp4kKs=\" crossorigin=\"anonymous\">    <style>\n"
"        button {\n"
"            touch-action: manipulation;\n"
"        }\n"
"    </style>\n"
"    <script>\n"
"        window.onload = function () {\n"
"            const min_steam_temp = 130;\n"
"            const max_steam_temp = 150;\n"
"            const min_brew_temp = 85;\n"
"            const max_brew_temp = 105;\n"
"            const max_offset = 11;\n"
"            const min_offset = -11;\n"
"            let refresh_temps_id;\n"
"            let refresh_targets_id;\n"
"            let is_target_loaded = false;\n"
"            let brew_target;\n"
"            let steam_target;\n"
"            let temp_offset;\n"
"            let temp_scale = \"C\";\n"
"            let cycle_time = 1000;\n"
"            let current_mode = \'brew\';\n"
"            let target_data = {};\n"
"            let output_data = {};\n"
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
"                        if(context.dataIndex) {\n"
"                            let pointTemp = context.dataset.data[context.dataIndex];\n"
"                            let target_temp = target_data[current_mode][context.dataIndex];\n"
"                            let acceptableError = (current_mode == \'brew\' && temp_scale == \'C\') ? 1 : 2;\n"
"                            if(pointTemp > (target_temp + acceptableError)) return \'#FF6384\';\n"
"                            if(pointTemp < (target_temp - acceptableError)) return \'#331F2E\';\n"
"                            else return \'#63FF8F\';\n"
"                        }\n"
"                        else return \'#4285F4\';\n"
"                    },\n"
"                    elements: {\n"
"                        point: {\n"
"                            radius: 0.75,\n"
"                        },\n"
"                    },\n"
"                    tension: 0.1,\n"
"                    yAxisID: \'y1\',\n"
"                }, {\n"
"                    type: \'line\',\n"
"                    label: \'Target\',\n"
"                    data: target_data[current_mode],\n"
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
"                    data: output_data.brew,\n"
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
"                        text: \'Brew Temperature\',\n"
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
"                            text: \'Time\',\n"
"                        },\n"
"                        ticks: {\n"
"                            callback: (value, index) => {\n"
"                                return index % 10 === 0 ? (value / 2): \'\';\n"
"                            },\n"
"                        },\n"
"                        spanGaps: true,\n"
"                    },\n"
"                    y1: {\n"
"                        title: {\n"
"                            display: true,\n"
"                            text: \'Temperature\',\n"
"                        },\n"
"                        position: \'left\',\n"
"                    },\n"
"                    y2: {\n"
"                        min: 0,\n"
"                        max: cycle_time,\n"
"                        title: {\n"
"                            display: true,\n"
"                            text: \'Output\',\n"
"                        },\n"
"                        position: \'right\',\n"
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
"                    chart.data.datasets[1].data = target_data.brew;\n"
"                    chart.data.datasets[2].data = output_data.brew;\n"
"                    current_mode = \"brew\";\n"
"                    switchTab(current_mode);\n"
"                    updateElements();\n"
"                    chart.render();\n"
"                },\n"
"            }, {\n"
"                name: \"Steam\",\n"
"                id: \"steam\",\n"
"                handler() {\n"
"                    chart.options.plugins.title.text = \"Steam Temperature\";\n"
"                    chart.data.datasets[1].data = target_data.steam;\n"
"                    chart.data.datasets[2].data = output_data.steam;\n"
"                    current_mode = \"steam\";\n"
"                    switchTab(current_mode);\n"
"                    updateElements();\n"
"                    chart.render();\n"
"                }\n"
"            }, {\n"
"                name: \"Advanced\",\n"
"                id: \"advanced\",\n"
"                handler() {\n"
"                    switchTab(\"advanced\");\n"
"                    refreshTunings();\n"
"                }\n"
"            }];\n"
"            \n"
"            function updateElements(){\n"
"                document.getElementById(\"brew_target_label\").innerHTML = brew_target + \" °\" + temp_scale;\n"
"                document.getElementById(\"steam_target_label\").innerHTML = steam_target + \" °\" + temp_scale;\n"
"                document.getElementById(\"offset_target_label\").innerHTML = temp_offset + \" °\" + temp_scale;\n"
"\n"
"                scale_c_button.classList.toggle(\"active\", temp_scale == \"C\");\n"
"                scale_f_button.classList.toggle(\"active\", temp_scale == \"F\");\n"
"\n"
"                let target_temp = brew_target;\n"
"                if(current_mode == \"steam\") target_temp = steam_target;\n"
"                chart.options.scales.y1.suggestedMin = target_temp % 5 === 0 ? Math.floor(target_temp / 5 - 1) * 5 : Math.floor(target_temp / 5) * 5;\n"
"                chart.options.scales.y1.suggestedMax = target_temp % 5 === 0 ? Math.ceil(target_temp / 5 + 1) * 5 : Math.ceil(target_temp / 5) * 5;\n"
"                chart.update();\n"
"\n"
"                clearTimeout(refresh_targets_id);\n"
"                refresh_targets_id = setTimeout(refreshTargets, 5000);\n"
"            }\n"
"\n"
"            function switchTab(mode) {\n"
"                brew_switch.classList.toggle(\"active\", mode === \"brew\");\n"
"                steam_switch.classList.toggle(\"active\", mode === \"steam\");\n"
"                advanced_switch.classList.toggle(\"active\", mode === \"advanced\");\n"
"\n"
"                brew_card.classList.toggle(\"show\", mode === \"brew\");\n"
"                steam_card.classList.toggle(\"show\", mode === \"steam\");\n"
"                advanced_card.classList.toggle(\"show\", mode === \"advanced\");\n"
"\n"
"                brew_card.classList.toggle(\"active\", mode === \"brew\");\n"
"                steam_card.classList.toggle(\"active\", mode === \"steam\");\n"
"                advanced_card.classList.toggle(\"active\", mode === \"advanced\");\n"
"\n"
"                resetTuningsTab();\n"
"\n"
"                if(mode == \"brew\") {\n"
"                    steam_temp_element.innerHTML = placeholder.outerHTML;\n"
"                }\n"
"                else if(mode == \"steam\"){\n"
"                    brew_temp_element.innerHTML = placeholder.outerHTML;\n"
"                }\n"
"                else if(mode == \"advanced\"){\n"
"                    brew_temp_element.innerHTML = placeholder.outerHTML;\n"
"                    steam_temp_element.innerHTML = placeholder.outerHTML;                }\n"
"            }\n"
"\n"
"            function convertScales(temp, is_offset){\n"
"                if(temp_scale == \"F\") {\n"
"                    if(is_offset) return Math.round(temp * 9/5);\n"
"                    else return Math.round(temp * 9/5 + 32);\n"
"                }\n"
"                else return temp;\n"
"            }\n"
"\n"
"            function incrementTarget(mode){\n"
"                let max_temp;\n"
"                let i = 0.5;\n"
"                if(temp_scale == \"F\" || mode == \"steam\") i = 1;\n"
"                switch(mode) {\n"
"                    case \"offset\":\n"
"                        max_temp = convertScales(max_offset, true);\n"
"                        temp_offset = (temp_offset < max_temp) ? temp_offset + i : max_temp;\n"
"                    case \"steam\":\n"
"                        max_temp = convertScales(max_steam_temp);\n"
"                        steam_target = (steam_target < max_temp) ? steam_target + i : max_temp;\n"
"                    default:\n"
"                        max_temp = convertScales(max_brew_temp);\n"
"                        brew_target = (brew_target < max_temp) ? brew_target + i: max_temp; \n"
"                }\n"
"                updateElements();\n"
"                callServer.post(\"/increment_target?mode=\" + mode);\n"
"            }\n"
"\n"
"            function decrementTarget(mode){\n"
"                let min_temp;\n"
"                let i = 0.5;\n"
"                if(temp_scale == \"F\" || mode == \"steam\") i = 1;\n"
"                switch(mode) {\n"
"                    case \"offset\":\n"
"                        min_temp = convertScales(min_offset, true);\n"
"                        temp_offset = (temp_offset > min_temp) ? temp_offset - i : min_temp;\n"
"                    case \"steam\":\n"
"                        min_temp = convertScales(min_steam_temp);\n"
"                        steam_target = (steam_target > min_temp) ? steam_target - i: min_temp;\n"
"                    default:\n"
"                        min_temp = convertScales(min_brew_temp);\n"
"                        brew_target = (brew_target > min_temp) ? brew_target - i : min_temp; \n"
"                }\n"
"                updateElements();\n"
"                callServer.post(\"/decrement_target?mode=\" + mode);\n"
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
"            \n"
"            function changeScale(scale) {\n"
"                callServer.post(\"/change_scale?scale=\" + scale);\n"
"                temp_scale = scale;\n"
"                document.getElementById(\"offset_target_label\").innerHTML = placeholder.outerHTML;\n"
"                refreshTargets();\n"
"                loadTemps();\n"
"            }\n"
"            \n"
"            let steam_switch = document.getElementById(\"switch_steam\");\n"
"            let brew_switch = document.getElementById(\"switch_brew\");\n"
"            let advanced_switch = document.getElementById(\"switch_advanced\");\n"
"            let brew_card = document.getElementById(\"brew_card\");\n"
"            let steam_card = document.getElementById(\"steam_card\");\n"
"            let advanced_card = document.getElementById(\"advanced_card\");\n"
"            let steam_temp_element = document.getElementById(\"steam_current-temp\");\n"
"            let brew_temp_element = document.getElementById(\"brew_current-temp\");\n"
"            let tunings_tab = document.getElementById(\"tunings_tab\");\n"
"            let tuning_button = document.getElementById(\"tuning_button\");\n"
"            let scale_c_button = document.getElementById(\"scale_C_button\");\n"
"            let scale_f_button = document.getElementById(\"scale_F_button\");\n"
"            \n"
"            document.getElementById(\"brew_increment\").addEventListener(\'click\', () => incrementTarget(\"brew\"));\n"
"            document.getElementById(\"brew_decrement\").addEventListener(\'click\', () => decrementTarget(\"brew\"));\n"
"            document.getElementById(\"steam_increment\").addEventListener(\'click\', () => incrementTarget(\"steam\"));\n"
"            document.getElementById(\"steam_decrement\").addEventListener(\'click\', () => decrementTarget(\"steam\"));\n"
"            document.getElementById(\"offset_increment\").addEventListener(\'click\', () => incrementTarget(\"offset\"));\n"
"            document.getElementById(\"offset_decrement\").addEventListener(\'click\', () => decrementTarget(\"offset\"));\n"
"\n"
"            scale_c_button.addEventListener(\'click\', () => changeScale(\'C\'));\n"
"            scale_f_button.addEventListener(\'click\', () => changeScale(\'F\'));\n"
"            \n"
"            tuning_button.addEventListener(\'click\', function(){\n"
"                new bootstrap.Collapse(tunings_tab);\n"
"                tuning_button.classList.toggle(\'active\');\n"
"            });\n"
"\n"
"\n"
"            function resetTuningsTab(){\n"
"                tuning_button.disabled = false;\n"
"                tuning_button.innerHTML = \"Set Tunings:\";\n"
"                tunings_tab.classList.remove(\"show\");\n"
"            }\n"
"\n"
"            tunings_tab.addEventListener(\'hide.bs.collapse\', function() {\n"
"                tuning_button.disabled = true;\n"
"                tuning_button.innerHTML = placeholder.outerHTML + \'Tuning...\';\n"
"                let kp = document.getElementById(\"kp\");\n"
"                let ki = document.getElementById(\"ki\");\n"
"                let kd = document.getElementById(\"kd\");\n"
"                let tunings = { \"kp\" : kp.value, \"ki\": ki.value, \"kd\": kd.value };\n"
"                callServer.put(\"/set_tunings?mode=\" + current_mode, JSON.stringify(tunings), resetTuningsTab)\n"
"            })\n"
"            tunings_tab.addEventListener(\'show.bs.collapse\', function() {\n"
"                tuning_button.innerHTML = \"Submit\";\n"
"            });\n"
"\n"
"            let placeholder = document.createElement(\"span\");\n"
"            placeholder.classList.add(\"spinner-border\", \"spinner-border-sm\");\n"
"\n"
"            function refreshTunings() {\n"
"                callServer.get(\"/get_tunings?mode=\" + current_mode, function(response){\n"
"                    if(response) {                        \n"
"                        Object.entries(response).forEach(entry => {\n"
"                            const [name, value] = entry;\n"
"                            let input = document.getElementById(name);\n"
"                            input.value = value;\n"
"                        });\n"
"                    }\n"
"                });\n"
"            }\n"
"\n"
"           function loadTemps() {\n"
"                clearTimeout(refresh_temps_id);\n"
"                callServer.get(\"/get_temps\", function(response){\n"
"                    let requested_labels = [];\n"
"                    let requested_temp_data = [];\n"
"                    let requested_brew_output = [];\n"
"                    let requested_steam_output = [];\n"
"                    let requested_brew_targets = [];\n"
"                    let requested_steam_targets = [];\n"
"                    \n"
"                    let sorted_outputs = response.sort((a, b) => a.time - b.time);                    \n"
"                    sorted_outputs.forEach((output, index) => {             \n"
"                        requested_labels.push(moment(output.time * 1000).fromNow());\n"
"                        requested_temp_data.push(output.temperature);\n"
"                        requested_brew_output.push(output.outputs.brew);\n"
"                        requested_steam_output.push(output.outputs.steam);\n"
"                        requested_brew_targets.push(output.targets.brew);\n"
"                        requested_steam_targets.push(output.targets.steam);\n"
"                    })\n"
"\n"
"                    let last_output = sorted_outputs[sorted_outputs.length - 1];\n"
"                    [\n"
"                        temp_scale, \n"
"                        brew_target, \n"
"                        steam_target, \n"
"                        temp_offset\n"
"                    ] = [\n"
"                        last_output.scale, \n"
"                        last_output.targets.brew, \n"
"                        last_output.targets.steam, \n"
"                        last_output.targets.offset\n"
"                    ];\n"
"\n"
"                    let current_temp = requested_temp_data[requested_temp_data.length - 1].toPrecision(4);\n"
"                    brew_temp_element.innerHTML = current_temp + \" °\" + temp_scale;\n"
"                    steam_temp_element.innerHTML = current_temp + \" °\" + temp_scale;\n"
"                    \n"
"                    target_data.brew = requested_brew_targets;\n"
"                    output_data.brew = requested_brew_output;\n"
"                    target_data.steam = requested_steam_targets;\n"
"                    output_data.steam = requested_steam_output;\n"
"\n"
"                    chart.data.labels = requested_labels;\n"
"                    chart.data.datasets[0].data = requested_temp_data;\n"
"                    chart.data.datasets[1].data = target_data[current_mode];\n"
"                    chart.data.datasets[2].data = output_data[current_mode];\n"
"\n"
"                    updateElements();\n"
"                    refresh_temps_id = setTimeout(refreshTemps, 1000);\n"
"                });\n"
"            }\n"
"            loadTemps();\n"
"            \n"
"            function refreshTemps(){\n"
"                clearTimeout(refresh_temps_id);\n"
"                callServer.get(\"/get_recent_temps\", function(response){\n"
"                    let current_temp = response.temperature.toPrecision(4);\n"
"                    let queue_size = chart.data.labels.length;\n"
"\n"
"                    brew_temp_element.innerHTML = current_temp + \" °\" + temp_scale;\n"
"                    steam_temp_element.innerHTML = current_temp + \" °\" + temp_scale;\n"
"                    \n"
"                    target_data.brew.push(response.targets.brew);\n"
"                    output_data.brew.push(response.outputs.brew);\n"
"                    target_data.steam.push(response.targets.steam);\n"
"                    output_data.steam.push(response.outputs.steam);\n"
"\n"
"                    if(queue_size > 120) {\n"
"                        chart.data.labels.shift();\n"
"                        chart.data.datasets[0].data.shift();\n"
"                        target_data.brew.shift();\n"
"                        target_data.steam.shift();\n"
"                        output_data.brew.shift();\n"
"                        output_data.steam.shift();\n"
"                    }\n"
"\n"
"                    chart.data.labels.push(moment(response.time * 1000).fromNow());\n"
"                    chart.data.datasets[0].data.push(current_temp);\n"
"                    chart.data.datasets[1].data = target_data[current_mode];\n"
"                    chart.data.datasets[2].data = output_data[current_mode];\n"
"\n"
"                    updateElements();\n"
"                    refresh_temps_id = setTimeout(refreshTemps, 1000);\n"
"                });\n"
"            }\n"
"            refresh_temps_id = setTimeout(refreshTemps, 1000);\n"
"\n"
"            function refreshTargets() {\n"
"                clearTimeout(refresh_targets_id);\n"
"                clearTimeout(refresh_temps_id);\n"
"                is_target_loaded = false;\n"
"                refresh_targets_id = setTimeout(refreshTargets, 5000);\n"
"            }\n"
"            refresh_targets_id = setTimeout(refreshTargets, 5000);\n"
"        }   \n"
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
"                                <button class=\"nav-link active\" id=\"switch_brew\" data-toggle=\"pill\">Brew</button>\n"
"                              </li>\n"
"                              <li class=\"nav-item\">\n"
"                                <button class=\"nav-link\" id=\"switch_steam\" data-toggle=\"pill\">Steam</button>\n"
"                              </li>\n"
"                              <li class=\"nav-item\">\n"
"                                  <button class=\"nav-link\" id=\"switch_advanced\" data-toggle=\"pill\"> Advanced </button>\n"
"                              </li>\n"
"                        </ul>\n"
"                    </div>\n"
"                    <div class=\"card-body tab-content\">\n"
"                        <div class=\"tab-pane fade show active\" id=\"brew_card\">\n"
"                            <div> \n"
"                                <h6>Temperature:</h6>\n"
"                                <div class=\"row\">\n"
"                                    <div class=\"col-12\"> Actual: \n"
"                                        <small class=\"text-muted\" id=\"brew_current-temp\"><span class=\"spinner-border spinner-border-sm\"></span></small>\n"
"                                    </div>\n"
"                                </div>\n"
"                                <div class=\"row\"> \n"
"                                    <div class=\"col-12\"> Target:\n"
"                                        <small class=\"text-muted\" id=\"brew_target_label\"><span class=\"spinner-border spinner-border-sm\"></span></small>\n"
"                                    </div>\n"
"                                    <div class=\"my-2\">\n"
"                                        <div class=\"btn-group\">\n"
"                                            <button class=\"btn btn-outline-danger\" id=\"brew_increment\"> + </button>\n"
"                                            <button class=\"btn btn-outline-secondary\" id=\"brew_decrement\"> - </button>\n"
"                                        </div>\n"
"                                    </div>\n"
"                                </div>\n"
"                            </div>\n"
"                            <hr>\n"
"                            <div> \n"
"                                <h6>Pressure:</h6>\n"
"                                <span class=\"placeholder placeholder-wave bg-primary w-100\"></span>\n"
"                            </div>\n"
"                            <hr>\n"
"                            <div> \n"
"                                <h6>Time:</h6>\n"
"                                <span class=\"placeholder placeholder-wave bg-primary w-100\"></span>\n"
"                            </div>\n"
"                        </div>\n"
"                        <div class=\"tab-pane fade\" id=\"steam_card\">\n"
"                            <div> \n"
"                                <h6>Temperature:</h6>\n"
"                                <div class=\"row\">\n"
"                                    <div class=\"col-12\"> Actual:\n"
"                                        <small class=\"text-muted\" id=\"steam_current-temp\"><span class=\"spinner-border spinner-border-sm\"></span></small>\n"
"                                    </div>\n"
"                                </div>\n"
"                                <div class=\"row\"> \n"
"                                    <div class=\"col-12\"> Target:\n"
"                                        <small class=\"text-muted\" id=\"steam_target_label\"><span class=\"spinner-border spinner-border-sm\"></span></small>\n"
"                                    </div>\n"
"                                    <div class=\"my-2\">\n"
"                                        <div class=\"btn-group\">\n"
"                                            <button class=\"btn btn-outline-danger\" id=\"steam_increment\"> + </button>\n"
"                                            <button class=\"btn btn-outline-secondary\" id=\"steam_decrement\"> - </button>\n"
"                                        </div>\n"
"                                    </div>\n"
"                                </div>\n"
"                            </div>\n"
"                            <hr>\n"
"                            <div> \n"
"                                <h6>Time:</h6>\n"
"                                <span class=\"placeholder placeholder-wave bg-primary w-100\"></span>\n"
"                            </div>\n"
"                        </div>\n"
"                        <div class=\"tab-pane fade\" id=\"advanced_card\">\n"
"                            <div class=\"row\"> \n"
"                                <div class=\"col-12\" for=\"offset_group\"> Offset: \n"
"                                    <small class=\"text-muted\" id=\"offset_target_label\"><span class=\"spinner-border spinner-border-sm\"></span></small>\n"
"                                </div>\n"
"                                <div class=\"my-2\">\n"
"                                    <div class=\"btn-group\" id=\"offset_group\">\n"
"                                        <button class=\"btn btn-outline-danger\" id=\"offset_increment\"> + </button>\n"
"                                        <button class=\"btn btn-outline-secondary\" id=\"offset_decrement\"> - </button>\n"
"                                    </div>\n"
"                                </div>\n"
"                            </div>\n"
"                            <div class=\"row\">\n"
"                                <div class=\"col-12\" id=\"temp_scale_label\" for=\"temp_scale_group\"> Scale:</div>\n"
"                                <div class=\"my-2\">\n"
"                                    <div class=\"btn-group\" id=\"temp_scale_group\">\n"
"                                        <button class=\"btn btn-outline-info active\" type=\"button\" id=\"scale_C_button\"> °C </button>\n"
"                                        <button class=\"btn btn-outline-info\" type=\"button\" id=\"scale_F_button\"> °F </button>\n"
"                                    </div>\n"
"                                </div>\n"
"                            </div>\n"
"                            <div id=\"tunerContainer\">\n"
"                                <div class=\"my-2\">\n"
"                                    <button class=\"btn btn-secondary\" type=\"button\" id=\"tuning_button\">\n"
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
"                                                    <span for=\"ki\" class=\"input-group-text\"> Ki: </span>\n"
"                                                    <input class=\"tuning-input form-control\" type=\"text\" id=\"ki\" value=\"\" placeholder=\"Integral\" min=\"0\">\n"
"                                                </div>\n"
"                                            </div>\n"
"                                            <div class=\"col\">\n"
"                                                <div class=\"tuning-button input-group\">\n"
"                                                    <span for=\"kd\" class=\"input-group-text\"> Kd: </span>\n"
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