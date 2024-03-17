import { useState } from "Keact";

function RadioExample() {
    let [state, setState] = useState('');
    let selectNone = () => {
        setState('');
    };
    return <div>
        <p>
            <p>
                <Radio label={"选项1"} value={state == 'blue'} onchange={() => setState('blue')} />
            </p>
            <p>
                <Radio label={"选项2"} value={state == 'red'} onchange={() => setState("red")} />
            </p>
        </p>
        <button onclick={selectNone}>select none</button>
    </div>;
}

function Radio({ label, value, onchange }) {
    return <button checked={value} onclick={() => onchange(value)}>
        <span class="icon-font">{value ? "\u{f192}" : "\u{f111}"}</span><span>{label}</span>
    </button>;
}

var Style = css`
.icon-font {
    font-family: "kwui";
}
button {
    display: block;
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
        root: <RadioExample />,
        stylesheet: Style,
    }
}