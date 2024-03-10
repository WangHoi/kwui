import { useState } from "Keact";

function RadioExample() {
    let [state, setState] = useState('');
    let selectNone = () => {
        setState('');
    };
    return <div>
        <p>
            <p>
                <Radio label={"blue"} value={state == 'blue'} onchange={() => setState('blue')} />
            </p>
            <p>
                <Radio label={"red"} value={state == 'red'} onchange={() => setState("red")} />
            </p>
        </p>
        <button onclick={selectNone}>select none</button>
    </div>;
}

function Radio({ label, value, onchange }) {
    return <button checked={value} onclick={() => onchange(value)}>
        <span class="icon-font">{value ? "\u{26ab}" : "\u{26aa}"}</span><span>label</span>
    </button>;
}

var Style = css`
.icon-font {
    font-family: "Segoe UI Symbol";
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