// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QCatUiKit

CatButton {
    text: "Installed Qt module"
    property string installedVersion: QCatUiKit.version
    property QtObject installedModel: TableModel {}
}
