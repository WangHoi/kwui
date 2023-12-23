"use strict";
"use math";
//import { useHook } from 'Keact';

function Button(props, kids) {
	let [n, setN] = useHook(0, (_, n) => {
		console.log("update n to ");
		return [n, true];
	});
	let onclick = () => setN(n + 1);
	if (n == 0) {
		return (<div class="center" style="width:100px;margin-top:100px">
			<div><button onclick={onclick}>{kids[0]}</button></div>
			<p>{"点击了" + n + "次"}</p>
		</div>);
	} else {
		return <div><p>Clicked</p><line_edit style="width:100px;height:20px;" /></div>
	}
}

function ImageButton(props, kids) {
	return (<image_button style="width:80px;height:80px;" onclick={props.onclick} src={props.src}>
	</image_button>)
}

var simple_css = `
.center {
	margin-left: auto;
	margin-right: auto;
}
button {
	padding: 4px 8px;
	border-color: #88e;
	border-radius: 4px;
	background-color: orangered;
	cursor: pointer;
	margin: 8px;
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

var onclick = function () {
	console.log("button clicked");
}

var button = (
	<Button>测试</Button>
);

var test_image_button = (
	<ImageButton onclick={onclick} src="cx_logo_2.svg"></ImageButton>
);

// console.log(JSON.stringify(button));

app.showDialog(test_image_button, simple_css);
