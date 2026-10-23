import QtQuick 2.6
import Sailfish.Silica 1.0

// Kleiner Ein/Aus-Schalter: runder Punkt zum Antippen, Beschriftung darunter
MouseArea {
    id: root
    property alias text: label.text
    property bool checked: false
    signal toggled()

    width: Math.max(Theme.itemSizeSmall, label.implicitWidth + Theme.paddingSmall)
    height: Theme.itemSizeMedium
    onClicked: toggled()

    Rectangle {
        id: dot
        anchors.horizontalCenter: parent.horizontalCenter
        y: Theme.paddingMedium
        width: Theme.iconSizeSmall
        height: width
        radius: width / 2
        color: root.checked ? Theme.highlightColor : "transparent"
        border.width: Math.max(2, Math.round(Theme.pixelRatio * 2))
        border.color: root.pressed ? Theme.highlightColor
                      : root.checked ? Theme.highlightColor : Theme.secondaryColor
    }
    Label {
        id: label
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: dot.bottom
        anchors.topMargin: Theme.paddingSmall
        font.pixelSize: Theme.fontSizeTiny
        color: root.checked ? Theme.highlightColor : Theme.secondaryColor
    }
}
