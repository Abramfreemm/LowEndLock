Low-End Lock 1.0.1 — Beta (macOS)
=================================

一个一键修复 Kick/Bass 低频反相抵消的插件：不是「压音量躲开」，
而是「相位对齐」把被抵消的低频能量找回来。

安装
----
1. 把 LowEndLock.component 复制到：
     ~/Library/Audio/Plug-Ins/Components/
2. 把 LowEndLock.vst3 复制到：
     ~/Library/Audio/Plug-Ins/VST3/
3. 重启宿主。

快速上手
--------
1. 在 Bass 轨插入 LowEndLock。
2. 在宿主顶部侧链菜单选择 Kick 轨。
3. 播放 kick+bass，点「Lock」。
4. 看「Cancelation Saved」数字，用 A/B 对比前后，或切 Manual 手动调。

注意
----
- Logic 使用 AU；Ableton / REAPER 使用 AU 或 VST3。
- 本版本固有约 20ms 延迟（相位对齐需要），适合混音、不适合实时监听。
