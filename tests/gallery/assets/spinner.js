function SpinnerExample() {
    return (<div>
        <style jsx>{`
spinner {
    width: 22px;
    height: 22px;
    vertical-align: -2px;
}
spinner + span {
    color: blue;
}
button {
    font-size: 24px;
    cursor: pointer;
    margin: 4px;
}
button:checked {
    /* color: green; */
}
        `}</style>
        <button><span>aaa</span><spinner foregroundColor="#4f4" /><span>Loading...</span></button>
    </div>);
}

export function builder() {
    return {
        root: <SpinnerExample />,
        stylesheet: css`span { color: red; }`,
    }
}