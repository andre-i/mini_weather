/**
 * Created by user on 24.10.17.
 *
 * There may create two form measurer:
 *  1. oval(circle)
 *  2. rect(round rect)
 *
 * Parameters compute on start create measurer might have
 * identified in startParams object :
 *      fontSize ( both )
 *      scaleWidth ( rect )
 * they must set to oval or rect object from startParams.
 *
 *  For replace default parameters you must set they in startParams object.
 *  General form startParams:
 *    var startParams =  {
 *          bounds : {
 *              maxValue: ... ,
 *              minValue: ... ,
 *              scaleDivideCount: ...
 *            },
 *           colors : {
 *              colorParameterName : colorValue ,
 *              .
 *              .
 *            },
 *           rect or oval :{
 *              paramName : paramValue,
 *              .
 *              .
 *              .
 *            }
 *      }
 *   bounds and colors is optional, rect(oval) - mandatory. Any object in startParams
 *   may be empty or have only parameters for override.
 *
 *   note for rect parameters - if width great than height measurer set to
 *   horizontal orientation.
 *
 * Example create rect form device with default parameters:
 *     var rectMeasurer = measurer(htmlElement, { rect:{} });
 * Example create circle form device with default parameters:
 *     var ovalMeasurer = measurer(htmlElement, { oval:{} }); *
 */


/**
 *  create widget to reflect given value in graphical form like physics
 *  measurer( thermometer, voltmeter and etc).
 *
 * @param deviceOwner - HTML container(It may have only one measurer device)
 * @param startParams - object with start parameters. It must have as
 *  minimum one empty object with name "rect" or "oval".
 *
 * @returns {*} object with functions to manipulate measurer widget
 */
