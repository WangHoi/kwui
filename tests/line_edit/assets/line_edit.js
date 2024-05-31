var line_edit_css = css`
#path-edit {
	display: inline-block;
	width: 200px;
	height: 32px;
}
button {
	background-color: lightcoral;
}
#bottom {
	position: absolute;
	width: 100%;
	top: 400;
}
`;
function TestLineEdit(props, kids) {
	return <div>
		<div style="text-align: center;">
			<span>before</span>
			<line_edit id="path-edit" value="abc"></line_edit>
			<span>after</span>
		</div>
		<button>Test Button</button>
		<div id="bottom" style="text-align: center; ">
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
