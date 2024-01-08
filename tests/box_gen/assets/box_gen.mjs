function BlockSibling() {
	return <div>
		<b1>line1</b1><b2>line2</b2>
		<c1>block1</c1>
		<b3>line3</b3>
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
	display: block;
}
c1, c2, c3 {
	display: block;
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
