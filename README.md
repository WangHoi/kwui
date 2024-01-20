# kwui

使用 JSX、CSS 构建简单的桌面应用。

```javascript
import { useState } from "Keact";

function HelloWorld(props, kids) {
    let [n, setN] = useState(0);
    return <button onclick={() => setN(n + 1)}>{`Click ${n} times`}</button>;
}

app.showDialog({
    title: "Hello World",
	root: <HelloWorld />,
	stylesheet: css`
	button { margin: 10px; padding: 4px; background-color: orange; }
	button:hover { background-color: orangered; }
    `
});
```

## 快速开始

1. checkout and compile
2. check examples/
3. define KWUI_STATIC_LIBRARY, and link to kwui_static.lib

## 特点

- 简单实用
  - 适合快速开发
  - Rust语言支持
  - 小巧无第三方依赖
- 兼容性好
  - 基于Direct2D实现GPU加速，电脑配置过低、GPU不可用时回落到软件渲染
  - 在生产环境，成千上万台电脑上使用
  - 优秀的多显示器与DPI缩放支持
- 脚本驱动：
  - 基于QuickJS扩展，原生JSX支持，i18n支持
  - 类似React Hooks的组件生命周期处理
- 布局功能丰富，样式美观
  - 符合 CSS 2.2 标准的排版与样式
  - 强大的图文混排，富文本支持
  - 中文输入法支持
- 易于扩展
  - 为原生渲染场景，如视频渲染做了优化
  - 支持JavaScript，C++，Rust编写业务逻辑
  - 支持C++扩展DOM，处理原生事件和渲染

## 样例展示

### 语音通话质量测试工具
![image](docs/VoIPTool.png)

### 远程桌面控制
![image](docs/KuDesk.png)

### 安装程序
![image](docs/installer.png)

## 常见问题

1. 已经有很多界面库了，为什么还要重复造轮子？
   
    为了解决工作中的问题：
    - 开发安装程序，需要界面美观，且自定义安装逻辑，为了国际化需要加强布局功能。
    - 多进程重构，不同的框架需要重写UI代码，然后调整样式，因此需要UI代码编写一次，处处复用。
    - 音视频开发，核心代码使用Rust和C++编写，需要快速编写测试小工具。

    因为有趣：
    - 浏览器标准非常复杂，锻炼做系统分析和模块拆分，标准实现，标准测试的软件开发能力。
