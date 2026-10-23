import QtQuick 2.6
import Sailfish.Silica 1.0

// Frequenzanzeige nach Gqrx-Vorbild: Ziffer antippen = auswählen (markiert),
// ▲/▼ rechts ändern die gewählte Stelle (gedrückt halten = fortlaufend),
// lange auf die Anzeige drücken = Direkteingabe (Signal directEntry).
// Gruppen: GHz . MHz . kHz . Hz (10 Stellen, bis 9,999999999 GHz).
Item {
    id: root

    property real hz: 0
    property real minHz: 0
    property real maxHz: 9999999999
    property int minDigit: 0            // niedrigste wählbare Stelle (0 = 1 Hz)
    property int selectedPos: 6         // gewählte Stelle (6 = 1 MHz)
    property color digitColor: Theme.highlightColor
    signal edited(real newHz)
    signal directEntry()

    readonly property int digits: 10
    // Große ▲/▼-Tasten mit deutlichem Abstand (fingerfreundlich)
    readonly property real buttonWidth: Theme.itemSizeMedium
    readonly property real buttonGap: Theme.paddingLarge * 1.5
    readonly property real digitWidth: Math.min((width - buttonWidth) / (digits + 3.5 + 1.4),
                                                Theme.itemSizeLarge * 0.62)

    implicitHeight: 2 * Theme.itemSizeMedium + buttonGap
    height: implicitHeight

    onMinDigitChanged: if (selectedPos < minDigit) selectedPos = minDigit

    function step(dir) {
        var v = hz + dir * Math.pow(10, selectedPos)
        v = Math.max(minHz, Math.min(maxHz, v))
        if (v !== hz)
            edited(v)
    }

    function digitAt(pos) {
        return Math.floor(hz / Math.pow(10, pos)) % 10
    }

    Row {
        id: row
        anchors.verticalCenter: parent.verticalCenter
        x: Math.max(0, (parent.width - root.buttonWidth - width) / 2)
        height: Theme.itemSizeLarge

        Repeater {
            model: root.digits
            delegate: Item {
                // index 0 = höchste Stelle
                readonly property int pos: root.digits - 1 - index
                readonly property bool leading: root.hz < Math.pow(10, pos) && pos > 6
                readonly property bool selectable: pos >= root.minDigit
                readonly property bool selected: pos === root.selectedPos
                width: root.digitWidth + (pos % 3 === 0 && pos > 0 ? root.digitWidth * 0.35 : 0)
                height: row.height

                Rectangle {
                    // Markierung der gewählten Ziffer
                    width: root.digitWidth
                    height: parent.height * 0.86
                    anchors.verticalCenter: parent.verticalCenter
                    radius: Theme.paddingSmall
                    color: Theme.rgba(Theme.highlightBackgroundColor, selected ? 0.35 : 0.0)
                    border.width: selected ? 2 : 0
                    border.color: Theme.highlightColor
                }
                Label {
                    width: root.digitWidth
                    height: parent.height
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: root.digitAt(pos)
                    font.pixelSize: root.digitWidth * 1.45
                    font.family: "monospace"
                    color: !selectable ? Theme.secondaryColor
                           : leading ? Theme.rgba(root.digitColor, 0.3) : root.digitColor
                }
                Label {
                    // Gruppentrenner nach jeder dritten Stelle
                    visible: pos % 3 === 0 && pos > 0
                    x: root.digitWidth
                    width: root.digitWidth * 0.35
                    height: parent.height
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: "."
                    font.pixelSize: root.digitWidth * 1.2
                    color: Theme.secondaryColor
                }
                MouseArea {
                    width: root.digitWidth
                    height: parent.height
                    onClicked: if (selectable) root.selectedPos = pos
                    onPressAndHold: root.directEntry()
                }
            }
        }
        Label {
            height: row.height
            verticalAlignment: Text.AlignVCenter
            leftPadding: root.digitWidth * 0.3
            text: "Hz"
            font.pixelSize: root.digitWidth * 0.75
            color: Theme.secondaryColor
            MouseArea { anchors.fill: parent; onPressAndHold: root.directEntry() }
        }
    }

    // Ein Paar Pfeile für die gewählte Ziffer
    Column {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: root.buttonGap
        IconButton {
            width: root.buttonWidth
            height: Theme.itemSizeMedium
            icon.source: "image://theme/icon-m-up"
            onClicked: root.step(1)
            onPressAndHold: repeatUp.start()
            onReleased: repeatUp.stop()
            onCanceled: repeatUp.stop()
            Timer { id: repeatUp; interval: 120; repeat: true; onTriggered: root.step(1) }
        }
        IconButton {
            width: root.buttonWidth
            height: Theme.itemSizeMedium
            icon.source: "image://theme/icon-m-down"
            onClicked: root.step(-1)
            onPressAndHold: repeatDown.start()
            onReleased: repeatDown.stop()
            onCanceled: repeatDown.stop()
            Timer { id: repeatDown; interval: 120; repeat: true; onTriggered: root.step(-1) }
        }
    }
}
