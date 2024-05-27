var line_edit_css = `
#path-edit {
	display: inline-block;
	width: 200px;
	height: 32px;
}
`;
function TestLineEdit(props, kids) {
	return <div>
		<div style="text-align: center;">
			<span>before</span>
			<line_edit id="path-edit" value="abc"></line_edit>
			<span>after</span>
		</div>
	</div>;
}

export function builder() {
    return {
        root: <TestLineEdit />,
        stylesheet: line_edit_css,
    };
}
