# LED workflows and motion input

Read only the section relevant to the requested UI.

## LED 工作流

需要灯点布局、预设、双 Deck 或多通道输出时，阅读 `src/uikit/doc/LEDWORKFLOW.md`。
使用公共 `LedProject` 可选装配或独立注入 LayoutModel/Registry/Repository/Deck/Mixer；不要依赖
`GalleryLedDemo`。`LedDeck`、`LedMixer`、`LedStorageBrowser` 及三个设置面板可直接复用。
预览亮度与 Deck/Place/Master 不同；RGBA alpha 不代表 W。设备分支读取公共 LightingFrame，
不用截图或 renderer 私有头，MemoryOutputBackend 只表示内存验证。


## Aircraft attitude and tilt input

Use CatAttitudeIndicator for AIRCRAFT telemetry only; pitchDegrees/rollDegrees are degrees and attitudeValid is owned by the application.
TiltController accepts application-supplied device-to-reference quaternions and monotonic microsecond timestamps; no Qt Sensors or MAVLink dependency is provided.
CatTiltControlPanel binds state/parameters and emits requests; wire explicit calibration, hold-to-engage and disengagement, including application inactivity.
Ready-state rollInput/pitchInput are previews: only outputValid allows control use. The application owns acquisition, coordinate conversion, authority, freshness at the sender and protocol release.
See src/uikit/doc/ATTITUDE.md for the complete input convention, states and integration. Gallery simulation helpers are not library APIs.
