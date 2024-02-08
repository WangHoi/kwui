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
        <CheckBox label={"blue"} value={isBlue} onchange={setBlue} />
        <CheckBox label={"red"} value={isRed} onchange={setRed} />
        </p>
        <button onclick={selectAll}>select all</button>
    </div>;
}

function CheckBox({ label, value, onchange }) {
    return <button checked={value} onclick={() => onchange(!value)}>
        <span>{label}</span>
    </button>;
}

var Style = css`
button {
    cursor: pointer;
}
button:checked {
    color: green;
}
`;

export function builder() {
    return {
        root: <CheckBoxExample />,
        stylesheet: Style,
    }
}