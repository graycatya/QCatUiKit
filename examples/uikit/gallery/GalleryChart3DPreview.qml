// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: control
    readonly property string exampleCode: Examples.code(19, {
        type: type, polar: polar && !(type === 3 && surfaceInput === 2), shape: shape, meshDetail: meshDetail,
        lightingModel: lightingModel, surface: surface, orthographic: orthographic,
        ambient: ambient.value, diffuse: diffuse.value, specular: specular.value, shininess: shininess.value,
        ground: groundFill.value, exposure: exposure.value,
        curveClosed: curveClosed, curveTension: curveTension, curveSteps: curveSteps
    })
    objectName: "chart3DPreview"
    property int type: 0
    property bool polar: false
    property int shape: 0
    property int meshDetail: CatChartSeries3D.MediumDetail
    property int lightingModel: CatChartScene3D.BlinnPhong
    property int surface: 2
    property int surfaceInput: 0
    property bool secondBars: true
    property bool curveClosed: false
    property real curveTension: 0
    property int curveSteps: 12
    property bool orthographic: false
    property int slice: 0
    property int sliceIndex: 3
    spacing: 12
    function populate() {
        const points = [];
        if (type === 3 && surfaceInput === 2) { data.setHeightMap(Qt.resolvedUrl("assets/chart-weave.png"), 2); return; }
        if (type === 2) {
            for (let i = 0; i < 90; ++i) {
                const t = i / 89 * Math.PI * 4;
                points.push({x: Math.cos(t), y: i / 45 - 1, z: Math.sin(t), label: qsTr("Node %1").arg(i)});
            }
            data.points = points;
        } else {
            const rows = type === 3 ? 21 : 9;
            const cols = type === 3 ? 40 : 12;
            for (let r = 0; r < rows; ++r) {
                for (let c = 0; c < cols; ++c) {
                    const x = c / (cols - 1) * 6 - 3;
                    const z = r / (rows - 1) * 6 - 3;
                    points.push({x: polar && type !== 1 ? c / cols * 360 : x,
                        y: Math.cos(x) * Math.sin(z), z: polar && type !== 1 ? r / (rows - 1) * 3 : z,
                        valid: !(type === 3 && r === 9 && c === 15),
                        label: qsTr("Row %1, column %2").arg(r).arg(c)});
                }
            }
            data.setGrid(rows, cols, points);
            if (type === 1) comparison.setGrid(rows, cols, points.map(p => ({x:p.x,y:p.y*0.6+0.2,z:p.z})));
        }
        chart.scene3D.clearSelection();
    }
    Component.onCompleted: populate()
    onTypeChanged: { if (type === 1 && shape === 0) shape = 1; populate(); }
    onPolarChanged: populate()
    onSurfaceInputChanged: populate()
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 700 ? 3 : 1
        columnSpacing: 12
        rowSpacing: 8
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("3D chart type") }
            CatComboBox {
                objectName: "chart3DType"
                Layout.fillWidth: true
                textRole: "text"; valueRole: "value"
                model: [{text: qsTr("3D scatter"), value: 0}, {text: qsTr("3D bars"), value: 1}, {text: qsTr("3D curve"), value: 2}, {text: qsTr("3D surface"), value: 3}]
                currentValue: control.type
                onActivated: index => control.type = index
                Accessible.name: qsTr("3D chart type")
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("3D point shape") }
            CatComboBox {
                objectName: "chart3DShape"
                Layout.fillWidth: true
                enabled: control.type !== 3
                textRole: "text"; valueRole: "value"
                model: control.type === 1
                    ? [{text: qsTr("Cube"), value: 1}, {text: qsTr("Cylinder"), value: 2}, {text: qsTr("Custom tetrahedron"), value: 3}, {text: qsTr("Rounded cube"), value: 4}]
                    : [{text: qsTr("Sphere"), value: 0}, {text: qsTr("Cube"), value: 1}, {text: qsTr("Cylinder"), value: 2}, {text: qsTr("Custom tetrahedron"), value: 3}, {text: qsTr("Rounded cube"), value: 4}]
                currentValue: control.shape
                onActivated: index => control.shape = model[index].value
                Accessible.name: qsTr("3D point shape")
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Surface drawing") }
            CatComboBox {
                objectName: "chart3DSurfaceMode"
                Layout.fillWidth: true
                enabled: control.type === 3
                textRole: "text"; valueRole: "value"
                model: [{text: qsTr("Filled"), value: 0}, {text: qsTr("Wireframe"), value: 1}, {text: qsTr("Filled and wireframe"), value: 2}]
                currentValue: control.surface
                onActivated: index => control.surface = index
                Accessible.name: qsTr("Surface drawing")
            }
        }
    }
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 700 ? 3 : 1
        columnSpacing: 12
        rowSpacing: 8
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Lighting model") }
            CatComboBox {
                objectName: "chart3DLightingModel"; Layout.fillWidth: true
                textRole: "text"; valueRole: "value"
                model: [{text: qsTr("Blinn–Phong"), value: CatChartScene3D.BlinnPhong},
                    {text: qsTr("Phong (legacy)"), value: CatChartScene3D.Phong}]
                currentValue: control.lightingModel
                onActivated: index => control.lightingModel = model[index].value
                Accessible.name: qsTr("Lighting model")
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Mesh detail") }
            CatComboBox {
                objectName: "chart3DMeshDetail"; Layout.fillWidth: true
                enabled: control.type !== 3 && control.shape !== CatChartSeries3D.CustomMesh && control.shape !== CatChartSeries3D.Cube
                textRole: "text"; valueRole: "value"
                model: [{text: qsTr("Low"), value: CatChartSeries3D.LowDetail},
                    {text: qsTr("Medium"), value: CatChartSeries3D.MediumDetail},
                    {text: qsTr("High"), value: CatChartSeries3D.HighDetail}]
                currentValue: control.meshDetail
                onActivated: index => control.meshDetail = model[index].value
                Accessible.name: qsTr("Mesh detail")
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Shininess: %1").arg(shininess.value.toFixed(0)) }
            CatSliderField {
                id: shininess; objectName: "chart3DShininess"; Layout.fillWidth: true; from: 1; to: 256; stepSize: 1; value: 32; Accessible.name: qsTr("Shininess")
                decimals: 0; defaultValue: 32; tickValues: [1, 32, 64, 128, 256]
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
    }
    Flow {
        Layout.fillWidth: true
        spacing: 10
        CatCheckBox {
            objectName: "chart3DSecondBars"; visible: control.type === 1
            text: qsTr("Second bar series"); checked: control.secondBars
            onClicked: control.secondBars = checked
        }
        CatCheckBox {
            objectName: "chart3DPolar"
            text: qsTr("Polar coordinates (degrees)")
            enabled: control.type === 0 || (control.type === 3 && control.surfaceInput !== 2)
            checked: control.polar
            onClicked: control.polar = checked
        }
        CatCheckBox {
            objectName: "chart3DOrthographic"
            text: qsTr("Orthographic projection")
            checked: control.orthographic
            onClicked: control.orthographic = checked
        }
        CatButton { text: qsTr("Isometric"); onClicked: chart.scene3D.setPreset(CatChartScene3D.Isometric) }
        CatButton { text: qsTr("Front"); onClicked: chart.scene3D.setPreset(CatChartScene3D.Front) }
        CatButton { text: qsTr("Top"); onClicked: chart.scene3D.setPreset(CatChartScene3D.Top) }
        CatButton { objectName: "chart3DReset"; text: qsTr("Reset camera"); onClicked: { control.orthographic = false; chart.scene3D.resetView(); } }
    }
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 700 ? 3 : 1
        columnSpacing: 12
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Ambient: %1").arg(ambient.value.toFixed(2)) }
            CatSliderField {
                id: ambient; objectName: "chart3DAmbient"; Layout.fillWidth: true; from: 0; to: 1; value: 0.25; Accessible.name: qsTr("Ambient")
                decimals: 2; stepSize: 0.01; defaultValue: 0.25; tickInterval: 0.25
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Diffuse: %1").arg(diffuse.value.toFixed(2)) }
            CatSliderField {
                id: diffuse; objectName: "chart3DDiffuse"; Layout.fillWidth: true; from: 0; to: 1; value: 0.7; Accessible.name: qsTr("Diffuse")
                decimals: 2; stepSize: 0.01; defaultValue: 0.7; tickInterval: 0.25
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Specular: %1").arg(specular.value.toFixed(2)) }
            CatSliderField {
                id: specular; objectName: "chart3DSpecular"; Layout.fillWidth: true; from: 0; to: 1; value: 0.5; Accessible.name: qsTr("Specular")
                decimals: 2; stepSize: 0.01; defaultValue: 0.5; tickInterval: 0.25
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
    }
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 700 ? 3 : 1
        columnSpacing: 12
        ColumnLayout {
            Layout.fillWidth: true
            enabled: control.lightingModel === CatChartScene3D.BlinnPhong
            CatLabel { text: qsTr("Ground fill: %1").arg(groundFill.value.toFixed(2)) }
            CatSliderField {
                id: groundFill; objectName: "chart3DGroundFill"; Layout.fillWidth: true; from: 0; to: 1; value: 0.6; Accessible.name: qsTr("Ground fill")
                decimals: 2; stepSize: 0.01; defaultValue: 0.6; tickInterval: 0.25
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            enabled: control.lightingModel === CatChartScene3D.BlinnPhong
            CatLabel { text: qsTr("Exposure: %1").arg(exposure.value.toFixed(2)) }
            CatSliderField {
                id: exposure; objectName: "chart3DExposure"; Layout.fillWidth: true; from: 0; to: 4; value: 1; Accessible.name: qsTr("Exposure")
                decimals: 2; stepSize: 0.01; defaultValue: 1; tickInterval: 1
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
        CatButton {
            objectName: "chart3DResetLighting"
            text: qsTr("Reset lighting")
            onClicked: {
                control.lightingModel = CatChartScene3D.BlinnPhong;
                ambient.reset(); diffuse.reset(); specular.reset();
                shininess.reset(); groundFill.reset(); exposure.reset();
            }
        }
    }
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 700 ? 3 : 1
        columnSpacing: 12
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Grid selection") }
            CatComboBox {
                objectName: "chart3DSelection"
                Layout.fillWidth: true
                textRole: "text"; valueRole: "value"
                model: [{text: qsTr("Point"), value: 0}, {text: qsTr("Row"), value: 1}, {text: qsTr("Column"), value: 2}]
                currentValue: chart.scene3D.selectionMode - 1
                onActivated: index => chart.scene3D.selectionMode = index + 1
                Accessible.name: qsTr("Grid selection")
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Grid slice") }
            CatComboBox {
                objectName: "chart3DSlice"
                Layout.fillWidth: true
                enabled: control.type === 1 || control.type === 3
                textRole: "text"; valueRole: "value"
                model: [{text: qsTr("Full scene"), value: 0}, {text: qsTr("Row slice"), value: 1}, {text: qsTr("Column slice"), value: 2}]
                currentValue: control.slice
                onActivated: index => control.slice = index
                Accessible.name: qsTr("Grid slice")
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Slice index (zero-based)") }
            CatSpinBox {
                objectName: "chart3DSliceIndex"
                Layout.fillWidth: true
                enabled: control.slice !== 0
                from: 0; to: control.slice === 1 ? Math.max(0, data.rows - 1) : Math.max(0, data.columns - 1)
                value: Math.min(control.sliceIndex, to)
                onValueModified: control.sliceIndex = value
                Accessible.name: qsTr("Slice index (zero-based)")
            }
        }
    }
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 700 ? 3 : 1
        columnSpacing: 12
        visible: control.type === 2 || control.type === 3
        ColumnLayout {
            Layout.fillWidth: true; visible: control.type === 3
            CatLabel { text: qsTr("Surface input") }
            CatComboBox {
                objectName: "chart3DSurfaceInput"; Layout.fillWidth: true
                textRole: "text"; valueRole: "value"
                model: [{text:qsTr("Generated grid"),value:0},{text:qsTr("Texture surface"),value:1},{text:qsTr("Image height map"),value:2}]
                currentValue: control.surfaceInput
                onActivated: index => control.surfaceInput = index
                Accessible.name: qsTr("Surface input")
            }
        }
        CatCheckBox {
            objectName: "chart3DClosedCurve"; visible: control.type === 2
            text: qsTr("Closed trajectory"); checked: control.curveClosed
            onClicked: control.curveClosed = checked
        }
        ColumnLayout {
            Layout.fillWidth: true; visible: control.type === 2
            CatLabel { text: qsTr("Trajectory tension: %1").arg(control.curveTension.toFixed(2)) }
            CatSliderField {
                objectName: "chart3DCurveTension"; Layout.fillWidth: true; from:0; to:1; value:control.curveTension; onValueEdited: value => control.curveTension=value; Accessible.name:qsTr("Trajectory tension")
                decimals: 2; stepSize: 0.01; defaultValue: 0; tickInterval: 0.25
                valueToolTipVisible: true; resetVisible: true; inputWidth: 90
                locale: Qt.locale(GalleryLanguage.localeName)
            }
        }
        ColumnLayout {
            Layout.fillWidth: true; visible: control.type === 2
            CatLabel { text: qsTr("Subdivisions per segment") }
            CatSpinBox { objectName:"chart3DCurveSteps"; Layout.fillWidth:true; from:1; to:64; value:control.curveSteps; onValueModified:control.curveSteps=value; Accessible.name:qsTr("Subdivisions per segment") }
        }
    }
    CatLabel {
        Layout.fillWidth: true
        Layout.preferredWidth: 0
        wrapMode: Text.Wrap
        text: qsTr("Drag to rotate; Shift-drag to pan; wheel to zoom. Touch: one finger rotates, two fingers pan and zoom. Click a visible point to select it.")
        color: Theme.tokens.textSecondary
    }
    CatChart {
        id: chart
        objectName: "chart3DItem"
        Layout.fillWidth: true
        Layout.preferredHeight: control.width >= 700 ? 520 : 380
        title: qsTr("3D data and lighting")
        scene3D.enabled: true
        scene3D.lightingModel: control.lightingModel
        scene3D.shininess: shininess.value
        scene3D.ambientGroundRatio: groundFill.value
        scene3D.exposure: exposure.value
        scene3D.ambient: ambient.value
        scene3D.diffuse: diffuse.value
        scene3D.specular: specular.value
        scene3D.projection: control.orthographic ? CatChartScene3D.Orthographic : CatChartScene3D.Perspective
        scene3D.sliceMode: (control.type === 1 || control.type === 3) ? control.slice : CatChartScene3D.NoSlice
        scene3D.sliceIndex: control.sliceIndex
        scene3D.series: [CatChartSeries3D {
            id: data
            objectName: "chart3DSeries"
            type: control.type
            name: qsTr("3D data")
            color: Theme.tokens.primary
            shape: control.shape
            meshDetail: control.meshDetail
            pointSize: control.type === 2 ? 0.08 : 0.095
            surfaceMode: control.surface
            colorByHeight: true
            nodesVisible: true
            polar: control.polar && !(control.type === 3 && control.surfaceInput === 2)
            closed: control.type === 2 ? control.curveClosed : control.type === 3 && control.polar && control.surfaceInput !== 2
            tension: control.curveTension
            subdivisions: control.curveSteps
            textureSource: control.type === 3 && control.surfaceInput === 1 ? Qt.resolvedUrl("assets/chart-weave.png") : ""
            customMesh: [Qt.vector3d(0,0.6,0), Qt.vector3d(-0.5,-0.4,0.4), Qt.vector3d(0.5,-0.4,0.4),
                Qt.vector3d(0,0.6,0), Qt.vector3d(0.5,-0.4,0.4), Qt.vector3d(0,-0.4,-0.5),
                Qt.vector3d(0,0.6,0), Qt.vector3d(0,-0.4,-0.5), Qt.vector3d(-0.5,-0.4,0.4),
                Qt.vector3d(-0.5,-0.4,0.4), Qt.vector3d(0,-0.4,-0.5), Qt.vector3d(0.5,-0.4,0.4)]
        }, CatChartSeries3D {
            id: comparison
            type: CatChartSeries3D.Bars
            name: qsTr("Comparison")
            visible: control.type === 1 && control.secondBars
            color: Theme.tokens.warning
            shape: control.shape
            meshDetail: control.meshDetail
            customMesh: data.customMesh
        }]
    }
    CatLabel {
        objectName: "chart3DSelectedLabel"
        Layout.fillWidth: true
        Layout.preferredWidth: 0
        wrapMode: Text.Wrap
        text: chart.scene3D.selectedPoint.index !== undefined
            ? qsTr("Selected point %1: (%2, %3, %4)").arg(chart.scene3D.selectedPoint.index).arg(chart.scene3D.selectedPoint.x.toFixed(2)).arg(chart.scene3D.selectedPoint.y.toFixed(2)).arg(chart.scene3D.selectedPoint.z.toFixed(2))
            : qsTr("No point selected")
    }
}
