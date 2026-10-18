// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery
TestCase {
    id:test; name:"GalleryM4"; when:windowShown
    property var gallery
    property var example
    Component { id:factory; Main { width:1200; height:840 } }
    function init() { failOnWarning(/.*/); GalleryLanguage.language="en";gallery=factory.createObject(test); }
    function cleanup() { if(example)example.destroy();example=null;gallery.close();gallery.destroy(); }
    function test_modes_data() { return [24,25,26,27,28,29].map(mode=>({tag:String(mode),mode:mode})); }
    function test_modes(data) {
        verify(gallery.showChartDemo(data.mode));wait(30);
        const preview=findChild(gallery.contentItem,"chartM4Preview");verify(preview!==null);
        const chart=preview.chart;tryVerify(()=>chart.series[0].count>0);
        let old=preview.exampleCode;
        const parameter=findChild(preview,"m4Parameter");parameter.valueEdited(data.mode===27?5:data.mode===29?.5:2);
        tryVerify(()=>preview.exampleCode!==old);old=preview.exampleCode;
        preview.optionA=!preview.optionA;preview.optionB=!preview.optionB;preview.optionC=!preview.optionC;
        tryVerify(()=>preview.exampleCode!==old);wait(30);
        example=Qt.createQmlObject("import QtQuick\nimport QCatUiKit\n"+preview.exampleCode,test,"m4_example_"+data.mode+".qml");verify(example!==null);example.visible=false;
        tryVerify(()=>example.chart.series.length>0&&example.chart.series[0].count>0);
        if(data.mode===24) {compare(preview.adapter.series.type,CatChartSeries.Ohlc);compare(preview.adapter.compressedTime,false);compare(chart.series[0].visible,false);}
        if(data.mode===25) {compare(preview.adapter.series.at(0).source,"summary");compare(preview.adapter.outlierSeries.visible,false);}
        if(data.mode===26) {
            compare(preview.adapter.dense,false);verify(preview.adapter.series.count<768);
            preview.adapter.colorScale.setRange(-2,2);
            const before=preview.adapter.values[132];findChild(preview,"m4CellUpdate").clicked();verify(preview.adapter.values[132]!==before);
            const origin=preview.adapter.origin.y;findChild(preview,"m4AppendRow").clicked();compare(preview.adapter.origin.y,origin+1);
        }
        if(data.mode===27) {verify(chart.series[0].count>0);verify(!chart.series[0].at(0).filled);}
        if(data.mode===28) {compare(preview.adapter.sizeMode,CatChartBubble.Area);compare(preview.adapter.colorMapping,false);}
        if(data.mode===29) {compare(chart.radarRanges.length,5);compare(chart.series[0].strokeStyle,CatChartSeries.DashLine);}
        GalleryLanguage.language="zh_CN";wait(20);verify(preview.exampleCode.length>100);
        gallery.width=420;Theme.density=ThemePalette.Mobile;wait(20);
        verify(parameter.width<=preview.width);verify(chart.width<=preview.width);
    }
}
