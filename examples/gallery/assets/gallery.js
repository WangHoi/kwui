"use strict";
"use math";

function Button(props, kids) {
	return <button>{kids[0]}</button>;
}

var simple_css = `
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

var onclick = function() {
	console.log("button clicked");
}

var button = (
	<Button>测试</Button>
);

console.log(JSON.stringify(button));

app.showDialog(button, simple_css);
