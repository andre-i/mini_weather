if (!window.debug)debug = false;
var serverRoot = "http://" + window.location.host + "/";


console.log('debug=' + debug);
if (debug)console.log(" start test execution ON host:" + serverRoot);


// show forecasts 
//var yahooCallbackFunction;


var chartBoard;


/**
 * build ajax request and set gotten data to callback function
 * @param url
 * @param callback
 */
function get(url, callback) {
//debug
//console.log("\nTry AJAX request to:'" + url + "'");
    var request = new XMLHttpRequest();
    request.open("GET", url);
    request.onreadystatechange = function () {
        if (request.readyState === 4 && request.status === 200) {
            //debug
            //if (debug)console.log("Success get answer on:" + url);
            var type = request.getResponseHeader("Content-Type");
            if (type.indexOf("xml") !== -1 && request.responseXML)
                callback(request.responseXML);              // Объект XML
            else if (type === "application/json")
                callback(JSON.parse(request.responseText)); // Объект JSON
            else callback(request.responseText);             // строка
        } else {
//debug
// console.log('fail ajax request [ request readyState:' + request.readyState + ' status: ' + request.status + ' ]')
        }
    };
    request.send(null); // отправить запрос
}

/**  init page :
 * get weather forecast, call method for: create - chartboard,
 *  set function for fill values to device panel
 */
(function () {
    var yahooWeather;
    var yahooCallbackFunction;
    var isInformer;
    var meteo = new MeteoViewer();
    //  device to show values
    var measurerPanel = meteo.createDevPanel();
    // form request data from server
    meteo.createActivityPanel(document.getElementsByName('activityPanel')[0]);
    /** get data from yahoo weather and create informer */
    function getYahooWeather() {
        if (yahooWeather != undefined) {
            document.body.parentNode.removeChild(yahooWeather);
        }
        yahooWeather = document.createElement('script');
        if (!yahooCallbackFunction) {
            yahooCallbackFunction = function (data) {
                isInformer = meteo.createInformer(document.getElementsByName('informer')[0], data);
            }
        }
        yahooWeather.setAttribute('src', 'https://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20weather.forecast%20where%20woeid%20%3D%202003841%20&format=json&callback=yahooCallbackFunction&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys');
        try {
            document.body.appendChild(yahooWeather);
        } catch (e) {
            isInformer = {end: 'no'};
        }
    }

//getYahooWeather();
    if (!debug) getYahooWeather();
    var timer = setInterval(createChart, 500);
    var n = 0;

    function createChart() {
        if (isInformer || n > 4) {
            clearInterval(timer);
            chartBoard = meteo.createChart(document.getElementById('graphCanvas'));
            // for test  chartBoard.create('Test many values',[ 6,2,1,-1,-4,-4,-3,-1,0,3,4,2 ]);
            if (n > 4 && !debug)alert("Не могу связаться с сервером погоды!\nВозможно нет интернет-соединения.");
        }
        n++;
    }

// get sensors Values from server and set it to devices
    setTimeout(setCurrentValues, 2500);
    setInterval(setCurrentValues, 20000);
    function setCurrentValues() {
        get(serverRoot + 'current', setDeviceValues);
    }

    /* __measurerPanel__
     * indoorTherm: tIn,
     * outdoorTherm: tOut,
     * barometer: baro,
     * hygrometer: humid
     */
    function setDeviceValues(data) {
        if (data && debug) {
            console.log("GET DATA:" + JSON.stringify(data));
        }
        try {
            measurerPanel.indoorTherm.setValue(data['tIn']);
            measurerPanel.hygrometer.setValue(data['humid']);
            measurerPanel.barometer.setValue(data['baro']);
            measurerPanel.outdoorTherm.setValue(data['tOut']);
        } catch (e) {
            console.log('in setDeviceValues ERROR\n' + e.name + ' ' + e.message);
        }
    }
}());


