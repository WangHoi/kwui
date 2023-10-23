框架
====

布局:
- 盒子生成：文本的anon-inline-box
- Relative的处理
- 盒子生成：inline中包含block，需要生成临时anon-block-box包裹;
- AbsoluteBlockHeightSolver

CSS:
- font-size 支持更多单位
- shorthand 支持

DOM:
- fragment 处理有问题，没有跳过

Control:
- RenderRect 需要不包含 margin、padding
- NodeAttributeValue 增加toString, toNumber
- 移植ButtonControl, ImageButtonControl
- LineEditControl 剪贴板支持

测试:
- 确定对话框模拟 js
- 测试 js 放单独目录
- 脚本转换 html 为 js

橙讯安装程序移植
====

- ResourceManager移植

签名自查程序移植
====

- 支持滚动条
- position: sticky
- DataGrid实现：部分JS接口
