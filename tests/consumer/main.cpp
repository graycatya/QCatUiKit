// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/ChatModel.h>
#include <QCatUiKit/TableModel.h>
#include <QCatUiKit/ThemePalette.h>
#include <QCoreApplication>
#include <QDebug>
#ifdef Q_OS_UNIX
#include <dlfcn.h>
#endif
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCatUiKit::ThemePalette theme;
    theme.setDensity(QCatUiKit::ThemePalette::Touch);
    if (theme.tokens().value("controlHeight").toInt() != 48)
        return 1;
    QCatUiKit::ChatModel chat;
    chat.append({{"content", "binary consumer"}});
    QCatUiKit::TableModel table;
    table.setColumns({"a"});
    table.append({{"a", 42}});
    if (chat.count() != 1 || table.data(table.index(0, 0)).toInt() != 42)
        return 2;
#ifdef Q_OS_UNIX
    Dl_info info{};
    if (dladdr(&QCatUiKit::ThemePalette::staticMetaObject, &info))
        qInfo() << "Loaded:" << info.dli_fname;
#endif
    qInfo() << "Consumer OK; public object sizes:" << sizeof(theme) << sizeof(chat)
            << sizeof(table);
    return 0;
}
