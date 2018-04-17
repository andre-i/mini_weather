if (!window.debug)debug = false;
var serverRoot = "http://" + window.location.host + "/";


console.log('debug=' + debug);
if (debug)console.log(" start test execution ON host:" + serverRoot);
if (debug)console.log('Browser: \'' + navigator.userAgent + '\'  ');

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
    //var yahooWeather;
    //  var yahooCallbackFunction;
    var isInformer;
    var meteo = new MeteoViewer();
    //  device to show values
    var measurerPanel = meteo.createDevPanel();
    // form request data from server
    meteo.createActivityPanel(document.getElementsByName('activityPanel')[0]);
    /** get data from yahoo weather and create informer */
    function createInformer() {
       // if (debug)console.log('call "CreateInformer()" ');
        if (yahooWeatherData)isInformer = meteo.createInformer(document.getElementsByName('informer')[0], yahooWeatherData);
        else console.log('Can`t get data from Yahoo Weather');
    }

    var timer = setInterval(createChart, 1000);
    var n = 0;

    function createChart() {
        if (isInformer || n > 4) {
            clearInterval(timer);
            chartBoard = meteo.createChart(document.getElementById('graphCanvas'));
            // for test  chartBoard.create('Test many values',[ 6,2,1,-1,-4,-4,-3,-1,0,3,4,2 ]);
            if (n > 4 && !debug)alert("Не могу связаться с сервером погоды!\nВозможно нет интернет-соединения.");
        } else {
            createInformer();
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
    // month select
    var month = form.elements['monthSelect'];
    month.addEventListener('click', setAvailableDays);
    // day select
    var day = form.elements['daySelect'];
    // first part URL for get periods data
    var startGetDataURL = 'sensorData?period=';

    var sensors = {
        'tIn': {
            edges: { min: 0, max: 40},
            yLabel: ' t ℃',
            color: '#a85',
            text: 'Термометр комната',
            numInData: 2
        },
        'tOut': {
            edges: { min: -50, max: 50},
            yLabel: 't ℃',
            color: '#959',
            text: 'Термометр улица',
            numInData: 3
        },
        'baro': {
            edges: { min: 600, max: 800},
            yLabel: 'мм.рт.ст',
            color: 'gray',
            text: 'Барометр',
            numInData: 4
        },
        'humid': {
            edges: { min: 0, max: 100},
            yLabel: 'влажность %',
            color: '#46b',
            text: 'Гигрометр',
            numInData: 5
        }
    };


    month.addEventListener('click', function () {
        if (month.selectedIndex > 0 && day.hasAttribute('disabled')) {
            day.removeAttribute('disabled');

        }
        if (month.selectedIndex < 1 && !day.hasAttribute('disabled'))day.setAttribute('disabled', 'disabled');
    });
    // return value option elements
    var getVal = function (select) {
        return select.getElementsByTagName('option')[select.selectedIndex].getAttribute('value');
    };

    var getSelectName = function (select) {
        return select.getElementsByTagName('option')[select.selectedIndex].innerHTML;
    };

    // sensor name , text
    function getSensor() {
        var name = getVal(form.elements['sensorSelect']);
        var sensor = sensors[name];
        sensor.name = name;
        return sensor;
    }

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
                var period = yearNum + '/' + monthName + '.txt';
                getFromCache(period,
                    function (data) {
                        day.selectedIndex < 1 ? drawMonthChart(sensor, data) : drawDayChart(sensor, data, day.selectedIndex);
                    });
            } else {
                // year chart
                drawYearChart(sensor, yearNum);
            }
        }
    }

    /**
     * get data for all available months and collect it into result object
     * after call callback function to draw year chart
     * @param yNum - selected year
     * @param callback - function for handle gotten data
     */
    function getDataForYear(yNum, callback) {
        var result = {};
        if (debug)console.log("test.js [ start fill data for year " + yNum + ' ]');
        if (availablePeriods.hasOwnProperty(yNum)) {
            var all = availablePeriods[yNum];
            if (all.length < 1)callback(result);
            var n = 0; // month number in available periods
            function addForMonth(monthNumber) {
                var period = yNum + '/' + all[n] + '.txt';
                getFromCache(period,
                    function (data) {
                       // if(debug)console.log('test.js fill data for: ' + period);
                        if (data)result[all[n]] = data;
                        n++; // shift for next iteration
                        if (n == all.length)callback(result);
                        else addForMonth(n);
                    });
            }
            addForMonth(n);
        } else {
            callback(result);
        }
    }

    /**
     * draw year chart for given sensor
     * if data for selected year not found, then draw empty chart
     * @param sensor - sensor type
     * @param yearNum
     * @returns {boolean} - if success draw chart return true
     */
    function drawYearChart(sensor, yearNum) {
        var toRu = function (enName) {
           // if (debug)console.log('get en: ' + enName);
            enName = enName.toLowerCase();
            switch (enName) {
                case 'jan' :
                    return 'Янв';
                case 'feb':
                    return 'Фев';
                case 'mar':
                    return 'Мар';
                case 'apr':
                    return 'Апр';
                case 'may':
                    return 'Май';
                case 'jun':
                    return 'Июнь';
                case 'jul':
                    return 'Июль';
                case 'aug':
                    return 'Авг';
                case 'sep':
                    return 'Сен';
                case 'oct':
                    return 'Окт';
                case 'nov':
                    return 'Нояб';
                case 'dec':
                    return 'Окт';
                default :
                    return false;
            }
        };
        var months = [];
        var meansInMonths = [];
        var n = 0;
        var cur, max;
        var curValue;
        var xLabel = 'Месяц';
        var chartName = ' ' + sensor.text + ' за ' + yearNum + 'г.';
        var firstMonthName = false;
        // get data for drav chart and compute mean value all months
        getDataForYear(yearNum, function (res) {
           // if (debug)console.log("GET data for year chart len=" + res.length + '  DATA:[ ' + JSON.stringify(res) + ' ]');
            if (JSON.stringify(res) == '{}') {
                console.log("test.js(drawYearChart) - Get empty year data for " + yearNum);
                alert("Не могу получить данные за " + yearNum + " год\nПроверьте работу погодной станции.");
                drawChart('', '', 'red', '', [0, 0, 0, 0, 0], {min: -1, max: 1});
                return 1;
            } else {
                var i;
                var val;
                for (var p in res) {
                   // if (debug)console.log("get Data For year month:" + toRu(p) + "  sensor:[" + sensor.text + " numInData:" + sensor.numInData + ' ]');
                    if(!firstMonthName)firstMonthName = p;
                    months[n] = toRu(p);
                    cur = res[p].split('\n');
                    max = cur.length - 1;
                    curValue = 0;
                    for (i = 0; i < max; i++) {
                        val = Number(cur[i].trim().split(/\s/)[sensor.numInData]);
                       // if (debug)console.log("value = " + val);
                        if ( val < sensor.edges.min || val > sensor.edges.max)continue;
                        curValue += val;
                    }
                    meansInMonths[n] = (curValue > 0) ? parseInt(curValue / (i + 1) + 0.5001) : parseInt(curValue / (i + 1) - 0.5001);
                    n++;
                  //  if (debug)console.log('month=' + p + '__ curValue=' + curValue + '  mean=' + meansInMonths);
                }
                // for one month in year
                if(months.length == 1){
                    if(debug)console.log('test.js try draw year chart as Month')
                    var allM = month.getElementsByTagName('option');
                    for( i = 0; i < allM.length; i++ ){
                        if(allM[i].getAttribute('value').indexOf(firstMonthName) > -1 )allM[i].setAttribute("selected", "selected");
                    }
                    getFromCache(yearNum + '/' + getVal(month) + '.txt', function(data){
                        drawMonthChart(sensor,data);
                        return true;
                    });
                }
                if (debug)console.log('year chart: [ ' + months + ' ]     [ ' + meansInMonths + ' ]');
               if(months.length != 1) drawChart(xLabel, sensor.yLabel, sensor.color, chartName, {values: meansInMonths, xAxisLabels: months}, sensor.edges);
                return false;
            }
        });
        //  if server response not found for 6 seconds - set nul chart
        setTimeout(checkOnGet(), 6000);
        function checkOnGet() {
            if (n < 1) {
                drawChart('', '', 'red',
                    "Нет данных для " + sensor.text + ' за ' + yearNum + 'г', [0, 0, 0, 0], {min: -1, max: 1})
            }
        }
        return true;
    }

    /**
     *
     * @param sensor - type of sensor for draw chart
     * @param data gotten data on month for all sensors
     */
    function drawMonthChart(sensor, data) {
        if (debug) console.log('drawMonthChart for sensor=' + sensor.name + '  month=' + getVal(month) + " data: \n " + data + "\n____EOF___");
        var all = data.split('\n');
        var maxLen = all.length - 1; // want for correct handle mean value in last day
        var monthYear = getSelectName(month) + ' ' + getVal(year) + 'г';
        var xLabel = 'число';
        // if empty
        if (maxLen < 0) {
            drawChart('', '', '#f99', 'НЕТ ДАННЫХ для ' + sensor.text + '  ' + monthYear, [0, 0, 0, 0, 0, 0, 0], {min: -1, max: 1});
            return;
        }
        var meanInDay = [];
        var dayInMonth = [];
        var cur = all[0].trim().split(/\s/);
        var day = cur[0];
        var n = 1;
        var dayVals = Number(cur[sensor.numInData]);
        if (isNaN(dayVals) || dayVals < sensor.edges.min || dayVals > sensor.edges.max)dayVals = 0;
        var numInData = 0;
        var hourVal = 0;
        for (var i = 1; i < all.length; i++) {
            cur = all[i].trim().split(/\s/);
            if (day == cur[0]) {
                hourVal = Number(cur[sensor.numInData]);
                if (isNaN(hourVal) || hourVal > sensor.edges.max || hourVal < sensor.edges.min)continue;
                dayVals += hourVal;
                n++;
            } else if (i < maxLen) {
                dayInMonth[numInData] = day;
                meanInDay[numInData] = (dayVals < 0) ? parseInt(dayVals / n - 0.5001) : parseInt(dayVals / n + 0.5001);//
                day = cur[0];
                dayVals = Number(cur[sensor.numInData]);
                n = 1;
                numInData++;
            } else {  //  right handle last value in month days
                dayInMonth[numInData] = day;
                meanInDay[numInData] = (dayVals < 0) ? parseInt(dayVals / n - 0.5001) : parseInt(dayVals / n + 0.5001); //
            }
        }
        if (numInData == 0) {
            drawChart(xLabel, '     ', sensor.color,
                'за ' + day + '/' + monthYear + ' среднее для ' + sensor.text + ' = ' + meanInDay[0],
                [0, 0, 0, 0], {min: -1, max: 1});
            return;
        }
        var chartName = ' ' + sensor.text + ' за ' + monthYear;
       // if (debug)console.log('To draw Month chart: \nvalues - ' + meanInDay + '\nlabels - ' + dayInMonth);
        var res = { values: meanInDay, xAxisLabels: dayInMonth};
        drawChart(xLabel, sensor.yLabel, sensor.color, chartName, res, sensor.edges);
    }

    function drawDayChart(sensor, data, dayNum) {
        //if (debug)console.log('drawDayChart for sensor=' + sensor.name + ' day=' + dayNum + " data:\n " + data + "\n____EOF___");
        var all = data.split('\n');
        var startHour, endHour;
        var dayData = []; // sensor values
        var hourInDay = [];  // hours for sensor data
        var n = 0;
        var minVal = all.length - 1;
        for (var i = 0; i < minVal; i++) {
            all[i] = all[i].trim();
            //  if (debug)console.log('for parse: ' + all[i]);
            if (all[i].length < 3)continue;
            var dayD = all[i].split(/\s/);
            var hourVal = NaN;
            if (dayD[0].length > 1) {
                var cur = (dayD[0].charAt(0) === '0') ? Number(dayD[0].charAt(1)) : Number(dayD[0]);
                if (cur === dayNum) {
                    if (n == 0)startHour = dayD[1];
                    else endHour = dayD[1];
                    hourVal = Number(dayD[sensor.numInData]);
                    if (!isNaN(hourVal) && ( hourVal > sensor.edges.min && hourVal < sensor.edges.max)) {
                        dayData[n] = hourVal;
                        hourInDay[n] = dayD[1];
                        n++;
                    }
                    // if (debug)console.log('dayData[' + n + '] = ' + dayData[n]);
                }
            } else {
                console.log('Error dayData is not String-\'' + dayD[0] + '\'  dayD[1]=' + dayD[1]);
            }
        }
        var xLabel = 'часы';
        var monthN = (month.selectedIndex < 10) ? '/0' + month.selectedIndex : '/' + month.selectedIndex;
        var chartName = ' ' + sensor.text + ' за ' + dayNum + monthN + '/' + getVal(year);
        var sEdges = sensor.edges;
        if (dayData.length < 1) {
            drawChart('', '', 'red', 'НЕТ ДАННЫХ для: ' + chartName, [0, 0, 0, 0, 0, 0], {min: -1, max: 1});
        } else if (dayData.length === hourInDay.length) {
            dayData = { values: dayData, xAxisLabels: hourInDay};
        }
        drawChart(xLabel, sensor.yLabel, sensor.color, chartName, dayData, sEdges);
    }

    /**
     * draw chart on surface
     * @param xLabel x axis label text
     * @param yLabel y axis label text
     * @param color chart color(in HTML form)
     * @param chartName string
     * @param periodData arrays values
     * @param edges min and max values in array( implied )
     */
    function drawChart(xLabel, yLabel, color, chartName, periodData, edges) {
        chartBoard.setAxisLabels(xLabel, yLabel);
        chartBoard.chartColor(color);
        chartBoard.create(chartName, periodData, edges);
    }

    /* draw chart for last hour given sensor */
    function drawLast(sensor, data) {
       // if (debug)console.log("drawLast [ name:" + sensor.name + ' text:' + sensor.text + ' ]');
        var arr, x, size;
        var xLabel = '    время';
        var chartName = sensor.text;
        var toDraw = { values: PrepareArr(sensor.name), xAxisLabels: getFiveMinutePeriods()};
        drawChart(xLabel, sensor.yLabel, sensor.color, chartName, toDraw, sensor.edges);
        // preapare array to show chart
        function PrepareArr(arrName) {
            x = data['last'];
            arr = new Array(data[arrName].length);
            size = arr.length;
            // debug
            // if(debug)console.log('last=' + x + ' array-size=' + size);
            for (var i = 0; i < size; i++)arr[i] = data[arrName][i];
            x = size + x + 1;
            for (i = 0; i < size; i++) {
                arr[i] = data[arrName][(x + i) % size ];
            }
            //debug
            // if (debug)console.log('after shift lastValues: ' + arr.toString())
            return arr;
        }

        // get last time as 5 minutes intervals
        function getFiveMinutePeriods() {
            var d = new Date();
            //  if(debug)console.log('now = ' + new Date().getHours() + ':' + new Date().getMinutes());
            var arr = [];
            var h, m;
            d.setSeconds(d.getSeconds() - 3900);
            for (var i = 0; i < 13; i++) {
                d.setSeconds(d.getSeconds() + 300);
                h = d.getHours();
                h = (h > 9) ? h : '' + '0' + h;
                m = d.getMinutes();
                m = (m > 9) ? m : '' + '0' + m;
                arr[i] = '' + h + ':' + m;
            }
            //  if(debug)console.log("Times: " + arr.toString());
            return arr;
        }
    }


    // for store available period from SPIFFS
    // format: { 'year_num':[ 'month1', 'month2', ...],'other_year_num':[ 'month1', 'month2', ...], ... };
    var availablePeriods = {};

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
            if (availablePeriods) {
                document.getElementsByName('fYearLabel')[0].style.color = '#403030';
                document.getElementsByName('fMonthLabel')[0].style.color = '#404030';
            }
        }
    }

    // interval Option Listener fill availablePeriods object
    function computePeriods(data) {
        var mName;
        //if(debug)console.log("getPeriods:" + JSON.stringify(data));
        if (!data || !(data instanceof Array) || data.length < 1) {
            availablePeriods = {};
            return;
        }
        var oldYear = "";
        var months;
        var j = 0;
        for (var i in data) {
            var year = data[i].substring(6, 10);
            if (debug && (i < 1))console.log("First year = " + year);
            if (oldYear != year) {
                if (oldYear != "")addYearToAvailable(oldYear, months);
                months = [];
                oldYear = year;
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
                //	if(debug)console.log("availablePeriods NOT have:" +  years[i].getAttribute('value'));
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

    /**
     * month option click listener
     * get from cache data for choose month, check available days and
     * set appropriate attribute for days in day options
     * ( on check time cache must contains data for given month )
     */
    function setAvailableDays() {
        // set available days by given data
        var computeAvailable = function (res) {
            var all = res.split('\n');
            if (all.length < 1)return;
            var availableDays = [];
            var cur;
            var before = -1;
            var counter = 0;
            for (var i = 0; i < all.length; i++) {
                cur = all[i].trim().split(/\s/)[0];
                if (cur != before) {
                    availableDays[counter] = cur;
                    before = cur;
                    counter++;
                }
            }
            // console.log('test.js available days ' + availableDays);
            var days = day.getElementsByTagName('option');
            day.selectedIndex = 0;
            var dayInMonth;
            for (i = 0; i < days.length; i++) {
                dayInMonth = days[i].getAttribute('value');
                if (dayInMonth < 9) dayInMonth = '0' + dayInMonth;
                if (availableDays.indexOf(dayInMonth) < 0) {
                    days[i].setAttribute('disabled', 'disabled');
                } else {
                    if (days[i].hasAttribute('disabled')) days[i].removeAttribute('disabled');
                }
            }
            if (day.hasAttribute('disabled'))day.removeAttribute('disabled');
            if(days[0].hasAttribute('disabled'))days[0].removeAttribute('disabled');
        }
        // if (debug)console.log('test.js on month click selectedIndex=' + month.selectedIndex);
        if (month.selectedIndex < 1)return;
        // if (debug)console.log('Try check available days');
        var period = getVal(year) + '/' + getVal(month) + '.txt';
        getFromCache(period, function(data){
            //  console.log('test.js: ' + JSON.stringify(data));
            if(data)computeAvailable(data);

        });
    }

    function hasInterval(yearNum, monthName) {
        var hasInterval = true;
        hasInterval = availablePeriods.hasOwnProperty(yearNum) && availablePeriods[yearNum] instanceof Array;
        //if (debug)console.log("get yearNum=" + yearNum + "  monthName=" + monthName + " months_in_periods: " + availablePeriods[yearNum]);
        if (hasInterval) {
            hasInterval = availablePeriods[yearNum].indexOf(monthName) >= 0;
        }
        return hasInterval;
    }

    function addToCache(period, data) {
        if (cache.hasOwnProperty(period))return false;
        cache[period] = data;
    }

    /**
     * if cache contains data - call callback with data for period
     * , else get data from server, verify it, set to cache and after call callback
     * @param period - year-month('yyyy/month_name.txt')
     * @param callback - function for handle data of period
     */
    function getFromCache(period, callback) {
        //if(debug)console.log('test.js getFromCache for: ' + period);
        // check given data on format
        var checkData = function (data) {
           // if (debug) console.log('test.js getData For Check: ' + data);
            var res = ''; // string with right data format
            var all = data.split('\n');
            if (all.length < 1)return false;
            var cur;
            for (var i = 0; i < all.length; i++) {
               // if(debug)console.log('test.js getFromCache getData currentString: ' + all[i]);
                if (all[i].search(/[^\s\d-]/) > -1) continue; // right data contains only digits and white spaces
                cur = all[i].trim().split(/\s/);
               // if (debug)console.log('test.js checkData length string data=' + cur.length);
                if (cur.length != 6)continue; // wrong count member in data string
                if(isNaN(cur[2]) || cur[2] > 1000 || cur[2] < -1000) continue;
                if(isNaN(cur[3]) || cur[3] > 1000 || cur[3] < -1000) continue;
                if(isNaN(cur[4]) || cur[4] > 1000 || cur[4] < -1000) continue;
                if(isNaN(cur[5]) || cur[5] > 1000 || cur[5] < -1000) continue;
                res += all[i] + '\n';
            }
            if (res.length < 14)return false;
            return res;
        }
        if (cache.hasOwnProperty(period)) {
            if (debug)console.log('get data from cache');
            callback(cache[period]);
        }
        else {
            if(debug)console.log('test.js try get data from server');
            get(startGetDataURL + period, function (data) {
                if (data) {
                    //if(debug)console.log('test.js i get from server: ' + data);
                    var res = checkData(data);
                    //if (debug)console.log('test.js after check data return: ' + res);
                    if (res) {
                        addToCache(period, res);
                    } else {
                        console.log('test.js [ GET WRONG DATA FOR PERIOD ' + period + ' ] ');
                    }
                    callback(res);
                } else {
                    console.log('test.js [ GET EMPTY DATA FOR PERIOD ' + period + ' ] ');
                    callback(false);
                }
            });
        }
    }

}());






