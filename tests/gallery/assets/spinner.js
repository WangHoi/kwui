import { useState } from "Keact";

function SpinnerExample() {
    return (<div>
        <button><spinner foregroundColor="#4f4" /><span>Loading...</span></button>
    </div>);
}

var style = css`
spinner {
    width: 22px;
    height: 22px;
    vertical-align: -2px;
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
        root: <SpinnerExample />,
        stylesheet: style,
    }
}