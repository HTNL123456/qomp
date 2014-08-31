import QtQuick 2.3

FocusScope {
	id: textlineedit

	property alias text: label.text;
	property alias value: lineedit.text;
	property alias input: lineedit.input

	height: 68 * scaler.scaleY

	anchors.left: parent.left
	anchors.right: parent.right

	Item {
		id: rectangle

		anchors.fill: parent

		Text {
			id: label
			wrapMode: Text.WrapAtWordBoundaryOrAnywhere
			verticalAlignment: Text.AlignVCenter
			horizontalAlignment: Text.AlignRight
			anchors {
				fill: parent
				margins: 8 * scaler.scaleMargins
				rightMargin: rectangle.width * 2 / 3
			}
			font.pixelSize: textlineedit.height / 3
		}

		LineEdit {
			id: lineedit

			anchors {
				fill: parent
				margins: 8 * scaler.scaleMargins
				leftMargin: rectangle.width / 3 + 20
			}
			//focus: true
		}
	}
}