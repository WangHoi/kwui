import { useState } from "Keact";

function RadioExample() {
    let [state, setState] = useState('');
    let selectNone = () => {
        setState('');
    };
    return <div>
        <p>
            <Radio label={"blue"} value={state == 'blue'} onchange={() => setState('blue')} />
            <Radio label={"red"} value={state == 'red'} onchange={() => setState("red")} />
        </p>
        <button onclick={selectNone}>select none</button>
    </div>;
}

function Radio({ label, value, onchange }) {
    return <button checked={value} onclick={() => onchange(value)}>
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
        root: <RadioExample />,
        stylesheet: Style,
    }
}