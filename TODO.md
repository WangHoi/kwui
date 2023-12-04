## 框架

### 布局:
- Relative的处理
- 盒子生成：inline中包含block，需要生成临时anon-block-box包裹;
- AbsoluteBlockHeightSolver
- 可滚动区域的计算要在布局完成后计算，inline-block有translate，不好边布局边算

### CSS:
- named color, #RGBA color
- cursor，:active 支持
- font-size 支持更多单位

### DOM:
- mouseover/mouseout 事件没有 bubble
- fragment 处理有问题，没有跳过

### Control:
- LineEditControl 输入法提示框位置不对，鼠标点击事件坐标不对
- RenderRect 需要不包含 margin、padding
- NodeAttributeValue 增加toString, toNumber
- LineEditControl 剪贴板支持

### 测试:
- 确定对话框模拟 js
- 测试 js 放单独目录
- 脚本转换 html 为 js

## 橙讯安装程序移植

## 签名自查程序移植
