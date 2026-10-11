// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

CatTextField {
    placeholderText: "192.168.1.1"
    inputMethodHints: Qt.ImhPreferNumbers | Qt.ImhNoPredictiveText
    validator: RegularExpressionValidator {
        regularExpression:
            /(?:(?:25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.){3}(?:25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])/
    }
    invalid: text.length > 0 && !acceptableInput && !activeFocus
}
