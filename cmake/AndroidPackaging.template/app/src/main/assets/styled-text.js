export function StyledTextExample() {
	return <div>
		<span class="bold fs-32">Styled Text Example</span>
		<TextDecoration />
        <AnchorStyle />
	</div>;
}

function TextDecoration() {
	return <p><span style="background-color:papayawhip;">This is <span class="red td-underline">underline</span>, <span class="green td-overline">overline</span> and <span class="blue td-line-through">line-through xXx</span> text.</span></p>;
}
function AnchorStyle() {
    return <p>Text link: <span class="td-underline" style="color:blue;">https://gitee.com/wanghoi/kwui</span></p>;
}