function RichText() {
	return <p class="fs-16">This is <span class="bold fs-12 lh-48">bold</span> rendering.</p>;
}

export var root = <RichText />;
export var stylesheet = css`
p {
	margin: 16px;
	font-size: 14px;
	background-color: lightcyan;
}
.italic { font-style: italic; }
.bold { font-weight: bold; }
.fs-10 { font-size: 10px; }
.fs-12 { font-size: 12px; }
.fs-16 { font-size: 16px; }
.fs-20 { font-size: 20px; }
.fs-24 { font-size: 24px; }
.fs-32 { font-size: 32px; }
.lh-16 { line-height: 16px; }
.lh-20 { line-height: 20px; }
.lh-24 { line-height: 24px; }
.lh-32 { line-height: 32px; }
.lh-48 { line-height: 48px; }
`;
