function RichTextExample() {
	/*
*/
	return <div>
		<Styled />
		<AlignTop />
		<AlignBottom />
		<AlignTextTop />
		<AlignTextBottom />
		<AlignMiddle />
		<AlignOffset />
		<AlignSubtree />
	</div>;
}

function Styled() {
	return <p>This is <span class="bold">bold</span>, <span class="italic">italic</span> and <span class="bold italic">bold-italic</span> text.</p>;
}
function AlignTop() {
	return <p>Align <span class="fs-16 va-top">top</span><span class="fs-32"> example.</span></p>;
}
function AlignBottom() {
	return <p>Align <span class="fs-16 va-bottom">bottom</span><span class="fs-32"> example.</span></p>;
}
function AlignTextTop() {
	return <p>Align <span class="fs-16 va-text-top">text-top</span><span class="fs-32"> example.</span></p>;
}
function AlignTextBottom() {
	return <p>Align <span class="fs-16 va-text-bottom">text-bottom</span><span class="fs-32"> example.</span></p>;
}
function AlignMiddle() {
	return <p>Align <span class="fs-32 va-middle">middle</span> example.</p>;
}
function AlignOffset() {
	return <p>Align <span class="fs-16 va-up-16">upper</span> and <span class="fs-16 va-down-8">down</span> example.</p>;
}
function AlignSubtree() {
	return [
		<p>Align <span class="fs-12 va-top">top <span class="va-up-8">subtree</span></span> example.</p>,
		<p>Align <span class="fs-12 va-bottom">bottom <span class="va-down-8">subtree</span></span> example.</p>,
	];
}

export var root = <RichTextExample />;
export var stylesheet = css`
div {
	margin: 8px;
}
p {
	margin-top: 8px;
	font-size: 20px;
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
.fs-48 { font-size: 48px; }
.lh-16 { line-height: 16px; }
.lh-20 { line-height: 20px; }
.lh-24 { line-height: 24px; }
.lh-32 { line-height: 32px; }
.lh-48 { line-height: 48px; }
.va-top { vertical-align: top; }
.va-bottom { vertical-align: bottom; }
.va-text-top { vertical-align: text-top; }
.va-text-bottom { vertical-align: text-bottom; }
.va-middle { vertical-align: middle; }
.va-up-8 { vertical-align: 8px; }
.va-up-16 { vertical-align: 16px; }
.va-up-32 { vertical-align: 32px; }
.va-down-8 { vertical-align: -8px; }
.va-down-16 { vertical-align: -16px; }
.va-down-32 { vertical-align: -32px; }
`;
