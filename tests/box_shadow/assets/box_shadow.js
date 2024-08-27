function BoxShadowExample() {
    return <div class="inner"><spinner></spinner></div>;
}

var stylesheet = css`
    kml {
        width: 100%;
        height: 100%;
        background-color: #add8e6;
    }

    div.inner {
        position: absolute;
        left: 64px;
        top: 64px;
        width: 500px;
        height: 300px;
        background-color: #add8e6;
        border-radius: 24px;
        box-shadow: #93b8c4 10px 10px 10px inset,
        #c7f8ff -10px -10px 10px inset,
        #93b8c4 10px 10px 10px,
        #c7f8ff -10px -10px 10px;
    }
`;

export function builder() {
    return {
        root: <BoxShadowExample />, stylesheet,
    }
}