import QtQuick
//2.15
import QtQuick.Window
//2.15
import QtQuick.Controls
//2.15
import QtQuick.Layouts
//1.15
import QtQuick.Dialogs

import QtPositioning
import QtLocation

Window {
    property real xToYScaling: (_visualization.maxLon - _visualization.minLon)
                               / (_visualization.maxLat - _visualization.minLat)
    property int zoomLv: 16
    property bool autoCenter: true
    property real mapCenterX: 0
    property real mapCenterY: 0
    property var arrayObjects: []

    Connections {
        target: _visualization
        function onConnectData() {

            if (autoCenter) {
                mapCenterX = (_visualization.maxLat + _visualization.minLat) / 2
                mapCenterY = (_visualization.maxLon + _visualization.minLon) / 2

                map.center = QtPositioning.coordinate(mapCenterX, mapCenterY)
            }
            image.requestPaint()
        }
        function onDataUpdated() {
            onConnectData()
        }
    }

    width: 624
    height: 624
    visible: true

    Component {
        id: comPortButton

        Button {
            onClicked: {
                _visualization.readingComPort(text)
            }
        }
    }

    function addButton(i) {
        var button = comPortButton.createObject(comPortSelection)
        button.text = _visualization.portsNameInfo[i]
        arrayObjects.push(button)
    }

    function clearButtonArray() {
        for (var i = arrayObjects.length; i > 0; i--) {
            arrayObjects[i - 1].destroy()
        }
        arrayObjects = []
    }

    // Qt6:
    FileDialog {
        id: inputDialog
        fileMode: FileDialog.OpenFile
        onAccepted: _visualization.selectFile(selectedFile.toString().replace(
                                                  "file:///", ""))
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin

        center: QtPositioning.coordinate(mapCenterX, mapCenterY)
        zoomLevel: zoomLv
        // onCenterChanged: console.log(center)
        Plugin {
            id: mapPlugin
            name: "osm"
            PluginParameter {
                name: "osm.mapping.custom.host"
                value: "https://tile.openstreetmap.org/"
            }
            PluginParameter {
                name: "osm.mapping.copyright"
                value: "OSM"
            }
            PluginParameter {
                name: "osm.mapping.cache.directory"
                value: "offlineMap/1"
            }
            PluginParameter {
                name: "osm.mapping.providersrepository.disabled"
                value: true
            }
        }
        PluginParameter {
            name: "osm.mapping.custom.host"
            value: "https://tile.thunderforest.com/cycle/%z/%x/%y.png?apikey=8ccf6a1e14a148378f45aa7cceb15ef3"
        }
        onSupportedMapTypesChanged: {
            map.activeMapType = map.supportedMapTypes[map.supportedMapTypes.length - 1]
            /*1, 4, 5 - ok*/
        }
        MapPolyline {
            id: mapLine
            line.width: 3
            line.color: 'black'
        }
        MouseArea {
            id: mouse
            anchors.fill: parent

            acceptedButtons: Qt.LeftButton

            property int lastX: -1
            property int lastY: -1

            onPressed: {
                lastX = mouseX
                lastY = mouseY
            }

            onPositionChanged: {
                image.requestPaint()
                map.pan(lastX - mouseX, lastY - mouseY)
                lastX = mouseX
                lastY = mouseY
            }
        }

        MapQuickItem {
            id: filePath
            anchorPoint.x: image.width / 2
            anchorPoint.y: image.height / 2
            coordinate: map.center

            sourceItem: Canvas {
                id: image
                width: map.width
                height: map.height
                contextType: "2d"
                onPaint: {
                    context.reset()
                    xToYScaling = (_visualization.maxLon - _visualization.minLon)
                            / (_visualization.maxLat - _visualization.minLat)

                    context.strokeStyle = "black"
                    context.lineWidth = 1
                    context.save() // Необходимо, чтобы толщина линиии масштабируемого графика осталось прежней

                    /*//////////////////////////////ЛИНИЯ ПУТИ//////////////////////////////////*/
                    if (_visualization.iterationInput.length > 0) {
                        context.beginPath()
                        var startingPoint = map.fromCoordinate(
                                    QtPositioning.coordinate(
                                        _visualization.iterationInput[1]["latitude"],
                                        _visualization.iterationInput[1]["longitude"]))

                        context.moveTo(startingPoint.x, startingPoint.y)

                        // Отрисовка графика
                        for (var i = 1; i < _visualization.iterationInput.length; i++) {
                            var tempLineCoords = map.fromCoordinate(
                                        QtPositioning.coordinate(
                                            _visualization.iterationInput[i]["latitude"],
                                            _visualization.iterationInput[i]["longitude"]))

                            context.lineTo(tempLineCoords.x, tempLineCoords.y)
                        }
                        context.lineWidth = 0
                        context.restore()
                        context.stroke()

                        /*//////////////////////////////////////////////////////////////////////////*/
                        for (var i = 0; i < _visualization.iterationInput.length; i++) {
                            var tempCoords = map.fromCoordinate(
                                        QtPositioning.coordinate(
                                            _visualization.iterationInput[i]["latitude"],
                                            _visualization.iterationInput[i]["longitude"]))

                            if (_visualization.iterationInput[i]["spike"]) {
                                context.beginPath()

                                var drawCircles = false
                                if (_visualization.iterationInput[i]["centralFreq"] >= 5300) {
                                    context.strokeStyle = "red"
                                    drawCircles = band4.checked
                                } else if (_visualization.iterationInput[i]["centralFreq"]
                                           >= 2300) {
                                    context.strokeStyle = "yellow"
                                    drawCircles = band3.checked
                                } else if (_visualization.iterationInput[i]["centralFreq"] >= 900) {
                                    context.strokeStyle = "green"
                                    drawCircles = band2.checked
                                } else if (_visualization.iterationInput[i]["centralFreq"] >= 300) {
                                    context.strokeStyle = "blue"
                                    drawCircles = band1.checked
                                } else {
                                    context.strokeStyle = "black"
                                }
                                context.save()
                                if (drawCircles) {
                                    var modSize = (map.zoomLevel)
                                            * ((_visualization.iterationInput[i]["level"]
                                                - _visualization.minLevel)
                                               / (_visualization.maxLevel
                                                  - _visualization.minLevel)) + 1

                                    context.arc(tempCoords.x, tempCoords.y,
                                                modSize, 0, 2 * Math.PI)
                                }
                                context.restore(
                                            ) //Необходимо, чтобы толщина линиии масштабируемого графика осталось прежней
                                context.stroke()
                            }
                        }
                    }
                }
            }
        }
    }

    RowLayout {

        Button {
            id: rangeView
            text: "FRQ view"
            onClicked: freqShow.open()
        }
        Popup {
            id: freqShow

            y: 30

            width: 600
            height: 150
            padding: 0
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

            ColumnLayout {
                anchors.centerIn: parent

                Layout.preferredWidth: parent.width
                Layout.preferredHeight: parent.height

                RowLayout {
                    TextField {
                        text: "Band 1"
                        readOnly: true
                        horizontalAlignment: TextInput.AlignHCenter
                    }
                    CheckBox {
                        id: band1
                        checked: true
                        onCheckedChanged: {
                            image.requestPaint()
                        }
                    }
                    TextField {
                        id: bandAverage1
                        text: "450"
                        readOnly: true
                        horizontalAlignment: TextInput.AlignHCenter
                    }
                    TextField {
                        id: minBand1
                        text: "-110"
                        readOnly: false
                        horizontalAlignment: TextInput.AlignHCenter
                        onEditingFinished: {
                            image.requestPaint()
                        }
                    }
                }
                RowLayout {
                    TextField {
                        text: "Band 2"
                        readOnly: true
                        horizontalAlignment: TextInput.AlignHCenter
                    }
                    CheckBox {
                        id: band2
                        checked: true
                        onCheckedChanged: {
                            image.requestPaint()
                        }
                    }
                    TextField {
                        id: bandAverage2
                        text: "1050"
                        readOnly: true
                        horizontalAlignment: TextInput.AlignHCenter
                    }
                    TextField {
                        id: minBand2
                        text: "-110"
                        readOnly: false
                        horizontalAlignment: TextInput.AlignHCenter
                        onEditingFinished: {
                            image.requestPaint()
                        }
                    }
                }
                RowLayout {
                    TextField {
                        text: "Band 3"
                        readOnly: true
                        horizontalAlignment: TextInput.AlignHCenter
                    }
                    CheckBox {
                        id: band3
                        checked: true
                        onCheckedChanged: {
                            image.requestPaint()
                        }
                    }
                    TextField {
                        id: bandAverage3
                        text: "2450"
                        readOnly: true
                        horizontalAlignment: TextInput.AlignHCenter
                    }
                    TextField {
                        id: minBand3
                        text: "-110"
                        readOnly: false
                        horizontalAlignment: TextInput.AlignHCenter
                        onEditingFinished: {
                            image.requestPaint()
                        }
                    }
                }
                RowLayout {
                    TextField {
                        text: "Band 4"
                        readOnly: true
                        horizontalAlignment: TextInput.AlignHCenter
                    }
                    CheckBox {
                        id: band4
                        checked: true
                        onCheckedChanged: {
                            image.requestPaint()
                        }
                    }
                    TextField {
                        id: bandAverage4
                        text: "5450"
                        readOnly: true
                        horizontalAlignment: TextInput.AlignHCenter
                    }
                    TextField {
                        id: minBand4
                        text: "-110"
                        readOnly: false
                        horizontalAlignment: TextInput.AlignHCenter
                        onEditingFinished: {
                            image.requestPaint()
                        }
                    }
                }
            }
        }

        Button {
            id: fileSelect
            text: "Select file"
            onClicked: inputDialog.open()
        }
        Button {
            id: centerButton
            text: "Auto-center on/off"
            onClicked: {
                autoCenter = !autoCenter
            }
        }
        Button {
            id: readingCOMPort
            text: "Read from Port"
            onClicked: {
                clearButtonArray()
                _visualization.countComPorts()
                for (var i = 0; i < _visualization.portsNameInfo.length; i++) {
                    addButton(i)
                }
                selectComPort.open()
            }
            // onClicked: _visualization.readingComPort()
        }
        Popup {
            id: selectComPort

            y: 30
            x: 230

            width: 50
            height: 150
            padding: 0
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

            ColumnLayout {
                id: comPortSelection
                anchors.centerIn: parent

                Layout.preferredWidth: parent.width
                Layout.preferredHeight: parent.height
            }
            onClosed: {
                clearButtonArray()
            }
        }
        Button {
            text: " + "
            onClicked: {
                zoomLv += 1
                image.requestPaint()
            }
        }
        Button {
            text: " - "
            onClicked: {
                zoomLv -= 1
                image.requestPaint()
            }
        }
    }

    // title: qsTr("Visualization")
}
