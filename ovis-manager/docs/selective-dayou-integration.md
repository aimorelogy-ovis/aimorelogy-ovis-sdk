# Dayou 算法选择性整合

## 选用范围

按用户选择整合 A、B、C、D，将 E、F 改成配置项并同步前端，排除 G、H、I、J。来源是按指定顺序叠加的四份 Dayou 补丁；适配基线为当前 `1857e6d57`，保留已完成的 UVC / RTSP 退出修复。

- A：VPSS 参数、裁剪、缩放设置与 Tensor 图像包装缓存。
- B：检测候选维护、关联、遮挡处理、预测显示及最近中心目标锁存。
- C：FastSAM 小目标候选评分、边界处理、跨尺度确认。
- D：FearTrack 多候选质量、全局及局部 GMC、尺度更新、模板更新与回滚、重捕身份保护，以及配套 TDL 接口和 Kalman 方法。
- E：通过配置选择点击位置或准星中心；通过另一个配置选择目标区域或固定 80×80 初始化种子。
- F：可选完整矩形或四角角标，继续使用现有跟踪颜色、丢失颜色和线宽；可选择丢失时隐藏。

没有引入 PIP 或其线程同步、新模型及安装规则、固定 AE 帧率、60 FPS 默认值、强制开启 AI、旧功能开关重置、默认网段修改、BNR 性能线程删减或 60 秒启动等待。现有模型、视频参数、跟踪分数配置、配网流程、MS7024 和子码流资源分配保留。

Dayou 的应用层跟踪代码原本依赖其常驻 NV12 通道布局。本次保留当前 OVIS 的共享通道准备、格式/缓存池切换、预热和恢复流程，使算法可以配合现有 INI 和 Manager 拓扑工作。

Dayou 新增的应用层逐帧 trace、SOT 内部 `flushDebugLog()` 文件写入和新增 `LOGIP` 状态输出已剥离。保留原有 `/tmp/object_track_fps` 输出格式以及可按需调用的算法诊断 API。

## 配置接口

`GET /api/v1/config/capabilities` 的 `schema_version` 为 8，增加：

```json
{
  "ai": {
    "features": [
      {
        "id": "single_object_tracking",
        "selection_modes": ["point", "reticle"],
        "initial_box_modes": ["target", "fixed_80"]
      }
    ]
  },
  "overlay": {
    "trackingBoxStyles": ["rectangle", "corners"],
    "trackingHideWhenLost": true
  }
}
```

上述 JSON 只展示新增能力字段。配置读写在原有对象上增加：

```json
{
  "tracking": {
    "single_object": {
      "selection_mode": "point",
      "initial_box_mode": "target"
    }
  },
  "overlay": {
    "tracking": {
      "boxStyle": "rectangle",
      "hideWhenLost": false
    }
  }
}
```

| API 字段 | INI 位置 | 默认值 | 应用方式 |
| --- | --- | --- | --- |
| `tracking.single_object.selection_mode` | `[ai_object_track_config] selection_mode` | `point` | 重启视频服务 |
| `tracking.single_object.initial_box_mode` | `[ai_object_track_config] initial_box_mode` | `target` | 重启视频服务 |
| `overlay.tracking.boxStyle` | `[osd_style] tracking_box_style` | `rectangle` | 仅修改 OSD 时热更新 |
| `overlay.tracking.hideWhenLost` | `[osd_style] tracking_hide_when_lost` | `0` | 仅修改 OSD 时热更新 |

旧 INI 只补齐缺失字段，不重置已有选择；旧客户端提交不含这些字段时继承当前配置。无效枚举或布尔值会被拒绝。前端只在设备公布对应能力时显示、校验和提交新选项，连接旧固件时不发送这些字段。

## E 的行为边界

- `point`：保留传入点的位置及原有 FastSAM / 颜色 / 框选流程，不再强制转换为中心目标。
- `reticle`：点选请求使用当前准星中心区域，直接初始化；按检测 ID 或显式框选仍使用对应目标。
- `target`：保留目标区域及既有分割细化方式。
- `fixed_80`：在选中位置附近使用跟踪输入坐标中的 80×80 种子，跳过初始化分割细化。边缘会向画面内约束，后续跟踪尺寸仍允许变化。
- 固定种子选项开启时，中心准星的预览区域同步换算到相应尺寸。
- `/tmp/det_track` 的最近中心检测目标选择保留，使用 B 提供的新鲜候选锁存；与直接按检测 ID 选择互不替代。

## F 的行为边界

矩形仍由原 VPSS 矩形路径绘制。选择四角样式时清除该矩形，使用 OSD canvas 绘制角标，并随跟踪结果唤醒更新。

颜色没有硬编码成白色；需要白色角标时，在原有跟踪颜色控件中设置 `#FFFFFF`。线宽继续采用原有 1～4 范围。小框会缩小角标线宽，宽高不足 4 像素时跳过绘制。

`hideWhenLost` 根据应用发布的丢失状态控制显示；算法判为暂时跟随、候选或可靠观测的时序仍由 D 决定。最终目标清理仍会清除跟踪框。

## 验证范围

- 前端增加新选项编辑、保存回读、角标预览、丢失框隐藏、OSD 单独热更新及旧固件不发送新字段的浏览器用例。
- Manager 配置事务测试补充新字段回读、旧客户端缺省继承、样式热更新、选取方式重启和非法枚举拒绝；能力版本检查同步更新。
- 按用户要求，不进行 SDK / Manager 编译，也不运行需要编译的 C 测试；不做前端生产构建。
- 尚无板上算法、温度、内存或帧率验证，不能把代码整合等同于这些指标已有提升。

## 板上验收建议

1. 更新相互匹配的 TDL 库、ipcamera 和 Manager；检查新能力和四个字段的默认值。
2. 保持当前模型和视频帧率，对比检测连续性、遮挡、快速相机运动、接近/远离目标时的跟踪表现。
3. 默认选项下点击画面边缘目标，确认没有被强制锁到中心；分别测试中心选取、固定种子及两者组合。
4. 测试 DET ID、FastSAM 点选、颜色、直接框选和 `/tmp/det_track`，确认各入口遵循配置并能恢复到检测/等待状态。
5. 在跟踪过程中热切换矩形/角标、颜色、线宽和丢失隐藏；确认没有旧矩形残留，也没有影响视频服务。
6. 分别验证 UVC、RTSP 主/子码流、MS7024 输出及输出模式切换；检查未启用 PIP、未改变网段和默认 AI 开关。
7. 持续跟踪时检查没有生成新增的 `/tmp/sot_trace.log`、`/tmp/sot_debug.log`，并观察内存及积热。
