export function VerticalAlignExample() {
	return <div>
		<span class="bold fs-32">Vertical Align Example</span>
		<Styled />
		<AlignTop />
		<AlignBottom />
		<AlignSuper />
		<AlignSub />
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
	return <p>Align <span class="fs-16 va-top">top</span><span class="fs-32"> text.</span></p>;
}
function AlignBottom() {
	return <p>Align <span class="fs-16 va-bottom">bottom</span><span class="fs-32"> text.</span></p>;
}
function AlignSuper() {
	return <p>Align x<span class="fs-12 va-super">2 superscript</span><span class="fs-32"> text.</span></p>;
}
function AlignSub() {
	return <p>Align H<span class="fs-12 va-sub">2 subscript</span><span class="fs-32"> text.</span></p>;
}
function AlignTextTop() {
	return <p>Align <span class="fs-16 va-text-top">text-top</span><span class="fs-32"> text.</span></p>;
}
function AlignTextBottom() {
	return <p>Align <span class="fs-16 va-text-bottom">text-bottom</span><span class="fs-32"> text.</span></p>;
}
function AlignMiddle() {
	return <p>Align <span class="fs-32 va-middle">middle</span> text.</p>;
}
function AlignOffset() {
	return <p>Align <span class="fs-16 va-up-16">upper</span> and <span class="fs-16 va-down-8">down</span> text.</p>;
}
function AlignSubtree() {
	return [
		<p>Align <span class="fs-12 va-top">top <span class="va-up-8">subtree</span></span> text.</p>,
		<p>Align <span class="fs-12 va-bottom">bottom <span class="va-down-8">subtree</span></span> text.</p>,
	];
}
