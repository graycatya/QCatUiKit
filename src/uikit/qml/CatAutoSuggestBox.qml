// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatTextField {
    id: control
    property var suggestions: []
    readonly property var filteredSuggestions: suggestions.filter(entry => String(
                                                                               entry).toLocaleLowerCase(
                                                                               ).includes(
                                                                               text.toLocaleLowerCase(
                                                                                   )))
    readonly property bool isSuggestionListOpen: suggestionsPopup.opened
    signal suggestionAccepted(string suggestion)
    function choose(index: int): void {
        if (index < 0 || index >= filteredSuggestions.length)
            return;
        const chosen = String(filteredSuggestions[index]);
        text = chosen;
        suggestionsPopup.close();
        suggestionAccepted(chosen);
    }
    onTextEdited: {
        if (text.length && filteredSuggestions.length)
            suggestionsPopup.open();
        else
            suggestionsPopup.close();
    }
    Keys.onDownPressed: event => {
        if (filteredSuggestions.length) {
            suggestionsPopup.open();
            options.currentIndex = Math.min(options.count - 1, options.currentIndex + 1);
            event.accepted = true;
        }
    }
    Keys.onUpPressed: event => {
        options.currentIndex = Math.max(0, options.currentIndex - 1);
        event.accepted = true;
    }
    Keys.onReturnPressed: event => {
        if (suggestionsPopup.visible) {
            choose(Math.max(0, options.currentIndex));
            event.accepted = true;
        } else
            event.accepted = false;
    }
    CatPopup {
        id: suggestionsPopup
        theme: control.theme
        y: control.height + 4
        width: control.width
        padding: 4
        height: Math.min(240, options.contentHeight + 8)
        modal: false
        focus: false
        contentItem: ListView {
            id: options
            model: control.filteredSuggestions
            clip: true
            currentIndex: 0
            delegate: CatButton {
                required property string modelData
                required property int index
                theme: control.theme
                text: modelData
                width: options.width
                flat: true
                checked: options.currentIndex === index
                onClicked: control.choose(index)
            }
        }
    }
}
