// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GalleryContracts"
    when: windowShown
    property var gallery
    Component {
        id: windowFactory
        Main {
            width: 1200
            height: 840
        }
    }

    function init() {
        failOnWarning(/.*/);
        GalleryLanguage.language = "en";
        wait(10);
        gallery = createTemporaryObject(windowFactory, test);
        verify(gallery !== null);
        verify(waitForRendering(gallery.contentItem));
    }
    function cleanup() {
        gallery.close();
        GalleryLanguage.language = "en";
        wait(10);
        Theme.reducedMotion = false;
    }
    function findControl(name) {
        const item = findChild(gallery.contentItem, name) || findChild(gallery, name);
        verify(item !== null, "Missing gallery control: " + name);
        return item;
    }
    function preview() {
        verify(waitForRendering(gallery.contentItem));
        wait(10);
        const loader = findControl("componentPreview");
        verify(loader !== null);
        tryCompare(loader, "status", Loader.Ready);
        verify(loader.item !== null);
        return loader;
    }
    function test_languageRoundTrip() {
        verify(gallery.title.includes("Component Gallery"));
        compare(findControl("navigation_home").text, "Overview");
        GalleryLanguage.language = "zh_CN";
        wait(10);
        compare(GalleryLanguage.localeName, "zh_CN");
        compare(findControl("navigation_home").text, "概览");
        compare(findControl("componentSearch").placeholderText, "搜索组件…");
        verify(gallery.title.includes("组件展示库"));
        compare(qsTranslate("CatPagination", "Next"), "下一页");
        compare(qsTranslate("CatChatInput", "Send"), "发送");
        GalleryLanguage.language = "en";
        wait(10);
        compare(findControl("navigation_home").text, "Overview");
        compare(qsTranslate("CatChatInput", "Send"), "Send");
        GalleryLanguage.language = "unsupported";
        compare(GalleryLanguage.language, "en");
    }
    function test_languagePreservesInput() {
        gallery.showComponent("textfield");
        preview();
        const field = findControl("demoTextField");
        field.text = "我的输入 / keep this text";
        GalleryLanguage.language = "zh_CN";
        wait(10);
        compare(gallery.componentId, "textfield");
        compare(field.text, "我的输入 / keep this text");
        compare(field.placeholderText, "输入一些内容…");
        compare(findControl("galleryPageTitle").text, "文本输入");
        GalleryLanguage.language = "en";
        wait(10);
        compare(field.text, "我的输入 / keep this text");
        compare(field.placeholderText, "Type something…");
    }
    function test_densityKeepsHeaderSearchAligned() {
        const search = findControl("componentSearch");
        search.forceActiveFocus();
        keyClick(Qt.Key_C);
        keyClick(Qt.Key_A);
        keyClick(Qt.Key_T);
        compare(search.text, "cat");
        compare(gallery.searchQuery, "cat");
        search.select(1, 3);
        for (const density of [ThemePalette.Touch, ThemePalette.Mobile, ThemePalette.Desktop,
                               ThemePalette.Mobile, ThemePalette.Touch, ThemePalette.Desktop]) {
            Theme.density = density;
            verify(waitForRendering(gallery.contentItem));
            compare(search.text, "cat");
            compare(gallery.searchQuery, "cat");
            compare(search.selectedText, "at");
            compare(search.activeFocus, true);
            const cursor = search.cursorRectangle;
            fuzzyCompare(cursor.y + cursor.height / 2, search.height / 2, 1,
                         "Header search text must stay centered at density " + density);
        }
        gallery.searchQuery = "no-such-component";
        compare(gallery.matchingComponents.length, 0);
        const empty = findControl("searchEmptyState");
        for (const density of [ThemePalette.Touch, ThemePalette.Mobile, ThemePalette.Desktop]) {
            Theme.density = density;
            verify(waitForRendering(gallery.contentItem));
            const cursor = search.cursorRectangle;
            fuzzyCompare(cursor.y + cursor.height / 2, search.height / 2, 1);
            compare(empty.visible, true);
            const position = empty.mapToItem(gallery.contentItem, 0, 0);
            verify(empty.height > 0);
            verify(position.y + empty.height <= gallery.height,
                   "Empty search results must not retain the previous result grid's height");
        }
    }
    function test_languagePreservesExtendedEditors_data() {
        return [{ tag: "multiline", id: "text-edit", name: "demoTextArea" },
                { tag: "tab_draft", id: "tab-view", name: "demoTabDraft" }];
    }
    function test_languagePreservesExtendedEditors(data) {
        gallery.showComponent(data.id);
        preview();
        const editor = findControl(data.name);
        editor.text = "保存我的草稿 / keep my draft";
        GalleryLanguage.language = "zh_CN";
        wait(10);
        compare(editor.text, "保存我的草稿 / keep my draft");
        GalleryLanguage.language = "en";
        wait(10);
        compare(editor.text, "保存我的草稿 / keep my draft");
    }
    function test_languagePreservesConversation() {
        gallery.showComponent("chat");
        preview();
        const input = findControl("demoChatInput");
        const model = findControl("demoChatModel");
        input.text = "my message";
        input.submit();
        input.text = "unsent draft";
        compare(model.count, 2);
        GalleryLanguage.language = "zh_CN";
        wait(10);
        compare(gallery.componentId, "chat");
        compare(model.count, 2);
        compare(model.get(1).content, "my message");
        compare(input.text, "unsent draft");
        compare(input.placeholderText, "输入消息");
        compare(model.get(0).content, "欢迎！试试在编写消息时切换语言。");
        compare(model.get(1).username, "你");
    }
    function test_calendarLocale() {
        gallery.showComponent("calendar");
        preview();
        const calendar = findControl("calendarPreview");
        calendar.selectDate(new Date(2026, 9, 11));
        GalleryLanguage.language = "zh_CN";
        wait(10);
        compare(calendar.locale.name, "zh_CN");
        compare(calendar.selectedDate.getDate(), 11);
        compare(calendar.selectedDate.getMonth(), 9);
        GalleryLanguage.language = "en";
        wait(10);
        compare(calendar.locale.name, "en_US");
        compare(calendar.selectedDate.getDate(), 11);
    }
    function test_bilingualSearch() {
        gallery.navigate("components", "all");
        gallery.searchQuery = "CatBusyIndicator";
        compare(gallery.matchingComponents.length, 1);
        compare(gallery.matchingComponents[0].id, "busy");
        GalleryLanguage.language = "zh_CN";
        wait(10);
        gallery.searchQuery = "日历";
        verify(gallery.matchingComponents.some(entry => entry.id === "calendar"));
        verify(gallery.matchingComponents.some(entry => entry.id === "calendar-date-picker"));
        gallery.searchQuery = "no-such-component";
        compare(gallery.matchingComponents.length, 0);
    }
    function test_marqueeScrolls_data() {
        return [
            {
                tag: "en_desktop",
                language: "en",
                width: 1200
            },
            {
                tag: "zh_desktop",
                language: "zh_CN",
                width: 1200
            },
            {
                tag: "en_mobile",
                language: "en",
                width: 390
            },
            {
                tag: "zh_mobile",
                language: "zh_CN",
                width: 390
            }
        ];
    }
    function test_marqueeScrolls(data) {
        gallery.width = data.width;
        GalleryLanguage.language = data.language;
        wait(10);
        gallery.showComponent("marquee");
        preview();
        const marquee = findControl("marqueePreview");
        const label = marquee.children[0];
        verify(marquee.width <= marquee.parent.width);
        verify(label.width > marquee.width, "Marquee example must overflow in both languages");
        tryVerify(() => label.x < -1, 2500, "Marquee example must actually scroll");
        Theme.reducedMotion = true;
        tryCompare(label, "x", 0);
        wait(100);
        compare(label.x, 0);
    }
    function test_overlayLanguage_data() {
        return [
            {
                tag: "popup",
                id: "popup",
                button: "openPopupButton",
                overlay: "demoPopup"
            },
            {
                tag: "drawer",
                id: "drawer",
                button: "openDrawerButton",
                overlay: "demoDrawer"
            }
        ];
    }
    function test_overlayLanguage(data) {
        gallery.showComponent(data.id);
        preview();
        mouseClick(findControl(data.button));
        const overlay = findControl(data.overlay);
        tryCompare(overlay, "opened", true);
        GalleryLanguage.language = "zh_CN";
        wait(10);
        compare(overlay.opened, true);
        compare(gallery.componentId, data.id);
        overlay.close();
    }
    function test_mobileNavigation_data() {
        const rows = [];
        for (const language of ["en", "zh_CN"])
            for (const width of [390, 800])
                for (const reducedMotion of [false, true])
                    rows.push({
                        tag: language + "_" + width + "_" + reducedMotion,
                        language: language,
                        width: width,
                        reducedMotion: reducedMotion
                    });
        return rows;
    }
    function test_mobileNavigation(data) {
        gallery.height = 540;
        gallery.width = data.width;
        GalleryLanguage.language = data.language;
        Theme.reducedMotion = data.reducedMotion;
        tryCompare(gallery, "compact", true);
        gallery.showComponent("button");
        preview();
        verify(findControl("codeSample").width > 0);
        const openButton = findControl("openNavigationButton");
        const drawer = findControl("galleryNavigationDrawer");
        mouseClick(openButton);
        tryCompare(drawer, "opened", true);
        compare(drawer.position, 1);
        const origin = drawer.background.mapToItem(null, 0, 0);
        verify(Math.abs(origin.x) < 1, "Navigation drawer must be on screen");
        verify(origin.y >= 0 && origin.y < gallery.height);
        verify(drawer.width > 0 && drawer.width <= gallery.width);
        GalleryLanguage.language = data.language === "en" ? "zh_CN" : "en";
        wait(10);
        compare(drawer.opened, true);
        compare(drawer.position, 1);
        compare(gallery.componentId, "button");
        const allComponents = findChild(drawer.contentItem, "navigation_components");
        verify(allComponents !== null);
        mouseClick(allComponents);
        tryCompare(drawer, "visible", false);
        compare(drawer.position, 0);
        compare(gallery.pageId, "components");
        compare(gallery.categoryId, "all");
        compare(gallery.componentId, "");
        mouseClick(openButton);
        tryCompare(drawer, "opened", true);
        compare(drawer.position, 1);
        mouseClick(gallery.contentItem, gallery.width - 8, 36);
        tryCompare(drawer, "visible", false);
        compare(drawer.position, 0);
        mouseClick(openButton);
        tryCompare(drawer, "opened", true);
        const scroll = drawer.contentItem;
        compare(scroll.objectName, "mobileNavigationScroll");
        scroll.contentItem.contentY = Math.max(0, scroll.contentItem.contentHeight - scroll.contentItem.height);
        wait(10);
        const settings = findChild(drawer.contentItem, "navigation_settings");
        verify(settings !== null);
        const settingsOrigin = settings.mapToItem(null, settings.width / 2, settings.height / 2);
        verify(settingsOrigin.y > 0 && settingsOrigin.y < gallery.height, "Short windows must allow scrolling to settings");
        mouseClick(settings);
        tryCompare(drawer, "visible", false);
        compare(gallery.pageId, "settings");
        mouseClick(openButton);
        tryCompare(drawer, "opened", true);
        gallery.width = 1200;
        tryCompare(gallery, "compact", false);
        tryCompare(drawer, "visible", false);
        compare(drawer.position, 0);
        gallery.width = data.width;
        tryCompare(gallery, "compact", true);
        wait(10);
        mouseClick(openButton);
        tryCompare(drawer, "opened", true);
        compare(drawer.position, 1);
        keyClick(Qt.Key_Escape);
        tryCompare(drawer, "visible", false);
        compare(drawer.position, 0);
    }
}
