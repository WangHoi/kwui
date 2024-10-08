add_library(skia STATIC "")
target_sources(
    skia
    PRIVATE
    src/c/sk_effects.cpp
    src/c/sk_imageinfo.cpp
    src/c/sk_paint.cpp
    src/c/sk_surface.cpp
    src/codec/SkAndroidCodec.cpp
    src/codec/SkAndroidCodecAdapter.cpp
    src/codec/SkBmpBaseCodec.cpp
    src/codec/SkBmpCodec.cpp
    src/codec/SkBmpMaskCodec.cpp
    src/codec/SkBmpRLECodec.cpp
    src/codec/SkBmpStandardCodec.cpp
    src/codec/SkCodec.cpp
    src/codec/SkCodecImageGenerator.cpp
    src/codec/SkColorTable.cpp
    src/codec/SkEncodedInfo.cpp
    src/codec/SkHeifCodec.cpp
    src/codec/SkMaskSwizzler.cpp
    src/codec/SkMasks.cpp
    src/codec/SkParseEncodedOrigin.cpp
    src/codec/SkSampledCodec.cpp
    src/codec/SkSampler.cpp
    src/codec/SkStreamBuffer.cpp
    src/codec/SkSwizzler.cpp
    src/codec/SkWbmpCodec.cpp
    src/core/SkAAClip.cpp
    src/core/SkATrace.cpp
    src/core/SkAlphaRuns.cpp
    src/core/SkAnalyticEdge.cpp
    src/core/SkAnnotation.cpp
    src/core/SkArenaAlloc.cpp
    src/core/SkAutoPixmapStorage.cpp
    src/core/SkBBHFactory.cpp
    src/core/SkBigPicture.cpp
    src/core/SkBitmap.cpp
    src/core/SkBitmapCache.cpp
    src/core/SkBitmapDevice.cpp
    src/core/SkBitmapProcState.cpp
    src/core/SkBitmapProcState_matrixProcs.cpp
    src/core/SkBlendMode.cpp
    src/core/SkBlendModeBlender.cpp
    src/core/SkBlitRow_D32.cpp
    src/core/SkBlitter.cpp
    src/core/SkBlitter_A8.cpp
    src/core/SkBlitter_ARGB32.cpp
    src/core/SkBlitter_RGB565.cpp
    src/core/SkBlitter_Sprite.cpp
    src/core/SkBlockAllocator.cpp
    src/core/SkBlurMF.cpp
    src/core/SkBlurMask.cpp
    src/core/SkBuffer.cpp
    src/core/SkCachedData.cpp
    src/core/SkCanvas.cpp
    src/core/SkCanvasPriv.cpp
    src/core/SkChromeRemoteGlyphCache.cpp
    src/core/SkClipStack.cpp
    src/core/SkClipStackDevice.cpp
    src/core/SkColor.cpp
    src/core/SkColorFilter.cpp
    src/core/SkColorFilter_Matrix.cpp
    src/core/SkColorSpace.cpp
    src/core/SkColorSpaceXformSteps.cpp
    src/core/SkCompressedDataUtils.cpp
    src/core/SkContourMeasure.cpp
    src/core/SkConvertPixels.cpp
    src/core/SkCpu.cpp
    src/core/SkCubicClipper.cpp
    src/core/SkCubicMap.cpp
    src/core/SkData.cpp
    src/core/SkDataTable.cpp
    src/core/SkDebug.cpp
    src/core/SkDeferredDisplayList.cpp
    src/core/SkDeferredDisplayListRecorder.cpp
    src/core/SkDeque.cpp
    src/core/SkDescriptor.cpp
    src/core/SkDevice.cpp
    src/core/SkDistanceFieldGen.cpp
    src/core/SkDocument.cpp
    src/core/SkDraw.cpp
    src/core/SkDrawLooper.cpp
    src/core/SkDrawShadowInfo.cpp
    src/core/SkDraw_atlas.cpp
    src/core/SkDraw_text.cpp
    src/core/SkDraw_vertices.cpp
    src/core/SkDrawable.cpp
    src/core/SkEdge.cpp
    src/core/SkEdgeBuilder.cpp
    src/core/SkEdgeClipper.cpp
    src/core/SkExecutor.cpp
    src/core/SkFlattenable.cpp
    src/core/SkFont.cpp
    src/core/SkFontDescriptor.cpp
    src/core/SkFontMgr.cpp
    src/core/SkFontStream.cpp
    src/core/SkFont_serial.cpp
    src/core/SkGaussFilter.cpp
    src/core/SkGeometry.cpp
    src/core/SkGlobalInitialization_core.cpp
    src/core/SkGlyph.cpp
    src/core/SkGlyphBuffer.cpp
    src/core/SkGlyphRun.cpp
    src/core/SkGlyphRunPainter.cpp
    src/core/SkGpuBlurUtils.cpp
    src/core/SkGraphics.cpp
    src/core/SkHalf.cpp
    src/core/SkICC.cpp
    src/core/SkIDChangeListener.cpp
    src/core/SkImageFilter.cpp
    src/core/SkImageFilterCache.cpp
    src/core/SkImageFilterTypes.cpp
    src/core/SkImageGenerator.cpp
    src/core/SkImageInfo.cpp
    src/core/SkKeyContext.cpp
    src/core/SkKeyHelpers.cpp
    src/core/SkLatticeIter.cpp
    src/core/SkLineClipper.cpp
    src/core/SkLocalMatrixImageFilter.cpp
    src/core/SkM44.cpp
    src/core/SkMD5.cpp
    src/core/SkMalloc.cpp
    src/core/SkMallocPixelRef.cpp
    src/core/SkMask.cpp
    src/core/SkMaskBlurFilter.cpp
    src/core/SkMaskCache.cpp
    src/core/SkMaskFilter.cpp
    src/core/SkMaskGamma.cpp
    src/core/SkMath.cpp
    src/core/SkMatrix.cpp
    src/core/SkMatrixImageFilter.cpp
    src/core/SkMatrixInvert.cpp
    src/core/SkMesh.cpp
    src/core/SkMiniRecorder.cpp
    src/core/SkMipmap.cpp
    src/core/SkMipmapAccessor.cpp
    src/core/SkModeColorFilter.cpp
    src/core/SkOpts.cpp
    src/core/SkOpts_erms.cpp
    src/core/SkOverdrawCanvas.cpp
    src/core/SkPaint.cpp
    src/core/SkPaintParamsKey.cpp
    src/core/SkPaintPriv.cpp
    src/core/SkPath.cpp
    src/core/SkPathBuilder.cpp
    src/core/SkPathEffect.cpp
    src/core/SkPathMeasure.cpp
    src/core/SkPathRef.cpp
    src/core/SkPath_serial.cpp
    src/core/SkPicture.cpp
    src/core/SkPictureData.cpp
    src/core/SkPictureFlat.cpp
    src/core/SkPictureImageGenerator.cpp
    src/core/SkPicturePlayback.cpp
    src/core/SkPictureRecord.cpp
    src/core/SkPictureRecorder.cpp
    src/core/SkPipelineData.cpp
    src/core/SkPixelRef.cpp
    src/core/SkPixmap.cpp
    src/core/SkPoint.cpp
    src/core/SkPoint3.cpp
    src/core/SkPromiseImageTexture.cpp
    src/core/SkPtrRecorder.cpp
    src/core/SkQuadClipper.cpp
    src/core/SkRRect.cpp
    src/core/SkRTree.cpp
    src/core/SkRasterClip.cpp
    src/core/SkRasterPipeline.cpp
    src/core/SkRasterPipelineBlitter.cpp
    src/core/SkReadBuffer.cpp
    src/core/SkRecord.cpp
    src/core/SkRecordDraw.cpp
    src/core/SkRecordOpts.cpp
    src/core/SkRecordedDrawable.cpp
    src/core/SkRecorder.cpp
    src/core/SkRecords.cpp
    src/core/SkRect.cpp
    src/core/SkRegion.cpp
    src/core/SkRegion_path.cpp
    src/core/SkResourceCache.cpp
    src/core/SkRuntimeEffect.cpp
    src/core/SkScalar.cpp
    src/core/SkScalerCache.cpp
    src/core/SkScalerContext.cpp
    src/core/SkScan.cpp
    src/core/SkScan_AAAPath.cpp
    src/core/SkScan_AntiPath.cpp
    src/core/SkScan_Antihair.cpp
    src/core/SkScan_Hairline.cpp
    src/core/SkScan_Path.cpp
    src/core/SkSemaphore.cpp
    src/core/SkShaderCodeDictionary.cpp
    src/core/SkSharedMutex.cpp
    src/core/SkSpecialImage.cpp
    src/core/SkSpecialSurface.cpp
    src/core/SkSpinlock.cpp
    src/core/SkSpriteBlitter_ARGB32.cpp
    src/core/SkSpriteBlitter_RGB565.cpp
    src/core/SkStream.cpp
    src/core/SkStrikeCache.cpp
    src/core/SkStrikeForGPU.cpp
    src/core/SkStrikeSpec.cpp
    src/core/SkString.cpp
    src/core/SkStringUtils.cpp
    src/core/SkStroke.cpp
    src/core/SkStrokeRec.cpp
    src/core/SkStrokerPriv.cpp
    src/core/SkSurfaceCharacterization.cpp
    src/core/SkSwizzle.cpp
    src/core/SkTSearch.cpp
    src/core/SkTaskGroup.cpp
    src/core/SkTextBlob.cpp
    src/core/SkTextBlobTrace.cpp
    src/core/SkThreadID.cpp
    src/core/SkTime.cpp
    src/core/SkTypeface.cpp
    src/core/SkTypefaceCache.cpp
    src/core/SkTypeface_remote.cpp
    src/core/SkUnPreMultiply.cpp
    src/core/SkUtils.cpp
    src/core/SkUtilsArm.cpp
    src/core/SkVM.cpp
    src/core/SkVMBlitter.cpp
    src/core/SkVertState.cpp
    src/core/SkVertices.cpp
    src/core/SkWriteBuffer.cpp
    src/core/SkWriter32.cpp
    src/core/SkXfermode.cpp
    src/core/SkXfermodeInterpretation.cpp
    src/core/SkYUVAInfo.cpp
    src/core/SkYUVAPixmaps.cpp
    src/core/SkYUVMath.cpp
    src/core/SkYUVPlanesCache.cpp
    src/effects/Sk1DPathEffect.cpp
    src/effects/Sk2DPathEffect.cpp
    src/effects/SkBlenders.cpp
    src/effects/SkColorMatrix.cpp
    src/effects/SkColorMatrixFilter.cpp
    src/effects/SkCornerPathEffect.cpp
    src/effects/SkDashPathEffect.cpp
    src/effects/SkDiscretePathEffect.cpp
    src/effects/SkEmbossMask.cpp
    src/effects/SkEmbossMaskFilter.cpp
    src/effects/SkHighContrastFilter.cpp
    src/effects/SkLayerDrawLooper.cpp
    src/effects/SkLumaColorFilter.cpp
    src/effects/SkOpPathEffect.cpp
    src/effects/SkOverdrawColorFilter.cpp
    src/effects/SkShaderMaskFilter.cpp
    src/effects/SkTableColorFilter.cpp
    src/effects/SkTableMaskFilter.cpp
    src/effects/SkTrimPathEffect.cpp
    src/effects/imagefilters/SkAlphaThresholdImageFilter.cpp
    src/effects/imagefilters/SkArithmeticImageFilter.cpp
    src/effects/imagefilters/SkBlendImageFilter.cpp
    src/effects/imagefilters/SkBlurImageFilter.cpp
    src/effects/imagefilters/SkColorFilterImageFilter.cpp
    src/effects/imagefilters/SkComposeImageFilter.cpp
    src/effects/imagefilters/SkCropImageFilter.cpp
    src/effects/imagefilters/SkDisplacementMapImageFilter.cpp
    src/effects/imagefilters/SkDropShadowImageFilter.cpp
    src/effects/imagefilters/SkImageImageFilter.cpp
    src/effects/imagefilters/SkLightingImageFilter.cpp
    src/effects/imagefilters/SkMagnifierImageFilter.cpp
    src/effects/imagefilters/SkMatrixConvolutionImageFilter.cpp
    src/effects/imagefilters/SkMergeImageFilter.cpp
    src/effects/imagefilters/SkMorphologyImageFilter.cpp
    src/effects/imagefilters/SkOffsetImageFilter.cpp
    src/effects/imagefilters/SkPictureImageFilter.cpp
    src/effects/imagefilters/SkRuntimeImageFilter.cpp
    src/effects/imagefilters/SkShaderImageFilter.cpp
    src/effects/imagefilters/SkTileImageFilter.cpp
    src/fonts/SkFontMgr_indirect.cpp
    src/fonts/SkRemotableFontMgr.cpp
    src/gpu/AtlasTypes.cpp
    src/gpu/Blend.cpp
    src/gpu/RectanizerPow2.cpp
    src/gpu/RectanizerSkyline.cpp
    src/gpu/ResourceKey.cpp
    src/gpu/ShaderErrorHandler.cpp
    src/gpu/Swizzle.cpp
    src/gpu/ganesh/BaseDevice.cpp
    src/gpu/ganesh/GrAHardwareBufferImageGenerator.cpp
    src/gpu/ganesh/GrAHardwareBufferUtils.cpp
    src/gpu/ganesh/GrAttachment.cpp
    src/gpu/ganesh/GrAuditTrail.cpp
    src/gpu/ganesh/GrBackendSemaphore.cpp
    src/gpu/ganesh/GrBackendSurface.cpp
    src/gpu/ganesh/GrBackendSurfaceMutableState.cpp
    src/gpu/ganesh/GrBackendTextureImageGenerator.cpp
    src/gpu/ganesh/GrBackendUtils.cpp
    src/gpu/ganesh/GrBlurUtils.cpp
    src/gpu/ganesh/GrBufferAllocPool.cpp
    src/gpu/ganesh/GrCaps.cpp
    src/gpu/ganesh/GrClientMappedBufferManager.cpp
    src/gpu/ganesh/GrColorInfo.cpp
    src/gpu/ganesh/GrColorSpaceXform.cpp
    src/gpu/ganesh/GrContextThreadSafeProxy.cpp
    src/gpu/ganesh/GrContext_Base.cpp
    src/gpu/ganesh/GrCopyRenderTask.cpp
    src/gpu/ganesh/GrDDLContext.cpp
    src/gpu/ganesh/GrDDLTask.cpp
    src/gpu/ganesh/GrDataUtils.cpp
    src/gpu/ganesh/GrDefaultGeoProcFactory.cpp
    src/gpu/ganesh/GrDirectContext.cpp
    src/gpu/ganesh/GrDirectContextPriv.cpp
    src/gpu/ganesh/GrDistanceFieldGenFromVector.cpp
    src/gpu/ganesh/GrDrawOpAtlas.cpp
    src/gpu/ganesh/GrDrawOpTest.cpp
    src/gpu/ganesh/GrDrawingManager.cpp
    src/gpu/ganesh/GrDriverBugWorkarounds.cpp
    src/gpu/ganesh/GrDynamicAtlas.cpp
    src/gpu/ganesh/GrEagerVertexAllocator.cpp
    src/gpu/ganesh/GrFinishCallbacks.cpp
    src/gpu/ganesh/GrFixedClip.cpp
    src/gpu/ganesh/GrFragmentProcessor.cpp
    src/gpu/ganesh/GrGeometryProcessor.cpp
    src/gpu/ganesh/GrGpu.cpp
    src/gpu/ganesh/GrGpuBuffer.cpp
    src/gpu/ganesh/GrGpuResource.cpp
    src/gpu/ganesh/GrImageContext.cpp
    src/gpu/ganesh/GrImageInfo.cpp
    src/gpu/ganesh/GrManagedResource.cpp
    src/gpu/ganesh/GrMemoryPool.cpp
    src/gpu/ganesh/GrMeshDrawTarget.cpp
    src/gpu/ganesh/GrOnFlushResourceProvider.cpp
    src/gpu/ganesh/GrOpFlushState.cpp
    src/gpu/ganesh/GrOpsRenderPass.cpp
    src/gpu/ganesh/GrPaint.cpp
    src/gpu/ganesh/GrPersistentCacheUtils.cpp
    src/gpu/ganesh/GrPipeline.cpp
    src/gpu/ganesh/GrProcessor.cpp
    src/gpu/ganesh/GrProcessorAnalysis.cpp
    src/gpu/ganesh/GrProcessorSet.cpp
    src/gpu/ganesh/GrProcessorUnitTest.cpp
    src/gpu/ganesh/GrProgramDesc.cpp
    src/gpu/ganesh/GrProgramInfo.cpp
    src/gpu/ganesh/GrProxyProvider.cpp
    src/gpu/ganesh/GrRecordingContext.cpp
    src/gpu/ganesh/GrRecordingContextPriv.cpp
    src/gpu/ganesh/GrRenderTarget.cpp
    src/gpu/ganesh/GrRenderTargetProxy.cpp
    src/gpu/ganesh/GrRenderTask.cpp
    src/gpu/ganesh/GrRenderTaskCluster.cpp
    src/gpu/ganesh/GrResourceAllocator.cpp
    src/gpu/ganesh/GrResourceCache.cpp
    src/gpu/ganesh/GrResourceProvider.cpp
    src/gpu/ganesh/GrRingBuffer.cpp
    src/gpu/ganesh/GrSPIRVUniformHandler.cpp
    src/gpu/ganesh/GrSPIRVVaryingHandler.cpp
    src/gpu/ganesh/GrSWMaskHelper.cpp
    src/gpu/ganesh/GrShaderCaps.cpp
    src/gpu/ganesh/GrShaderVar.cpp
    src/gpu/ganesh/GrStagingBufferManager.cpp
    src/gpu/ganesh/GrStencilSettings.cpp
    src/gpu/ganesh/GrStyle.cpp
    src/gpu/ganesh/GrSurface.cpp
    src/gpu/ganesh/GrSurfaceInfo.cpp
    src/gpu/ganesh/GrSurfaceProxy.cpp
    src/gpu/ganesh/GrTestUtils.cpp
    src/gpu/ganesh/GrTexture.cpp
    src/gpu/ganesh/GrTextureProxy.cpp
    src/gpu/ganesh/GrTextureRenderTargetProxy.cpp
    src/gpu/ganesh/GrTextureResolveRenderTask.cpp
    src/gpu/ganesh/GrThreadSafeCache.cpp
    src/gpu/ganesh/GrThreadSafePipelineBuilder.cpp
    src/gpu/ganesh/GrTransferFromRenderTask.cpp
    src/gpu/ganesh/GrUniformDataManager.cpp
    src/gpu/ganesh/GrUtil.cpp
    src/gpu/ganesh/GrVertexChunkArray.cpp
    src/gpu/ganesh/GrWaitRenderTask.cpp
    src/gpu/ganesh/GrWritePixelsRenderTask.cpp
    src/gpu/ganesh/GrXferProcessor.cpp
    src/gpu/ganesh/GrYUVABackendTextures.cpp
    src/gpu/ganesh/GrYUVATextureProxies.cpp
    src/gpu/ganesh/SkGr.cpp
    src/gpu/ganesh/SurfaceContext.cpp
    src/gpu/ganesh/SurfaceFillContext.cpp
    src/gpu/ganesh/effects/GrBezierEffect.cpp
    src/gpu/ganesh/effects/GrBicubicEffect.cpp
    src/gpu/ganesh/effects/GrBitmapTextGeoProc.cpp
    src/gpu/ganesh/effects/GrBlendFragmentProcessor.cpp
    src/gpu/ganesh/effects/GrConvexPolyEffect.cpp
    src/gpu/ganesh/effects/GrCoverageSetOpXP.cpp
    src/gpu/ganesh/effects/GrCustomXfermode.cpp
    src/gpu/ganesh/effects/GrDisableColorXP.cpp
    src/gpu/ganesh/effects/GrDistanceFieldGeoProc.cpp
    src/gpu/ganesh/effects/GrGaussianConvolutionFragmentProcessor.cpp
    src/gpu/ganesh/effects/GrMatrixConvolutionEffect.cpp
    src/gpu/ganesh/effects/GrMatrixEffect.cpp
    src/gpu/ganesh/effects/GrModulateAtlasCoverageEffect.cpp
    src/gpu/ganesh/effects/GrOvalEffect.cpp
    src/gpu/ganesh/effects/GrPorterDuffXferProcessor.cpp
    src/gpu/ganesh/effects/GrRRectEffect.cpp
    src/gpu/ganesh/effects/GrShadowGeoProc.cpp
    src/gpu/ganesh/effects/GrSkSLFP.cpp
    src/gpu/ganesh/effects/GrTextureEffect.cpp
    src/gpu/ganesh/effects/GrYUVtoRGBEffect.cpp
    src/gpu/ganesh/geometry/GrAAConvexTessellator.cpp
    src/gpu/ganesh/geometry/GrAATriangulator.cpp
    src/gpu/ganesh/geometry/GrPathUtils.cpp
    src/gpu/ganesh/geometry/GrQuad.cpp
    src/gpu/ganesh/geometry/GrQuadUtils.cpp
    src/gpu/ganesh/geometry/GrShape.cpp
    src/gpu/ganesh/geometry/GrStyledShape.cpp
    src/gpu/ganesh/geometry/GrTriangulator.cpp
    src/gpu/ganesh/gl/GrGLAssembleGLESInterfaceAutogen.cpp
    src/gpu/ganesh/gl/GrGLAssembleGLInterfaceAutogen.cpp
    src/gpu/ganesh/gl/GrGLAssembleHelpers.cpp
    src/gpu/ganesh/gl/GrGLAssembleInterface.cpp
    src/gpu/ganesh/gl/GrGLAssembleWebGLInterfaceAutogen.cpp
    src/gpu/ganesh/gl/GrGLAttachment.cpp
    src/gpu/ganesh/gl/GrGLBuffer.cpp
    src/gpu/ganesh/gl/GrGLCaps.cpp
    src/gpu/ganesh/gl/GrGLContext.cpp
    src/gpu/ganesh/gl/GrGLExtensions.cpp
    src/gpu/ganesh/gl/GrGLGLSL.cpp
    src/gpu/ganesh/gl/GrGLGpu.cpp
    src/gpu/ganesh/gl/GrGLGpuProgramCache.cpp
    src/gpu/ganesh/gl/GrGLInterfaceAutogen.cpp
    src/gpu/ganesh/gl/GrGLOpsRenderPass.cpp
    src/gpu/ganesh/gl/GrGLProgram.cpp
    src/gpu/ganesh/gl/GrGLProgramDataManager.cpp
    src/gpu/ganesh/gl/GrGLRenderTarget.cpp
    src/gpu/ganesh/gl/GrGLSemaphore.cpp
    src/gpu/ganesh/gl/GrGLTexture.cpp
    src/gpu/ganesh/gl/GrGLTextureRenderTarget.cpp
    src/gpu/ganesh/gl/GrGLTypesPriv.cpp
    src/gpu/ganesh/gl/GrGLUniformHandler.cpp
    src/gpu/ganesh/gl/GrGLUtil.cpp
    src/gpu/ganesh/gl/GrGLVertexArray.cpp
    src/gpu/ganesh/gl/builders/GrGLProgramBuilder.cpp
    src/gpu/ganesh/gl/builders/GrGLShaderStringBuilder.cpp
    src/gpu/ganesh/gl/win/GrGLMakeNativeInterface_win.cpp
    src/gpu/ganesh/glsl/GrGLSL.cpp
    src/gpu/ganesh/glsl/GrGLSLBlend.cpp
    src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.cpp
    src/gpu/ganesh/glsl/GrGLSLProgramBuilder.cpp
    src/gpu/ganesh/glsl/GrGLSLProgramDataManager.cpp
    src/gpu/ganesh/glsl/GrGLSLShaderBuilder.cpp
    src/gpu/ganesh/glsl/GrGLSLUniformHandler.cpp
    src/gpu/ganesh/glsl/GrGLSLVarying.cpp
    src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.cpp
    src/gpu/ganesh/gradients/GrGradientBitmapCache.cpp
    src/gpu/ganesh/gradients/GrGradientShader.cpp
    src/gpu/ganesh/mock/GrMockCaps.cpp
    src/gpu/ganesh/mock/GrMockGpu.cpp
    src/gpu/ganesh/mock/GrMockTypes.cpp
    src/gpu/ganesh/ops/AAConvexPathRenderer.cpp
    src/gpu/ganesh/ops/AAHairLinePathRenderer.cpp
    src/gpu/ganesh/ops/AALinearizingConvexPathRenderer.cpp
    src/gpu/ganesh/ops/AtlasInstancedHelper.cpp
    src/gpu/ganesh/ops/AtlasPathRenderer.cpp
    src/gpu/ganesh/ops/AtlasRenderTask.cpp
    src/gpu/ganesh/ops/AtlasTextOp.cpp
    src/gpu/ganesh/ops/ClearOp.cpp
    src/gpu/ganesh/ops/DashLinePathRenderer.cpp
    src/gpu/ganesh/ops/DashOp.cpp
    src/gpu/ganesh/ops/DefaultPathRenderer.cpp
    src/gpu/ganesh/ops/DrawAtlasOp.cpp
    src/gpu/ganesh/ops/DrawAtlasPathOp.cpp
    src/gpu/ganesh/ops/DrawMeshOp.cpp
    src/gpu/ganesh/ops/DrawableOp.cpp
    src/gpu/ganesh/ops/FillRRectOp.cpp
    src/gpu/ganesh/ops/FillRectOp.cpp
    src/gpu/ganesh/ops/GrMeshDrawOp.cpp
    src/gpu/ganesh/ops/GrOp.cpp
    src/gpu/ganesh/ops/GrOvalOpFactory.cpp
    src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.cpp
    src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelperWithStencil.cpp
    src/gpu/ganesh/ops/LatticeOp.cpp
    src/gpu/ganesh/ops/OpsTask.cpp
    src/gpu/ganesh/ops/PathInnerTriangulateOp.cpp
    src/gpu/ganesh/ops/PathStencilCoverOp.cpp
    src/gpu/ganesh/ops/PathTessellateOp.cpp
    src/gpu/ganesh/ops/QuadPerEdgeAA.cpp
    src/gpu/ganesh/ops/RegionOp.cpp
    src/gpu/ganesh/ops/ShadowRRectOp.cpp
    src/gpu/ganesh/ops/SmallPathAtlasMgr.cpp
    src/gpu/ganesh/ops/SmallPathRenderer.cpp
    src/gpu/ganesh/ops/SmallPathShapeData.cpp
    src/gpu/ganesh/ops/SoftwarePathRenderer.cpp
    src/gpu/ganesh/ops/StrokeRectOp.cpp
    src/gpu/ganesh/ops/StrokeTessellateOp.cpp
    src/gpu/ganesh/ops/TessellationPathRenderer.cpp
    src/gpu/ganesh/ops/TextureOp.cpp
    src/gpu/ganesh/ops/TriangulatingPathRenderer.cpp
    src/gpu/ganesh/tessellate/GrPathTessellationShader.cpp
    src/gpu/ganesh/tessellate/GrStrokeTessellationShader.cpp
    src/gpu/ganesh/tessellate/GrTessellationShader.cpp
    src/gpu/ganesh/tessellate/PathTessellator.cpp
    src/gpu/ganesh/tessellate/StrokeTessellator.cpp
    src/gpu/ganesh/text/GrAtlasManager.cpp
    src/gpu/ganesh/text/GrDistanceFieldAdjustTable.cpp
    src/gpu/ganesh/text/GrSDFMaskFilter.cpp
    src/gpu/ganesh/text/GrSDFTControl.cpp
    src/gpu/ganesh/text/GrSlug.cpp
    src/gpu/ganesh/text/GrTextBlob.cpp
    src/gpu/ganesh/text/GrTextBlobRedrawCoordinator.cpp
    src/gpu/ganesh/v1/ClipStack.cpp
    src/gpu/ganesh/v1/Device.cpp
    src/gpu/ganesh/v1/Device_drawTexture.cpp
    src/gpu/ganesh/v1/PathRenderer.cpp
    src/gpu/ganesh/v1/PathRendererChain.cpp
    src/gpu/ganesh/v1/StencilMaskHelper.cpp
    src/gpu/ganesh/v1/SurfaceDrawContext.cpp
    src/gpu/ganesh/v1/SurfaceFillContext_v1.cpp
    #   src/gpu/ganesh/vk/GrVkAMDMemoryAllocator.cpp
    #   src/gpu/ganesh/vk/GrVkBuffer.cpp
    #   src/gpu/ganesh/vk/GrVkCaps.cpp
    #   src/gpu/ganesh/vk/GrVkCommandBuffer.cpp
    #   src/gpu/ganesh/vk/GrVkCommandPool.cpp
    #   src/gpu/ganesh/vk/GrVkDescriptorPool.cpp
    #   src/gpu/ganesh/vk/GrVkDescriptorSet.cpp
    #   src/gpu/ganesh/vk/GrVkDescriptorSetManager.cpp
    #   src/gpu/ganesh/vk/GrVkExtensions.cpp
    #   src/gpu/ganesh/vk/GrVkFramebuffer.cpp
    #   src/gpu/ganesh/vk/GrVkGpu.cpp
    #   src/gpu/ganesh/vk/GrVkImage.cpp
    #   src/gpu/ganesh/vk/GrVkImageView.cpp
    #   src/gpu/ganesh/vk/GrVkInterface.cpp
    #   src/gpu/ganesh/vk/GrVkMSAALoadManager.cpp
    #   src/gpu/ganesh/vk/GrVkMemory.cpp
    #   src/gpu/ganesh/vk/GrVkOpsRenderPass.cpp
    #   src/gpu/ganesh/vk/GrVkPipeline.cpp
    #   src/gpu/ganesh/vk/GrVkPipelineState.cpp
    #   src/gpu/ganesh/vk/GrVkPipelineStateBuilder.cpp
    #   src/gpu/ganesh/vk/GrVkPipelineStateCache.cpp
    #   src/gpu/ganesh/vk/GrVkPipelineStateDataManager.cpp
    #   src/gpu/ganesh/vk/GrVkRenderPass.cpp
    #   src/gpu/ganesh/vk/GrVkRenderTarget.cpp
    #   src/gpu/ganesh/vk/GrVkResourceProvider.cpp
    #   src/gpu/ganesh/vk/GrVkSampler.cpp
    #   src/gpu/ganesh/vk/GrVkSamplerYcbcrConversion.cpp
    #   src/gpu/ganesh/vk/GrVkSecondaryCBDrawContext.cpp
    #   src/gpu/ganesh/vk/GrVkSemaphore.cpp
    #   src/gpu/ganesh/vk/GrVkTexture.cpp
    #   src/gpu/ganesh/vk/GrVkTextureRenderTarget.cpp
    #   src/gpu/ganesh/vk/GrVkTypesPriv.cpp
    #   src/gpu/ganesh/vk/GrVkUniformHandler.cpp
    #   src/gpu/ganesh/vk/GrVkUtil.cpp
    #   src/gpu/ganesh/vk/GrVkVaryingHandler.cpp
    src/gpu/tessellate/FixedCountBufferUtils.cpp
    src/gpu/tessellate/Tessellation.cpp
    src/image/SkImage.cpp
    src/image/SkImage_Gpu.cpp
    src/image/SkImage_GpuBase.cpp
    src/image/SkImage_GpuYUVA.cpp
    src/image/SkImage_Lazy.cpp
    src/image/SkImage_Raster.cpp
    src/image/SkRescaleAndReadPixels.cpp
    src/image/SkSurface.cpp
    src/image/SkSurface_Gpu.cpp
    src/image/SkSurface_Raster.cpp
    src/images/SkImageEncoder.cpp
    src/lazy/SkDiscardableMemoryPool.cpp
    src/opts/SkOpts_avx.cpp
    src/opts/SkOpts_crc32.cpp
    src/opts/SkOpts_hsw.cpp
    src/opts/SkOpts_skx.cpp
    src/opts/SkOpts_sse41.cpp
    src/opts/SkOpts_sse42.cpp
    src/opts/SkOpts_ssse3.cpp
    src/pathops/SkAddIntersections.cpp
    src/pathops/SkDConicLineIntersection.cpp
    src/pathops/SkDCubicLineIntersection.cpp
    src/pathops/SkDCubicToQuads.cpp
    src/pathops/SkDLineIntersection.cpp
    src/pathops/SkDQuadLineIntersection.cpp
    src/pathops/SkIntersections.cpp
    src/pathops/SkOpAngle.cpp
    src/pathops/SkOpBuilder.cpp
    src/pathops/SkOpCoincidence.cpp
    src/pathops/SkOpContour.cpp
    src/pathops/SkOpCubicHull.cpp
    src/pathops/SkOpEdgeBuilder.cpp
    src/pathops/SkOpSegment.cpp
    src/pathops/SkOpSpan.cpp
    src/pathops/SkPathOpsAsWinding.cpp
    src/pathops/SkPathOpsCommon.cpp
    src/pathops/SkPathOpsConic.cpp
    src/pathops/SkPathOpsCubic.cpp
    src/pathops/SkPathOpsCurve.cpp
    src/pathops/SkPathOpsDebug.cpp
    src/pathops/SkPathOpsLine.cpp
    src/pathops/SkPathOpsOp.cpp
    src/pathops/SkPathOpsQuad.cpp
    src/pathops/SkPathOpsRect.cpp
    src/pathops/SkPathOpsSimplify.cpp
    src/pathops/SkPathOpsTSect.cpp
    src/pathops/SkPathOpsTightBounds.cpp
    src/pathops/SkPathOpsTypes.cpp
    src/pathops/SkPathOpsWinding.cpp
    src/pathops/SkPathWriter.cpp
    src/pathops/SkReduceOrder.cpp
    #   src/ports/SkDebug_win.cpp
    src/ports/SkDiscardableMemory_none.cpp
    #   src/ports/SkFontHost_win.cpp
    #   src/ports/SkFontMgr_win_dw.cpp
    src/ports/SkGlobalInitialization_default.cpp
    src/ports/SkImageGenerator_none.cpp
    src/ports/SkMemory_malloc.cpp
    #   src/ports/SkOSFile_stdio.cpp
    #   src/ports/SkOSFile_win.cpp
    #   src/ports/SkOSLibrary_win.cpp
    #   src/ports/SkRemotableFontMgr_win_dw.cpp
    #   src/ports/SkScalerContext_win_dw.cpp
    #   src/ports/SkTLS_win.cpp
    #   src/ports/SkTypeface_win_dw.cpp
    src/sfnt/SkOTTable_name.cpp
    src/sfnt/SkOTUtils.cpp
    src/shaders/SkBitmapProcShader.cpp
    src/shaders/SkColorFilterShader.cpp
    src/shaders/SkColorShader.cpp
    src/shaders/SkComposeShader.cpp
    src/shaders/SkImageShader.cpp
    src/shaders/SkLocalMatrixShader.cpp
    src/shaders/SkPerlinNoiseShader.cpp
    src/shaders/SkPictureShader.cpp
    src/shaders/SkShader.cpp
    src/shaders/SkTransformShader.cpp
    src/shaders/gradients/Sk4fGradientBase.cpp
    src/shaders/gradients/Sk4fLinearGradient.cpp
    src/shaders/gradients/SkGradientShader.cpp
    src/shaders/gradients/SkLinearGradient.cpp
    src/shaders/gradients/SkRadialGradient.cpp
    src/shaders/gradients/SkSweepGradient.cpp
    src/shaders/gradients/SkTwoPointConicalGradient.cpp
    src/sksl/SkSLAnalysis.cpp
    src/sksl/SkSLBuiltinMap.cpp
    src/sksl/SkSLBuiltinTypes.cpp
    src/sksl/SkSLCompiler.cpp
    src/sksl/SkSLConstantFolder.cpp
    src/sksl/SkSLContext.cpp
    src/sksl/SkSLDSLParser.cpp
    src/sksl/SkSLDehydrator.cpp
    src/sksl/SkSLErrorReporter.cpp
    src/sksl/SkSLInliner.cpp
    src/sksl/SkSLLexer.cpp
    src/sksl/SkSLMangler.cpp
    src/sksl/SkSLOperator.cpp
    src/sksl/SkSLOutputStream.cpp
    src/sksl/SkSLPool.cpp
    src/sksl/SkSLPosition.cpp
    src/sksl/SkSLRehydrator.cpp
    src/sksl/SkSLSampleUsage.cpp
    src/sksl/SkSLSharedCompiler.cpp
    src/sksl/SkSLString.cpp
    src/sksl/SkSLThreadContext.cpp
    src/sksl/SkSLUtil.cpp
    src/sksl/analysis/SkSLCanExitWithoutReturningValue.cpp
    src/sksl/analysis/SkSLCheckProgramStructure.cpp
    src/sksl/analysis/SkSLFinalizationChecks.cpp
    src/sksl/analysis/SkSLGetLoopUnrollInfo.cpp
    src/sksl/analysis/SkSLIsConstantExpression.cpp
    src/sksl/analysis/SkSLIsSameExpressionTree.cpp
    src/sksl/analysis/SkSLProgramUsage.cpp
    src/sksl/analysis/SkSLSwitchCaseContainsExit.cpp
    src/sksl/codegen/SkSLGLSLCodeGenerator.cpp
    src/sksl/codegen/SkSLMetalCodeGenerator.cpp
    src/sksl/codegen/SkSLPipelineStageCodeGenerator.cpp
    src/sksl/codegen/SkSLSPIRVCodeGenerator.cpp
    src/sksl/codegen/SkSLSPIRVtoHLSL.cpp
    src/sksl/codegen/SkSLVMCodeGenerator.cpp
    src/sksl/codegen/SkSLWGSLCodeGenerator.cpp
    src/sksl/dsl/DSLBlock.cpp
    src/sksl/dsl/DSLCase.cpp
    src/sksl/dsl/DSLCore.cpp
    src/sksl/dsl/DSLExpression.cpp
    src/sksl/dsl/DSLFunction.cpp
    src/sksl/dsl/DSLLayout.cpp
    src/sksl/dsl/DSLRuntimeEffects.cpp
    src/sksl/dsl/DSLStatement.cpp
    src/sksl/dsl/DSLSymbols.cpp
    src/sksl/dsl/DSLType.cpp
    src/sksl/dsl/DSLVar.cpp
    src/sksl/dsl/priv/DSLFPs.cpp
    src/sksl/dsl/priv/DSLWriter.cpp
    src/sksl/ir/SkSLBinaryExpression.cpp
    src/sksl/ir/SkSLBlock.cpp
    src/sksl/ir/SkSLChildCall.cpp
    src/sksl/ir/SkSLConstructor.cpp
    src/sksl/ir/SkSLConstructorArray.cpp
    src/sksl/ir/SkSLConstructorArrayCast.cpp
    src/sksl/ir/SkSLConstructorCompound.cpp
    src/sksl/ir/SkSLConstructorCompoundCast.cpp
    src/sksl/ir/SkSLConstructorDiagonalMatrix.cpp
    src/sksl/ir/SkSLConstructorMatrixResize.cpp
    src/sksl/ir/SkSLConstructorScalarCast.cpp
    src/sksl/ir/SkSLConstructorSplat.cpp
    src/sksl/ir/SkSLConstructorStruct.cpp
    src/sksl/ir/SkSLDoStatement.cpp
    src/sksl/ir/SkSLExpression.cpp
    src/sksl/ir/SkSLExpressionStatement.cpp
    src/sksl/ir/SkSLFieldAccess.cpp
    src/sksl/ir/SkSLForStatement.cpp
    src/sksl/ir/SkSLFunctionCall.cpp
    src/sksl/ir/SkSLFunctionDeclaration.cpp
    src/sksl/ir/SkSLFunctionDefinition.cpp
    src/sksl/ir/SkSLIfStatement.cpp
    src/sksl/ir/SkSLIndexExpression.cpp
    src/sksl/ir/SkSLModifiers.cpp
    src/sksl/ir/SkSLPostfixExpression.cpp
    src/sksl/ir/SkSLPrefixExpression.cpp
    src/sksl/ir/SkSLSetting.cpp
    src/sksl/ir/SkSLSwitchStatement.cpp
    src/sksl/ir/SkSLSwizzle.cpp
    src/sksl/ir/SkSLSymbolTable.cpp
    src/sksl/ir/SkSLTernaryExpression.cpp
    src/sksl/ir/SkSLType.cpp
    src/sksl/ir/SkSLTypeReference.cpp
    src/sksl/ir/SkSLVarDeclarations.cpp
    src/sksl/ir/SkSLVariable.cpp
    src/sksl/ir/SkSLVariableReference.cpp
    src/sksl/tracing/SkVMDebugTrace.cpp
    src/sksl/tracing/SkVMDebugTracePlayer.cpp
    src/sksl/transform/SkSLBuiltinVariableScanner.cpp
    src/sksl/transform/SkSLEliminateDeadFunctions.cpp
    src/sksl/transform/SkSLEliminateDeadGlobalVariables.cpp
    src/sksl/transform/SkSLEliminateDeadLocalVariables.cpp
    src/sksl/transform/SkSLEliminateUnreachableCode.cpp
    src/text/gpu/GlyphVector.cpp
    src/text/gpu/StrikeCache.cpp
    src/text/gpu/SubRunAllocator.cpp
    src/utils/SkAnimCodecPlayer.cpp
    src/utils/SkBase64.cpp
    src/utils/SkCamera.cpp
    src/utils/SkCanvasStack.cpp
    src/utils/SkCanvasStateUtils.cpp
    src/utils/SkCharToGlyphCache.cpp
    src/utils/SkClipStackUtils.cpp
    src/utils/SkCustomTypeface.cpp
    src/utils/SkDashPath.cpp
    src/utils/SkEventTracer.cpp
    src/utils/SkFloatToDecimal.cpp
    src/utils/SkJSON.cpp
    src/utils/SkJSONWriter.cpp
    src/utils/SkMatrix22.cpp
    src/utils/SkMultiPictureDocument.cpp
    src/utils/SkNWayCanvas.cpp
    src/utils/SkNullCanvas.cpp
    src/utils/SkOSPath.cpp
    src/utils/SkOrderedFontMgr.cpp
    src/utils/SkPaintFilterCanvas.cpp
    src/utils/SkParse.cpp
    src/utils/SkParseColor.cpp
    src/utils/SkParsePath.cpp
    src/utils/SkPatchUtils.cpp
    src/utils/SkPolyUtils.cpp
    src/utils/SkShaderUtils.cpp
    src/utils/SkShadowTessellator.cpp
    src/utils/SkShadowUtils.cpp
    src/utils/SkShaperJSONWriter.cpp
    src/utils/SkTextUtils.cpp
    src/utils/SkThreadUtils_pthread.cpp
    src/utils/SkThreadUtils_win.cpp
    src/utils/SkUTF.cpp
    src/utils/SkVMVisualizer.cpp
    #   src/utils/win/SkDWrite.cpp
    #   src/utils/win/SkDWriteFontFileStream.cpp
    #   src/utils/win/SkDWriteGeometrySink.cpp
    #   src/utils/win/SkHRESULT.cpp
    #   src/utils/win/SkIStream.cpp
    #   src/utils/win/SkWGL_win.cpp
    #  third_party/d3d12allocator/D3D12MemAlloc.cpp
    third_party/skcms/skcms.cc
    #   third_party/vulkanmemoryallocator/GrVulkanMemoryAllocator.cpp
    tools/gpu/FlushFinishTracker.cpp
    tools/gpu/TestContext.cpp
    #  tools/gpu/d3d/D3DTestContext.cpp
    #  tools/gpu/d3d/D3DTestUtils.cpp
    #  tools/gpu/vk/VkTestContext.cpp
    #  tools/gpu/vk/VkTestHelper.cpp
    #  tools/gpu/vk/VkTestUtils.cpp
    #  tools/gpu/vk/VkYcbcrSamplerHelper.cpp
    #   tools/sk_app/VulkanSharedWindowContext.cpp
    #   tools/sk_app/VulkanWindowContext.cpp
    #   tools/sk_app/win/D3D12WindowContext_win.cpp
    #   tools/sk_app/win/GLWindowContext_win.cpp
    #   tools/sk_app/win/RasterWindowContext_win.cpp
    #   tools/sk_app/win/VulkanWindowContext_win.cpp
    tools/sk_app/GLWindowContext.cpp
    tools/sk_app/Window.cpp
    tools/sk_app/WindowContext.cpp
)
if (WIN32)
    target_sources(
        skia
        PRIVATE src/ports/SkDebug_win.cpp
        src/ports/SkFontHost_win.cpp
        src/ports/SkFontMgr_win_dw.cpp
        src/ports/SkFontMgr_win_dw_factory.cpp
        src/ports/SkOSFile_stdio.cpp
        src/ports/SkOSFile_win.cpp
        src/ports/SkRemotableFontMgr_win_dw.cpp
        src/ports/SkScalerContext_win_dw.cpp
        src/ports/SkTLS_win.cpp
        src/ports/SkTypeface_win_dw.cpp
        src/ports/SkImageGeneratorWIC.cpp
        src/utils/win/SkAutoCoInitialize.cpp
        src/utils/win/SkDWrite.cpp
        src/utils/win/SkDWriteFontFileStream.cpp
        src/utils/win/SkDWriteGeometrySink.cpp
        src/utils/win/SkHRESULT.cpp
        src/utils/win/SkIStream.cpp
        src/utils/win/SkWGL_win.cpp

        src/gpu/ganesh/d3d/GrD3DAMDMemoryAllocator.cpp
        src/gpu/ganesh/d3d/GrD3DAttachment.cpp
        src/gpu/ganesh/d3d/GrD3DBuffer.cpp
        src/gpu/ganesh/d3d/GrD3DCaps.cpp
        src/gpu/ganesh/d3d/GrD3DCommandList.cpp
        src/gpu/ganesh/d3d/GrD3DCommandSignature.cpp
        src/gpu/ganesh/d3d/GrD3DCpuDescriptorManager.cpp
        src/gpu/ganesh/d3d/GrD3DDescriptorHeap.cpp
        src/gpu/ganesh/d3d/GrD3DDescriptorTableManager.cpp
        src/gpu/ganesh/d3d/GrD3DGpu.cpp
        src/gpu/ganesh/d3d/GrD3DOpsRenderPass.cpp
        src/gpu/ganesh/d3d/GrD3DPipelineState.cpp
        src/gpu/ganesh/d3d/GrD3DPipelineStateBuilder.cpp
        src/gpu/ganesh/d3d/GrD3DPipelineStateDataManager.cpp
        src/gpu/ganesh/d3d/GrD3DRenderTarget.cpp
        src/gpu/ganesh/d3d/GrD3DResourceProvider.cpp
        src/gpu/ganesh/d3d/GrD3DRootSignature.cpp
        src/gpu/ganesh/d3d/GrD3DSemaphore.cpp
        src/gpu/ganesh/d3d/GrD3DTexture.cpp
        src/gpu/ganesh/d3d/GrD3DTextureRenderTarget.cpp
        src/gpu/ganesh/d3d/GrD3DTextureResource.cpp
        src/gpu/ganesh/d3d/GrD3DTypesMinimal.cpp
        src/gpu/ganesh/d3d/GrD3DTypesPriv.cpp
        src/gpu/ganesh/d3d/GrD3DUtil.cpp

        third_party/d3d12allocator/D3D12MemAlloc.cpp

        tools/sk_app/win/GLWindowContext_win.cpp
    )

    target_sources(
        skia
        PRIVATE third_party/skcms/skcms.cc)
