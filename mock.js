"use strict";
"use math";

function Button(props, kids) {
	return <button>{kids[0]}</button>;
}

var simple_css = `
.title-bar {
	position: absolute;
	top: 0;
	right: 0;
	margin: 10;
}
img {
	display: inline-block;
	width: 80px;
	height: 80px;
}
button {
	padding: 4px 8px;
	border-color: #88e;
	border-radius: 4px;
	background-color: orangered;
	cursor: pointer;
	margin-left: 4px;
}
button:hover {
	background-color: orange;
}
button:active {
	background-color: red;
}
line_edit {
	width: 100%;
	height: 100%;
	cursor: text;
}
`;

var onclick = function() {
	console.log("button clicked");
}

var hello_world_full = (<body>
    <div style="background-color: #0f0">橙讯安装程序</div>
    <div class="title-bar" style="background-color: #e0ffff80">
        <button>最小化</button>
        <button>最大化</button>
        <button>关闭</button>
    </div>
    <div style="margin-top: 40; text-align: center">
        <img src="cx_logo_2.svg" />
    </div>
    <div style="margin-top: 40; text-align: center">
        <span style="position:relative; font-size: 20">
            {"橙讯"}
            <span style="position:absolute; left:100%; top:-10; font-size:10">2.7.0</span>
        </span>
    </div>
    <div style="text-align:center">
        <button style="margin-top: 40;margin-left:0px;font-size:16px;" onclick={onclick}>安装</button>
    </div>
	<div style="width: 200px; height: 20px; margin:0px auto; background-color: #eee">
		<line_edit />
	</div>
</body>);

var hello_world = (
    <div style="margin-top: 40; text-align: center">
        <span style="position:relative; font-size: 20">
            {"橙讯"}
            <span style="position:absolute; left:100%; top:-10; font-size:10">2.7.0</span>
        </span>
    </div>
);

var title_bar = (
	<div class="title-bar" style="background-color: #0ff">
        <button>最小化</button>
        <button style="background-color: #f0f">最大化</button>
        <button>关闭</button>
    </div>
);

var hello_world3 = (
	<div style="margin-top: 40; text-align: center">
		<span style="position:relative; font-size: 20">
			{"橙讯"}
			<span style="position:absolute; left:100%; top:-10; font-size:10">2.7.0</span>
		</span>
	</div>
);

console.log(JSON.stringify(hello_world_full));

app.showDialog({
	root: hello_world_full,
	stylesheet: simple_css
});
