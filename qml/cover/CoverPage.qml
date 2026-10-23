import QtQuick 2.6
import Sailfish.Silica 1.0

CoverBackground
{
    readonly property QtObject source: hub.current

    Column {
        anchors.centerIn: parent
        width: parent.width - 2 * Theme.paddingLarge
        spacing: Theme.paddingMedium

        Label {
            text: source.name
            font.pixelSize: Theme.fontSizeMedium
            color: Theme.highlightColor
            wrapMode: Text.Wrap
            width: parent.width
        }
        Label {
            text: source.running ? qsTr("running") : qsTr("ready")
            color: Theme.secondaryColor
        }
        Label {
            text: (source.kind === "sdrplay" && source.panadapter ? source.vfoMHz : source.centerMHz).toFixed(source.centerMHz < 1000 ? 3 : 0) + " MHz"
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
        }
    }

    CoverActionList {
        CoverAction {
            iconSource: source.running ? "image://theme/icon-cover-pause"
                                        : "image://theme/icon-cover-play"
            onTriggered: source.running ? source.stop() : source.start()
        }
    }
}