endif ()
if (ANDROID)
    target_sources(
        skia
        PRIVATE
        src/gpu/ganesh/gl/egl/GrGLMakeEGLInterface.cpp
        src/gpu/ganesh/gl/egl/GrGLMakeNativeInterface_egl.cpp
        src/ports/SkImageEncoder_NDK.cpp
        src/ports/SkImageGeneratorNDK.cpp
        src/ports/SkNDKConversions.cpp
        src/ports/SkOSFile_posix.cpp
        src/ports/SkOSFile_stdio.cpp
        src/ports/SkDebug_android.cpp
        src/ports/SkFontMgr_android.cpp
        src/ports/SkFontMgr_android_parser.cpp
        src/ports/SkFontMgr_android_factory.cpp
        src/ports/SkFontHost_FreeType.cpp
        src/ports/SkFontHost_FreeType_common.cpp
        src/ports/SkFontHost_FreeType_common.h
        tools/sk_app/android/GLWindowContext_android.cpp
    )
endif ()

target_include_directories(skia PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(skia PRIVATE
    third_party/d3d12allocator
    include/third_party/skcms
)
if (WIN32)
    target_compile_definitions(skia PUBLIC
        NOMINMAX
        SKIA_BACKEND
        SK_GL
        SK_DEBUG
        # SK_VULKAN
        # SK_USE_VMA
        SK_DIRECT3D
        SK_INTERNAL
        SK_GAMMA_SRGB
        SK_GAMMA_APPLY_TO_A8
        SK_SCALAR_TO_FLOAT_EXCLUDED
        SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1
        SK_SUPPORT_GPU=1
        SK_CPU_SSE_LEVEL=30
        SK_SUPPORT_OPENCL=0
        SK_FORCE_DISTANCE_FIELD_TEXT=0
        GR_GL_FUNCTION_TYPE=__stdcall
        SK_BUILD_FOR_WIN32)
endif ()
if (ANDROID)
    target_compile_definitions(skia PUBLIC
        SKIA_BACKEND
        SK_GL
        SK_DEBUG
        # SK_VULKAN
        # SK_USE_VMA
        SK_INTERNAL
        SK_GAMMA_SRGB
        SK_GAMMA_APPLY_TO_A8
        SK_SCALAR_TO_FLOAT_EXCLUDED
        SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1
        SK_SUPPORT_GPU=1
        SK_FORCE_DISTANCE_FIELD_TEXT=0
        SK_BUILD_FOR_ANDROID
        SK_ENABLE_NDK_IMAGES)
endif ()
target_compile_definitions(skia PRIVATE SKIA_IMPLEMENTATION=1)
if (WIN32)
    target_link_libraries(skia PUBLIC d3d12.lib d3dcompiler.lib DXGI.lib OpenGL32.lib)
endif ()
if (ANDROID)

    set(EXPAT_SHARED_LIBS OFF)
    set(EXPAT_BUILD_TOOLS OFF)
    set(EXPAT_BUILD_EXAMPLES OFF)
    set(EXPAT_BUILD_TESTS OFF)
    set(EXPAT_BUILD_DOCS OFF)
    set(EXPAT_ENABLE_INSTALL OFF)
    set(EXPAT_BUILD_PKGCONFIG OFF)
    add_subdirectory(third_party/expat-2.6.2)

    set(BUILD_SHARED_LIBS_SAVED "${BUILD_SHARED_LIBS}")
    set(BUILD_SHARED_LIBS OFF)
    set(SKIP_INSTALL_ALL ON)
    add_subdirectory(third_party/freetype-2.13.2)
    set(BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS_SAVED}")

    target_link_libraries(skia PUBLIC EGL GLESv2 jnigraphics android log expat freetype)
endif ()
