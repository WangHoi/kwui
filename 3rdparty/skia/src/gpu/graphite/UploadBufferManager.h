/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_UploadBufferManager_DEFINED
#define skgpu_graphite_UploadBufferManager_DEFINED

#include <vector>

#include "include/core/SkRefCnt.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawTypes.h"

namespace skgpu::graphite {

class Buffer;
class CommandBuffer;
class ResourceProvider;

class UploadBufferManager {
public:
    UploadBufferManager(ResourceProvider*);
    ~UploadBufferManager();

    std::tuple<UploadWriter, BindBufferInfo> getUploadWriter(size_t requiredBytes,
                                                             size_t requiredAlignment);

    // Finalizes all buffers and transfers ownership of them to the CommandBuffer.
    void transferToCommandBuffer(CommandBuffer*);

private:
    ResourceProvider* fResourceProvider;

    sk_sp<Buffer> fReusedBuffer;
    size_t fReusedBufferOffset = 0;

    std::vector<sk_sp<Buffer>> fUsedBuffers;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_UploadBufferManager_DEFINED
