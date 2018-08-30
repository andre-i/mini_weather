/**
 * Created by user on 03.11.17.
 */


function MeteoViewer() {
    // last year in years list for check year
    var maxYear = 2025;


    /** return day in week and month name in browser local */
    var toLocal = function (name, isShort) {
        if (name.length < 3)return '';
        name = name.trim().substring(0, 3).toLowerCase();
        return isShort ? shortWord[name] : word[name];
    }

    var forecastDays = [];
    var svgns = 'http://www.w3.org/2000/svg';

    //   out data elements
    var chart;
    // thermometers
    var tIn, tOut;
    // barometer hygrometer
    var baro, humid;


	/**
	*  create panel with thermometers, barometer and hyhrometer
	*/
    function createDevicesPanel() {

        //  debug
        console.log("value of word =" + word);

        var colors = {
            devBordStartCol: 'rgba(75,95,130,0.3)'
        };
        var rect = {
            // hasRoundRect: false,
            label: "&nbsp;t℃"
        }
//  inDoor thermometer
        var tInBounds = {
            minValue: 0,
            maxValue: 40,
            scaleDeviceCount: 20
        }
        tIn = new Measurer(document.getElementsByName("inThermOwner")[0],
            {
                colors: colors,
                bounds: tInBounds,
                rect: rect
            });
// outDooor thermometer
        var tOutBounds = {
            minValue: -50,
            maxValue: 50,
            scaleDeviceCount: 30
        }
        tOut = new Measurer(document.getElementsByName("outThermOwner")[0],
            {
                colors: colors,
                bounds: tOutBounds,
                rect: rect
            });
// barometer
        colors = {
            devBordStopCol: 'rgba(80,90,10,0.2)'
        }
        baro = new Measurer(document.getElementsByName("baroOwner")[0],
            {
                bounds: {
                    minValue: 650,
                    maxValue: 810,
                    scaleDivideCount: 16
                },
                oval: {
                    label: "&nbsp;" + word['baro.label'],
                    startAngle: -35,
                    stopAngle: 215,
                    hasRound: false
                },
                colors: colors
            });

// hygrometer
        humid = new Measurer(document.getElementsByName("hygroOwner")[0],
            {
                bounds: {
                    minValue: 0,
                    maxValue: 100,
                    scaleDeviceCount: 10
                },
                oval: {
                    startAngle: -10,
                    stopAngle: 190,
                    label: "&nbsp;%",
                    hasRound: false
                },
                colors: colors
            });

        return {
            indoorTherm: tIn,
            outdoorTherm: tOut,
            barometer: baro,
            hygrometer: humid
        }
    }

    /**
     *
     * @param owner HTML container( div , table and etc)
     * @returns {boolean} false if can not create activity panel
     */
    function createActivityPanel(owner) {
        if (screen.width < screen.height)return null;
//        console.log("callCreateActivity with owner:" + owner);
        if (!(owner instanceof HTMLElement)) {
            console.log("ERROR can`t create activity panel: I get not HTMLElement:" + owner);
            return null;
        }
        var monthsEn = [ "   ", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
        var monthsLocal = [ word.all, word.jan , word.feb, word.mar, word.apr, word.may, word.jun, word.jul, word.aug, word.sep, word.oct, word.nov, word.dec];
        var selected = {
            year: 0,
            month: 0,
            day: 0
        }
        var i;
        var form = document.forms['checkDuration'];
        var labels = form.getElementsByTagName('label');

        var setPeriodDisabled = function () {
            year.setAttribute('disabled', 'disabled');
            month.setAttribute('disabled', 'disabled');
            day.setAttribute('disabled', 'disabled');
            for (i = 0; i < labels.length; i++) {
                labels[i].setAttribute('class', 'label');
            }
        }


        //  listener
        var checkListener = function (e) {
//console.log('click');
// console.log('day: ' + day.selectedIndex + ' month: ' + month.selectedIndex + ' year: ' + year.selectedIndex)
            if (sType.selectedIndex == 0) {
                interval.setAttribute('disabled', 'disabled');
                setPeriodDisabled();
            }
            if (sType.selectedIndex > 0) {
                if (interval.hasAttribute('disabled'))interval.removeAttribute('disabled');
            }
            if (sType.selectedIndex > 0 && interval.selectedIndex == 1) {
                setPeriodDisabled();
            }
        }
// sensors type check
        var sType = form.elements['sensorSelect'];
        var sensorsNames = {
            empty: '',
            tOut: word['tOut.text'],
            tIn: word['tIn.text'],
            baro: word['baro.text'],
            humid: word['humid.text']
        }
        for (i in sensorsNames)sType.appendChild(new Option(sensorsNames[i], i));
        var interval = form.elements['intervalSelect'];
        interval.appendChild(new Option('  ', 'empty'));
        interval.appendChild(new Option( '  ' + word.lastHour + ' ', 'lastHour'));
        interval.appendChild(new Option(word.checkInterval, 'checkInterval'));
        interval.setAttribute('disabled', 'disabled');
//year check
        var year = form.elements['yearSelect'];
        year.appendChild(new Option('?', 'empty'));
        for (i = 2018; i < (maxYear + 1); i++)year.appendChild(new Option((i), (i) + ''));
        for (i = 0; i < year.getElementsByTagName('option').length; i++)
            year.getElementsByTagName('option')[i].setAttribute('style', 'color:');
        year.setAttribute('disabled', 'disabled');
//month check
        var month = form.elements['monthSelect'];
        for (i = 0; i < monthsEn.length; i++)month.appendChild(new Option(monthsLocal[i], monthsEn[i]));
        month.setAttribute('disabled', 'disabled');
//day check
        var day = form.elements['daySelect'];
        day.appendChild(new Option(word.all, " "));
        for (i = 1; i < 32; i++)day.appendChild(new Option(i + "", i));
        day.setAttribute('disabled', 'disabled');
//  submit button
        var submitButton = form.elements['submitButton']  //document.createElement('button');;
        submitButton.setAttribute('value', 'getChart');
        submitButton.setAttribute('disabled', 'disabled');
        submitButton.setAttribute('class', 'submitButton');
        submitButton.innerHTML = word.build;
        // submitButton.innerHTML='<div style="width: 40;height: 40"></div>';
        setPeriodDisabled();
        form.addEventListener('click', function (e) {
            checkListener(e)
        }, false);
        return form;
    }


    function createChartCanvas(chartOwner) {
        if (screen.width < screen.height)return null;
        var chartWidth = document.body.clientWidth -
            1.1 * document.getElementsByTagName('aside')[0].offsetWidth;
        var padd = getComputedStyle(document.getElementsByTagName('aside')[0]).padding;
        padd = padd.substring(0, padd.length - 2);
        var informerHeight = document.getElementsByName('informer')[0].clientHeight;
        var chartHeight = document.getElementsByTagName('aside')[0].clientHeight -
            Math.max(document.getElementsByName('activityPanel')[0].offsetHeight, informerHeight) - 4 * padd;
        //2 * document.getElementsByName('activityPanel')[0].offsetTop;
        chart = new Plotter(
            chartOwner,
            chartWidth,  // document.getElementsByName('activityPanel')[0].offsetWidth ,
            chartHeight,
            {
                borderCol: getComputedStyle(
                    document.getElementsByTagName('table')[0]).borderColor /* get border color of device panel*/
            }
        );
        return chart;
    }


    function createWeatherInformer(informerOwner, data) {
//if(debug)console.log(JSON.stringify(data));
        addForecastButton();
        var informer = document.createElement('div');
        informer.setAttribute('class', 'informerBoard inlineTop wavesBackGround'); //
        var resp = data['query']['results']['channel'];
        var image = resp['image'];
        addLogo(image);
        var descr = resp['item']['description'];
        var img;
        addPict(descr);
        var atm = resp["atmosphere"];
        var forecast = resp['item']['forecast']; // aray
        addCurrent(atm, forecast[0]);
        fillForecasts(forecast);
        informerOwner.appendChild(informer);

        function addForecastButton() {
            var fore = document.createElement('div');
            fore.setAttribute('class', 'inlineTop informerFore'); // informerImgBord
            fore.onclick = showForecast;
            fore.innerHTML = word.forecastButtonName;
            informerOwner.appendChild(fore);
        }

        function addLogo(image) {
            var a = document.createElement('a');
            a.setAttribute('href', image['link']);
            a.setAttribute('target', '_blank');
            a.setAttribute('style', 'text-align:center');
            var img = document.createElement('img');
            img.setAttribute('src', image['url']);
            img.setAttribute('width', image['width']);
            img.setAttribute('height', image['height']);
            img.setAttribute('alt', image['title']);
            img.setAttribute('class', 'informerImgBord logo right');
            a.appendChild(img);
            informer.appendChild(a);
            //  table.getElementsByTagName('tr')[0].getElementsByTagName('td')[0].appendChild(a);
        }

        /** set weather icon */
        function addPict(descr, owner) {
            var now = new Date().getHours();
            var min = new Date().getMinutes() / 100;
            now += min;
            var sr = resp['astronomy']['sunrise'] + "";
            sr = sr.substring(0, sr.indexOf(' ')).replace(':', '.');
            sr = sr * 1;
            var ss = resp['astronomy']['sunset'] + "";
            ss = ss.substring(0, ss.indexOf(' ')).replace(':', '.');
            ss = 12 + ss * 1;
            var bg = (now > sr && now < ss) ? 'left imgDay informerImgBord' : 'left imgNight informerImgBord';
            var pict = descr.substring(9, descr.indexOf('>') + 1);
            img = document.createElement('div');
            img.setAttribute('class', bg);
            img.innerHTML = pict;
            informer.appendChild(img);
        }

        function addCurrent(atm, today) {
            var date = today['date'];
            date = toLocal(today['day'], true) + '&nbsp;' +  getLocalMonth(date, false);
            var infoText = document.createElement('div');
            infoText.setAttribute('class', 'informerText');
            var res = '<div><span class="informer labelColor"> ' + date + '</span></div>' +
                '<span  class="informer labelColor">' + word['humid.label'] + ' ' + atm['humidity'] + '&nbsp;%</span><br />' +
                '<span  class="informer labelColor">' + word.pressure + '&nbsp;' +
                (atm['pressure'] / 1000 * 750.0637 + '').substring(0, 3) + '&nbsp;' + word['baro.label'] + '</span>' +
                //  '<hr class="informerDivider">' +
                '<div style="margin-top: 0.3em;margin-bottom: 3px">&nbsp;<span  class="informerDay">' +
                word.day + ': ' + ((today['high'] - 32) * 5 / 9 + "").substring(0, 4) + ' ' + word['celcius'] +' </span>&nbsp;<span class="informerNight">'
                + word.night + ': ' + ((today['low'] - 32) * 5 / 9 + "").substring(0, 4) + ' ' + word['celcius'] + ' &nbsp;</span></div>';
            infoText.innerHTML += res;
            informer.appendChild(infoText);
        }

        function fillForecasts(forecast) {
            var s = '';
            for (var i = 0; i < forecast.length; i++) {
                var day = {};
                day.picture = 'http://l.yimg.com/a/i/us/we/52/' + forecast[i]['code'] + '.gif';
                day.max = Math.round((forecast[i]['high'] - 32) * 500 / 9) / 100;
                day.min = Math.round((forecast[i]['low'] - 32) * 500 / 9) / 100;
                day.date = toLocal(forecast[i]['day'], true) + ' ' + getLocalMonth(forecast[i]['date'], true);
                forecastDays[i] = day;
            }
        }

        function getLocalMonth(date, isShort) {
            var sp = isShort ? '\n' : '&nbsp;';
            var mName = toLocal(date.split(' ')[1], isShort);
            if (!isShort && navigator.language.search("ru") > -1)mName = (mName.indexOf('ь') > 0 || mName.indexOf('й') > 0) ?
                mName.substring(0, mName.length - 1) + 'я' : mName + 'а';
            return date.split(' ')[0] + sp + mName;
        }

        return {end: 'yes'};
    }

    var owner;

    function showForecast() {
        if (forecastDays.length < 2)return;
        if (document.body.clientWidth < screen.height)return;
        var forecastData = prepareDataForChart(forecastDays);
//console.log('\nFORECAST OBJECT:\n' + JSON.stringify(forecastData));
        if (owner == undefined) owner = document.createElement('div');
        owner.setAttribute('class', 'bigSize');
        var svgDiv = document.createElement('div');    
        svgDiv.width = document.body.scrollWidth * 4 / 5;
        svgDiv.height = screen.height*4/6;
        svgDiv.setAttribute('style', 'margin-top:' + screen.height / 12  + 'px');
        owner.appendChild(svgDiv);
        document.body.appendChild(owner);
//console.log("Try create forecast diagram" );        
        chart.simpleChart(svgDiv, forecastData);
        owner.onclick = function () {
            chart.clearSimpleChart(svgDiv);
            owner.setAttribute('class', 'noSize');
        }

    }

    //================  PRIVATE  =====================//

    function prepareDataForChart(forecastDays) {
        // <image x="0" y="0" width="153.416" height="122.682" xlink:href="picture.jpg"/>
        var minThemp = [], maxThemp = [];
        var res = {};
        var max = forecastDays[0].max, min = forecastDays[0].min;
        for (var i = 0; i < forecastDays.length; i++) {
            if (forecastDays[i].max > max) max = forecastDays[i].max;
            if (forecastDays[i].min < min) min = forecastDays[i].min;
            var svg = document.createElementNS(svgns, 'image');
            svg.setAttribute('width', '52');
            svg.setAttribute('height', '52');
            svg.setAttributeNS(svgns, 'xlink:href', forecastDays[i]['picture']);
            minThemp[i] = forecastDays[i].min;
            maxThemp[i] = { val: forecastDays[i].max, svg: svg, text: forecastDays[i].date};
        }
        res.min = Math.round(min - (max - min) / 5);
        res.max = Math.round(max + (max - min) / 10);
        res.label = { name: '&nbsp; &nbsp;' + word.forecast , color: '#752', points: []};
        res.space0 = { name: '', color: 'rgba(0,0,0,0)', points: []};
        res.maxThemp = { name: word.day + word['tOut.label'], color: '#909', points: maxThemp };
        res.space1 = { name: '', color: 'rgba(0,0,0,0)', points: []};
        res.minThemp = {name: word.night + word['tOut.label'], color: '#069', points: minThemp };
        return res;
    }


    return {
        createDevPanel: createDevicesPanel,
        createActivityPanel: createActivityPanel,
        createChart: createChartCanvas,
        createInformer: createWeatherInformer,
        showForecast: showForecast
    }

};