/**  1. check available for years and months for sensors data
 *    set enables for appropriate options
 *    2. user request handler
 */
(function () {
    var cache = {}; // cache for data by months

    //  form
    var form = document.forms['checkDuration'];
    // submit button
    var sButton = form.elements['submitButton'];
    sButton.setAttribute('disabled', 'disabled');
    sButton.addEventListener('click', showChartByUserRequest);
    // select interval option
    var interval = form.elements['intervalSelect'];
    interval.addEventListener('click', setAvailableIntervals);
    // year select
    var year = form.elements['yearSelect'];
    year.addEventListener('click', setAvailableMonths);
    var month = form.elements['monthSelect'];
    var day = form.elements['daySelect'];

    var sensors = {
        'tIn': {
            edges: { min: 0, max: 40},
            yLabel: 'здание t ℃',
            color: '#a85',
            text: 'Термометр комната'
        },
        'tOut': {
            edges: { min: -50, max: 50},
            yLabel: 'улица ℃',
            color: '#959',
            text: 'Термометр улица'
        },
        'baro': {
            edges: { min: 600, max: 800},
            yLabel: 'мм.рт.ст',
            color: 'gray',
            text: 'Барометр'
        },
        'humid': {
            edges: { min: 0, max: 100},
            yLabel: 'влажность %',
            color: '#46b',
            text: 'Гигрометр'
        }
    }


    month.addEventListener('click', function () {
        if (month.selectedIndex > 0 && day.hasAttribute('disabled'))day.removeAttribute('disabled');
        if (month.selectedIndex < 1 && !day.hasAttribute('disabled'))day.setAttribute('disabled', 'disabled');
    });
    // return value option elements
    var getVal = function (select) {
        return select.getElementsByTagName('option')[select.selectedIndex].getAttribute('value');
    };

    // sensor name , text
    function getSensor() {
        var name = getVal(form.elements['sensorSelect']);
        var sensor = sensors[name];
        sensor.name = name;
        return sensor;
    };

    /*
     chart API:
     1) setAxisLabels(xAxisLabel, yAxisLabel)    strings
     2) create(chart_name , Array_of_values, edges)     string, numbers array, object
     3) hasVisibleValues(isVible) 				  boolean
     4) chartColor(color) 						  color of chart
     5) simpleChart(Object)  					  draw any count diagrams from gotten data
     .   											(data is complex object - details in plotter.js)
     6) clearSimpleChart()
     }
     */
    /**
     *  chart drawer
     *   handler for submit button from Index.htm
     */
    function showChartByUserRequest() {
        var sensor = getSensor();
        var url, monthName, yearNum;
        if (interval.selectedIndex === 1) {
            url = 'lastValues?sensor=' + sensor.name;
            get(url, function (data) {
                if (data)drawLast(sensor, data);
                else console.log("not get data");
            });
        }
        if (interval.selectedIndex === 2) {
            yearNum = getVal(year);
            if (month.selectedIndex > 0) {
                monthName = getVal(month);
                if (!hasInterval(yearNum, monthName)) {
                    console.log("test.js: such interval NOT FOUND \"" + yearNum + '/' + monthName + '\"');
                    alert("Error");
                    return;
                }
                url = 'sensorData?period=';
                var period = yearNum + '/' + monthName + '.txt';
                var res = getFromCache(period);
                if (res) {
                    if(debug)console.log('get data from cache');
                    day.selectedIndex < 1 ? drawMonthChart(sensor, res) : drawDayChart(sensor, res, day.selectedIndex);
                }else{
                    get(url + period, function (data) {
                        if (data) {
                            addToCache(period, data);
                            day.selectedIndex < 1 ? drawMonthChart(sensor, data) :
                                drawDayChart(sensor, data, day.selectedIndex);
                        } else console.log('not get data for month: ' + monthName);
                    });
                }
            } else {
                // year chart
                drawYearChart(sensor, yearNum);
            }
        }
    }

    var flag = true;

    function getDataForYear(yNum, callback) {
        var next = (function () {
            var n = 0;
            return function () {
                return n++;
            };
        }());
        var result = {};
        var headURL = 'sensorData?period=';
        if (debug)console.log("chart for year " + yNum);
        if (availablePeriods.hasOwnProperty(yNum)) {
            var all = availablePeriods[yNum];

            for (var i = 0; i < all.length; i++) {
                var period = yNum + '/' + all[i] + '.txt';
                var res = getFromCache(period);
                if (res){
                    if(debug)console.log('month ' + all[i] + ' get from cache');
                    result[all[i]] = res;
                    if(i == all.length - 1)callback(result);
                }
                else {
                    get(headURL + period, function (data) {
                        var j = next();
                        if (debug)console.log("add " + all[j] + " month data from server");
                        if (data) {
                            addToCache(yNum + '/' + all[j] + '.txt', data);
                            result[all[j]] = data;
                            if (data.length < 1)return 'ERROR - empty month data for: \"' + all[j] + '\"';
                            if (j == all.length - 1)callback(result);
                        }
                    });
                }
            }
        } else {
            console.log('test.js: can`t find year \"' + yNum + '\" for draw year chart');
            alert('Error for: ' + yNum);
            return 'ERROR - cant find data for year:\"' + yNum + '\"';
        }
    }

    function drawYearChart(sensor, yearNum) {
        getDataForYear(yearNum, function (res) {
            if (JSON.stringify(res) == '{}') {
                console.log("test.js(drawYearChart) - Get empty year data for " + yearNum);
                alert("Не могу получить данные за " + yearNum + " год\nПроверьте работу погодной станции.");
                return 1;
            }

        });

    }

    function drawMonthChart(sensor, data) {
        if (debug) {
            console.log('drawMonthChart for sensor=' + sensor.name + '  month=' + getVal(month) +
                "data: \n " + data + "\n____EOF___");
        }
    }

    function drawDayChart(sensor, data, dayNum) {
        if (debug) {
            console.log('drawDayChart for sensor=' + sensor.name + ' day=' + dayNum +
                " data:\n " + data + "\n____EOF___");
        }
    }


    /* draw chart for last hour given sensor */
    function drawLast(sensor, data) {
        if (debug)console.log("drawLast [ name:" + sensor.name + ' text:' + sensor.text + ' ]');
        // edges for sensors data
        var edges = sensor.edges;
        var color = sensor.color;
        var yLabel = sensor.yLabel;
        var arr, x, size;
        var xlabel = 'деление 5 минут';
        chartBoard.setAxisLabels(xlabel, yLabel);
        var chartName = sensor.text;
        chartBoard.chartColor(color);
        chartBoard.create(chartName, PrepareArr(sensor.name), edges);
        // preapare array to show chart
        function PrepareArr(arrName) {
            x = data['last'];
            arr = new Array(data[arrName].length);
            size = arr.length;
            // debug
            // if(debug)console.log('last=' + x + ' array-size=' + size);
            for (var i = 0; i < size; i++)arr[i] = data[arrName][i];
            x = size + x + 1;
            for (var i = 0; i < size; i++) {
                arr[i] = data[arrName][(x + i) % size ];
            }
            //debug
            // if (debug)console.log('after shift lastValues: ' + arr.toString())
            return arr;
        }
    }


    // for store available period from SPIFFS
    // format: { 'year_num':[ 'month1', 'month2', ...],'other_year_num':[ 'month1', 'month2', ...], ... };
    var availablePeriods = {};

    //   -----  intervals -------

    var year = form.elements['yearSelect'];
    var month = form.elements['monthSelect'];

    function checkInterval() {

    }

    /**    interval Option Listener
     *  get from server intervals and set it enabled on options
     *
     */
    function setAvailableIntervals() {
//		 console.log("Selected index=" + interval.selectedIndex);
        if (interval.selectedIndex == 1 && sButton.hasAttribute('disabled'))sButton.removeAttribute('disabled');
        if (interval.selectedIndex == 2) {
            var url = serverRoot + 'availablePeriod';
            get(url, computePeriods);
        }
    }

    // interval Option Listener fill availablePeriods object
    function computePeriods(data) {
        var mName = '';
        //if(debug)console.log("getPeriods:" + JSON.stringify(data));
        if (!data || !(data instanceof Array) || data.length < 1) {
            availablePeriods = {};
            return;
        }
        var oldYear = "";
        var oldMonth = "";
        var months;
        var j = 0;
        for (var i in data) {
            var year = data[i].substring(6, 10);
            if (debug && (i < 1))console.log("First year = " + year);
            if (oldYear != year) {
                if (oldYear != "")addYearToAvailable(oldYear, months);
                oldYear = year;
                months = [];
                j = 0;
            } else {
                j++;
            }
            mName = data[i].substring(11, 14);
            months[j] = mName;
        }
        if (oldYear != "")addYearToAvailable(oldYear, months);
        if (debug)console.log('availablePeriods: ' + JSON.stringify(availablePeriods));
        setAttributeToPeriods();
    }

    function addYearToAvailable(year, months) {
        availablePeriods[year] = months;
    }

    function setAttributeToPeriods() {
        var years = year.getElementsByTagName('option');
        var isSet = false;
        for (var i = 0; i < years.length; i++) {
            if (availablePeriods[years[i].getAttribute('value')]) {
                //	console.log("availablePeriods have:" +  years[i].getAttribute('value'));
                if (years[i].hasAttribute('disabled'))years[i].removeAttribute('disabled');
                if (!isSet) {
                    year.selectedIndex = i;
                    isSet = true;
                }
            } else {
                //	console.log("availablePeriods NOT have:" +  years[i].getAttribute('value'));
                years[i].setAttribute('disabled', 'disabled');
            }

        }
        month.selectedIndex = 0;
        day.selectedIndex = 0;
        if (years.length > 0) {
            if (sButton.hasAttribute('disabled'))sButton.removeAttribute('disabled');
            if (year.hasAttribute('disabled'))year.removeAttribute('disabled');
        }
    };

    /**   Year option Listener
     * set enabled state to month option
     */
    function setAvailableMonths() {
        if (year.selectedIndex < 1)return;
        var months = month.getElementsByTagName('option');
        var available = availablePeriods[getVal(year)];
        for (var i = 1; i < months.length; i++) {
            if (available.indexOf(months[i].getAttribute('value')) < 0) {
                months[i].setAttribute('disabled', 'disabled');
            } else {
                if (months[i].hasAttribute('disabled')) months[i].removeAttribute('disabled');
            }
        }
        if (months[0].hasAttribute('disabled')) months[0].removeAttribute('disabled');
        month.selectedIndex = 0;
        day.selectedIndex = 0;
        if (!day.hasAttribute('disabled')) day.setAttribute('disabled', 'disabled');
        if (available.length > 0) {
            if (sButton.hasAttribute('disabled'))sButton.removeAttribute('disabled');
            if (month.hasAttribute('disabled'))month.removeAttribute('disabled');
        }
    }

    function hasInterval(yearNum, monthName) {
        var hasInterval = true;

        hasInterval = availablePeriods.hasOwnProperty(yearNum) && availablePeriods[yearNum] instanceof Array;
        if (debug)console.log("get yearNum=" + yearNum + "  monthName=" + monthName + " months_in_periods: " + availablePeriods[yearNum]);
        if (hasInterval) {
            hasInterval = availablePeriods[yearNum].indexOf(monthName) >= 0;
        }
        return hasInterval;
    }

    function addToCache(period, data) {
        if (cache.hasOwnProperty(period))return false;
        cache[period] = data;
    }

    function getFromCache(period) {
        if (cache.hasOwnProperty(period))return cache[period];
        return false;
    }

}());






