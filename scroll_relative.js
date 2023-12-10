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

var simple_stylesheet = `
.rel-pos {
    position: relative;
}
.abs-pos {
    position: absolute;
}
.parent {
    width: 400px;
    height: 400px;
    background-color: lightblue;
    overflow: auto;
    border: 1px solid purple
}
.content {
    width: 100%;
    height: 100%;
    left: 50%;
    top: 50%;
    background-color: lightgreen;
}
.child {
    bottom: 0;
    right: 0;
    width: 40px;
    height: 40px;
    background-color: blue;
}
`;

var rel_overflow = (<body>
    <div class="parent rel-pos">
        <div class="content rel-pos"></div>
    </div>
</body>);

app.showDialog(rel_overflow, simple_stylesheet);
