function RichText() {
	return <p>This is <span class="bold">bold</span> rendering.</p>;
}

export var root = <RichText />;
export var stylesheet = `
p {
	margin: 16px;
	font-size: 14px;
}
.italic {
	font-style: italic
}
.bold {
	font-weight: bold;
}
.fs-16 {
	font-size: 16px;
}
.fs-20 {
	font-size: 20px;
}
.fs-24 {
	font-size: 24px;
}
`;
