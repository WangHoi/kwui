function NotEqualCornerRadius({label}) {
    return <div class="box">
        <style jsx>{`
            div {
                border-style: solid;
                border-width: 4px;
                border-color: black;
                border-bottom-left-radius: 8px;
                border-bottom-right-radius: 8px;
            }
        `}</style>
        <span>{label}</span>
    </div>;
}

function NoRightBorder({label}) {
    return <div class="box">
        <style jsx>{`
            div {
                border-style: solid;
                border-top-width: 1px;
                border-left-width: 1px;
                border-bottom-width: 1px;
                border-color: black;
                border-bottom-left-radius: 8px;
            }
        `}</style>
        <span>{label}</span>
    </div>;
}

function BorderRadiusTextExample() {
    return <div>
        <NotEqualCornerRadius label="Not equal corner radius"/>
        <NoRightBorder label="Not equal radius and no right border"/>
    </div>;
}

var stylesheet = css`
    kml {
        overflow-y: auto;
        height: 100%;
    }

    div {
        margin: 8px 8px;
    }

    .box {
        display: inline-block;
        background-color: lightblue;
        width: auto;
        padding: 8px;
        vertical-align: center;
    }

    span {
        font-size: 18px;
    }

    .italic {
        font-style: italic;
    }

    .bold {
        font-weight: bold;
    }

    .fs-10 {
        font-size: 10px;
    }

    .fs-12 {
        font-size: 12px;
    }

    .fs-16 {
        font-size: 16px;
    }

    .fs-20 {
        font-size: 20px;
    }

    .fs-24 {
        font-size: 24px;
    }

    .fs-32 {
        font-size: 32px;
    }

    .fs-48 {
        font-size: 48px;
    }

    .lh-16 {
        line-height: 16px;
    }

    .lh-20 {
        line-height: 20px;
    }

    .lh-24 {
        line-height: 24px;
    }

    .lh-32 {
        line-height: 32px;
    }

    .lh-48 {
        line-height: 48px;
    }

    .va-top {
        vertical-align: top;
    }

    .va-bottom {
        vertical-align: bottom;
    }

    .va-super {
        vertical-align: super;
    }

    .va-sub {
        vertical-align: sub;
    }

    .va-text-top {
        vertical-align: text-top;
    }

    .va-text-bottom {
        vertical-align: text-bottom;
    }

    .va-middle {
        vertical-align: middle;
    }

    .va-up-4 {
        vertical-align: 4px;
    }

    .va-up-8 {
        vertical-align: 8px;
    }

    .va-up-16 {
        vertical-align: 16px;
    }

    .va-up-32 {
        vertical-align: 32px;
    }

    .va-down-4 {
        vertical-align: -4px;
    }

    .va-down-8 {
        vertical-align: -8px;
    }

    .va-down-16 {
        vertical-align: -16px;
    }

    .va-down-32 {
        vertical-align: -32px;
    }

    .red {
        color: red;
    }

    .green {
        color: green;
    }

    .blue {
        color: blue;
    }

    .td-underline {
        text-decoration: underline;
    }

    .td-overline {
        text-decoration: overline;
    }

    .td-line-through {
        text-decoration: line-through;
    }

    img {
        display: inline-block;
        width: 16px;
        height: 16px;
    }
`;

export function builder() {
    return {
        root: <BorderRadiusTextExample/>, stylesheet,
    }
}