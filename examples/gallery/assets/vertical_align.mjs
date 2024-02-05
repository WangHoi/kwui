import { P1 } from "./paragraph.mjs";

export function VerticalAlignTest(props, kids) {
    let edit = <div id="path-edit">
        <line_edit
            value="abc"
            fontSize={14}
            backgroundColor="#0000"
            color="black"
            caretColor="black"
            selectionBackgroundColor="#FF650040"
            innerHPadding={4}
        />
    </div>;
    let btn = <button id="browse-button">
        <span>Install</span>
    </button>;
    return (
        <div style="text-align: center;">
            {edit}
            {btn}
            <P1></P1>
            <p>miao woo</p>
        </div>);
}

export var VerticalAlignTestStyle = css`
#path-edit {
    overflow: hidden;
    display: inline-block;
    position: relative;
    height: 32px;
    background-color: white;
    border-radius: 4px;
}
line_edit {
    margin-top: 4px;
    width: 100px;
    height: 24px;
}
#browse-button {
    margin-left: 8px;
    overflow: hidden;
    font-size: 14;
    height: 32px;
    background-color: orangered;
    border-radius: 4px;
}
#browse-button span {
    display: inline-block;
    height: 24px;
    margin: 8px 8px 0px 8px;
}
#browse-button:hover {
    color: white;
}
p {
    position: relative;
    top: 20px;
    color: gray;
}
`;

export function builder() {
    return {
        root: <VerticalAlignTest />,
        stylesheet: VerticalAlignTestStyle,
    }
}