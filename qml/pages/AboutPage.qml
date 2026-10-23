import QtQuick 2.6
import Sailfish.Silica 1.0

// Über wfscope: Version, Hinweise zum Empfang, Lizenz, Quellcode, Sprache
Page {
    id: page
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingMedium

            PageHeader { title: qsTr("About wfscope") }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "/usr/share/icons/hicolor/128x128/apps/wfscope.png"
                width: Theme.iconSizeExtraLarge
                height: Theme.iconSizeExtraLarge
            }
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "wfscope"
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.highlightColor
            }
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Version %1").arg(appVersion)
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryHighlightColor
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Spectrum analyser and waterfall for Sailfish OS: internal FM radio with RDS, HackRF and SDRplay RSPdx (also as panadapter), Wi-Fi spectrum via ICAP.")
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.highlightColor
                text: qsTr("Early stage and very experimental.")
            }

            SectionHeader { text: qsTr("Reception tips (internal FM)") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                text: qsTr("• A USB-C cable plugged into the phone acts as the antenna. The antenna signal runs on the SBU1/SBU2 pins (A8/B8), so the cable must carry the SBU wires: fully wired USB 3.x/USB4 cables do (e.g. 40 Gbps), simple charging and USB 2.0 cables do not. Tested: 1.3 m USB4 cable with e-marker.\n• Let the cable hang straight; coiled up it receives much worse.\n• Leave the other end free: a connected laptop adds a lot of noise.\n• Without such a cable reception is very weak; station names (RDS) need a good signal.")
            }

            SectionHeader { text: qsTr("Warning") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                text: qsTr("The Wi-Fi spectrum (ICAP) puts the Wi-Fi chip into test mode and switches Wi-Fi off. On the Jolla J2 it has crashed the device; use it only if you can live with a reboot.")
            }

            SectionHeader { text: qsTr("Language") }
            ComboBox {
                id: languageBox
                width: parent.width
                label: qsTr("Language")
                description: qsTr("Takes effect at the next start")
                currentIndex: {
                    for (var i = 0; i < language.available.length; ++i)
                        if (language.available[i].code === language.code)
                            return i
                    return 0
                }
                menu: ContextMenu {
                    Repeater {
                        model: language.available
                        MenuItem { text: modelData.name }
                    }
                }
                onCurrentIndexChanged: language.code = language.available[currentIndex].code
            }

            SectionHeader { text: qsTr("License") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                text: qsTr("GNU General Public License v3 or later (GPLv3+), with an exception that allows using the proprietary SDRplay API at runtime.\nContains tinyalsa and libhackrf (BSD licences). The SDRplay API is not part of the package.")
            }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Source code on GitHub")
                onClicked: Qt.openUrlExternally("https://github.com/JimKnopfIoT/wfscope")
            }
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "github.com/JimKnopfIoT/wfscope"
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
            }
        }
        VerticalScrollDecorator {}
    }
}
