"use strict";
"use math";

class __ComponentState__ extends Object {
	renderFn;
	props;
	children;

	constructor(builder, props, children) {
		super();
		this.renderFn = builder;
		this.props = props;
		this.children = children;
	}
	update() {
		// this.scene.updateComponent(this);
	}
	render() {
		return this.renderFn(this.props, this.children);
	}
};


function JSX(tag, atts, kids) {
	if (typeof tag == 'function') {
		return new __ComponentState__(tag, atts, kids);
	} else {
		return {
			tag,
			atts,
			kids,
		};
	}
}

function Div(props, kids) {
	return <div>
		<p>{props.a}</p>
		{kids}
		<p>next</p>
	</div>;
}

var simple_stylesheet = css`
.outer {
	display: block;
	overflow: auto;
	width: 200px;
	height: 100px;
	margin-left: 8px;
	margin-top: 8px;
	padding: 50px 0px;
	background-color: #fff;
	border-width: 2px;
	border-color: #000;
}
.inner {
	overflow: auto;
	width: 200px;
	height: 100px;
	background-color: #00ffff;
	font-size: 16px;
	border-width: 2px;
	border-color: #080;
	margin-left: 50px;
}
span {
	font-size: 16;
}
line_edit {
	width: 100;
	height: 20;
	background-color: #fff;
}
`;

var span1 = <span>This property is a shorthand for the following CSS properties: border-top-left-radius border-top-right-radius border-bottom-right-radius border-bottom-left-radius Syntax</span>;
var span2 = <span>a b c d e f g h i j k l m o p q r s t u v w x y z</span>;
//	<p>{span2}</p>
var complex_overflow = (<div class="outer">
	<qqq style="background-color: #0aa; height:120; overflow-y: scroll">{span1}</qqq>
    <div class="inner"><line_edit /><div class="inner" /></div>
</div>);

var text_overflow = (<div class="outer">
	<p><span style="text-decoration: underline; font-weight:bold;font-size:20px;">border-radius</span></p>
	{span1}
</div>);

var box_overflow = (<div class="outer">
	<div class="inner" />
    <div class="inner" />
	<div class="inner" />
</div>);

export function builder() {
	return {
		root: text_overflow,
		stylesheet: simple_stylesheet,
	}
}