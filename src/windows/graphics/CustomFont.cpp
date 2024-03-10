#include "CustomFont.h"

namespace windows {

ComPtr<IDWriteFontFace> CDWriteExt::DwCreateFontFaceFromStream(
	IDWriteFactory* pDwFactory,
	const uint8_t* pData,
	uint32_t size,
	int simulation_style) {
	ComPtr<IDWriteFontFile> pDwFontFile;
	ComPtr<IDWriteFontFace> pDwFontFace;
	BOOL isSupportedFontType = FALSE;
	DWRITE_FONT_FILE_TYPE fontFileType;
	DWRITE_FONT_FACE_TYPE fontFaceType;
	UINT32 numberOfFaces;
	DWRITE_FONT_SIMULATIONS fontStyle =
		(DWRITE_FONT_SIMULATIONS)(simulation_style & 3);
	HRESULT hr = S_OK;
	hr = pDwFactory->CreateCustomFontFileReference(
		(void const*)pData, (UINT32)size, CDwFontFileLoader::getLoader(),
		pDwFontFile.GetAddressOf());
	if (FAILED(hr)) {
		goto failed;
	}
	hr = pDwFontFile->Analyze(&isSupportedFontType, &fontFileType, &fontFaceType,
		&numberOfFaces);
	if (FAILED(hr) || !isSupportedFontType ||
		fontFaceType == DWRITE_FONT_FACE_TYPE_UNKNOWN) {
		goto failed;
	}
	IDWriteFontFile* file_list[] = { pDwFontFile.Get() };
	hr = pDwFactory->CreateFontFace(fontFaceType, 1, file_list, 0, fontStyle,
		pDwFontFace.GetAddressOf());
	if (FAILED(hr)) {
		goto failed;
	}
	return pDwFontFace;
failed:
	return nullptr;
}
HRESULT CDwFontFileStream::RuntimeClassInitialize(
	void const* fontFileReferenceKey,
	UINT32 fontFileReferenceKeySize
) {
	resourcePtr_ = fontFileReferenceKey;
	resourceSize_ = fontFileReferenceKeySize;
	return S_OK;
}
HRESULT STDMETHODCALLTYPE
CDwFontFileStream::ReadFileFragment(void const** fragmentStart,
	UINT64 fileOffset,
	UINT64 fragmentSize,
	OUT void** fragmentContext) {
	if (fileOffset <= resourceSize_ &&
		fragmentSize <= resourceSize_ - fileOffset) {
		*fragmentStart = static_cast<uint8_t const*>(resourcePtr_) +
			static_cast<size_t>(fileOffset);
		*fragmentContext = nullptr;
		return S_OK;
	}
	*fragmentStart = nullptr;
	*fragmentContext = nullptr;
	return E_FAIL;
}
void STDMETHODCALLTYPE
CDwFontFileStream::ReleaseFileFragment(void* fragmentContext) {}
HRESULT STDMETHODCALLTYPE CDwFontFileStream::GetFileSize(OUT UINT64* fileSize) {
	*fileSize = resourceSize_;
	return S_OK;
}
HRESULT STDMETHODCALLTYPE
CDwFontFileStream::GetLastWriteTime(OUT UINT64* lastWriteTime) {
	*lastWriteTime = 0;
	return E_NOTIMPL;
}

ComPtr<IDWriteFontFileLoader> CDwFontFileLoader::instance_;

HRESULT STDMETHODCALLTYPE CDwFontFileLoader::CreateStreamFromKey(
	void const* fontFileReferenceKey,
	UINT32 fontFileReferenceKeySize,
	OUT IDWriteFontFileStream** fontFileStream) {
	*fontFileStream = nullptr;
	return WRL::MakeAndInitialize<CDwFontFileStream>(
		fontFileStream, fontFileReferenceKey, fontFileReferenceKeySize);
}

}
