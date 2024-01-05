"use strict";
"use math";
//import { useHook } from 'Keact';

function TestDtor(props, kids) {
	let [n, setN] = useHook(() => props.n, (_, n) => {
		console.log("update n to", n);
		return [n, true];
	}, (n) => { console.log("destructor n =", n); });
	return <p>{"AAA" + props.n}</p>;
}

function Button(props, kids) {
	let [n, setN] = useHook(() => 0, (_, n) => {
		console.log("update n to", n);
		return [n, true];
	});
	let onclick = () => setN(n + 1);
	if (n >= 0) {
		return (<div class="center" style="width:100px;margin-top:100px">
			<div><button onclick={onclick}>{kids[0]}</button></div>
			<p>{"点击了" + n + "次"}</p>
			<TestDtor n={n} />
		</div>);
	} else {
		return <div><p>Clicked</p><line_edit style="width:100px;height:20px;" /></div>
	}
}

function ImageButton(props, kids) {
	return (<button style="width:80px;height:80px;" onclick={props.onclick} src={props.src}>
		<div class="image-button" style="width:100%;height:100%;"></div>
	</button>)
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
	background-image: url("close_button.png");
	cursor: pointer;
	margin: 8px;
}
button:hover {
	background-color: orangered;
	background-image: url("close_button_hover.png");
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

var test_button = (
	<Button>测试</Button>
);

var test_image_button = (
	<ImageButton onclick={onclick} src="cx_logo_2.svg"></ImageButton>
);

function makeUpdateFn(updater) {
	return function (id, arg) {
		// console.log("animation-event fn called");
		updater(arg);
	};
}

function useAnimationEvent() {
	let [state, _] = useHook((updater) => {
		console.log("init fn called");
		let event_handler = makeUpdateFn(updater);
		app.addListener("dialog:animation-event", event_handler);
		return {
			handler: event_handler,
		};
	}, (state, arg) => {
		let start_timestamp = state.start_timestamp || arg.timestamp;
		state.timestamp = arg.timestamp - start_timestamp;
		state.start_timestamp = start_timestamp;
		return [state, true];
	}, (state) => {
		app.removeListener("dialog:animation-event", state.handler);
	});
	return state.timestamp || 0;
}

function TimestampDisplay(props, kids) {
	let ts = useAnimationEvent();
	return <p>{"time: " + ts}</p>
}

function FlatIconTextButton(props, kids) {
	// 		<p class="button-icon">aaa</p>
	let close_handler = () => {
		app.removeListener("dialog:request-close", close_handler);
		app.closeDialog(this.dialogId);
	};
	app.addListener("dialog:request-close", close_handler);
	/* <p class="button-icon" /> */
	return <button class="flat-icon-text-button">
		<img class="button-icon" src="expand.png"></img>
		<div id="wrapper" style="display:inline-block">
			<span id="text">更多选项</span>
		</div>
	</button>
}

var flat_icon_css = `
.center {
	margin-left: auto;
	margin-right: auto;
}
.button-icon {
	display: inline-block;
	width: 24px;
	height: 24px;
	background-image: url("expand.png");
	vertical-align: bottom;
}
.flat-icon-text-button #text {
	color: #777;
	font-size: 14px;
}
.flat-icon-text-button > #wrapper {
	display:inline-block;
	margin-top: -6px;
	margin-right: 4px;
}
button:hover .button-icon {
	background-image: url("collapse.png");
}
button {
	padding: 0px 0px;
	border-color: #88e;
	border-radius: 0px;
	cursor: pointer;
	margin: 8px;
}
`;

var line_edit_css = `
#path-edit {
	display: inline-block;
	width: 200px;
	height: 32px;
}
`;
function TestLineEdit(props, kids) {
	let id = this.dialogId;
	let close_handler = () => {
		app.removeListener("dialog:request-close", close_handler);
		app.closeDialog(id);
	};
	app.addListener("dialog:request-close", close_handler);
	let [targetDir, text_changed] = useHook(
		() => "abc",
		(_, new_value) => [new_value, true]);
	return <div>
		<div style="text-align: center;">
			<span>before</span>
			<line_edit id="path-edit" value={targetDir} onchange={text_changed}></line_edit>
			<span>after</span>
		</div>
	</div>;
}
/*
app.showDialog({
	root: <TestLineEdit></TestLineEdit>,
	stylesheet: line_edit_css,
});
*/

app.showDialog({
	root: <div>
		<FlatIconTextButton></FlatIconTextButton>
	</div>,
	stylesheet: flat_icon_css,
});
