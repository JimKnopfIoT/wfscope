import QtQuick 2.6
import Sailfish.Silica 1.0
import wfscope 1.0

Page
{
    id: page
    allowedOrientations: Orientation.All

    readonly property QtObject source: hub.current
    readonly property string kind: source ? source.kind : ""
    readonly property bool panadapterMode: kind === "sdrplay" && source.panadapter
    property var bands: source ? source.bands : []
    property int bandIndex: 0
    property bool firstBand: true

    function selectBand(i) {
        if (!bands || bands.length === 0)
            return
        i = Math.max(0, Math.min(bands.length - 1, i))
        bandIndex = i
        source.centerMHz = bands[i].center
        source.spanMHz = bands[i].span
    }

    function parseMHz(t) {
        return parseFloat(String(t).replace(",", "."))
    }

    onSourceChanged: {
        selectBand(firstBand ? initialBand : 0)
        firstBand = false
    }
    Component.onCompleted: {
        selectBand(initialBand)
        firstBand = false
    }

    Connections {
        target: page.source
        onSweep: scope.pushSweep(bins, centerMHz, spanMHz)
    }

    SilicaFlickable
    {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            Repeater {
                model: hub.names
                MenuItem {
                    text: (index === hub.currentIndex ? "● " : "") + modelData
                    onClicked: hub.select(index)
                }
            }
            Repeater {
                model: page.panadapterMode ? [] : page.bands
                MenuItem {
                    text: (index === page.bandIndex ? "◆ " : "") + modelData.name
                    onClicked: page.selectBand(index)
                }
            }
            MenuItem {
                text: source.running ? "Stopp" : "Start"
                onClicked: source.running ? source.stop() : source.start()
            }
        }

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: source.name; description: source.status }

            SpectrumView {
                id: scope
                width: parent.width
                height: Math.round(page.height * 0.72)
                autoRange: autoSwitch.checked
            }

            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                Label {
                    width: parent.width / 2
                    text: "Peak\n" + (scope.peakHoldMHz < 1000 ? scope.peakHoldMHz.toFixed(3)
                                                                 : scope.peakHoldMHz.toFixed(1)) + " MHz"
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeSmall
                }
                Label {
                    width: parent.width / 2
                    horizontalAlignment: Text.AlignRight
                    text: scope.peakHoldDbm.toFixed(0) + " dB"
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                }
            }

            ComboBox {
                width: parent.width
                label: "Quelle"
                currentIndex: hub.currentIndex
                menu: ContextMenu {
                    Repeater {
                        model: hub.names
                        MenuItem { text: modelData }
                    }
                }
                onCurrentIndexChanged: if (currentIndex !== hub.currentIndex) hub.select(currentIndex)
            }

            ComboBox {
                width: parent.width
                visible: !page.panadapterMode
                label: "Band"
                currentIndex: page.bandIndex
                menu: ContextMenu {
                    Repeater {
                        model: page.bands
                        MenuItem { text: modelData.name }
                    }
                }
                onCurrentIndexChanged: if (currentIndex !== page.bandIndex) page.selectBand(currentIndex)
            }

            // ---- HackRF ----
            Column {
                width: parent.width
                visible: page.kind === "hackrf"
                SectionHeader { text: "HackRF" }
                Slider {
                    width: parent.width
                    minimumValue: 0; maximumValue: 40; stepSize: 8
                    value: page.kind === "hackrf" ? source.lnaGain : 16
                    label: "LNA (ZF-Verstärkung)"
                    valueText: value.toFixed(0) + " dB"
                    onReleased: if (page.kind === "hackrf") source.lnaGain = value
                }
                Slider {
                    width: parent.width
                    minimumValue: 0; maximumValue: 62; stepSize: 2
                    value: page.kind === "hackrf" ? source.vgaGain : 20
                    label: "VGA (Basisband-Verstärkung)"
                    valueText: value.toFixed(0) + " dB"
                    onReleased: if (page.kind === "hackrf") source.vgaGain = value
                }
                TextSwitch {
                    text: "HF-Vorverstärker (+14 dB)"
                    description: "Nur für schwache Signale; übersteuert leicht"
                    checked: page.kind === "hackrf" && source.ampEnable
                    onClicked: if (page.kind === "hackrf") source.ampEnable = checked
                }
            }

            // ---- SDRplay RSPdx ----
            Column {
                width: parent.width
                visible: page.kind === "sdrplay"
                SectionHeader { text: "RSPdx" }
                TextSwitch {
                    text: "Panadapter FT-991A"
                    description: "Tuner fest auf der ZF (TSP IFace 2), Achse in VFO-Frequenz"
                    checked: page.kind === "sdrplay" && source.panadapter
                    onClicked: if (page.kind === "sdrplay") source.panadapter = checked
                }
                TextField {
                    width: parent.width
                    visible: page.panadapterMode
                    label: "VFO-Frequenz (MHz)"
                    placeholderText: label
                    text: page.kind === "sdrplay" ? source.vfoMHz.toFixed(4) : ""
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    EnterKey.onClicked: { source.vfoMHz = page.parseMHz(text); focus = false }
                }
                TextField {
                    width: parent.width
                    visible: page.panadapterMode
                    label: "Erste ZF des Funkgeräts (MHz)"
                    placeholderText: label
                    text: page.kind === "sdrplay" ? source.ifMHz.toFixed(3) : ""
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    EnterKey.onClicked: { source.ifMHz = page.parseMHz(text); focus = false }
                }
                Slider {
                    width: parent.width
                    visible: page.panadapterMode
                    minimumValue: -25; maximumValue: 25; stepSize: 0.1
                    value: page.kind === "sdrplay" ? source.offsetKHz : 0
                    label: "ZF-Feinkorrektur"
                    valueText: (value >= 0 ? "+" : "") + value.toFixed(1) + " kHz"
                    onReleased: if (page.kind === "sdrplay") source.offsetKHz = value
                }
                TextSwitch {
                    visible: page.panadapterMode
                    text: "Spektrum spiegeln"
                    description: "Falls Signale beim Drehen am VFO in die falsche Richtung wandern"
                    checked: page.kind === "sdrplay" && source.mirror
                    onClicked: if (page.kind === "sdrplay") source.mirror = checked
                }
                ComboBox {
                    width: parent.width
                    label: "Antenneneingang"
                    currentIndex: page.kind === "sdrplay" ? source.antenna : 0
                    menu: ContextMenu {
                        MenuItem { text: "A" }
                        MenuItem { text: "B" }
                        MenuItem { text: "C" }
                    }
                    onCurrentIndexChanged: if (page.kind === "sdrplay") source.antenna = currentIndex
                }
                Slider {
                    width: parent.width
                    minimumValue: 0; maximumValue: 27; stepSize: 1
                    value: page.kind === "sdrplay" ? source.lnaState : 4
                    label: "LNA-Stufe (0 = max. Verstärkung)"
                    valueText: value.toFixed(0)
                    onReleased: if (page.kind === "sdrplay") source.lnaState = value
                }
                TextSwitch {
                    text: "AGC"
                    checked: page.kind === "sdrplay" && source.agc
                    onClicked: if (page.kind === "sdrplay") source.agc = checked
                }
            }

            SectionHeader { text: "Anzeige" }

            TextSwitch {
                id: autoSwitch
                text: "Auto-Kontrast"
                description: "Farbskala folgt dem Rauschboden"
                checked: true
                onCheckedChanged: if (!checked) { scope.floorDbm = floorSlider.value; scope.ceilDbm = floorSlider.value + 40 }
            }

            Slider {
                id: floorSlider
                enabled: !autoSwitch.checked
                opacity: enabled ? 1.0 : 0.4
                onValueChanged: if (!autoSwitch.checked) { scope.floorDbm = value; scope.ceilDbm = value + 40 }
                width: parent.width
                minimumValue: -140
                maximumValue: -20
                value: -100
                stepSize: 1
                label: "Rauschflur"
                valueText: value.toFixed(0) + " dB"
            }
        }
    }
}
