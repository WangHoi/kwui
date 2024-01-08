function BlockSibling() {
	return <div>
		<b1>line1</b1>
		<c1>block1</c1>
		<b2>line2</b2>
	</div>;
}
function BlockChild() {
	return <div>
		<b1>line1<c1>block1<b3>line3</b3></c1></b1>
		<b2>line2</b2>
	</div>;
}

var test_css = `
b1, b2, b3 {
	display: inline;
}
c1, c2, c3 {
	display: block;
}
`;

app.showDialog({
	root: <div>
		<p>--- Block sibing test ---</p>
		<BlockSibling />
		<p>--- Block child test ---</p>
		<BlockChild />
	</div>,
	stylesheet: test_css,
});
