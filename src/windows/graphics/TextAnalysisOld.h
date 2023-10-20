#pragma once
#include "windows/windows_header.h"
#include "absl/types/span.h"
#include <string>

namespace windows {
namespace graphics {

struct TextSpanScriptAnalysisResult {
    UINT32 text_position = 0;
    UINT32 text_length = 0;
    DWRITE_SCRIPT_ANALYSIS script_analysis = {};
};

struct GlyphRun {
    UINT32 text_position = 0;
    UINT32 text_length = 0;
    const DWRITE_SCRIPT_ANALYSIS* script_analysis = nullptr;
};

class TextAnalysis : public WRL::RuntimeClass<
	WRL::RuntimeClassFlags<WRL::ClassicCom>,
	IDWriteTextAnalysisSource, IDWriteTextAnalysisSink
> {
public:
    HRESULT RuntimeClassInitialize(
        const std::wstring& text,
        ComPtr<IDWriteFontFace> font_face,
        ComPtr<IDWriteTextAnalyzer> analyzer);
    void buildLayout();
    // IDWriteTextAnalysisSource implementation
    IFACEMETHODIMP GetTextAtPosition(UINT32 text_position,
        OUT WCHAR const** text_string,
        OUT UINT32* text_length) throw();
    IFACEMETHODIMP GetTextBeforePosition(UINT32 text_position,
        OUT WCHAR const** text_string,
        OUT UINT32* text_length) throw();
    IFACEMETHODIMP_(DWRITE_READING_DIRECTION)
        GetParagraphReadingDirection() throw();
    IFACEMETHODIMP GetLocaleName(UINT32 text_position, OUT UINT32* text_length,
        OUT WCHAR const** localeName) throw();
    IFACEMETHODIMP GetNumberSubstitution(
        UINT32 text_position, OUT UINT32* text_length,
        OUT IDWriteNumberSubstitution** numberSubstitution) throw();

    // IDWriteTextAnalysisSink implementation
    IFACEMETHODIMP
        SetScriptAnalysis(UINT32 text_position, UINT32 text_length,
            DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis) throw();
    IFACEMETHODIMP SetLineBreakpoints(
        UINT32 text_position, UINT32 text_length,
        const DWRITE_LINE_BREAKPOINT* lineBreakpoints // [text_length]
    ) throw();
    IFACEMETHODIMP SetBidiLevel(UINT32 text_position, UINT32 text_length,
        UINT8 explicitLevel,
        UINT8 resolvedLevel) throw();
    IFACEMETHODIMP SetNumberSubstitution(
        UINT32 text_position, UINT32 text_length,
        IDWriteNumberSubstitution* numberSubstitution) throw();

private:
    std::wstring text_;
    ComPtr<IDWriteFontFace> font_face_;
    ComPtr<IDWriteTextAnalyzer> analyzer_;
    std::vector<DWRITE_LINE_BREAKPOINT> line_breakpoints_;
    std::vector<TextSpanScriptAnalysisResult> script_analysis_results_;

    std::vector<UINT16> cluster_maps_;
    std::vector<DWRITE_SHAPING_TEXT_PROPERTIES> text_props_;
    std::vector<UINT16> glyph_indices_;
    std::vector<DWRITE_SHAPING_GLYPH_PROPERTIES> glyph_props_;
};

}
}
