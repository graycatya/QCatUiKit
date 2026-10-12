// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

B.ApplicationWindow {
    id: window
    property bool darkTheme: false
    property int initialDensity: 0
    property string initialPage: "home"
    property string pageId: initialPage === "home" || initialPage === "settings" ? initialPage : "components"
    property string categoryId: "all"
    property string componentId: pageId === "components" && initialPage !== "all" ? initialPage : ""
    property string searchQuery: ""
    readonly property bool compact: width < 860
    readonly property var currentComponent: GalleryCatalog.componentForId(componentId)
    readonly property string pageTitle: pageId === "home" ? qsTr("Overview") : pageId === "settings" ? qsTr("Settings") : currentComponent ? currentComponent.title : GalleryCatalog.categoryTitle(categoryId)
    readonly property var matchingComponents: GalleryCatalog.components.filter(entry => {
        const query = searchQuery.trim().toLowerCase();
        return (categoryId === "all" || entry.category === categoryId) && (!query || (entry.name + " " + entry.title + " " + entry.description).toLowerCase().includes(query));
    })
    readonly property bool invalidInitialPage: initialPage !== "home" && initialPage !== "all" && initialPage !== "settings" && !GalleryCatalog.componentForId(initialPage)
    width: 1200
    height: 840
    minimumWidth: 360
    minimumHeight: 540
    visible: true
    title: qsTr("QCatUiKit %1 · Component Gallery").arg(QCatUiKit.version)
    color: Theme.tokens.background
    locale: Qt.locale(GalleryLanguage.localeName)

    function navigate(page: string, category: string): void {
        pageId = page;
        categoryId = category;
        componentId = "";
        searchQuery = "";
        mobileNavigation.close();
    }
    function showComponent(id: string): void {
        const entry = GalleryCatalog.componentForId(id);
        if (!entry)
            return;
        pageId = "components";
        categoryId = entry.category;
        componentId = id;
        searchQuery = "";
    }
    onCompactChanged: if (!compact)
        mobileNavigation.close()
    onSearchQueryChanged: {
        if (searchQuery.length) {
            pageId = "components";
            categoryId = "all";
            componentId = "";
        }
    }
    Component.onCompleted: {
        if (currentComponent)
            categoryId = currentComponent.category;
        Theme.colorScheme = darkTheme ? ThemePalette.Dark : ThemePalette.Light;
        Theme.density = initialDensity;
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            visible: !window.compact
            Layout.preferredWidth: 232
            Layout.fillHeight: true
            color: Theme.tokens.surface
            Rectangle {
                width: 1
                height: parent.height
                anchors.right: parent.right
                color: Theme.tokens.border
                opacity: 0.5
            }
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 28
                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 10
                    Layout.leftMargin: 10
                    spacing: 12
                    GalleryLogo {
                        objectName: "sidebarBrandLogo"
                    }
                    ColumnLayout {
                        spacing: 2
                        CatLabel {
                            text: "QCatUiKit"
                            font.pixelSize: 17
                            font.bold: true
                        }
                        CatLabel {
                            text: qsTr("Component Gallery")
                            font.pixelSize: 11
                            color: Theme.tokens.textSecondary
                        }
                    }
                }
                CatScrollView {
                    id: navigationScroll
                    objectName: "galleryNavigationScroll"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth
                    GalleryNavigation {
                        width: parent.width
                        height: Math.max(implicitHeight, navigationScroll.availableHeight)
                        pageId: window.pageId
                        categoryId: window.categoryId
                        onNavigate: (page, category) => window.navigate(page, category)
                    }
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 72
                color: Theme.tokens.surface
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: window.compact ? 16 : 32
                    anchors.rightMargin: window.compact ? 16 : 32
                    spacing: 12
                    CatButton {
                        visible: window.compact
                        implicitWidth: 36
                        padding: 6
                        objectName: "openNavigationButton"
                        Accessible.name: qsTr("Open navigation")
                        contentItem: GalleryIcon {
                            name: "menu"
                        }
                        onClicked: mobileNavigation.open()
                    }
                    GalleryLogo {
                        objectName: "headerBrandLogo"
                        visible: window.compact
                        compact: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                    }
                    CatLabel {
                        text: window.compact ? "QCatUiKit" : window.pageTitle
                        font.pixelSize: window.compact ? 16 : 18
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    CatTextField {
                        id: desktopSearch
                        objectName: "componentSearch"
                        visible: !window.compact
                        Layout.preferredWidth: Math.min(300, window.width * 0.25)
                        placeholderText: qsTr("Search components…")
                        text: window.searchQuery
                        onTextEdited: window.searchQuery = text
                        Accessible.name: qsTr("Search components")
                    }
                    GalleryComboBox {
                        objectName: "languageSelector"
                        Layout.preferredWidth: 108
                        model: ["English", "中文"]
                        currentIndex: GalleryLanguage.language === "zh_CN" ? 1 : 0
                        Accessible.name: qsTr("Language")
                        onActivated: index => GalleryLanguage.language = index === 1 ? "zh_CN" : "en"
                    }
                    CatButton {
                        implicitWidth: 36
                        padding: 6
                        Accessible.name: Theme.colorScheme === ThemePalette.Dark ? qsTr("Switch to light theme") : qsTr("Switch to dark theme")
                        contentItem: GalleryIcon {
                            name: Theme.colorScheme === ThemePalette.Dark ? "sun" : "moon"
                        }
                        onClicked: Theme.colorScheme = Theme.colorScheme === ThemePalette.Dark ? ThemePalette.Light : ThemePalette.Dark
                    }
                }
            }
            CatTextField {
                visible: window.compact
                Layout.fillWidth: true
                Layout.margins: 16
                placeholderText: qsTr("Search components…")
                text: window.searchQuery
                onTextEdited: window.searchQuery = text
                Accessible.name: qsTr("Search components")
            }
            CatScrollView {
                id: contentScroll
                objectName: "galleryContentScroll"
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth
                clip: true
                onContentItemChanged: if (contentItem)
                    contentItem.boundsBehavior = Flickable.StopAtBounds
                ColumnLayout {
                    width: contentScroll.availableWidth
                    spacing: 0
                    Loader {
                        id: pageLoader
                        Layout.fillWidth: true
                        Layout.margins: window.compact ? 20 : 32
                        sourceComponent: window.pageId === "home" ? homePage : window.pageId === "settings" ? settingsPage : window.currentComponent ? componentPage : explorerPage
                        onLoaded: if (contentScroll.contentItem)
                            contentScroll.contentItem.contentY = 0
                    }
                    Item {
                        Layout.preferredHeight: 24
                    }
                }
            }
        }
    }
    Component {
        id: homePage
        GalleryHome {
            onSelectComponent: id => window.showComponent(id)
            onBrowse: category => window.navigate("components", category)
        }
    }
    Component {
        id: settingsPage
        GallerySettingsPage {}
    }
    Component {
        id: componentPage
        GalleryComponentPage {
            entry: window.currentComponent
            onBack: window.navigate("components", window.categoryId)
        }
    }
    Component {
        id: explorerPage
        ColumnLayout {
            spacing: 20
            CatLabel {
                objectName: "galleryPageTitle"
                text: window.searchQuery ? qsTr("Search results") : GalleryCatalog.categoryTitle(window.categoryId)
                font.pixelSize: 30
                font.bold: true
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            CatLabel {
                text: window.searchQuery ? qsTr("%1 results for “%2”").arg(window.matchingComponents.length).arg(window.searchQuery) : qsTr("Choose a component to explore its behavior and QML example.")
                color: Theme.tokens.textSecondary
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            GridLayout {
                visible: window.matchingComponents.length > 0
                Layout.fillWidth: true
                columns: window.width - (window.compact ? 40 : 296) >= 840 ? 3 : window.width - (window.compact ? 40 : 296) >= 500 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 16
                uniformCellWidths: true
                Repeater {
                    model: window.matchingComponents
                    delegate: GalleryEntryCard {
                        required property var modelData
                        entry: modelData
                        Layout.fillWidth: true
                        onClicked: window.showComponent(modelData.id)
                    }
                }
            }
            CatPane {
                objectName: "searchEmptyState"
                visible: window.matchingComponents.length === 0
                Layout.fillWidth: true
                contentItem: ColumnLayout {
                    CatLabel {
                        Layout.fillWidth: true
                        text: qsTr("No components found. Try a different name.")
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }
    CatDrawer {
        id: mobileNavigation
        objectName: "galleryNavigationDrawer"
        width: Math.min(288, window.width - 40)
        height: window.height
        locale: window.locale
        contentItem: CatScrollView {
            id: mobileNavigationScroll
            objectName: "mobileNavigationScroll"
            contentWidth: availableWidth
            clip: true
            GalleryNavigation {
                width: mobileNavigationScroll.availableWidth
                height: Math.max(implicitHeight, mobileNavigationScroll.availableHeight)
                pageId: window.pageId
                categoryId: window.categoryId
                onNavigate: (page, category) => window.navigate(page, category)
            }
        }
    }
}
