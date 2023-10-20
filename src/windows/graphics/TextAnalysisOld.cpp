#include "TextAnalysis.h"
#include "base/log.h"

namespace windows {
namespace graphics {

static inline UINT32 estimate_glyph_count(size_t text_length)
{
    return 3 * (UINT32)text_length / 2 + 16;
}

HRESULT TextAnalysis::RuntimeClassInitialize(
    const std::wstring& text,
    ComPtr<IDWriteFontFace> font_face, 
    ComPtr<IDWriteTextAnalyzer> analyzer)
{
    text_ = text;
    font_face_ = font_face;
    analyzer_ = analyzer;
    return S_OK;
}
void TextAnalysis::buildLayout()
{
    line_breakpoints_.resize(text_.length());

    HRESULT hr;
    hr = analyzer_->AnalyzeLineBreakpoints(this, 0, (UINT32)text_.length(), this);
    hr = analyzer_->AnalyzeBidi(this, 0, (UINT32)text_.length(), this);
    hr = analyzer_->AnalyzeScript(this, 0, (UINT32)text_.length(), this);

    cluster_maps_.resize(text_.length());
    text_props_.resize(text_.length());
    glyph_indices_.resize(estimate_glyph_count(text_.length()));
    glyph_props_.resize(estimate_glyph_count(text_.length()));
    UINT32 glyph_start = 0;
    for (auto& sr : script_analysis_results_) {
        UINT32 max_glyph_count = (UINT32)glyph_indices_.size() - glyph_start;
        UINT32 actual_glyph_count = 0;
        hr = analyzer_->GetGlyphs(
            text_.c_str(),
            (UINT32)text_.length(),
            font_face_.Get(),
            FALSE,
            FALSE,
            &sr.script_analysis,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            max_glyph_count,
            &cluster_maps_[sr.text_position],
            &text_props_[sr.text_position],
            &glyph_indices_[glyph_start],
            &glyph_props_[glyph_start],
            &actual_glyph_count);
        if (FAILED(hr))
            break;
        glyph_start += actual_glyph_count;
    }
    glyph_indices_.resize(glyph_start);
    glyph_props_.resize(glyph_start);
}
IFACEMETHODIMP TextAnalysis::GetTextAtPosition(UINT32 text_position,
    OUT WCHAR const** text_string,
    OUT UINT32* text_length) throw()
{
    if (text_position >= (UINT32)text_.length()) {
        // Return no text if a query comes for a position at the end of
        // the range. Note that is not an error and we should not return
        // a failing HRESULT. Although the application is not expected
        // to return any text that is outside of the given range, the
        // analyzers may probe the ends to see if text exists.
        *text_string = NULL;
        *text_length = 0;
    } else {
        *text_string = &text_[text_position];
        *text_length = (UINT32)text_.length() - text_position;
    }
    return S_OK;
}
IFACEMETHODIMP TextAnalysis::GetTextBeforePosition(UINT32 text_position,
    OUT WCHAR const** text_string,
    OUT UINT32* text_length) throw()
{
    if (text_position == 0 || text_position > (UINT32)text_.length()) {
        // Return no text if a query comes for a position at the end of
        // the range. Note that is not an error and we should not return
        // a failing HRESULT. Although the application is not expected
        // to return any text that is outside of the given range, the
        // analyzers may probe the ends to see if text exists.
        *text_string = NULL;
        *text_length = 0;
    } else {
        *text_string = &text_[0];
        *text_length = text_position -
            0; // text length is valid from current position backward
    }
    return S_OK;
}
IFACEMETHODIMP_(DWRITE_READING_DIRECTION)
TextAnalysis::GetParagraphReadingDirection() throw()
{
    return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
}
IFACEMETHODIMP TextAnalysis::GetLocaleName(UINT32 text_position, OUT UINT32* text_length,
    OUT WCHAR const** localeName) throw()
{
    *localeName = L"";
    *text_length = (UINT32)text_.length() - text_position;

    return S_OK;
}
IFACEMETHODIMP TextAnalysis::GetNumberSubstitution(
    UINT32 text_position, OUT UINT32* text_length,
    OUT IDWriteNumberSubstitution** numberSubstitution) throw()
{
    *numberSubstitution = NULL;
    *text_length = (UINT32)text_.length() - text_position;

    return S_OK;
}

IFACEMETHODIMP
TextAnalysis::SetScriptAnalysis(UINT32 text_position, UINT32 text_length,
    DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis) throw()
{
    LOG(INFO)
        << "TextAnalysis::SetScriptAnalysis " << text_position << "/" << text_length
        << " uid " << scriptAnalysis->script;
    script_analysis_results_.emplace_back(TextSpanScriptAnalysisResult { text_position, text_length, *scriptAnalysis });
    return S_OK;

}

IFACEMETHODIMP TextAnalysis::SetLineBreakpoints(
    UINT32 text_position, UINT32 text_length,
    const DWRITE_LINE_BREAKPOINT* lineBreakpoints // [text_length]
) throw()
{
    LOG(INFO) << "TextAnalysis::SetLineBreakpoints " << text_position << "/" << text_length;
    if (text_length > 0) {
        memcpy(&line_breakpoints_[text_position], lineBreakpoints, text_length * sizeof(lineBreakpoints[0]));
    }
    return S_OK;
}

IFACEMETHODIMP TextAnalysis::SetBidiLevel(UINT32 text_position, UINT32 text_length,
    UINT8 explicitLevel,
    UINT8 resolvedLevel) throw()
{
    LOG(INFO)
        << "TextAnalysis::SetBidiLevel " << text_position << "/" << text_length
        << " level " << explicitLevel << "/" << resolvedLevel;
    return S_OK;
}

IFACEMETHODIMP TextAnalysis::SetNumberSubstitution(
    UINT32 text_position, UINT32 text_length,
    IDWriteNumberSubstitution* numberSubstitution) throw()
{
    LOG(INFO)
        << "TextAnalysis::SetNumberSubstitution " << text_position << "/" << text_length;
    return S_OK;
}

}
}
