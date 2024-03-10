#pragma once
#include "windows/windows_header.h"
#include <stdint.h>

namespace windows {

class CDwFontFileStream final
    : public WRL::RuntimeClass<
    WRL::RuntimeClassFlags<WRL::ClassicCom>,
    IDWriteFontFileStream
    > {
public:
    HRESULT RuntimeClassInitialize(
        void const* fontFileReferenceKey,
        UINT32 fontFileReferenceKeySize
    );

    // IDWriteFontFileStream.
    HRESULT STDMETHODCALLTYPE
        ReadFileFragment(void const** fragmentStart,
            UINT64 fileOffset,
            UINT64 fragmentSize,
            OUT void** fragmentContext) override;
    void STDMETHODCALLTYPE ReleaseFileFragment(void* fragmentContext) override;
    HRESULT STDMETHODCALLTYPE GetFileSize(OUT UINT64* fileSize) override;
    HRESULT STDMETHODCALLTYPE
        GetLastWriteTime(OUT UINT64* lastWriteTime) override;
    bool IsInitialized() { return !!resourcePtr_; }
private:
    void const* resourcePtr_;
    DWORD resourceSize_;
};

class CDwFontFileLoader final
    : public WRL::RuntimeClass<
    WRL::RuntimeClassFlags<WRL::ClassicCom>,
    IDWriteFontFileLoader
    > {
public:
    // IDWriteFontFileLoader.
    HRESULT STDMETHODCALLTYPE
        CreateStreamFromKey(void const* fontFileReferenceKey,
            UINT32 fontFileReferenceKeySize,
            OUT IDWriteFontFileStream** fontFileStream) override;
    
    static IDWriteFontFileLoader* getLoader() {
        if (!instance_) {
            auto hr = WRL::MakeAndInitialize<CDwFontFileLoader>(&instance_);
        }
        return instance_.Get();
    }
    static bool isLoaderInitialized() { return !!instance_; }
private:
    static ComPtr<IDWriteFontFileLoader> instance_;
};

class CDWriteExt {
public:
    static ComPtr<IDWriteFontFace> DwCreateFontFaceFromStream(
        IDWriteFactory* factory,
        const uint8_t* pData,
        uint32_t size,
        int simulation_style);
};

}