import { useState } from "Keact";

function CheckBoxExample() {
    let [isBlue, setBlue] = useState(false);
    let [isRed, setRed] = useState(false);
    let selectAll = () => {
        setBlue(true);
        setRed(true);
    };
    return <div>
        <p>
            <p><CheckBox label={"blue"} value={isBlue} onchange={setBlue} /></p>
            <p><CheckBox label={"red"} value={isRed} onchange={setRed} /></p>
        </p>
        <button onclick={selectAll}>select all</button>
    </div>;
}

function CheckBox({ label, value, onchange }) {
    return <button checked={value} onclick={() => onchange(!value)}>
        <span class="icon-font">{value ? "\u{2611}" : "\u{2610}"}</span><span>label</span>
    </button>;
}

var Style = css`
.icon-font {
    font-family: "Segoe UI Symbol";
}
button {
    font-size: 24px;
    cursor: pointer;
    margin: 4px;
}
button:checked {
    /* color: green; */
}
`;

export function builder() {
    return {
        root: <CheckBoxExample />,
        stylesheet: Style,
    }
}