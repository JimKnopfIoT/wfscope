import QtQuick 2.6
import Sailfish.Silica 1.0
import wfscope 1.0
import "../components"

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

    // kleinste sinnvolle Spanne je Quelle (MHz)
    readonly property real minSpanMHz: kind === "icap" ? 20 : kind === "hackrf" ? 5
                                       : kind === "sdrplay" ? 0.05 : 1

    // Mitte/Spanne setzen. Mitte nur auf den Bereich der Quelle begrenzt — NICHT von der
    // Spanne abhängig (Fenster darf über die Bandgrenze hinausragen, dort bleibt es leer)
    function setPlan(c, s) {
        if (!source) return
        var lo = source.minMHz, hi = source.maxMHz
        s = Math.max(minSpanMHz, Math.min(hi - lo, s))
        c = Math.max(lo, Math.min(hi, c))
        source.centerMHz = c
        source.spanMHz = s
        bandIndex = -1
        for (var i = 0; i < bands.length; ++i)
            if (Math.abs(bands[i].center - c) < 1e-6 && Math.abs(bands[i].span - s) < 1e-6)
                bandIndex = i
    }

    // Abgestimmte Frequenz: Panadapter = VFO, UKW intern = Hörfrequenz, sonst Ansichtsmitte
    readonly property real tunedMHz: !source ? 0
                                     : panadapterMode ? source.vfoMHz
                                     : kind === "fm" ? source.listenMHz : source.centerMHz

    // Auf eine Frequenz abstimmen; begrenzt nur auf den Bereich der Quelle
    function tune(mhz) {
        if (!source || isNaN(mhz)) return
        if (panadapterMode) {
            source.vfoMHz = Math.max(0.001, Math.min(470, mhz))
            return
        }
        mhz = Math.max(source.minMHz, Math.min(source.maxMHz, mhz))
        // Abgestimmte Frequenz liegt immer unter dem roten Mittelstrich
        if (kind === "fm")
            source.listenMHz = mhz
        setPlan(mhz, source.spanMHz)
    }

    // Auf das Kanalraster einrasten
    function snap(mhz) {
        var r = rasterKHz / 1000
        return Math.round(mhz / r) * r
    }

    // Magnetisch: auf die stärkste Spitze im Umkreis einrasten, sonst aufs Kanalraster.
    // radiusPx: Fangbereich in Bildpunkten (Tippen: halbe Fingerbreite, Schieben: enger)
    function magnet(mhz, radiusPx) {
        var p = scope.snap ? scope.peakNear(mhz, source.spanMHz * radiusPx / scope.width) : NaN
        return isNaN(p) ? snap(mhz) : p
    }

    // Frequenz unter den Mittelstrich holen; UKW intern: dort hören
    function tuneAndListen(mhz) {
        tune(mhz)
        if (kind === "fm")
            source.listenTo(source.listenMHz)
    }

    // Kanalraster für < > (kHz)
    readonly property var rasters: [1, 6.25, 25 / 3, 12.5, 25, 100]
    property int rasterIndex: 4
    readonly property real rasterKHz: rasters[rasterIndex]
    function rasterText(k) { return (k === 25 / 3 ? "8,33" : String(k).replace(".", ",")) + " kHz" }

    // MHz kompakt mit deutschem Komma: 3 Stellen unter 1000 MHz, sonst 1
    function fmt(mhz) {
        return (mhz < 1000 ? mhz.toFixed(mhz < 200 ? 1 : 0) : mhz.toFixed(0)).replace(".", ",")
    }

    function parseMHz(t) {
        return parseFloat(String(t).replace(",", "."))
    }

    onKindChanged: rasterIndex = (kind === "fm") ? 5 : 4
    // UKW intern: mit der zuletzt genutzten Frequenz unter dem Mittelstrich beginnen
    function startView() {
        selectBand(firstBand ? initialBand : 0)
        firstBand = false
        if (kind === "fm")
            setPlan(source.listenMHz, source.spanMHz)
    }
    onSourceChanged: startView()
    Component.onCompleted: startView()

    // UKW intern: die Hörfrequenz liegt immer unter dem roten Mittelstrich, egal woher sie kommt
    Connections {
        target: page.kind === "fm" ? source : null
        ignoreUnknownSignals: true
        onListenMHzChanged: {
            if (Math.abs(source.centerMHz - source.listenMHz) > 0.001)
                page.setPlan(source.listenMHz, source.spanMHz)
        }
    }

    // Direkteingabe der Frequenz (lange auf die Frequenzanzeige drücken)
    // Sendername fehlt: Grund erklären (aus der RDS-Statistik), Name von Hand eintragen
    Component {
        id: nameDialog
        Dialog {
            id: nameDlg
            property real mhz: 0
            property string currentName: ""
            property var info: ({})
            canAccept: true
            onAccepted: source.setManualName(mhz, nameField.text)   // leer = löschen

            function reason() {
                if (info.listenedSec < 5)
                    return qsTr("Not listened to long enough yet. Listen to the station for about 20 seconds so that its name can be read via RDS.")
                if (info.reads === 0)
                    return qsTr("No RDS data received in %1 s (level %2 dBm). The signal is too weak for RDS, or the station does not send RDS.")
                           .arg(info.listenedSec).arg(info.dbm)
                if (!info.pi)
                    return qsTr("RDS data received (%1 times), but too disturbed to identify the station. The signal is too weak for RDS.")
                           .arg(info.reads)
                return qsTr("Station identified (PI %1), but its name has not been received yet. Keep listening a little longer.")
                       .arg(info.pi)
            }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium
                DialogHeader { acceptText: qsTr("Save") }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    text: currentName !== "" ? qsTr("Name for %1 MHz").arg(mhz.toFixed(1))
                                             : qsTr("No name for %1 MHz").arg(mhz.toFixed(1))
                    font.pixelSize: Theme.fontSizeLarge
                    color: Theme.highlightColor
                    wrapMode: Text.Wrap
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    visible: currentName === ""
                    text: reason()
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.secondaryHighlightColor
                }
                TextField {
                    id: nameField
                    width: parent.width
                    text: currentName
                    label: qsTr("Station name")
                    placeholderText: qsTr("Enter the station name")
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: accept()
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    text: currentName !== ""
                          ? qsTr("Names received via RDS are learned again when you listen to the station.")
                          : info.pi
                          ? qsTr("The name is stored together with the station code (PI %1). Where another station uses this frequency, it is dropped again.").arg(info.pi)
                          : qsTr("Without a station code (PI) the name is stored for this frequency only. If RDS later identifies a station here, its name takes precedence.")
                }
                Button {
                    visible: currentName !== ""
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Delete name")
                    onClicked: {
                        source.setManualName(nameDlg.mhz, "")
                        pageStack.pop()
                    }
                }
            }
        }
    }

    Component {
        id: freqDialog
        Dialog {
            id: dlg
            property real mhz: NaN
            canAccept: !isNaN(mhz) && mhz > 0
            onAccepted: {
                page.tune(mhz)
                if (page.kind === "fm")
                    source.listening = true
            }
            Column {
                width: parent.width
                DialogHeader { acceptText: qsTr("Tune") }
                TextField {
                    id: entry
                    width: parent.width
                    focus: true
                    label: qsTr("Frequency in MHz")
                    placeholderText: page.kind === "fm" ? "z. B. 103,5" : "z. B. 2437"
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    onTextChanged: dlg.mhz = page.parseMHz(text)
                    EnterKey.enabled: dlg.canAccept
                    EnterKey.onClicked: dlg.accept()
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.secondaryColor
                    text: qsTr("Range of this source: %1 – %2 MHz").arg(source.minMHz).arg(source.maxMHz)
                }
            }
        }
    }

    Connections {
        target: page.source
        onSweep: scope.pushSweep(bins, centerMHz, spanMHz)
    }

    SilicaFlickable
    {
        id: flick
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("About wfscope")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
            Repeater {
                model: hub.names
                MenuItem {
                    text: (index === hub.currentIndex ? "● " : "") + modelData
                    onClicked: hub.select(index)
                }
            }
            Repeater {
                // nur bei mehreren Bändern (UKW intern hat nur eins: dann genügt der Quelleneintrag)
                model: page.panadapterMode || page.bands.length < 2 ? [] : page.bands
                MenuItem {
                    text: (index === page.bandIndex ? "◆ " : "") + modelData.name
                    onClicked: page.selectBand(index)
                }
            }
            MenuItem {
                text: source.running ? qsTr("Stop") : qsTr("Start")
                onClicked: source.running ? source.stop() : source.start()
            }
        }

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            // Querformat: keine Kopfzeile, Frequenz und Raster nebeneinander — Platz fürs Spektrum
            PageHeader { visible: !page.isLandscape; title: source.name; description: source.status }

            Grid {
                id: tuneArea
                width: parent.width
                columns: page.isLandscape ? 2 : 1
                spacing: page.isLandscape ? 0 : Theme.paddingMedium
                readonly property real freqWidth: page.isLandscape ? width * 0.58 : width

            Item {
                width: tuneArea.freqWidth
                height: freq.height
            FrequencyDisplay {
                id: freq
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                hz: Math.round(page.tunedMHz * 1e6)
                minHz: Math.round((page.panadapterMode ? 0.001 : source.minMHz) * 1e6)
                maxHz: Math.round((page.panadapterMode ? 470 : source.maxMHz) * 1e6)
                minDigit: 0                       // jede Stelle jederzeit änderbar
                selectedPos: page.kind === "fm" ? 5 : 6
                digitColor: page.panadapterMode ? "#ffb347" : Theme.highlightColor
                onDirectEntry: pageStack.push(freqDialog)
                onEdited: page.tune(newHz / 1e6)
            }
            }

            Column {
                width: tuneArea.width - (page.isLandscape ? tuneArea.freqWidth : 0)
                // Querformat: Quelle und Status hier statt in der Kopfzeile
                Label {
                    visible: page.isLandscape
                    x: Theme.paddingLarge
                    width: parent.width - Theme.paddingLarge - Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignRight
                    truncationMode: TruncationMode.Fade
                    text: source.name + " · " + source.status
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.highlightColor
                }
            // Kanalraster: < = eine Rasterstufe nach unten, > = nach oben (halten = wiederholen)
            Row {
                // weiter nach innen gerückt, große Tasten mit Abstand zur Raster-Auswahl
                x: page.isLandscape ? Theme.paddingSmall : Theme.horizontalPageMargin + Theme.paddingLarge
                width: parent.width - x - (page.isLandscape ? Theme.horizontalPageMargin : x)
                spacing: Theme.paddingLarge
                IconButton {
                    id: stepDown
                    width: Theme.itemSizeMedium
                    height: Theme.itemSizeMedium
                    anchors.verticalCenter: parent.verticalCenter
                    icon.source: "image://theme/icon-m-left"
                    onClicked: page.tune(page.tunedMHz - page.rasterKHz / 1000)
                    onPressAndHold: repDown.start()
                    onReleased: repDown.stop()
                    onCanceled: repDown.stop()
                    Timer { id: repDown; interval: 150; repeat: true; onTriggered: page.tune(page.tunedMHz - page.rasterKHz / 1000) }
                }
                ComboBox {
                    width: parent.width - 2 * stepDown.width - 2 * parent.spacing
                    label: qsTr("Step")
                    currentIndex: page.rasterIndex
                    menu: ContextMenu {
                        Repeater {
                            model: page.rasters
                            MenuItem { text: page.rasterText(modelData) }
                        }
                    }
                    onCurrentIndexChanged: page.rasterIndex = currentIndex
                    description: qsTr("Span %1 · pinch the spectrum to zoom")
                                 .arg(source.spanMHz >= 1 ? page.fmt(source.spanMHz) + " MHz"
                                                          : (source.spanMHz * 1000).toFixed(0) + " kHz")
                }
                IconButton {
                    width: Theme.itemSizeMedium
                    height: Theme.itemSizeMedium
                    anchors.verticalCenter: parent.verticalCenter
                    icon.source: "image://theme/icon-m-right"
                    onClicked: page.tune(page.tunedMHz + page.rasterKHz / 1000)
                    onPressAndHold: repUp.start()
                    onReleased: repUp.stop()
                    onCanceled: repUp.stop()
                    Timer { id: repUp; interval: 150; repeat: true; onTriggered: page.tune(page.tunedMHz + page.rasterKHz / 1000) }
                }
            }
            }
            }

            SpectrumView {
                id: scope
                width: parent.width
                // so hoch, dass die Bedienzeile darunter ohne Scrollen am unteren Rand sichtbar bleibt
                height: Math.round(Math.max(page.height * 0.35,
                                            page.height - y - viewRow.height - Theme.paddingMedium))
                autoRange: autoSwitch.checked

                // Gesten wie in Gqrx: Inhalt folgt live dem Finger (Vorschau in SpectrumView),
                // umgestimmt wird erst beim Loslassen
                // listen: nach dem Schieben (nicht nach dem Zoomen) bei UKW intern auch hören
                function commitGesture(listen) {
                    var span = source.spanMHz, s = scope.viewScale, o = scope.viewOffset
                    if (Math.abs(o) < 1e-4 && Math.abs(s - 1) < 1e-4)
                        return
                    scope.commitView()
                    // Was jetzt unter dem Mittelstrich liegt, wird abgestimmt
                    var c = page.magnet(source.centerMHz - span * o / s, Theme.paddingLarge)
                    page.setPlan(c, span / s)
                    if (listen)
                        page.tuneAndListen(c)
                    else
                        page.tune(c)
                }

                PinchArea {
                    id: pinch
                    anchors.fill: parent
                    enabled: !page.panadapterMode
                    property real startOffset: 0
                    onPinchStarted: { startOffset = scope.viewOffset; flick.interactive = false }
                    onPinchUpdated: {
                        scope.viewScale = pinch.scale
                        scope.viewOffset = startOffset + (pinch.center.x - pinch.startCenter.x) / scope.width
                    }
                    onPinchFinished: { scope.commitGesture(false); flick.interactive = true }

                    MouseArea {
                        anchors.fill: parent
                        enabled: !page.panadapterMode
                        preventStealing: true
                        property real startX: 0
                        property bool dragging: false
                        onPressed: { startX = mouse.x; dragging = false; flick.interactive = false }
                        onPositionChanged: {
                            if (!dragging && Math.abs(mouse.x - startX) > Theme.startDragDistance)
                                dragging = true
                            if (dragging)
                                scope.viewOffset = (mouse.x - startX) / width
                        }
                        onReleased: {
                            flick.interactive = true
                            if (dragging) {
                                scope.commitGesture(true)
                            } else {
                                // Tippen: diese Frequenz unter den Mittelstrich holen (UKW intern: hören)
                                var span = source.spanMHz
                                page.tuneAndListen(page.magnet(source.centerMHz - span / 2 + mouse.x / width * span,
                                                               Theme.itemSizeExtraSmall / 2))
                            }
                        }
                        onCanceled: { flick.interactive = true; scope.viewOffset = 0; scope.viewScale = 1 }
                    }
                }

                // Roter Mittelstrich (wie Gqrx): bleibt stehen, das Band wird darunter verschoben;
                // nur über dem Spektrum (oberste 30 % von SpectrumView), nicht im Wasserfall
                Rectangle {
                    visible: !page.panadapterMode
                    width: scope.lineWidth
                    height: Math.round(parent.height * 0.30)
                    x: Math.round((parent.width - width) / 2)
                    color: "#ff3030"
                    opacity: 0.85
                }
                // Beschriftung der Senderspitzen: Frequenzen oben, RDS-Namen unten über dem
                // Rauschboden; jeweils auf versetzten Zeilen, damit nahe Sender sich nicht überdecken.
                // Folgt live der Gesten-Vorschau (viewOffset/viewScale wie in SpectrumView).
                readonly property real labelPx: Theme.fontSizeTiny
                FontMetrics { id: labelMetrics; font.pixelSize: scope.labelPx; font.bold: true }   // exakte Textbreite fürs Überdeckungs-Prüfen
                readonly property real lineWidth: Math.max(2, Math.round(Theme.pixelRatio * 2))   // roter Mittelstrich
                readonly property real traceHeight: height * 0.30     // Spektrumsbereich in SpectrumView
                function placeLabels(names) {
                    if (page.kind !== "fm" || !source.stations)
                        return []
                    if (names ? !source.labelNames : !source.labelFreqs)
                        return []
                    var span = source.spanMHz / scope.viewScale
                    var lo = source.centerMHz - source.spanMHz * scope.viewOffset / scope.viewScale - span / 2
                    var items = []
                    for (var i = 0; i < source.stations.length; ++i) {
                        var st = source.stations[i]
                        // ohne RDS-Namen: „?“ zum Antippen (Erklärung, Name von Hand)
                        var t = names ? (st.name ? st.name : "?") : st.mhz.toFixed(1).replace(".", ",")
                        items.push({ mhz: st.mhz, text: t, verified: st.verified, unknown: names && !st.name,
                                     x: (st.mhz - lo) / span * scope.width,
                                     w: labelMetrics.advanceWidth(t) + Theme.paddingSmall,
                                     // Namen beginnen an der Spitze und laufen nach rechts, Frequenzen zentriert
                                     left: names ? (st.mhz - lo) / span * scope.width + scope.lineWidth * 1.5   // halber Strich + eine Strichbreite Abstand
                                                 : (st.mhz - lo) / span * scope.width - (labelMetrics.advanceWidth(t) + Theme.paddingSmall) / 2,
                                     here: source.listening && Math.abs(st.mhz - source.listenMHz) < 0.05 })
                    }
                    // stärkste Sender zuerst (Liste kommt so sortiert); was in keine Zeile passt,
                    // entfällt bei Frequenzen, Namen kommen notfalls in die letzte Zeile
                    var placed = [], maxRows = names ? 5 : 3   // Namen: mehr Zeilen, damit auch dichte Bänder lesbar bleiben
                    for (var n = 0; n < items.length; ++n) {
                        var it = items[n], row = -1
                        for (var r = 0; r < maxRows && row < 0; ++r) {
                            var free = true
                            for (var k = 0; k < placed.length && free; ++k)
                                if (placed[k].row === r && placed[k].left < it.left + it.w && it.left < placed[k].left + placed[k].w)
                                    free = false
                            if (free) row = r
                        }
                        if (row < 0) {
                            if (!names) continue
                            row = maxRows - 1
                        }
                        it.row = row
                        placed.push(it)
                    }
                    return placed
                }
                Repeater {
                    model: scope.placeLabels(false)
                    Label {
                        visible: modelData.x > -modelData.w && modelData.x < scope.width + modelData.w
                        x: Math.round(modelData.x - width / 2)
                        y: Theme.paddingSmall + modelData.row * (scope.labelPx * 1.25)
                        text: modelData.text
                        font.pixelSize: scope.labelPx
                        font.bold: modelData.here
                        color: modelData.here ? "#ff6060" : "#8a98a8"
                    }
                }
                Repeater {
                    model: scope.placeLabels(true)
                    Label {
                        visible: modelData.left > -modelData.w && modelData.left < scope.width
                        x: Math.round(modelData.left)
                        y: scope.traceHeight - Theme.paddingSmall - (modelData.row + 1) * (scope.labelPx * 1.25)
                        text: modelData.text
                        font.pixelSize: scope.labelPx
                        font.bold: modelData.here
                        font.italic: !modelData.unknown && !modelData.verified   // gespeichert, noch nicht per PI geprüft
                        color: modelData.unknown ? "#ffc040"
                               : modelData.here ? "#ff6060" : modelData.verified ? "#e8f4ff" : "#8a98a8"
                        MouseArea {
                            // „?“: Grund + Name eintragen; Name: ändern oder löschen
                            anchors.centerIn: parent
                            width: Math.max(parent.width, Theme.itemSizeExtraSmall)
                            height: Theme.itemSizeExtraSmall
                            onClicked: pageStack.push(nameDialog, { mhz: modelData.mhz,
                                                                    currentName: modelData.unknown ? "" : modelData.text,
                                                                    info: source.rdsInfo(modelData.mhz) })
                        }
                    }
                }
            }

            // Kleine Schalter: Beschriftung (UKW intern), Snap; daneben die Intensität des Wasserfalls
            Row {
                id: viewRow
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                spacing: Theme.paddingSmall
                ToggleDot {
                    visible: page.kind === "fm"
                    text: qsTr("Frequencies")
                    checked: page.kind === "fm" && source.labelFreqs
                    onToggled: source.labelFreqs = !source.labelFreqs
                }
                ToggleDot {
                    visible: page.kind === "fm"
                    text: qsTr("Names")
                    checked: page.kind === "fm" && source.labelNames
                    onToggled: source.labelNames = !source.labelNames
                }
                ToggleDot {
                    // Band neu abtasten (beim Hören pausiert der Ton so lange); leuchtet während des Durchlaufs
                    visible: page.kind === "fm"
                    text: qsTr("Scan")
                    checked: page.kind === "fm" && source.rescanning
                    onToggled: source.rescan()
                }
                ToggleDot {
                    visible: !page.panadapterMode
                    text: qsTr("Snap")
                    checked: scope.snap
                    onToggled: scope.snap = !scope.snap
                }
                Slider {
                    id: intensitySlider
                    width: viewRow.width - x
                    anchors.verticalCenter: parent.verticalCenter
                    leftMargin: Theme.paddingMedium
                    rightMargin: Theme.paddingMedium
                    minimumValue: 0; maximumValue: 100; stepSize: 5
                    value: scope.intensity
                    label: qsTr("Intensity")
                    onValueChanged: if (pressed) scope.intensity = value
                    onReleased: scope.intensity = value
                }
            }

            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                Label {
                    width: parent.width / 2
                    text: qsTr("Peak\n%1 MHz").arg(scope.peakHoldMHz < 1000 ? scope.peakHoldMHz.toFixed(3)
                                                                                 : scope.peakHoldMHz.toFixed(1))
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
                label: qsTr("Source")
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
                label: qsTr("Band")
                currentIndex: page.bandIndex
                // eigener Bereich (gewischt/gezoomt/eingegeben): Bereich unter dem Feld zeigen
                description: page.bandIndex >= 0 ? ""
                             : qsTr("custom: %1 – %2 MHz · choosing a band jumps back")
                               .arg(page.fmt(source.centerMHz - source.spanMHz / 2))
                               .arg(page.fmt(source.centerMHz + source.spanMHz / 2))
                menu: ContextMenu {
                    Repeater {
                        model: page.bands
                        MenuItem { text: modelData.name }
                    }
                }
                onCurrentIndexChanged: if (currentIndex !== page.bandIndex) page.selectBand(currentIndex)
            }

            // ---- UKW intern ----
            Column {
                width: parent.width
                visible: page.kind === "fm"
                SectionHeader { text: qsTr("FM radio (internal)") }
                TextSwitch {
                    text: qsTr("Built-in short antenna")
                    description: qsTr("Off: antenna via a headset (if the device supports one)")
                    checked: page.kind === "fm" && source.shortAntenna
                    onClicked: if (page.kind === "fm") source.shortAntenna = checked
                }
                TextSwitch {
                    text: qsTr("Listen") + (page.kind === "fm" && source.listening
                                     ? " — " + source.listenMHz.toFixed(1) + " MHz"
                                       + (source.rdsName !== "" ? " · " + source.rdsName : "")
                                       + " · " + source.listenDbm + " dBm" : "")
                    description: page.kind === "fm" && source.listening && source.rdsText !== ""
                                 ? source.rdsText
                                 : qsTr("Slide a station under the red line or tap it; scanning pauses")
                    checked: page.kind === "fm" && source.listening
                    onClicked: if (page.kind === "fm") source.listening = checked
                }
                Row {
                    x: Theme.horizontalPageMargin
                    visible: page.kind === "fm" && source.listening
                    spacing: Theme.paddingLarge
                    IconButton {
                        icon.source: "image://theme/icon-m-left"
                        onClicked: page.tune(source.listenMHz - 0.1)
                    }
                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: page.kind === "fm" ? source.listenMHz.toFixed(1) + " MHz" : ""
                        font.pixelSize: Theme.fontSizeLarge
                        color: Theme.highlightColor
                    }
                    IconButton {
                        icon.source: "image://theme/icon-m-right"
                        onClicked: page.tune(source.listenMHz + 0.1)
                    }
                }
                Slider {
                    width: parent.width
                    minimumValue: 0; maximumValue: 100; stepSize: 5
                    value: page.kind === "fm" ? source.volume : 30
                    label: qsTr("Volume")
                    valueText: value.toFixed(0) + " %"
                    onValueChanged: if (page.kind === "fm" && pressed) source.volume = value
                    onReleased: if (page.kind === "fm") source.volume = value
                }
                SectionHeader {
                    text: qsTr("Stations")
                    visible: page.kind === "fm" && source.stations.length > 0
                }
                Flow {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    spacing: Theme.paddingSmall
                    Repeater {
                        model: page.kind === "fm" ? source.stations : []
                        BackgroundItem {
                            width: stLabel.width + 2 * Theme.paddingMedium
                            height: Theme.itemSizeExtraSmall
                            highlighted: down || (source.listening && Math.abs(source.listenMHz - modelData.mhz) < 0.05)
                            onClicked: page.tuneAndListen(modelData.mhz)
                            Label {
                                id: stLabel
                                anchors.centerIn: parent
                                text: modelData.mhz.toFixed(1) + (modelData.name ? " " + modelData.name : "")
                                      + (modelData.valid ? " ●" : "")
                                color: parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                                font.pixelSize: Theme.fontSizeSmall
                            }
                        }
                    }
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    text: qsTr("Level per 100 kHz channel (no IQ), about 50 ms per channel; ● = recognised as a station by the chip. Runs without root, Wi-Fi stays on.")
                }
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
                    label: qsTr("LNA (IF gain)")
                    valueText: value.toFixed(0) + " dB"
                    onReleased: if (page.kind === "hackrf") source.lnaGain = value
                }
                Slider {
                    width: parent.width
                    minimumValue: 0; maximumValue: 62; stepSize: 2
                    value: page.kind === "hackrf" ? source.vgaGain : 20
                    label: qsTr("VGA (baseband gain)")
                    valueText: value.toFixed(0) + " dB"
                    onReleased: if (page.kind === "hackrf") source.vgaGain = value
                }
                TextSwitch {
                    text: qsTr("RF preamp (+14 dB)")
                    description: qsTr("Only for weak signals; overloads easily")
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
                    text: qsTr("Panadapter FT-991A")
                    description: qsTr("Tuner fixed on the IF (TSP IFace 2), axis in VFO frequency")
                    checked: page.kind === "sdrplay" && source.panadapter
                    onClicked: if (page.kind === "sdrplay") source.panadapter = checked
                }
                TextField {
                    width: parent.width
                    visible: page.panadapterMode
                    label: qsTr("VFO frequency (MHz)")
                    placeholderText: label
                    text: page.kind === "sdrplay" ? source.vfoMHz.toFixed(4) : ""
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    EnterKey.onClicked: { source.vfoMHz = page.parseMHz(text); focus = false }
                }
                TextField {
                    width: parent.width
                    visible: page.panadapterMode
                    label: qsTr("First IF of the radio (MHz)")
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
                    label: qsTr("IF fine correction")
                    valueText: (value >= 0 ? "+" : "") + value.toFixed(1) + " kHz"
                    onReleased: if (page.kind === "sdrplay") source.offsetKHz = value
                }
                TextSwitch {
                    visible: page.panadapterMode
                    text: qsTr("Mirror spectrum")
                    description: qsTr("If signals move the wrong way when turning the VFO")
                    checked: page.kind === "sdrplay" && source.mirror
                    onClicked: if (page.kind === "sdrplay") source.mirror = checked
                }
                ComboBox {
                    width: parent.width
                    label: qsTr("Antenna input")
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
                    label: qsTr("LNA state (0 = max. gain)")
                    valueText: value.toFixed(0)
                    onReleased: if (page.kind === "sdrplay") source.lnaState = value
                }
                TextSwitch {
                    text: qsTr("AGC")
                    checked: page.kind === "sdrplay" && source.agc
                    onClicked: if (page.kind === "sdrplay") source.agc = checked
                }
            }

            SectionHeader { text: qsTr("Display") }

            TextSwitch {
                id: autoSwitch
                text: qsTr("Auto contrast")
                description: qsTr("Colour scale follows the noise floor")
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
                label: qsTr("Noise floor")
                valueText: value.toFixed(0) + " dB"
            }
        }
    }
}
