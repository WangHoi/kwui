function BlockSibling() {
	return <div>
		<c1>line1</c1><c2>line2</c2>
		<b1>block1</b1>
		<c3>line3</c3>
	</div>;
}
function BlockChild() {
	return <div>
		<c1>line1<b1>block1<c3>line3</c3></b1></c1>
		<c2>line2</c2>
	</div>;
}

var test_css = `
b1, b2, b3 {
	display: block;
}
c1, c2, c3 {
	display: inline;
}
.red {
	background-color:red;
}
`;
// 	<BlockChild />
app.showDialog({
	root: (<body style="margin: 0px;">
		<b1><b11>line1</b11></b1>
		<b2><b21>line2</b21></b2>
	</body>),
	stylesheet: test_css,
});
