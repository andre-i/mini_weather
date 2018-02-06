//  здесь рисуем график
//
/*
 startProps object structure:
 {
 borderCol: "#col1",
 bgCol: "#col2",
 axisCol: "#col3",
 labelCol: "#col4",
 }
 is colors set to draw the chart.
 */


function Plotter(canvasOwner, width, height, startProps) {
    /*
     * small summ sign on right-top corner chart board flip
     *  show or not values on chart points
     */

//console.log("\n === canvas ===\n height: " + height + "  width:" + width);

    // check parameters
    if (canvasOwner == null && !(canvasOwner instanceof HTMLElement)) {
        console.log("'painter' parameter must be HTML container. I get: " + canvasOwner);
        return null;
    }
    if (width !== width || height !== height || width < 40 || height < 40) {
        console.log("plotter.js:  WARNING wrong width or height parameters");
        return null;
    }


    //  variables
    var svgns = 'http://www.w3.org/2000/svg';

    var colors = {
        borderCol: "#623",
        bgCol: "#faf9e5",
        axisCol: "#541",
        labelCol: "#255"
    }


    var owner = canvasOwner;
    var svgGraph;


    //  elements sizes
    var isSmall = false;
    var offset = Math.round(height / 100);
    var strokeWidth = Math.round(height / 250);
    if (strokeWidth < 1)strokeWidth = 1;

    // axis lines
    var startX, startY, endX, endY, delta;
    var xAxisPos; // y coordinate of x axis

    // chart
    var hasSetValues = false; // has set numbers value on diagram points
    var chartCol = "#776";
    var chartWidth = strokeWidth;
    var points = [];
    var edge = { min: 0, max: 0}; // edge chart
    var name, values;
    var edges;// hold edges values if it set, otherwise edges compute here

    // text
    var fontSize = Math.round(height / 25);
    var hArrowLabel = "", vArrowLabel = "";


    // prepare on starts
    (function () {
        // set colors if exists in start params
        for (var color in colors) {
            if (startProps && startProps.hasOwnProperty(color)) colors[color] = startProps[color];
        }
        // create svg canvas
        owner.width = width;
        owner.height = height;
        owner.innerHTML =
            "<svg  width='" + width + "' height='" + height +
                "' viewBox='" + "0,0 " + width + " " + height +
                "' xmlns='http://www.w3.org/2000/svg'></svg>";
        height = height - 2 * offset;
        width = width - 2 * offset;
        svgGraph = owner.getElementsByTagName("svg")[0];
        startX = 4 * offset + 1.3*fontSize;
        delta = 3 * offset;
        startY = height - 2 * offset;
        endX = width - 2 * offset;
        endY = 2 * offset;
        if (fontSize < 10) {
            isSmall = true;
            fontSize = 12;
        }
        drawBorder();
    })();

// ============== PUBLIC =============================

    /**
     * call utils methods for compute and draw Chart
     * @param chartName - text on top the part of chart board
     * @param array - values for build the chart must be only numerical values
     * @returns {boolean} - true if success
     */
    function buildGraph(chartName, array, newEdges) {
        edges = newEdges;
        if (!array || !(array instanceof  Array) || array.length < 1) {
            console.log(
                "buildGraph [wrong parameter:second parameter must be array with numerical values]");
            return false;
        }
        if (!(typeof chartName == 'string')) {
            console.log("buildGraph [wrong parameter: first parameter must be string values");
            return false;
        }
        //debug
        //  if(debug)console.log('getArray: ' + array.toString());
        name = chartName;
        values = array;
        // debug
        // if (debug && edges)console.log('edges:' + edges + '  min:' + edges['min']);
        if (edges && edges.hasOwnProperty('min') && edges.hasOwnProperty('max')) {
            //debug
            // console.log("get edges [ min=" + edges['min'] + '  max=' + edges.max + ' ]');
            edge.min = edges['min'];
            edge.max = edges['max'];
        } else {
            edge.min = values[0];
            edge.max = values[0];
            values.forEach(function (val) {
                if (val !== val) {
                    console.log("array for chart contains not numbers:" + val);
                    return false;
                }
                if (edge.min > val) edge.min = val;
                if (edge.max < val) edge.max = val;
            });
            if (edge.min == edge.max) {
                if (edge.min == 0) {
                    edge.max = 10;
                    edge.min = -10;
                } else {
                    edge.max = (edge.max > 0) ? edge.max * 2 : edge.min;
                    edge.min = (edge.min < 0) ? 2 * edge.min : edge.min;
                }
            }
            edges.min = edge.min;
            edges.max = edge.max;
        }
        //debug
        // if(debug)console.log("min=" + edge.min + "  max=" + edge.max);
        var vAxisPar = computeValuesParam();
        var xSpace = (endX - startX - 3 * offset) / (values.length + 1);
        prepareCanvas(values.length, xSpace, vAxisPar);
        addNameToCanvas(name);
        drawChart(values, xSpace, vAxisPar);
        drawToggle();
        return true;
    }

    /**
     * set Given strings on ends x and y axis
     * @param xAxisLabel - text beside x  axis arrow
     * @param yAxisLabel - text beside y axis arrow
     */
    function setAxisLabels(xAxisLabel, yAxisLabel) {
        hArrowLabel = xAxisLabel;
        vArrowLabel = yAxisLabel;
    }

    /**
     * color to draw chart default - "#776"(~ dark gray)
     * @param color - string with colors in accessible HTML form
     */
    function setChartColor(color) {
        chartCol = color;
    }

    /**
     *  has draw beside the points it values
     * @param isVisible (boolean) if true values be show
     */
    function hasSetPointValues(isVisible) {
        hasSetValues = isVisible;
    }


//  ====================  PRIVATE  ===================

    /**
     *
     * draw parts of chart surface(axis, dashes, and etc)
     * note : vAxisPar.startDash count starts with bottom part
     * @param valCount
     * @param xSpace division value x axis
     * @param vAxisPar object contains computed parameters of y axis
     */
    function prepareCanvas(valCount, xSpace, vAxisPar) {
        svgGraph.innerHTML = '';
        drawBorder();
        drawAxisY();
        drawAxisX(vAxisPar.xAxisPos);
        drawYAxisDashes(vAxisPar);
        drawLabels(vAxisPar.xAxisPos);
        addXAxisDashes(valCount, xSpace, vAxisPar.xAxisPos);
    }

    /**
     * set startDash on vAxisPar
     * compute Y coordinate for horizontal axis
     * @param vAxisPar
     * @returns {number}
     **/
    function computeStartPos(vAxisPar) {
        var i = 0;
        if (edge.max < 0) {
            while (edge.max + vAxisPar.vOd * i < 0)i++;
            vAxisPar.startDash = vAxisPar.count + i - 1;
            vAxisPar.xAxisPos = startY - (vAxisPar.count + 1) * vAxisPar.ySpace;
        }
        else if (edge.min > 0) {
            while (edge.min + i * vAxisPar.vOd > 0)i--;
            vAxisPar.startDash = i + 1;
            vAxisPar.xAxisPos = startY;
        }
        else if (edge.max >= 0 && edge.min <= 0) {
            while (edge.min + vAxisPar.vOd * i < 0) i++;
            vAxisPar.startDash = i;
            vAxisPar.xAxisPos = startY - i * vAxisPar.ySpace;
        }
    }

    /**
     * compute numbers parameter for draw horizontal axis
     * @returns {{count, dash count
     *       vOd, values of divide gotten data
     *       startDash, start position (from bottom part)
     *       xAxisPos, y coordinates of horizontal axis
     *       ySpace  division value of vertical axis}}
     */
    function computeValuesParam() {
        // compute division value
        // dash count on vertical axis
        var vAxisPar = {};
        var scale = Math.abs(edge.min - edge.max);
        scale = (scale > 4 && scale < 10.1) ? 11 : scale;
        var pow = 0;
        var res, count = 0;
        var sign = (scale < 10) ? -1 : 1;
        //if(debug)console.log("before: scale=" + scale + ' sign=' + sign + '  10^(sign*pow)=' + Math.pow(10, sign*pow));
        while (true) {
            res = scale / (Math.pow(10, sign * pow));
            if (res < 101 && res > 10)break;
            pow++;
        }
        pow *= sign;
        vAxisPar.pow = pow;
//if (debug)console.log("\nscale=" + scale + " first=" + res);
        res = Math.round(res) + 1;
        if (res <= 31) {
            count = res;
            res = Math.pow(10, pow);
        }
        else if (res <= 61) {
            while (res % 2 != 0)res++;
            count = res / 2;
            res = 2 * Math.pow(10, pow);
        } else if (res < 91) {
            while (res % 3 != 0)res++;
            count = res / 3;
            res = 3 * Math.pow(10, pow);
        } else {
            count = 25;
            res = 4 * Math.pow(10, pow);
        }
        vAxisPar.count = count;
        vAxisPar.vOd = res;
        // for xAxis position
        vAxisPar.ySpace = (startY - endY) / (count + 2);
        computeStartPos(vAxisPar);

//if (debug)console.log("\ndashCount =" + count + " vOd=" + res);
        return vAxisPar;
    }


//  =============  DRAW Functions  ===========================


    function drawBorder() {
        var rect = "<rect name='border' x='" + (offset - 2) + "' y='" + (offset - 2) + "' " + "" +
            "  rx='" + (4 * strokeWidth ) + "' ry='" + (4 * strokeWidth) + "' " +
            " height='" + (height + 4) + "' width='" + (width + 4) + "'" +
            " stroke='" + colors.borderCol + "' stroke-width='" + strokeWidth +
            "' fill='" + colors.bgCol + "' />";
        svgGraph.innerHTML = rect;
    }

    function drawAxisY() {
        // vertical Arrow (values)
        var p = startX + "," + (startY + offset) + " " +
            startX + "," + (endY + delta ) + " " +
            (startX - offset) + "," + (endY + delta ) +
            " " + startX + "," + endY + " " +
            (startX + offset) + "," + (endY + delta ) + " " +
            startX + "," + (endY + delta );
        var vArrow = "<polyline  points='" + p + "' stroke='" + colors.axisCol + "' " +
            " stroke-width='" + strokeWidth + "'  />";
        svgGraph.innerHTML += vArrow;
    }

    function drawLabels(xAxisPos) {
        var yLabel = "<text  x ='" + (startX + 2 * offset) + "' y='" + (endY + delta) + "' " +
            " font-size='" + fontSize + "' text-anchor='start'  fill='" + colors.labelCol + "' >" +
            vArrowLabel + "</text>";
        var shift = (height - xAxisPos > 2 * fontSize) ? 1.5 * fontSize : -0.2 * fontSize;
        var xLabel = "<text " +
            " x='" + (width - delta) + "' y='" + (xAxisPos + shift) + "' " +
            "' text-anchor='end' fill='" + colors.labelCol + "' " +
            " font-size='" + fontSize + "' >" + hArrowLabel + "</text>";
        svgGraph.innerHTML += xLabel + yLabel;
    }


    //  compute and draw vertical params

    function drawYAxisDashes(vAxisPar) {
        if (height < 260) return 0;
        var y , res = "";
        var x2 = startX + fontSize / 3;
        var dashCount = vAxisPar.count + 2;
        var startDash = vAxisPar.startDash;
        if (startDash > dashCount) {
            //  dashCount--;
            startDash = dashCount;
            drawHorizontalDottedLine(Math.round(vAxisPar.xAxisPos + vAxisPar.ySpace / 2));
            drawValueLabels(vAxisPar, (-vAxisPar.startDash + 1) * vAxisPar.vOd, 1);
        } else if (startDash < 0) {
            startDash = 0;
            drawHorizontalDottedLine(Math.round(vAxisPar.xAxisPos - vAxisPar.ySpace / 2));
            drawValueLabels(vAxisPar, (-vAxisPar.startDash + 1) * vAxisPar.vOd, 1,
                vAxisPar.count * values.vOd, vAxisPar.count);
        } else {
            drawValueLabels(vAxisPar, 0, vAxisPar.startDash);
        }
        for (var i = 0; i < dashCount; i++) {
            y = vAxisPar.xAxisPos - (i - startDash) * vAxisPar.ySpace;
            res += "<line x1='" + startX + "' y1='" + y + "' x2='" + x2 + "'  y2='" + y + "' " +
                " stroke-width='" + strokeWidth / 2 + "' stroke='" + colors.axisCol + "'  fill='black' />";
            res += "<line x1='" + startX + "' y1='" + y + "' x2='" + endX + "' y2='" + y + "' " +
                " stroke-width='" + strokeWidth / 5 + "' stroke='#ccc'  fill='#fff' />";
        }
        svgGraph.innerHTML += res;
    }

    function drawHorizontalDottedLine(y) {
        var res;
        var x = delta;
        var shift = 1.4 * delta;
        res = '<path d="';
        while (x < endX) {
            res += 'M' + x + ',' + y + ' L' + (x + shift) + ',' + y + ' ';
            x += 2 * delta;
        }
        res += '" stroke="#ec0" stroke-width="' + 2 * strokeWidth + '" />';
        svgGraph.innerHTML += res;
    }

    function checkPos(val) {
        if (typeof val == 'string' && val.length > 4)return startX + 2.5 * fontSize;
        return startX - 3 * strokeWidth;
    }

    function drawValueLabels(vAxisPar, startValue, startDash) {
        //values
        var secondValue = numberToString(vAxisPar, startValue + vAxisPar.vOd);
        var endValue = numberToString(vAxisPar, (vAxisPar.count - vAxisPar.startDash) * vAxisPar.vOd);
        startValue = numberToString(vAxisPar, startValue);
//        console.log("start=" + startValue + ' hasNumber:' + (typeof startValue) + "\nsecond=" + secondValue + "\nendVal=" + endValue);
        // drawing
        var yStart = startY - vAxisPar.ySpace * startDash + fontSize / 3;
        var yEnd = startY - vAxisPar.ySpace * vAxisPar.count + fontSize / 3;
        var textPar = ' fill="' + colors.labelCol + '" font-size="' + fontSize + '" text-anchor="end" ';
        var lab = '<text x="' + checkPos(startValue) + '" y="' + yStart + '" ' +
            textPar + ' >' + startValue + '</text>' +
            '<text x="' + checkPos(secondValue) + '" y="' + (yStart - vAxisPar.ySpace) + '" ' +
            textPar + ' >' + secondValue + '</text>';
        svgGraph.innerHTML += lab;
        lab = '<text x="' + checkPos(endValue) + '" y="' + yEnd + '" ' +
            textPar + ' >' + endValue + '</text>';
        svgGraph.innerHTML += lab;
    }

    function numberToString(vAxisPar, number) {
        //console.log("get numb=" + number);
        if (number == 0 || Math.abs(number) < 0.0000001)return 0;
        if (Math.abs(number) < 0.1) {
            number = Math.round(number * 1000);
            console.log("rounded number=" + number);
            return number / 100 + "*E-2";
        }
        if (Math.abs(number) > 0.1 && Math.abs(number) < 1000 || number == 0) {
            if (Math.round(number) != number)number = Math.round(number * 100) / 100;
// console.log("return as digit=" + number);
            return number + "";
        }
        number = number / Math.pow(10, vAxisPar.pow) + "";
        number = (number.length < 3) ? number += '.0' : number.substring(0, 2);
        console.log("return=" + number);
        return number + "*E" + vAxisPar.pow;
    }

    function drawAxisX(xAxisPos) {
        var p = (startX - offset) + "," + xAxisPos + " " +
            (endX - delta) + "," + xAxisPos + " " +
            (endX - delta) + "," + (xAxisPos - offset) + " " +
            endX + "," + xAxisPos + " " +
            (endX - delta) + "," + (xAxisPos + offset) + " " +
            (endX - delta) + "," + xAxisPos;
        var hArrow = "<polyline points='" + p + "' stroke='" + colors.axisCol + "' " +
            "' stroke-width='" + strokeWidth + "' />";
        svgGraph.innerHTML += hArrow;
    }

    function addXAxisDashes(count, xSpace, xAxisPos) {
        var res = "";
        var y1 = xAxisPos;
        var y2 = (xAxisPos <= endY) ? xAxisPos + fontSize / 3 : xAxisPos - fontSize / 3;
        var x;
        for (var i = 1; i < count; i++) {
            x = startX + i * xSpace;
            res += "<line x1='" + x + "' y1='" + y1 + "' " +
                " x2='" + x + "' y2='" + y2 + "' " +
                " stroke='" + colors.axisCol + "' stroke-width='" + strokeWidth / 2 + "' />";
        }
        svgGraph.innerHTML += res;
    }

    function addNameToCanvas(name) {
        var name = "<text x='" + width / 2 + "' + y='" + (1.2 * fontSize) + "' " +
            " font-size='" + fontSize + "' text-anchor='middle' fill='" + colors.labelCol + // "' >" +
            "' style='font-style:italic;text-decoration:underline;' ' >" + name + "</text>";
        svgGraph.innerHTML += name;
    }

    function drawToggle() {
//        console.log("Draw Togle");
        var sign = hasSetValues ? "-" : "+";
        var fig = '<text x="' + (endX - fontSize / 2) + '" y="' + (0.9 * fontSize + offset) + '" ' +
            ' font-size="' + (fontSize + 2 * strokeWidth) + '" font-weight="bold" text-anchor="middle" fill="#065" >' + sign + '</text>' +
            '<rect name="toggle" x="' + (endX - fontSize) + '" y="' + offset + '" width="' + fontSize + '"' +
            ' height="' + fontSize + '" fill="white" opacity="0.5" stroke="' + colors.borderCol + '" ' +
            ' stroke-width="' + strokeWidth / 3 + '" />';
        svgGraph.innerHTML += fig;
        for (var i = 0; i < svgGraph.childNodes.length; i++) {
            var elem = svgGraph.childNodes[i]
            if (!(elem instanceof Text) &&
                elem.hasAttribute('name') &&
                elem.getAttribute('name') == 'toggle')elem.onclick = function () {
                hasSetValues = !hasSetValues;
                buildGraph(name, values, edges);
                return 0;
            };
        }
    }


    /**
     * draw lines in accord with gotten data. All given points surrounded
     * by small circle
     * @param values - array contains data to draw
     * @param xSpace  - division values x axis
     * @param vAxisPar - object contains vertical axis parameters
     */
    function drawChart(values, xSpace, vAxisPar) {
        var remainder = " fill='" + colors.bgCol +
            "' stroke='" + chartCol + "' stroke-width='" + chartWidth + "' />";
        var r = strokeWidth * 1.2;
        var shiftY = vAxisPar.ySpace;
        var start = startY - vAxisPar.startDash * shiftY;
        var mult = shiftY / vAxisPar.vOd;
        var x = startX;
        var y1, y2;
        var chart = "<g name='chart'>";
        var i = 1;
        for (var i = 1; i < values.length; i++) {
            if (!isInsideEdges(values[i - 1], values[i])) {
                x += xSpace;
                continue;
            }
            y1 = start - mult * values[i - 1];
            y2 = start - mult * values[i];
            chart += "<line x1='" + x + "' y1='" + y1 +
                "' x2='" + (x + xSpace) + "' y2='" + y2 + "' " + remainder;
            if (hasSetValues) {
                chart += "<circle cx='" + x + "' cy='" + y1 + "' r='" + r + "' " + remainder;
                chart += "<text x='" + x + "' y='" + (y1 - 4 * strokeWidth) + "' text-anchor='middle' " +
                    " fill='black' font-size='" + fontSize + "' >" + (values[i - 1] + "") + "</text>";
            }
            x += xSpace;
        }
        chart += "<circle cx='" + x + "' cy='" + y2 + "' r='" + r + "' " + remainder;
        if (hasSetValues) chart += "<text x='" + x + "' y='" + (y2 - 4 * strokeWidth) + "' text-anchor='middle' " +
            " fill='black' font-size='" + fontSize + "' >" + (values[i - 1] + "") + "</text>";
        chart += "</g>"
        svgGraph.innerHTML += chart;
    }

    /**
     * return true if botch values lie inside edges
     * @param first
     * @param second
     * @returns {boolean}
     */
    function isInsideEdges(first, second) {
        // console.log('check bounds first=' +first + 'second=' + second + '[ ' +edges.min + ' , ' + edges.max + ' ]');
        if (first < edges.min || first > edges.max)return false;
        if (second < edges.min || second > edges.max) return false;
        console.log('return TRUE');
        return true;
    }


//  ===============================================================
// ==============  draw simple estimated diagrams ===================
//  ===============================================================

    var simpleChart;

    /**
     *  values object have structure:
     *  var values = {
     *      max: maxVal, //number
     *      min: minVal, //number
     *      item1 : {
     *          name: Name, //string appear on top side chart
     *          color: color, //string color fol chart line
     *          points: [ { val: val_11, svg: svgPart_1, labelX: label_1},
      *                    { val: val_2, svg: svgPart_2, labelX: label_2},
      *                    ...,
      *                   ]
      *               },
      *       item2 : {
      *             same as before
      *                },
      *       ...
      *       }
     *  }
     *  Notes for property 'item':
     *          1) item:points may be only array with digit values and not object
     *          2) item:points - svg some small svg part for reflect it beside point
     *          3) item:points - labelX - text on x axis bottom point
     *                  svg and label - is optional,
     * @param container - html block element
     * @param values - data to draw
     */
    function drawEstimated(container, values) {
        // curY = y0 + multiplier(max-current)
        // cur y= minuend -multiplier*current;
        if (!values.max || !values.min)return;
        if (container.getElementsByTagName('svg').length > 0)return;
        // create svg canvas
        var contW = container.width - 3 * offset;
        var contH = container.height - 3 * offset;
        var maxY = contH - 5 * offset, y0 = fontSize + 2 * offset;
        var maxX = contW - 4 * offset, x0 = maxX / 6;
        var i, min, max, multiplier, minuend, stepX;
        container.innerHTML = "<svg  width='" + contW + "' height='" + contH +
            "' viewBox='" + "0,0 " + contW + " " + contH +
            "' xmlns='http://www.w3.org/2000/svg'></svg>";
        simpleChart = container.getElementsByTagName("svg")[0];
        simpleChart.setAttribute('style', 'cursor:pointer')
        simpleChart.onclick = function () {
            clearEstimated(container);
            container.setAttribute('class', 'noSize');
        };
        prepareToDraw();
        for (i in values) {
            if (i == 'min' || i == 'max')continue;
            drawChart(values[i]['points']);
        }


        function prepareToDraw() {
            i = 1;
            min = values['min'];
            max = values['max'];
            stepX = 0;
            multiplier = maxY / (max - min);
            minuend = y0 + multiplier * max;
            drawCanvas();
            for (var p in values) {
                if (p == 'min' || p == 'max')continue;
                if (stepX == 0 && values[p]['points'].length > 0) stepX = (maxX - x0) / values[p]['points'].length;
                drawName(values[p]['name'], values[p]['color'], offset + i * fontSize);
                i++;
            }
            if (max > 0 && min < 0)drawNullLine(x0, maxX, minuend, fontSize);
        }

        function drawNullLine(x1, x2, nullY, fontSize) {
            simpleChart.innerHTML += '<text x="' + (x1 - offset) + '" y="' + (nullY + fontSize / 2) + '" ' +
                ' stroke="' + colors.axisCol + '" stroke-width="' + 2 * strokeWidth / 3 + '" ' +
                ' font-size="' + 2 * fontSize / 3 + '" text-anchor="end" >' + 0 + '</text>';
            simpleChart.innerHTML += '<line x1="' + x1 + '" y1="' + nullY + '" x2="' + x2 + '" ' +
                ' y2="' + nullY + '" stroke="' + colors.axisCol + '" stroke-width="' + strokeWidth / 2 + '" />';
        }

        function drawCanvas() {
            simpleChart.innerHTML += '<rect x="10" y="10" rx="' + strokeWidth * 5 + '" ry="' + strokeWidth * 5 +
                '" width="' + (contW - 20) +
                '" height="' + (contH - 20) + '" fill="' + colors.bgCol + '" stroke="' + colors.borderCol +
                '" stroke-width="' + strokeWidth + '" />';
        }

        function drawName(name, color, y) {
//console.log('drawName: ' + name + ' ' + color + ' ' + y);
            simpleChart.innerHTML += '<text x="' + 3 * offset + '" y="' + y + '" fill="' + color + '" ' +
                ' font-size="' + fontSize + '" >' + name + '</text>';
        }

        function drawChart(points) {
// console.log('call drawChart');
            if (points instanceof Array && points.length > 0) {
                if (typeof points[0] == 'number')drawSimpleChart(points);
                else drawComplexChart(points);
            }
        }

        function drawSimpleChart(points) {
            var min = points[0], max = points[0];
            var chart = '<polyline points="';
            for (var j = 0; j < points.length; j++) {
                if (points[j] > max)max = points[j];
                if (points[j] < min) min = points[j];
                chart += (x0 + j * stepX) + ',' + (minuend - multiplier * points[j]) + ' ';
            }
            chart += '" fill="none" stroke="' + values[i]['color'] + '" ' +
                ' stroke-width="' + strokeWidth + '" />';
            max = Math.round(max);
            min = Math.round(min);
//console.log('max=' + max + '  min=' + min);
            drawHorisontalsLines(max, min, minuend - multiplier * max, minuend - multiplier * min, values[i]['color']);
            simpleChart.innerHTML += chart;
        }

        function drawComplexChart(points) {
            var legendFont = Math.round(2 * fontSize / 3);
            var max = points[0]['val'], min = points[0]['val'];
            var chart = '<polyline points="';
            var svg, svgElements = [], dx, dy, curY;
            for (var j = 0; j < points.length; j++) {
                curY = points[j]['val'];
                if (curY > max)max = curY;
                if (curY < min)min = curY;
                dx = x0 + j * stepX;
                dy = (minuend - multiplier * curY);
                chart += dx + ',' + dy + ' ';
                if (points[j]['svg']) {
                    if (dy < 60)dy = 60;
                    if (dy > (maxY - 60))dy = maxY - 60;
                    svg = points[j]['svg'];
                    svg.setAttribute('x', dx - svg.getAttribute('width') / 3);
                    svg.setAttribute('y', dy - svg.getAttribute('height') + 4);
                    svgElements[j] = svg;
                }
                if (points[j]['text']) {
                    var text = (points[j]['text']).split(/\n/);
                    simpleChart.innerHTML += '<line x1="' + dx + '" y1 ="' + y0 + '" ' +
                        ' x2="' + dx + '" y2="' + (maxY - text.length * legendFont) + '" stroke="'
                        + colors.axisCol + '" ' + ' stroke-width="' + strokeWidth / 3 + '" />';
                    if (text.length == 1) {
                        simpleChart.innerHTML += '<text x="' + dx + '" y="' + maxY + '" ' +
                            ' font-size="' + legendFont + '" text-anchor="middle" ' +
                            ' stroke="' + colors.labelCol + '" stroke-width="1" >' +
                            points[j]['text'] + '</text>';
                    } else drawManyStr(dx, text, legendFont);
                }
            }
            chart += '" fill="none" stroke="' + values[i]['color'] + '" ' +
                ' stroke-width="' + strokeWidth + '" />';
            simpleChart.innerHTML += chart;
            max = Math.round(max);
            min = Math.round(min);
            drawHorisontalsLines(max, min, minuend - multiplier * max, minuend - multiplier * min, values[i]['color']);
            for (j in svgElements) {
                simpleChart.appendChild(svgElements[j]);
            }
        }

        function drawHorisontalsLines(maxText, minText, maxVal, minVal, color) {
            var fig = '';
            var stroke = 'stroke="' + color + '" ' +
                ' stroke-width="' + strokeWidth / 2 + '" ';
            var font = 'font-size="' + 2 * fontSize / 3 + '" text-anchor="end" ';
            fig += '<line x1="' + x0 + '" y1="' + minVal +
                '" x2="' + maxX + '" y2="' + minVal + '" ' + stroke + ' />';
            fig += '<text x="' + maxX + '" y="' + (minVal - 2) + '" ' + stroke + font + '>' +
                minText + '</text>';
            fig += '<line x1="' + x0 + '" y1="' + maxVal +
                '" x2="' + maxX + '" y2="' + maxVal + '" ' + stroke + ' />';
            fig += '<text x="' + maxX + '" y="' + (maxVal + fontSize) + '" ' + stroke + font + '>' +
                maxText + '</text>';
            simpleChart.innerHTML += fig;
        }

        function drawManyStr(x, arr, legendFont) {
            var y0 = maxY - (arr.length - 1) * legendFont + 2 * strokeWidth;
            for (var i = 0; i < arr.length; i++) {
                simpleChart.innerHTML += '<text x="' + x + '" y="' + (y0 + i * legendFont) + '" ' +
                    'font-size="' + legendFont + '" text-anchor="middle" stroke="' + colors.labelCol + '" ' +
                    'stroke-width="1" >' + arr[i] + '</text>';
            }
        }

    }

    function clearEstimated(container) {
        if (simpleChart) {
            simpleChart.innerHTML = "";
            container.innerHTML = "";
        }

    }


    return {
        setAxisLabels: setAxisLabels,   // labels beside axis ends(arrow)
        create: buildGraph,  // starts process build and draw chart have 3 parameters - name, array, edges(object)
        hasVisibleValues: hasSetPointValues, // has visible values on points of chart
        chartColor: setChartColor, // color of chart
        simpleChart: drawEstimated,  // draw any count diagrams from gotten data
        clearSimpleChart: clearEstimated
    };

}


