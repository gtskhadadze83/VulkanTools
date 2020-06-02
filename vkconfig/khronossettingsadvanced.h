#ifndef KHRONOSSETTINGSADVANCED_H
#define KHRONOSSETTINGSADVANCED_H

/*
 * Copyright (c) 2020 Valve Corporation
 * Copyright (c) 2020 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Richard S. Wright Jr. <richard@lunarg.com>
 *
 * Khronos layer advanced settings dialog.
 *
 */

#include <QWidget>
#include <QTreeWidgetItem>
#include <QRadioButton>

#include <layerfile.h>

namespace Ui {
class KhronosSettingsAdvanced;
}

class KhronosSettingsAdvanced : public QWidget
{
    Q_OBJECT

public:
    explicit KhronosSettingsAdvanced(QWidget *parent,  QVector<TLayerSettings *>& layerSettings, QString qsText);
    ~KhronosSettingsAdvanced();

    bool CollectSettings(void);
    void SetEnabled(bool bEnabled);

private:
    Ui::KhronosSettingsAdvanced *ui;
    QTreeWidgetItem *pCoreChecksParent;

    TLayerSettings *pDisables;
    TLayerSettings *pEnables;

    QTreeWidgetItem *pShaderBasedBox;
    QTreeWidgetItem *pGPUAssistBox;
    QRadioButton    *pGPURadio;
    QTreeWidgetItem *pReserveBox;
    QTreeWidgetItem *pDebugPrintfBox;
    QRadioButton    *pDebugRadio;

public Q_SLOTS:

    void itemChanged(QTreeWidgetItem *pItem, int nColumn);
    void itemClicked(QTreeWidgetItem *pItem, int nColumn);
    void gpuToggled(bool toggle);
    void printfToggled(bool toggle);

};

#endif // KHRONOSSETTINGSADVANCED_H