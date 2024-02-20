var simple_stylesheet = css`
kml {
	height: 100%;
	overflow: auto;
}
.outer {
	display: block;
	overflow: auto;
	width: 176px;
	height: 176px;
	margin: 0px;
	padding: 10px 10px;
	background-color: #fff;
	border-width: 2px;
	border-color: #000;
}
.overflow-auto {
	overflow: auto;
}
.inner {
	width: 200px;
	height: 100px;
	background-color: #00ffff;
	font-size: 16px;
	border-width: 2px;
	border-color: #080;
	margin-left: 50px;
}
span {
	font-size: 16;
}
line_edit {
	width: 100;
	height: 20;
	background-color: #fff;
}
.inline-block {
	display: inline-block;
}
button {
	display: inline-block;
	border: 1px solid black;
	padding: 0px 4px;
	height: 24px;
	line-height: 24px;
}
button:hover {
	background-color: lightblue;
}
button:active {
	background-color: lightcyan;
}
`;

function ComboxBox() {
	let parent = this.dialogId;
	let show_popup = () => {
		app.showDialog({
			// width: 200,
			// height: 120,
			screenRect: {
				left: 80,
				top: 80,
				right: 280,
				bottom: 180,
			},
			flags: 2,
			parent,
			root: <div>
				<p><button>First option</button></p>
				<p><button>Second option</button></p>
				<p><button>Third option</button></p>
			</div>,
			stylesheet: simple_stylesheet,
		});
	};
	return <button onclick={show_popup}>Show popup</button>;
}
function PopupExample() {
	return <div style="margin: 16px;">
		<ComboxBox></ComboxBox>
	</div>;
}

export function builder() {
	return {
		root: <PopupExample />,
		stylesheet: simple_stylesheet,
	}
}