function Measurer(deviceOwner, startParams) {
    "use strict";
    var debug = false;  //  true;  //

    var svgns = 'http://www.w3.org/2000/svg';

    // for firefox font size and labels sets individually
    var nav = navigator.userAgent.toLowerCase();
    var isShiftInFirefox = !( nav.search('firefox') < 0);
    if(isShiftInFirefox){
        if(nav.substring(nav.length - 4) > 58.9)isShiftInFirefox = false;

    }
    
//  global color scheme for devices
    var colors = {
        strokeCol: "#744"     /*defStrokeColor*/,
        devBordStartCol: "#eef", /*def inner part DeviceBorderColor */
        devBordStopCol: "#851", /*def outer part DeviceBorderColor */
        fill: "#ffe", /*defFill*/
        textCol: "#332", /* color of text value window on device*/
        brightnessCol: "#fff", /* brightness device surface*/
        devBorderCol: "#530", /* def color for device border */
        labelCol: "#322", /* def color for label on device */
        scaleCol: "red", /* color for fill scale thermometer*/
        scaleBordCol: "#132",
        arrStrokeCol: "#010", /*defStrokeArrow*/
        arrFillCol: "#864" /*defFillArrow*/
    };

// bounds of measure
    var bounds = {
        minValue: 0,
        maxValue: 100,
        scaleDivideCount: 10
    };

//   rect parameter only
// can have an additional fontSize

    var rectParams = {
        // orientation: "vertical", /*orientation scale of device */
        height: Math.round(getHeight()*0.36), /* default height */
        width: Math.round(getHeight()*0.12), /* default width */
        curValue: 0, /* hold current value( this is on start*/
        strokeWidth: 1, /*Math.round(screen.availHeight/400), /* width border */
        hasScaleLabels: true, /* whether device have show value on scale dashes */
        hasLabel: true, /* whether set label on device housing */
        hasEdgeCircle: true, /* whether has on side circle(like thermometer) */
        hasGlass: true, /* whether cover scale glass or plain color */
        hasRoundRect: true, /* whether corners of device rounded */
        label: "" /* label on top device */
    };

// oval parameter only
// can have an additional fontSize and scaleWidth
    var ovalParams = {
        "label": "", /* label on device surface */
        "alpha": 0     /*defValue*/,
        "startAngle": 0     /*defScaleStart*/,
        "stopAngle": 180    /*defScaleStop*/,
        "width": Math.round(getHeight() / 4.1)/*defWidth*/,
        "strokeWidth": 1    /*defStrokeWidth*/,
        "hasRound": true /* form device(true - circle, false - oval) */
    };

    function getHeight(){
        var scrollHeight = Math.max(
            document.body.scrollHeight, document.documentElement.scrollHeight,
            document.body.offsetHeight, document.documentElement.offsetHeight,
            document.body.clientHeight, document.documentElement.clientHeight
        );
        return scrollHeight;
    }

    /* to resize */
    var startFont;
    var random = (Math.random()*new Date().getSeconds() + "").substring(0,6);

    var bigContainer = document.createElement('div');
    var parent = document.createElement('div');
    parent.appendChild(bigContainer);
    parent.setAttribute("class", "bigSize");
    var body = document.getElementsByTagName('body')[0];

     //   insertBefore(table, document.getElementsByTagName('body')[0].childNodes[0]);


//  ==============================================================
//                  oval measurer
//  ==============================================================

    /**
     *  create measurer in form oval(circle)
     *
     * @param par
     *  Object with start parameters( colors, bounds of values and etc)
     *
     * @returns {*}
     */
    function ovalMeasurer(par) {

        /*    set angle value on oval device
         /
         /        90 or -270
         /
         /            =
         /        =        =
         /      =            =
         /     =              =
         /  0 =  <----*        = 180 or -180
         /     =              =
         /      =            =
         /        =        =
         /             =
         /
         /         270 or -90
         /
         /
         */

        function emptySVG() {
            return "<svg  width='" + par.width + "' height='" + par.width +
                "' viewBox='" + "0,0 " + par.width + " " + par.width +
                "' xmlns='http://www.w3.org/2000/svg'></svg>";
        }

        var radius = 40;
        var brightness; // startup brightness color
        var brightnessElem = null,  // SVG circle(brightness) element
            arrowElem = null, // SVG polygon(arrow) element
            textElem = null;    // SVG text element


        // mutable values
        var deviceSVG = undefined;
        var isLarge = false;
        var startWidth = 0; // for resize
        var isShowValueAsText = false; // show value as text on device surface
        var currentValue = 0; // value as number
        var fontSize;


        /**
         * set font size on startup draw device
         * @returns {boolean} false if parameter "fontSize" not correct
         */
        function setFontSize() {
            if (par.hasOwnProperty("fontSize")) {
                var fSize = startParams["fontSize"];
                if (fSize !== fSize || fSize < 8 || fSize > 100) {
                    console.log("ERROR Illegal start parameter fontSize it must be from 8 to 100");
                    return false;
                }
                fontSize = fSize;
            } else {
                fontSize = Math.round(par.width * radius / 340);
                if (fontSize < 8)fontSize = 8;
            }
            if (debug) console.log("ovalDevice on start fontSize=" + fontSize);
            return true;
        }

//  ==============  public function ===================
        /**
         *  if hasVisible true current value be show on device panel
         * @param hasVisible
         */
        function setShowValue(hasVisible) {
            isShowValueAsText = hasVisible;
            setText((hasVisible) ? currentValue: "?");
        }


        /**
         *  angle starts from horizontal line( alpha = -90︒) see comment on start function
         *   given value recompute to angle in degree and a arrow turn according
         */
        function rotateArrow(newValue) {
            if (arrowElem == null)return false;
            if (newValue !== newValue || newValue < par.minValue || newValue > par.maxValue) {
                console.log("WARNING out of bounds leave: (" +
                    par.maxValue + " - " + par.minValue + ")  i get: " + newValue);
                currentValue = "ERR";
                setText(currentValue);
                return false;
            }
            par.alpha = par.stopAngle -
                (par.stopAngle - par.startAngle) * (par.maxValue - newValue) / (par.maxValue - par.minValue);
            var rotate = "rotate(" + par.alpha + " " + par.width / 2 + " " + par.width / 2 + ")";
            arrowElem.setAttribute("transform", rotate);
            currentValue = newValue;
            if (isShowValueAsText)setText(currentValue);
            return true;
        }

        /**
         * set Brightness to device
         * a center part device given color be have
         * @param color
         * @returns {boolean} false if element not found
         */
        function setBrightness(color) {
            if(brightnessElem == null) return false;
            brightnessElem.setAttribute("stop-color", color);
           // brightnessElem.setAttribute("fill", color);
            par.brightnessCol = color;
            return true;
        }


//  ============  private function  ===============

        /**
         *  set current value on window in panel as text
         * @param currentValueInText
         * @returns {boolean} false if  element to set text not found
         */
        function setText(currentValueInText) {
            if(textElem == null) return false;
             textElem.textContent = currentValueInText;
            return true;
        }

        /**
         *  resize device to large size and wise versa
         */
        function flip() {
            if (isLarge) {
                restore();
            } else {
                large();
            }
        }

        /**
         *  set big size of device about browser height
         */
        function large() {
            if (isLarge) return;
            body.appendChild(parent);
          //  bigContainer.setAttribute('style', par.hasRound ? 'padding-top:3%':'padding-top:6%');
            startFont = fontSize;
            startWidth = par.width;
            var h = screen.height - screen.height/8; //document.body.clientHeight;
            var w = document.body.clientWidth;
            par.width = (w > h) ? Math.round(0.85 * h) : Math.round(0.85 * w);
            fontSize = fontSize * par.width / startWidth;
            deviceSVG.innerHTML = emptySVG();
            bigContainer.setAttribute('style','padding-top:' + (h-par.width)/2 + 'px; height: '+ par.width + 'px;');
            drawSVG(bigContainer);
            isLarge = true;
        }

        /**
         *  set device size to normal
         */
        function restore() {
            if (!isLarge) return;
            parent.parentNode.removeChild(parent);
            bigContainer.innerHTML = "";
            par.width = startWidth;
            fontSize = startFont;
            drawSVG(par.owner);
            isLarge = false;
        }


        function elementsAsVariable() {
            var stops=deviceSVG.getElementsByTagNameNS(svgns, 'stop');
            for(var i = 0; i < stops.length; i++)if(stops[i].hasAttribute('name')
                && stops[i].getAttribute('name') == 'brightness') brightnessElem = stops[i];
            brightnessElem.addEventListener("click", flip);;
            var name = '';
            var elem;
            for ( i = 0; i < deviceSVG.childNodes.length; i++) {
                elem = deviceSVG.childNodes[i];

                try{
                if (!(elem instanceof Text) && elem instanceof Node && elem.hasAttribute('name')) {
                    name = elem.getAttribute('name');
                    if (name == 'arrow') arrowElem = elem;
                    if (name == 'text') textElem = elem;
                    if (name == 'flip') elem.addEventListener('click', flip);
                    if (name == 'valuesPane') elem.addEventListener('click', flipText);
                }
                }catch(err){ console.log(err)}// if node not is element - it try exception. Drop it!
                             // who sort SVG elements i do not know
            }
        }

        function flipText(){
            setShowValue(!isShowValueAsText);
        }

        /*
         =======================================================================
         drawing func (create string with SVG content)
         =======================================================================
         */

        /**
         * call on create( resize) device
         * here create svg device tag and call methods for create draw strings
         * contains parts the device. after set given string as innerHTML
         * in svg tag.
         * @param owner - html container to hold device
         */
        function drawSVG(owner) {
            //     console.log(" draw new Device hasRound = " + par.hasRound);
            var height = par.hasRound ? par.width : par.width * 0.9;
            owner.innerHTML =
                "<svg  width='" + par.width + "' height='" + height +
                    "' viewBox='" + "0,0 " + par.width + " " + par.width +
                    "' xmlns='http://www.w3.org/2000/svg'></svg>";
            deviceSVG = owner.getElementsByTagName('svg')[0];
            deviceSVG.innerHTML = "";
            var figure = addGradient() + addDeviceBorder();
            figure += addCircle(radius, undefined, 2*par.strokeWidth, "url(#" + par.gradId + ")"); // scale border
          //  figure += addBrightness();  // brightness
            figure += addDashes(); //  dashes on border
            figure += addValuesOnDashes();
            figure += addLabel();
            figure += addValuesPane();
            figure += addArrow();
            deviceSVG.innerHTML = figure;
            elementsAsVariable();
        }

        /*
         ----------  draw parts of device  ---------------------
         */

        function addGradient() {
            par.gradId = par.brightnessId + "Cgrad";
            var grad = "<defs> <radialGradient id='" + par.gradId + "' cx='50%' y='30%'> ";
            grad += " <stop name='brightness' offset='5%' stop-color='" + par.brightnessCol + "' />" +
                " <stop offset='25%' stop-color='" + par.brightnessCol+ "'  />" +
                " <stop offset='45%' stop-color='" + par.fill + "' />" +
                " <stop offset='93%' stop-color='" + par.fill + "' />" +
                " <stop offset='98%' stop-color='" + par.devBordStartCol + "' />" +
                // " <stop offset='100%' id='" + par.brightnessId + "' stop-color='" + par.brightnessCol + "' />" +
                " </radialGradient> </defs> " +
                "<defs><radialGradient id='" + (par.gradId + "1") + "' cx='50%' y='50%'> ";
            grad += " <stop offset='80%' stop-color='" + par.devBordStartCol + "'/>";
            grad += "  <stop offset='90%' stop-color='" + par.devBordStopCol + "'  />";
            grad += "  <stop offset='93%' stop-color='" + par.devBordStopCol + "'  />";
            grad += " <stop offset='98%' stop-color='" + par.devBordStartCol + "' />";
            grad += " <stop offset='100%' stop-color='" + par.devBordStartCol + "'  />";
            grad += "</radialGradient> </defs>";
            return grad;
        }

        function addDeviceBorder() {
            var bord = "<ellipse name='flip' ";
            var rx = par.width / 2  - par.strokeWidth;
          //  (par.width / 2);
            var ry = par.hasRound ? rx : 0.43 * par.width;
            bord += " cx='" + par.width / 2 + "' cy='" + par.width / 2 + "' " +
                " rx='" + rx + "' ry='" + ry + "'" +
                " stroke='" + par.devBorderCol + "' stroke-width='" + par.strokeWidth*2 + "' " +
                " fill='url(#" + (par.gradId + "1") + ")' /> " +
                "<ellipse cx='" + par.width / 2 + "' cy='" + par.width / 2 + "' " +
                " rx='" + (rx) + "' ry='" + (ry) + "'" +
                " stroke='" + par.devBorderCol + "' stroke-width='" + par.strokeWidth*2 + "' " +
                " fill='none' /> ";
            return bord;

        }

        var circlePar = [ " r='", "' cx='", "' cy='", "', stroke='", "' stroke-width='", "' fill='"  ];

        function addCircle(r_percents, strokeColor, strokeWidth, fill, cx, cy) {
            if (r_percents !== r_percents || r_percents < 1 || r_percents > 50) {
                if (r_percents > 50) {
                    r_percents = 50
                }
                if (r_percents < 1) {
                    r_percents = 1
                }
                console.log("WARNING gigro: \"radius oval`s must be digits in percents (from 1 to 50)\" ");
            }
            var c = "<circle  name='flip' ";
            if (strokeColor == undefined)strokeColor = par.strokeCol;
            if (strokeWidth == undefined) strokeWidth = par.strokeWidth;
            if (fill == undefined)fill = "none";
            if (cx == undefined)cx = par.width / 2;
            if (cy == undefined) cy = par.width / 2;
            var val = [ Math.round(par.width * r_percents / 100) - strokeWidth, cx, cy, strokeColor, strokeWidth, fill];
            for (var i = 0; i < 6; i++) {
                c += circlePar[i] + val[i];
            }
            c += "' /> ";
            return c;
        }

        function addLine(isRotateCenter, x1, y1, x2, y2, rotate, stroke, strokeWidth) {
            if (stroke == undefined)stroke = par.strokeCol;
            if (strokeWidth == undefined)strokeWidth = par.strokeWidth;
            var l = "<line ";
            if (x2 == undefined)x2 = par.width / 2;
            if (y2 == undefined)y2 = par.width / 2;
            l += "x1='" + x1 + "' y1='" + y1 + "' x2='" + x2 + "' y2='" + y2 + "' ";
            l += " stroke='" + stroke + "' stroke-width='" + strokeWidth + "' ";
            if (rotate != undefined && rotate != 0) {
                l += " transform='rotate(" + rotate;
                if (isRotateCenter)l += ", " + (par.width / 2) + ", " + (par.width / 2) + ")' ";
                else l += ")' ";
            }
            l += " /> ";
            return l;
        }

        /* return dashes (short lines) on device scale  */
        function addDashes() {
            var d = "";
            var shift = (par.stopAngle - par.startAngle) / par.scaleDivideCount;
            var x1 = par.width / 2 + 2 * par.strokeWidth - Math.round(radius * par.width / 100);
            var x2 = x1 + Math.round(radius * par.width / 650);
            var y = par.width / 2;
            var dashWidth = (par.width > 400) ? 4*par.strokeWidth : 5*par.strokeWidth/3;
            for (var i = 0; i <= par.scaleDivideCount; i++) {
                d += addLine(true, x1, y, x2, y,
                    par.startAngle + (i * shift), par.strokeCol, dashWidth);
            }
            if(shift > 10){
                x2 = x1 + (x2-x1)/2;
                shift = shift/5;
                dashWidth = dashWidth/2;
                var count = par.scaleDivideCount * 5 + 1;
                for( i= 0; i < count; i++){
                    d += addLine(true, x1, y, x2, y,
                        par.startAngle + (i * shift), par.strokeCol, dashWidth);
                }
            }
            return d;
        }

        function addArrow() {
            var cx = par.width / 2;
            var cy = par.width / 2;
            var shift = (par.width > 400) ? radius / 3: radius/6 ;
            var poly = "<polygon name='arrow' ";
            // points
            poly += " points='" + (cx + 2 * shift) + "," + cy + " " + (cx + 3 * shift) + "," + (cy + shift) + " ";
            poly += (cx - Math.round(radius * par.width / 110)) + "," + cy +
                " " + (cx + 3 * shift) + "," + (cy - shift) + "' ";
            poly += " stroke='" + par.arrStrokeCol + "' stroke-width='" + 2*par.strokeWidth / 3 + "' ";
            // rotate
            poly += " transform='rotate(" + par.alpha + " " + cx + " " + cy + ")' fill='" + par.arrFillCol + "' /> ";
            poly += "<circle r='" + shift/4 + "' cx='" + cx + "' cy='" + cy + "' fill='black' />";
            return poly;
        }

        function addLabel() {
            //      if (debug) console.log("Label Font size=" + fontSize);
            // добавлен параметр startOffset к тегу textPath для сдвига текста иначе
            // в firefox надпись больше и уезжает 
            // в firefox сдвиг - 50% в остальных 1% 
            var  startOffset = isShiftInFirefox ? '50%' : '0%';
            var labelFont = isShiftInFirefox ? fontSize / 1.42 : fontSize /1.3;
            var shift = par.width * radius / 220;
            var center = par.width / 2;
            var y = 4 * par.width / 7;
            var lab = "<defs> <path  id='" + par.id + "labelPath' " +
                " d='M " + (center - shift + fontSize / 6) + ", " + (y + fontSize / 5) + " " +
                "C " + (center - shift + fontSize / 2) + ", " + y / 2 + " " + (center + shift - fontSize / 2) + ", " + y / 2 + " " +
                (center + shift - fontSize / 4) + ", " + y + "' /> </defs>" +
                "<text " + " x='" + (center - shift - fontSize / 6) + "' y='0%' " +
                " font-size='" + labelFont + "' fill='" + par.labelCol + "' " +  //
                " text-anchor='middle' > <textPath xlink:href='#" + par.id + "labelPath'" +
                " startOffset='" + startOffset + "' >" + par.label + "</textPath></text> ";
            return lab;
        }

        function addValuesPane() {
            //      if (debug) console.log("Font size=" + fontSize);
            var curText = (isShowValueAsText) ? currentValue : "?";
            var fSize = fontSize * 1.4;
            var borderWidth = Math.round(fSize / 4);
            var y = Math.round(par.width * (1 / 2 + 3 * radius / 400));  // radius give in percents of width device
            //      if (debug)console.log("try create Text Panel y=" + y);
            var text = "<rect x='" + ( par.width / 2 - 1.1 * fSize ) + "' y='" + (y - 3.2 * borderWidth) + "' " +
                " width='" + 2.2 * fSize + "' height='" + (fSize - 0.2 * borderWidth) + "' " +
                " stroke='" + par.devBorderCol + "' stroke-width='" + (borderWidth / 5) + "' " +
                " fill='" + par.devBordStartCol + "' />";
            text += "<text x='" + par.width / 2 + "' y='" + y + "' " +
                " name='text' text-anchor='middle' opacity='0.8' " +
                " font-size='" + fSize + "' fill='" + par.textCol + "' >" + curText + "</text> ";
            text += "<rect name='valuesPane' x='" + ( par.width / 2 - 1.15 * fSize ) + "' y='" + (y - 3.25 * borderWidth) + "' " +
                " width='" + 2.1 * fSize + "' height='" + (fSize - 0.3 * borderWidth) + "' " +
                " fill='" + par.devBordStartCol + "' opacity='0.1' />";

            return text;
        }

        function addValuesOnDashes() {
            var angleShift = (par.stopAngle - par.startAngle) / par.scaleDivideCount;
            var r = radius * par.width / 150;
            // firefox font size smaler than other
            var fSize = isShiftInFirefox ?  fontSize / 2.4 : fontSize / 2;
            var x, y;
            var res = " ";
            var max = par.scaleDivideCount + 1;
            var large = 0;
            var isAll = (radius * par.width / 100 > 80);
            var hasOdd = par.scaleDivideCount > 14;
            for (var i = 0; i < max; i++) {
                var a = Math.PI / 180 * (par.startAngle + i * angleShift);
                if (i == 0 || i == par.scaleDivideCount)large = fSize / 4;
                if (i == 0 || i == par.scaleDivideCount || isAll) {
                    if (hasOdd && i%2 != 0) continue;
                    x = Math.round(par.width / 2 - r * Math.cos(Math.PI / 180 + a));
                    y = Math.round(par.width / 2 - r * Math.sin(Math.PI / 180 + a));
                    res += "<text name='flip' text-anchor='middle' font-size='" + (fSize + large) + "' ";
                    res += " fill='" + par.textCol + "' x='" + x + "' y='" + y + "' >";
                    res += (par.minValue + i * Math.round((par.maxValue - par.minValue) / par.scaleDivideCount)) + "</text>";
                }
            }
//        console.log("POINTS : \n" + res);
            return res;
        };

        function addBrightness() {
            var fig = "<circle name='brightness' "; // id=";
            fig += "'" + par.brightnessId + "' cx='" + par.width / 2 + "' cy='" + par.width / 2 + "' " +
                //   " stroke='" + par.devBordStartCol + "' stroke-width='" + par.strokeWidth*2 + "' " +
                " r='" + (radius * par.width / 190) + "' fill='" + par.brightnessCol + "' opacity='0.5' />";
            return fig;
        }

        //================ RETURN OBJECT ===================

        var isCreate = false; // it value set to true if might create device
        /**
         * try create oval measurer
         */
        (function () {
            par.owner.innerHTML = "";
            brightness = par.brightnessCol;
            try {
                par.width = Math.round(par.width);
                if (par.width % 2 != 0)par.width++;
                startWidth = par.width;
                isCreate = setFontSize();
                par.id = random;
                par.brightnessId = random + "brigID";
                drawSVG(par.owner);
            } catch (err) {
                console.log("measurer.js - ERROR on start device: " + err.toString());
                console.log("ON START DEVICE I have parameters:");
//                for (var p in  par)  console.log(p + " : " + par[p]);
                isCreate = false;
                throw err;
            }
        })();


        var publicFunc = {
            flip: flip,
            setValue: rotateArrow,
            brightness: setBrightness,
            getBrightness: brightness,
            showValue: setShowValue
        };

        if (isCreate)return publicFunc;
        else return null;
    }


//  ===================================================================
//  =
//  =                rect measurer
//  =
//  ===================================================================

    /*
     //  if (width > height) ==> scale of device set to horizontal
     */
    function rectMeasurer(par) {
        /* you can adjust scale width create parameter "scaleWidth" where value is percent from device width */

        var hasHorizontal = false;
        var fontSize = 0;
        var startBrightness;

        var deviceSVG = null;
        var brightnessElem = null,
            textElem = null,
            scaleElem = null;
        // values computed before draw device
        var scaleW,
            scaleH,
            interval,
            startPos,
            stopPos,
            cx,
            delta;


//  ==========  PUBLIC  ========================

        var currentValue = 0;
        var isShowValue = false;

        /**
         *
         * @param value
         * @returns {boolean}
         */
        function setValue(value) {
            //           if (debug)console.log("linearDev try set Value" + value);
            if(scaleElem == null)return false;
            if (value > par.maxValue || value < par.minValue) {
                console.log("WARNING linearDevice get wrong current value " + value);
                currentValue = "ERR";
                showValueAsText(currentValue);
                return false;
            }
            currentValue = value;
            value = value - par.minValue;
            var height = value * scaleH / Math.abs(par.minValue - par.maxValue);
            var y = "" + (startPos - height);
            scaleElem.setAttribute("y", y);
            scaleElem.setAttribute("height", "" + height);
            if (isShowValue)showValueAsText(currentValue);
            return true
        }

        function setShowValue(hasShowValue) {
            isShowValue = hasShowValue;
            var text = hasShowValue ? currentValue : "?";
            showValueAsText(text);
        }

        function setBrightness(color) {
            if (brightnessElem == null) return false;
            brightnessElem.setAttribute("fill", color);
            par.brightnessCol = color;
            return true;
        }


//  ==================  PRIVATE  ========================

//
// very common op for create device prepare and etc
//
        /**
         * check number param on digits and very common check it values
         * @returns {boolean}
         */
        function checkParams() {
            var res = true;
            if (par.hasOwnProperty("scaleWidth")) {
                var sw = par.scaleWidth;
                if (sw !== sw || Math.abs(sw - 100) > 91)res = false
            }
            if (par.hasOwnProperty("fontSize")) {
                if (fontSize !== fontSize || fontSize < 8 || fontSize > 100)
                    return false;
            }
            if (par.height !== par.height ||
                par.height < 10 ||
                par.width !== par.width ||
                par.width < 10 ||
                par.strokeWidth !== par.strokeWidth ||
                par.strokeWidth < 1 ||
                par.maxValue !== par.maxValue ||
                par.minValue !== par.minValue ||
                par.scaleDivideCount !== par.scaleDivideCount ||
                par.scaleDivideCount < 1) {
                res = false;
            }
            if (!res) {
                console.log("ERROR : wrong start parameter");
                throw new Error("Illegal value og parameter");
            }
            return res;
        }

        function computeSizes(resize) {
            if (par.hasOwnProperty("scaleWidth")) {
                scaleW = Math.round(par.scaleWidth * par.width / 100);
            } else scaleW = Math.round(par.width / 10);
            startPos = Math.round(0.87 * par.height);
            stopPos = Math.round(0.12 * par.height);
            scaleH = startPos - stopPos;
            interval = (startPos - stopPos) / par.scaleDivideCount;
            delta = par.height / 40;
            par.strokeWidth = Math.round(par.height / 200);
            if (delta < 2)delta = 2;
            cx = Math.round(par.width / 2 - delta);
            fontSize = scaleH / par.scaleDivideCount - 4 * par.strokeWidth;
            if (resize)
            //fontSize = 2* scaleH / (2.7 * par.scaleDivideCount);
                if (fontSize < 8) fontSize = 8;
        }

        function showValueAsText(value) {
            if(textElem == null)return false;
              textElem.textContent = "" + value;
            return true;
        }

        /* resize device on click */
        var isLarge = false;

        function flip() {
            if (debug)console.log("CALL flip on linearDevice");
            if (isLarge)restore();
            else large();

        }

        var startHeight;
        var startWidth;

        function large() {
            if (isLarge) return;
            body.appendChild(parent);
            startFont = fontSize;
            startHeight = par.height;
            startWidth = par.width;
            par.height = Math.round(0.85 * screen.height);
            par.width = Math.round(startWidth * par.height / startHeight);
            computeSizes(true);
            bigContainer.setAttribute('style', 'height:' + par.height + 'px;padding-top:' + (par.height*3/34) + 'px;');
            drawSVG(bigContainer);
            setValue(currentValue);
            isLarge = true;
        }

        function restore() {
            if (!isLarge) return;
            parent.parentNode.removeChild(parent);
            bigContainer.innerHTML = "";
            par.height = startHeight;
            par.width = startWidth;
            fontSize = startFont;
            computeSizes(true);
            drawSVG(par.owner);
            setValue(currentValue);
            isLarge = false;
        }

        /**
         *  there appoint elements bu it names, and set click listeners
         */
        function elementsAsVariable() {
            var group = deviceSVG.childNodes[0];
            var name = '';
            var elem;
            for (var i = 0; i < group.childNodes.length; i++) {
                elem = group.childNodes[i];
                try{
                    if (!(elem instanceof Text) && elem.hasAttribute('name')) {
                        name = elem.getAttribute('name');
                        if (name == 'scale') scaleElem = elem;
                        if (name == 'brightness') {
                            elem.addEventListener('click', flip);
                            brightnessElem = elem;
                        }
                        if (name == 'text') textElem = elem;
                        if (name == 'valuesPane') elem.addEventListener('click', flipText);
                        if (name == 'housing') elem.addEventListener('click', flip);
                    }
                }catch(err){}// if node not is element - it try exception. Drop it!
                // who sort SVG elements i do not know
            }
        }

        /** flip visibility on text pane */
        function flipText(){
// console.log("click on text");
            setShowValue(!isShowValue);
        }

        /*
         ==============================================================================
         drawing device function
         ==============================================================================
         */


        function drawSVG(owner) {
            var rotate;
            var canvasWidth,
                canvasHeight;
            if (hasHorizontal) {
                canvasHeight = par.width;
                canvasWidth = par.height;
                rotate = " transform='rotate(90) translate(0, -" + par.height + ")' >";
            } else {
                canvasHeight = par.height;
                canvasWidth = par.width;
                rotate = " >";
            }
            owner.innerHTML =
                "<svg width='" + canvasWidth + "' height='" + canvasHeight +
                    "' viewBox='" + "0,0 " + canvasWidth + " " + canvasHeight +
                    "' xmlns='http://www.w3.org/2000/svg'></svg>";
            deviceSVG =  owner.getElementsByTagName("svg")[0];
            var canvas = "<g id='" + par.id + "group" + "' " + rotate;
            canvas += addGradient();
            canvas += addScaleGradient();
            canvas += addHousing();
            canvas += addBrightness();
            if (par.hasLabel) canvas += addLabel();
            canvas += addScaleBorder();
            if (par.hasEdgeCircle)canvas += addEdgeCircle();
            canvas += addDivisionDashes();
            if (par.hasScaleLabels) canvas += addDashLabels();
            canvas += addScale();
            if (par.hasGlass) canvas += addGlass();
            canvas += addValuesPane();
            deviceSVG.innerHTML = canvas + "</g>";
            elementsAsVariable();
    //        deviceSVG.addEventListener('click', flip);
            return true;
        }

        function addGradient() {
            par.gradId = par.brightnessId + "Rgrad";
            var grad = "<defs> <radialGradient id='" + par.gradId + "' cx='50%' cy='50%' > ";
            grad += " <stop offset='25%' stop-color='" + par.devBordStartCol + "' opacity='0.5' />";
            grad += " <stop offset='75%' stop-color='" + par.devBordStartCol + "'/>";
            grad += "  <stop offset='99%' stop-color='" + par.devBordStopCol + "'  />";
            grad += "</radialGradient>";
            return grad;
        }

        function addScaleGradient() {
            return "<linearGradient " +
                " id='" + (par.id + "scale") + "' x1='0' y1='0' x2='100%' y2='0' >" +
                "<stop offset='0%' stop-color='black' stop-opacity='0.5' />" +
                "<stop offset='37%' stop-color='#fff' stop-opacity='0.2' />" +
                "<stop offset='45%' stop-color='#fff' stop-opacity='0.4' />" +
                "<stop offset='55%' stop-color='#fff' stop-opacity='0.2' />" +
                "<stop offset='100%' stop-color='black' stop-opacity='0.6' />" +
                "</linearGradient></defs> ";
        }

        function addHousing() {
            var yTop = par.strokeWidth, // / 2,
                yBot = par.height - par.strokeWidth; // / 2;
            var cornerRad = "";
            if (par.hasRoundRect) cornerRad = " rx='" + 2*delta + "' ry='" +  2*delta + "' ";
            return "<rect name='housing' x='" + yTop + "' y='" + yTop + "' " +
                " width='" + (par.width - 2*par.strokeWidth) +
                "' height='" + (par.height - 2*par.strokeWidth) + "' " + cornerRad +
                " stroke='" + par.devBorderCol +  "' stroke-width='" + 1.3*par.strokeWidth +
                "' fill='" + par.devBordStartCol + "' />" +
                "<line x1='" + 2*delta + "' y1='" + yTop + "'" +
                " x2='" + (par.width - 2*delta) + "' y2='" + yTop + "'" +
                " stroke='" + par.devBorderCol + "' stroke-width='" + par.strokeWidth + "' />" +
                "<line x1='" + 2*delta + "' y1='" + yBot + "'" +
                " x2='" + (par.width - 2*delta) + "' y2='" + yBot + "'" +
                " stroke='" + par.devBorderCol + "' stroke-width='" + par.strokeWidth + "' />";
        }

        function addBrightness() {
            var x = cx - fontSize;
            var fig = "<rect ";
            fig += " name='brightness' x='" + x + "' y='" + ( stopPos - fontSize/2 ) + "' " +
                " width='" + ( 2 * fontSize) + "' height='" + (scaleH + fontSize) + "' " +
                " rx='" + fontSize/2 + "' ry='" + fontSize/3 + "' opacity='0.4' fill='" + par.brightnessCol + "' />";
            return fig;
        }

        function addLabel() {
            var fSize = scaleH / 10 - delta / 2;
            var transform = " ";
            var x = 1.2 * delta;
            var y = fSize + delta / 2;
            if (hasHorizontal) {
                transform = " transform='rotate(-90 " + y + " " + y +
                    ") translate( -" + scaleH + " , -" + 2 * par.strokeWidth + ")' ";
            }
            var fig = "<text name='housing' " +
                " x='" + x + "' y='" + y + "' fill='" + par.labelCol + "' " +
                transform + " font-size='" + fSize + "' >" +
                par.label + "</text>";
            return fig;
        }

        function addEdgeCircle() {
            var rad = 0.9 * scaleW;
            var cy = startPos + rad; //+ par.strokeWidth/2;
            var fig = "<circle r='" + rad + "' cx='" + cx + "' cy='" + cy + "' " +
                " stroke='" + par.scaleBordCol + "' stroke-width='" + par.strokeWidth + "' " +
                " fill='" + par.scaleCol + "' />";
            if (par.hasGlass) fig += "<circle r='" + (rad - par.strokeWidth / 2) + "' cx='" + cx + "' cy='" + cy + "' " +
                "  fill='url(#" + (par.id + "scale") + ")' />";
            fig += "<rect  x='" + (cx - scaleW / 2 + par.strokeWidth / 2) +
                "' y='" + (startPos ) + "' " +
                " width='" + (scaleW - par.strokeWidth) + "' height='" + 1.3 * par.strokeWidth + "' " +
                " fill='" + par.scaleCol + "' /> ";

            return fig;
        }

        function addScaleBorder() {
            var fig = "<rect ";
            fig += " x='" + (cx - scaleW / 2) + "' y='" + (stopPos - par.strokeWidth / 2) + "' " +
                " width='" + scaleW + "' height='" + (scaleH + par.strokeWidth) + "' " +
                " stroke='" + par.scaleBordCol + "' stroke-width='" + par.strokeWidth + "' " +
                " fill='none' /> ";
            return fig;
        }

        function addDivisionDashes() {
            var x1 = cx + scaleW / 2 + par.strokeWidth;
            //var x2 = cx + 2.4 * scaleW;
            var x2 = cx + scaleW / 2 + par.width / 7;
            var fig = "";
            var dashCount = 2 * par.scaleDivideCount + 1;
            var space = interval / 2;
            var y;
            for (var i = 0; i < dashCount; i++) {
                y = stopPos + i * space;
                fig += " <line" + " x1='" + x1 + "' y1='" + y + "'" +
                    "' x2='" + (x2 - i % 2 * fontSize / 3 ) + "' y2='" + y + "'" + // - (i % 2 ? 0 : fontSize / 5)
                    " stroke='" + par.strokeCol + "' stroke-width='" + par.strokeWidth + "' /> "
            }
            return fig;
        }

        function addDashLabels() {
            var y = 0;
            var x = cx + scaleW / 2 + par.width / 7 + fontSize * 0.8;
            if (fontSize < 10)return addOnlyEdge(x);
            var startLabPos = startPos + par.strokeWidth / 2;
            var shiftVal = (par.maxValue - par.minValue) / par.scaleDivideCount;
            var font = "' fill='" + par.textCol + "' font-style='plain' " +
                " text-anchor='middle'";
            var fig = "";
            for (var i = 0; i < par.scaleDivideCount + 1; i++) {
                var res = Math.round((par.minValue + shiftVal * i) * 10) / 10;
                y = (startLabPos - i * interval );
                fig += "<text name='housing' " + " x='" + x + "' y='" + y + "' " + font;
                if (hasHorizontal)fig += " transform='rotate(-90 " + x + " " + y + ")' ";
                if (res == 0) {
                    fig += " font-weight='bold' font-size='" + (fontSize + 2 * par.strokeWidth) + "' >"
                } else {
                    fig += " font-size='" + fontSize + "' >";
                }
                fig += res + "</text>";
            }
            return fig;
        }

        function addOnlyEdge(x) {
            return "<text x='" + x + "' y='" + (startPos + 7) + "'" +
                " font-size='14' fill='" + par.strokeCol + "' >" +
                par.minValue + "</text> " +
                "<text x='" + x + "' y='" + (stopPos + 4) + "'" +
                " font-size='14' fill='" + par.strokeCol + "' >" +
                par.maxValue + "</text> ";

        }

        function addGlass() {
            var x = cx - scaleW / 2;
            var bottomEdge;
            if (par.hasRound)bottomEdge = scaleH + 2.3 * par.strokeWidth;
            else bottomEdge = scaleH + 1.5 * par.strokeWidth;
            return "<rect" +
                " x='" + x + "' y='" + (stopPos ) + "'" +
                " width='" + (scaleW) + "' height='" + bottomEdge + "'" +
                " fill='url(#" + par.id + "scale" + ")' />";
        }

        function addScale() {
            return "<rect name='scale' " +
                " x='" + (cx - scaleW / 2 + par.strokeWidth / 2) + "' y='" + (startPos - par.strokeWidth) + "' " +
                " width='" + (scaleW - par.strokeWidth) + "' height='" + 0 + "' fill='" + par.scaleCol + "' />";
        }

        function addValuesPane() {
            var fSize = (scaleH / 10 - delta / 2);
            var y = par.height - fSize - delta / 2 - 2* par.strokeWidth;
            var pane = "<rect x='" + (3 * par.width / 4 - 1.2 * fSize) + "' y='" + y + "' " +
                " width='" + 2.15 * fSize + "' height='" + fSize + "' " +
                " fill='" + par.devBordStartCol + "' " + " stroke='" + par.strokeCol + "' " +
                " stroke-width='" + par.strokeWidth + "' opacity='1' />" +
                "<text  name='text'" +
                "' x='" + (3 * par.width / 4 - par.strokeWidth) + "' y='" + (par.height - delta - par.strokeWidth) + "'" +
                " text-anchor='middle' fill='" + par.textCol + "' font-size='" + fSize + "' >?</text>";
            pane += "<rect name='valuesPane'" +
                " x='" + (3 * par.width / 4 - 1.1 * fSize) + "' y='" + (y + par.strokeWidth) + "' " +
                " width='" + 2 * fSize + "' height='" + (fSize - 2*par.strokeWidth) + "' " +
                " fill='" + par.devBordStartCol + "' opacity='0.1' />";
            return pane;
        }

        /*  ==================== return function =============================  */

        var isWrongParam = false;

        (function () {
            startBrightness = par.brightnessCol;
            var cur;
            if (par.width % 5 != 0) {
                cur = Math.round(par.width);
                while (cur % 6 != 0)cur++;
                par.width = cur;
            }
            if (checkParams()) {
                if (par.height <= par.width) {
                    var buf = par.width;
                    par.width = par.height;
                    par.height = buf;
                    hasHorizontal = true;
                }
                computeSizes(false);
                drawSVG(par.owner);
                isWrongParam = false;
            } else {
                isWrongParam = true;
                throw new Error("init param wrong");
            }
        })();

        return  (isWrongParam) ? null :
        {
            setValue: setValue,
            brightness: setBrightness,
            getBrightness: startBrightness,
            showValue: setShowValue
        };

    }

    /*
     *
     *  set parameter for measurer if exists , create and return measurer
     *
     */
    function setBounds() {
        var b;
        if (startParams.hasOwnProperty("bounds"))b = startParams.bounds;
        for (var p in bounds) {
            if (b && b.hasOwnProperty(p))params[p] = b[p];
            else params[p] = bounds[p];
        }
    }


    function setColors() {
        var nc;
        if (startParams.hasOwnProperty("colors"))nc = startParams.colors;
        for (var col in colors) {
            if (nc && nc.hasOwnProperty(col))params[col] = nc[col];
            else params[col] = colors[col];
        }
    }


    function setRectParams() {
        var nc = startParams.rect;
        for (var r in rectParams) {
            if (nc && nc.hasOwnProperty(r))params[r] = nc[r];
            else params[r] = rectParams[r];
        }
        if (nc && nc.hasOwnProperty("fontSize"))rectParams.fontSize = nc["fontSize"];
        if (nc && nc.hasOwnProperty("scaleWidth"))rectParams.scaleWidth = nc["scaleWidth"];
    }

    function setOvalParams() {
        var nc = startParams.oval;
        for (var r in ovalParams) {
            if (nc.hasOwnProperty(r))params[r] = nc[r];
            else params[r] = ovalParams[r];
        }
        if (nc.hasOwnProperty("fontSize"))ovalParams.fontSize = nc["fontSize"];
        if (nc.hasOwnProperty("hasRound") && nc.hasRound == false) {
            params.width = Math.round(params.width + params.width / 10);
        }
    }

    var retDev;
    var params = {};
    if (startParams) {
        if (deviceOwner == undefined || !(deviceOwner instanceof HTMLElement)) {
            console.log(" new created device has no owner container(tag) i get - " + deviceOwner);
            retDev = null;
            return;
        }
        retDev = null;
        setBounds();
        setColors();
        params.owner = deviceOwner;
        if (startParams.hasOwnProperty("rect")) {
            setRectParams();
            retDev = rectMeasurer(params);
        }
        if (startParams.hasOwnProperty("oval")) {
            setOvalParams();
            retDev = ovalMeasurer(params);
        }
    }
    if (debug)console.log("retDev=" + retDev);
    return retDev;
}

