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
`;

app.showDialog({
	/*<p>--- Block child test ---</p>
	<BlockChild />*/
	root: (<div style="margin: 16px;">
		<p>--- Block sibing test ---</p>
		<BlockSibling />
	</div>),
	stylesheet: test_css,
});
