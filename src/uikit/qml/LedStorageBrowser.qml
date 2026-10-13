// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    property LedDeckRuntime runtime
    property LedStorageRepository writableRepository
    property string selectedPlace: ""
    property string operation: "save"
    property alias currentIndex: grid.currentIndex
    readonly property LedStorageRepository repository: runtime ? runtime.repository : null
    readonly property LedStorageRepository destination: repository?.readOnly ? writableRepository : repository
    readonly property string storageId: runtime ? runtime.browsingStorage : ""
    onStorageIdChanged: { selectedPlace=""; if(grid)grid.currentIndex=-1; }
    onRepositoryChanged: { selectedPlace=""; if(grid)grid.currentIndex=-1; }
    spacing: Theme.tokens.spacing
    LedPlaceModel { id: places; repository: control.repository; registry: control.runtime ? control.runtime.registry : null; storageId: control.storageId }
    RowLayout {
        Layout.fillWidth: true
        CatComboBox {
            Layout.fillWidth: true
            model: control.repository ? control.repository.storages : []
            textRole: "name"; valueRole: "id"
            currentIndex: indexOfValue(control.storageId)
            onActivated: index => control.runtime.browseStorage(String(valueAt(index)))
        }
        CatButton {
            text: qsTr("+ Storage"); enabled: control.repository && !control.repository.readOnly
            onClicked: {
                const id=control.repository.createStorage(qsTr("New storage"),12);
                if(id)control.runtime.browseStorage(id);
            }
        }
    }
    GridView {
        id: grid
        objectName: "ledPlaceGrid"
        Layout.fillWidth: true; Layout.preferredHeight: 185
        clip: true; model: places
        cellWidth: width/Math.max(2,Math.floor(width/108)); cellHeight: 88
        keyNavigationEnabled: true; activeFocusOnTab: true
        onCurrentIndexChanged: control.selectedPlace=String(places.get(currentIndex).id ?? "")
        Keys.onReturnPressed: if(control.selectedPlace)control.runtime.activatePlace(control.storageId,control.selectedPlace)
        delegate: CatButton {
            id: placeButton
            required property string placeId
            required property string name
            required property bool empty
            required property string error
            required property int number
            required property int index
            width: grid.cellWidth-6; height: grid.cellHeight-6
            focus: GridView.isCurrentItem
            primary: control.runtime?.activeRepositoryId===control.repository?.repositoryId && control.runtime?.activeStorage===control.storageId && control.runtime?.activePlace===placeId
            Accessible.name: qsTr("Place %1: %2").arg(number).arg(name || qsTr("Empty"))
            contentItem: ColumnLayout {
                spacing: 2
                Image {
                    Layout.fillWidth: true; Layout.preferredHeight: 35
                    fillMode: Image.Stretch; smooth: false; sourceSize: Qt.size(128,64)
                    source: {
                        const revision=control.repository ? control.repository.thumbnailRevision : 0;
                        return !placeButton.empty && !placeButton.error && control.repository ? control.repository.thumbnail(control.storageId,placeButton.placeId,control.runtime?.registry ?? null) : "";
                    }
                }
                CatLabel {
                    Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; elide: Text.ElideRight
                    color: placeButton.primary ? Theme.tokens.onPrimary : Theme.tokens.text
                    text: placeButton.number+" · "+(placeButton.empty ? qsTr("Empty") : placeButton.name)
                }
                CatLabel { visible: !placeButton.empty && !!placeButton.error; text: qsTr("Unavailable"); font.pixelSize: 10 }
            }
            onClicked: {grid.currentIndex=index;control.selectedPlace=placeId;control.runtime.activatePlace(control.storageId,placeId)}
            TapHandler { acceptedButtons: Qt.RightButton; onTapped: {grid.currentIndex=placeButton.index;control.selectedPlace=placeButton.placeId;} }
        }
        CatScrollBar.vertical: CatScrollBar {}
    }
    Flow {
        Layout.fillWidth: true; spacing: 6
        CatButton { text: qsTr("Load paused"); enabled: !!control.selectedPlace; onClicked: control.runtime.loadPlace(control.storageId,control.selectedPlace) }
        CatButton { text: qsTr("Save / new place"); enabled: !!control.destination && !control.destination.readOnly && !!control.runtime?.workingPlace.effectId; onClicked: { control.operation="save";saveDialog.open(); } }
        CatButton { text: qsTr("Copy"); enabled: !!control.selectedPlace && !!control.destination && !control.destination.readOnly; onClicked: {control.operation="copy";saveDialog.open();} }
        CatButton { text: qsTr("Rename"); enabled: !!control.selectedPlace && !!control.repository && !control.repository.readOnly; onClicked: renameDialog.open() }
        CatButton { text: qsTr("Clear slot"); enabled: !!control.selectedPlace && !!control.repository && !control.repository.readOnly; onClicked: control.repository.clearPlace(control.storageId,control.selectedPlace) }
        CatButton { text: qsTr("Import / export"); enabled: !!control.repository; onClicked: exchange.open() }
    }
    CatLabel {
        Layout.fillWidth: true;wrapMode: Text.Wrap;visible: !!control.repository?.readOnly
        text: qsTr("Read-only source. Save a working copy to the writable repository.")
    }
    CatLabel { Layout.fillWidth: true;wrapMode: Text.Wrap;visible: text.length>0;text: control.repository ? control.repository.lastError : "" }
    property CatDialog renamePopup: CatDialog {
        id: renameDialog
        title: qsTr("Rename selected place")
        onOpened: rename.text=String(control.repository.place(control.storageId,control.selectedPlace).name ?? "")
        contentItem: CatTextField { id: rename }
        footer: Flow {
            spacing: 8;padding: 12
            CatButton {text: qsTr("Cancel");onClicked: renameDialog.close()}
            CatButton {text: qsTr("Rename");onClicked: if(control.repository.renamePlace(control.storageId,control.selectedPlace,rename.text))renameDialog.close()}
        }
    }
    property CatDialog savePopup: CatDialog {
        id: saveDialog
        title: qsTr("Save to a specific slot")
        implicitWidth: Math.min(420,parent ? parent.width-24 : 420)
        contentItem: ColumnLayout {
            CatLabel { Layout.fillWidth: true;wrapMode: Text.Wrap;text: qsTr("The destination slot will be replaced. Loaded Deck snapshots stay unchanged.") }
            CatComboBox { id: targetStorage;Layout.fillWidth: true;model: control.destination ? control.destination.storages : [];textRole:"name";valueRole:"id" }
            CatComboBox { id: targetSlot;Layout.fillWidth: true;model: control.destination ? control.destination.places(String(targetStorage.currentValue ?? "")) : [];textRole:"id";valueRole:"id";displayText: currentIndex>=0 ? qsTr("Slot %1").arg(currentIndex+1) : qsTr("Select a slot") }
            CatTextField { id: saveName;Layout.fillWidth: true;placeholderText: qsTr("Preset name") }
        }
        onOpened: saveName.text=String(control.runtime.workingPlace.name ?? "")
        footer: Flow {
            spacing:8;padding:12
            CatButton {text:qsTr("Cancel");onClicked:saveDialog.close()}
            CatButton {
                text:qsTr("Save");primary:true;enabled:targetSlot.currentIndex>=0
                onClicked: {
                    const s=String(targetStorage.currentValue),p=String(targetSlot.currentValue);
                    let ok=false;
                    if(control.operation==="copy") {
                        const config=control.repository.place(control.storageId,control.selectedPlace);
                        config.name=saveName.text;ok=control.destination.savePlace(s,p,config);
                    } else ok=control.runtime.saveTo(control.destination,s,p,saveName.text);
                    if(ok)saveDialog.close();
                }
            }
        }
    }
    property CatDialog exchangePopup: CatDialog {
        id: exchange
        title: qsTr("Storage / Place JSON")
        implicitWidth: Math.min(650,parent ? parent.width-24 : 650)
        implicitHeight: Math.min(650,parent ? parent.height-24 : 650)
        contentItem: ColumnLayout {
            CatComboBox {id: scope;Layout.fillWidth:true;model:[qsTr("Whole repository"),qsTr("Browsing storage"),qsTr("Selected place")];currentIndex:1}
            CatScrollView {Layout.fillWidth:true;Layout.fillHeight:true;CatTextArea {id: json;selectByMouse:true;wrapMode:TextEdit.NoWrap}}
            CatTextField {id: path;Layout.fillWidth:true;placeholderText:qsTr("Local file URL, e.g. file:///tmp/presets.json")}
            CatLabel {Layout.fillWidth:true;wrapMode:Text.Wrap;text:control.repository ? control.repository.lastError : ""}
        }
        onOpened: json.text=control.repository.exportJson(control.storageId)
        footer: Flow {
            spacing:6;padding:12
            CatButton {text:qsTr("Export JSON");onClicked:json.text=control.repository.exportJson(scope.currentIndex===0?"":control.storageId,scope.currentIndex===2?control.selectedPlace:"")}
            CatButton {text:qsTr("Import JSON");enabled:control.repository && !control.repository.readOnly;onClicked:control.repository.importJson(json.text,control.storageId,control.selectedPlace)}
            CatButton {text:qsTr("Save file");onClicked:control.repository.saveFile(path.text,scope.currentIndex===0?"":control.storageId,scope.currentIndex===2?control.selectedPlace:"")}
            CatButton {text:qsTr("Load file");enabled:control.repository && !control.repository.readOnly;onClicked:control.repository.loadFile(path.text,control.storageId,control.selectedPlace)}
            CatButton {text:qsTr("Close");onClicked:exchange.close()}
        }
    }
}
