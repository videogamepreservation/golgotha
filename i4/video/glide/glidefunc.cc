/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <windows.h>
#include "video/glide/glide.h"

HINSTANCE glideDLLInst = 0;

void FreeTheGlideDll()
{
  if (glideDLLInst)
  {
    FreeLibrary(glideDLLInst);
    glideDLLInst=0;
  }
}

FxBool GetProcAddresses()
{
  glideDLLInst = LoadLibrary("glide2x.dll");
  if(!glideDLLInst) return FALSE;

	ConvertAndDownloadRle = (ConvertAndDownloadRle_fpt)GetProcAddress(glideDLLInst,"_ConvertAndDownloadRle@64");
	if(! ConvertAndDownloadRle) return FALSE;
	
  grAADrawLine = (grAADrawLine_fpt)GetProcAddress(glideDLLInst,"_grAADrawLine@8");
	if(! grAADrawLine) return FALSE;
	
  grAADrawPoint = (grAADrawPoint_fpt)GetProcAddress(glideDLLInst,"_grAADrawPoint@4");
	if(! grAADrawPoint) return FALSE;
	
  grAADrawPolygon = (grAADrawPolygon_fpt)GetProcAddress(glideDLLInst,"_grAADrawPolygon@12");
	if(! grAADrawPolygon) return FALSE;
	
  grAADrawPolygonVertexList = (grAADrawPolygonVertexList_fpt)GetProcAddress(glideDLLInst,"_grAADrawPolygonVertexList@8");
	if(! grAADrawPolygonVertexList) return FALSE;
	
  grAADrawTriangle = (grAADrawTriangle_fpt)GetProcAddress(glideDLLInst,"_grAADrawTriangle@24");
	if(! grAADrawTriangle) return FALSE;
	
  grAlphaBlendFunction = (grAlphaBlendFunction_fpt)GetProcAddress(glideDLLInst,"_grAlphaBlendFunction@16");
	if(! grAlphaBlendFunction) return FALSE;
	
  grAlphaCombine = (grAlphaCombine_fpt)GetProcAddress(glideDLLInst,"_grAlphaCombine@20");
	if(! grAlphaCombine) return FALSE;
	
  grAlphaControlsITRGBLighting = (grAlphaControlsITRGBLighting_fpt)GetProcAddress(glideDLLInst,"_grAlphaControlsITRGBLighting@4");
	if(! grAlphaControlsITRGBLighting) return FALSE;
	
  grAlphaTestFunction = (grAlphaTestFunction_fpt)GetProcAddress(glideDLLInst,"_grAlphaTestFunction@4");
	if(! grAlphaTestFunction) return FALSE;
	
  grAlphaTestReferenceValue = (grAlphaTestReferenceValue_fpt)GetProcAddress(glideDLLInst,"_grAlphaTestReferenceValue@4");
	if(! grAlphaTestReferenceValue) return FALSE;
	
  grBufferClear = (grBufferClear_fpt)GetProcAddress(glideDLLInst,"_grBufferClear@12");
	if(! grBufferClear) return FALSE;
	
  grBufferNumPending = (grBufferNumPending_fpt)GetProcAddress(glideDLLInst,"_grBufferNumPending@0");
	if(! grBufferNumPending) return FALSE;
	
  grBufferSwap = (grBufferSwap_fpt)GetProcAddress(glideDLLInst,"_grBufferSwap@4");
	if(! grBufferSwap) return FALSE;
	
  grCheckForRoom = (grCheckForRoom_fpt)GetProcAddress(glideDLLInst,"_grCheckForRoom@4");
	if(! grCheckForRoom) return FALSE;
	
  grChromakeyMode = (grChromakeyMode_fpt)GetProcAddress(glideDLLInst,"_grChromakeyMode@4");
	if(! grChromakeyMode) return FALSE;
	
  grChromakeyValue = (grChromakeyValue_fpt)GetProcAddress(glideDLLInst,"_grChromakeyValue@4");
	if(! grChromakeyValue) return FALSE;
	
  grClipWindow = (grClipWindow_fpt)GetProcAddress(glideDLLInst,"_grClipWindow@16");
	if(! grClipWindow) return FALSE;
	
  grColorCombine = (grColorCombine_fpt)GetProcAddress(glideDLLInst,"_grColorCombine@20");
	if(! grColorCombine) return FALSE;
	
  grColorMask = (grColorMask_fpt)GetProcAddress(glideDLLInst,"_grColorMask@8");
	if(! grColorMask) return FALSE;
	
  grConstantColorValue = (grConstantColorValue_fpt)GetProcAddress(glideDLLInst,"_grConstantColorValue@4");
	if(! grConstantColorValue) return FALSE;
	
  grConstantColorValue4 = (grConstantColorValue4_fpt)GetProcAddress(glideDLLInst,"_grConstantColorValue4@16");
	if(! grConstantColorValue4) return FALSE;
	
  grCullMode = (grCullMode_fpt)GetProcAddress(glideDLLInst,"_grCullMode@4");
	if(! grCullMode) return FALSE;
	
  grDepthBiasLevel = (grDepthBiasLevel_fpt)GetProcAddress(glideDLLInst,"_grDepthBiasLevel@4");
	if(! grDepthBiasLevel) return FALSE;
	
  grDepthBufferFunction = (grDepthBufferFunction_fpt)GetProcAddress(glideDLLInst,"_grDepthBufferFunction@4");
	if(! grDepthBufferFunction) return FALSE;
	
  grDepthBufferMode = (grDepthBufferMode_fpt)GetProcAddress(glideDLLInst,"_grDepthBufferMode@4");
	if(! grDepthBufferMode) return FALSE;
	
  grDepthMask = (grDepthMask_fpt)GetProcAddress(glideDLLInst,"_grDepthMask@4");
	if(! grDepthMask) return FALSE;
	
  grDisableAllEffects = (grDisableAllEffects_fpt)GetProcAddress(glideDLLInst,"_grDisableAllEffects@0");
	if(! grDisableAllEffects) return FALSE;
	
  grDitherMode = (grDitherMode_fpt)GetProcAddress(glideDLLInst,"_grDitherMode@4");
	if(! grDitherMode) return FALSE;
	
  grDrawLine = (grDrawLine_fpt)GetProcAddress(glideDLLInst,"_grDrawLine@8");
	if(! grDrawLine) return FALSE;
	
  grDrawPlanarPolygon = (grDrawPlanarPolygon_fpt)GetProcAddress(glideDLLInst,"_grDrawPlanarPolygon@12");
	if(! grDrawPlanarPolygon) return FALSE;
	
  grDrawPlanarPolygonVertexList = (grDrawPlanarPolygonVertexList_fpt)GetProcAddress(glideDLLInst,"_grDrawPlanarPolygonVertexList@8");
	if(! grDrawPlanarPolygonVertexList) return FALSE;
	
  grDrawPoint = (grDrawPoint_fpt)GetProcAddress(glideDLLInst,"_grDrawPoint@4");
	if(! grDrawPoint) return FALSE;
	
  grDrawPolygon = (grDrawPolygon_fpt)GetProcAddress(glideDLLInst,"_grDrawPolygon@12");
	if(! grDrawPolygon) return FALSE;
	
  grDrawPolygonVertexList = (grDrawPolygonVertexList_fpt)GetProcAddress(glideDLLInst,"_grDrawPolygonVertexList@8");
	if(! grDrawPolygonVertexList) return FALSE;
	
  grDrawTriangle = (grDrawTriangle_fpt)GetProcAddress(glideDLLInst,"_grDrawTriangle@12");
	if(! grDrawTriangle) return FALSE;
	
  grErrorSetCallback = (grErrorSetCallback_fpt)GetProcAddress(glideDLLInst,"_grErrorSetCallback@4");
	if(! grErrorSetCallback) return FALSE;
	
  grFogColorValue = (grFogColorValue_fpt)GetProcAddress(glideDLLInst,"_grFogColorValue@4");
	if(! grFogColorValue) return FALSE;
	
  grFogMode = (grFogMode_fpt)GetProcAddress(glideDLLInst,"_grFogMode@4");
	if(! grFogMode) return FALSE;
	
  grFogTable = (grFogTable_fpt)GetProcAddress(glideDLLInst,"_grFogTable@4");
	if(! grFogTable) return FALSE;
	
  grGammaCorrectionValue = (grGammaCorrectionValue_fpt)GetProcAddress(glideDLLInst,"_grGammaCorrectionValue@4");
	if(! grGammaCorrectionValue) return FALSE;
	
  grGlideGetState = (grGlideGetState_fpt)GetProcAddress(glideDLLInst,"_grGlideGetState@4");
	if(! grGlideGetState) return FALSE;
	
  grGlideGetVersion = (grGlideGetVersion_fpt)GetProcAddress(glideDLLInst,"_grGlideGetVersion@4");
	if(! grGlideGetVersion) return FALSE;
	
  grGlideInit = (grGlideInit_fpt)GetProcAddress(glideDLLInst,"_grGlideInit@0");
	if(! grGlideInit) return FALSE;
	
  grGlideSetState = (grGlideSetState_fpt)GetProcAddress(glideDLLInst,"_grGlideSetState@4");
	if(! grGlideSetState) return FALSE;
	
  grGlideShamelessPlug = (grGlideShamelessPlug_fpt)GetProcAddress(glideDLLInst,"_grGlideShamelessPlug@4");
	if(! grGlideShamelessPlug) return FALSE;
	
  grGlideShutdown = (grGlideShutdown_fpt)GetProcAddress(glideDLLInst,"_grGlideShutdown@0");
	if(! grGlideShutdown) return FALSE;
	
  grHints = (grHints_fpt)GetProcAddress(glideDLLInst,"_grHints@8");
	if(! grHints) return FALSE;
	
  grLfbConstantAlpha = (grLfbConstantAlpha_fpt)GetProcAddress(glideDLLInst,"_grLfbConstantAlpha@4");
	if(! grLfbConstantAlpha) return FALSE;
	
  grLfbConstantDepth = (grLfbConstantDepth_fpt)GetProcAddress(glideDLLInst,"_grLfbConstantDepth@4");
	if(! grLfbConstantDepth) return FALSE;
	
  grLfbLock = (grLfbLock_fpt)GetProcAddress(glideDLLInst,"_grLfbLock@24");
	if(! grLfbLock) return FALSE;
	
  grLfbReadRegion = (grLfbReadRegion_fpt)GetProcAddress(glideDLLInst,"_grLfbReadRegion@28");
	if(! grLfbReadRegion) return FALSE;
	
  grLfbUnlock = (grLfbUnlock_fpt)GetProcAddress(glideDLLInst,"_grLfbUnlock@8");
	if(! grLfbUnlock) return FALSE;
	
  grLfbWriteColorFormat = (grLfbWriteColorFormat_fpt)GetProcAddress(glideDLLInst,"_grLfbWriteColorFormat@4");
	if(! grLfbWriteColorFormat) return FALSE;
	
  grLfbWriteColorSwizzle = (grLfbWriteColorSwizzle_fpt)GetProcAddress(glideDLLInst,"_grLfbWriteColorSwizzle@8");
	if(! grLfbWriteColorSwizzle) return FALSE;
	
  grLfbWriteRegion = (grLfbWriteRegion_fpt)GetProcAddress(glideDLLInst,"_grLfbWriteRegion@32");
	if(! grLfbWriteRegion) return FALSE;
	
  grRenderBuffer = (grRenderBuffer_fpt)GetProcAddress(glideDLLInst,"_grRenderBuffer@4");
	if(! grRenderBuffer) return FALSE;
	
  grResetTriStats = (grResetTriStats_fpt)GetProcAddress(glideDLLInst,"_grResetTriStats@0");
	if(! grResetTriStats) return FALSE;
	
  grSplash = (grSplash_fpt)GetProcAddress(glideDLLInst,"_grSplash@20");
	if(! grSplash) return FALSE;
	
  grSstControl = (grSstControl_fpt)GetProcAddress(glideDLLInst,"_grSstControl@4");
	if(! grSstControl) return FALSE;
	
  grSstIdle = (grSstIdle_fpt)GetProcAddress(glideDLLInst,"_grSstIdle@0");
	if(! grSstIdle) return FALSE;
	
  grSstIsBusy = (grSstIsBusy_fpt)GetProcAddress(glideDLLInst,"_grSstIsBusy@0");
	if(! grSstIsBusy) return FALSE;
	
  grSstOrigin = (grSstOrigin_fpt)GetProcAddress(glideDLLInst,"_grSstOrigin@4");
	if(! grSstOrigin) return FALSE;
	
  grSstPerfStats = (grSstPerfStats_fpt)GetProcAddress(glideDLLInst,"_grSstPerfStats@4");
	if(! grSstPerfStats) return FALSE;
	
  grSstQueryBoards = (grSstQueryBoards_fpt)GetProcAddress(glideDLLInst,"_grSstQueryBoards@4");
	if(! grSstQueryBoards) return FALSE;
	
  grSstQueryHardware = (grSstQueryHardware_fpt)GetProcAddress(glideDLLInst,"_grSstQueryHardware@4");
	if(! grSstQueryHardware) return FALSE;
	
  grSstResetPerfStats = (grSstResetPerfStats_fpt)GetProcAddress(glideDLLInst,"_grSstResetPerfStats@0");
	if(! grSstResetPerfStats) return FALSE;
	
  grSstScreenHeight = (grSstScreenHeight_fpt)GetProcAddress(glideDLLInst,"_grSstScreenHeight@0");
	if(! grSstScreenHeight) return FALSE;
	
  grSstScreenWidth = (grSstScreenWidth_fpt)GetProcAddress(glideDLLInst,"_grSstScreenWidth@0");
	if(! grSstScreenWidth) return FALSE;
	
  grSstSelect = (grSstSelect_fpt)GetProcAddress(glideDLLInst,"_grSstSelect@4");
	if(! grSstSelect) return FALSE;
	
  grSstStatus = (grSstStatus_fpt)GetProcAddress(glideDLLInst,"_grSstStatus@0");
	if(! grSstStatus) return FALSE;
	
  grSstVRetraceOn = (grSstVRetraceOn_fpt)GetProcAddress(glideDLLInst,"_grSstVRetraceOn@0");
	if(! grSstVRetraceOn) return FALSE;
	
  grSstVideoLine = (grSstVideoLine_fpt)GetProcAddress(glideDLLInst,"_grSstVideoLine@0");
	if(! grSstVideoLine) return FALSE;
	
  grSstWinClose = (grSstWinClose_fpt)GetProcAddress(glideDLLInst,"_grSstWinClose@0");
	if(! grSstWinClose) return FALSE;
	
  grSstWinOpen = (grSstWinOpen_fpt)GetProcAddress(glideDLLInst,"_grSstWinOpen@28");
	if(! grSstWinOpen) return FALSE;
	
  grTexCalcMemRequired = (grTexCalcMemRequired_fpt)GetProcAddress(glideDLLInst,"_grTexCalcMemRequired@16");
	if(! grTexCalcMemRequired) return FALSE;
	
  grTexClampMode = (grTexClampMode_fpt)GetProcAddress(glideDLLInst,"_grTexClampMode@12");
	if(! grTexClampMode) return FALSE;
	
  grTexCombine = (grTexCombine_fpt)GetProcAddress(glideDLLInst,"_grTexCombine@28");
	if(! grTexCombine) return FALSE;
	
  grTexCombineFunction = (grTexCombineFunction_fpt)GetProcAddress(glideDLLInst,"_grTexCombineFunction@8");
	if(! grTexCombineFunction) return FALSE;
	
  grTexDetailControl = (grTexDetailControl_fpt)GetProcAddress(glideDLLInst,"_grTexDetailControl@16");
	if(! grTexDetailControl) return FALSE;
	
  grTexDownloadMipMap = (grTexDownloadMipMap_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadMipMap@16");
	if(! grTexDownloadMipMap) return FALSE;
	
  grTexDownloadMipMapLevel = (grTexDownloadMipMapLevel_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadMipMapLevel@32");
	if(! grTexDownloadMipMapLevel) return FALSE;
	
  grTexDownloadMipMapLevelPartial = (grTexDownloadMipMapLevelPartial_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadMipMapLevelPartial@40");
	if(! grTexDownloadMipMapLevelPartial) return FALSE;
	
  grTexDownloadTable = (grTexDownloadTable_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadTable@12");
	if(! grTexDownloadTable) return FALSE;
	
  grTexDownloadTablePartial = (grTexDownloadTablePartial_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadTablePartial@20");
	if(! grTexDownloadTablePartial) return FALSE;
	
  grTexFilterMode = (grTexFilterMode_fpt)GetProcAddress(glideDLLInst,"_grTexFilterMode@12");
	if(! grTexFilterMode) return FALSE;
	
  grTexLodBiasValue = (grTexLodBiasValue_fpt)GetProcAddress(glideDLLInst,"_grTexLodBiasValue@8");
	if(! grTexLodBiasValue) return FALSE;
	
  grTexMaxAddress = (grTexMaxAddress_fpt)GetProcAddress(glideDLLInst,"_grTexMaxAddress@4");
	if(! grTexMaxAddress) return FALSE;
	
  grTexMinAddress = (grTexMinAddress_fpt)GetProcAddress(glideDLLInst,"_grTexMinAddress@4");
	if(! grTexMinAddress) return FALSE;
	
  grTexMipMapMode = (grTexMipMapMode_fpt)GetProcAddress(glideDLLInst,"_grTexMipMapMode@12");
	if(! grTexMipMapMode) return FALSE;
	
  grTexMultibase = (grTexMultibase_fpt)GetProcAddress(glideDLLInst,"_grTexMultibase@8");
	if(! grTexMultibase) return FALSE;
	
  grTexMultibaseAddress = (grTexMultibaseAddress_fpt)GetProcAddress(glideDLLInst,"_grTexMultibaseAddress@20");
	if(! grTexMultibaseAddress) return FALSE;
	
  grTexNCCTable = (grTexNCCTable_fpt)GetProcAddress(glideDLLInst,"_grTexNCCTable@8");
	if(! grTexNCCTable) return FALSE;
	
  grTexSource = (grTexSource_fpt)GetProcAddress(glideDLLInst,"_grTexSource@16");
	if(! grTexSource) return FALSE;
	
  grTexTextureMemRequired = (grTexTextureMemRequired_fpt)GetProcAddress(glideDLLInst,"_grTexTextureMemRequired@8");
	if(! grTexTextureMemRequired) return FALSE;
	
  grTriStats = (grTriStats_fpt)GetProcAddress(glideDLLInst,"_grTriStats@8");
	if(! grTriStats) return FALSE;
	
  gu3dfGetInfo = (gu3dfGetInfo_fpt)GetProcAddress(glideDLLInst,"_gu3dfGetInfo@8");
	if(! gu3dfGetInfo) return FALSE;
	
  gu3dfLoad = (gu3dfLoad_fpt)GetProcAddress(glideDLLInst,"_gu3dfLoad@8");
	if(! gu3dfLoad) return FALSE;
	
  guAADrawTriangleWithClip = (guAADrawTriangleWithClip_fpt)GetProcAddress(glideDLLInst,"_guAADrawTriangleWithClip@12");
	if(! guAADrawTriangleWithClip) return FALSE;
	
  guAlphaSource = (guAlphaSource_fpt)GetProcAddress(glideDLLInst,"_guAlphaSource@4");
	if(! guAlphaSource) return FALSE;
	
  guColorCombineFunction = (guColorCombineFunction_fpt)GetProcAddress(glideDLLInst,"_guColorCombineFunction@4");
	if(! guColorCombineFunction) return FALSE;
	
  guDrawPolygonVertexListWithClip = (guDrawPolygonVertexListWithClip_fpt)GetProcAddress(glideDLLInst,"_guDrawPolygonVertexListWithClip@8");
	if(! guDrawPolygonVertexListWithClip) return FALSE;
	
  guDrawTriangleWithClip = (guDrawTriangleWithClip_fpt)GetProcAddress(glideDLLInst,"_guDrawTriangleWithClip@12");
	if(! guDrawTriangleWithClip) return FALSE;
	
  guEncodeRLE16 = (guEncodeRLE16_fpt)GetProcAddress(glideDLLInst,"_guEncodeRLE16@16");
	if(! guEncodeRLE16) return FALSE;
	
  guEndianSwapBytes = (guEndianSwapBytes_fpt)GetProcAddress(glideDLLInst,"_guEndianSwapBytes@4");
	if(! guEndianSwapBytes) return FALSE;
	
  guEndianSwapWords = (guEndianSwapWords_fpt)GetProcAddress(glideDLLInst,"_guEndianSwapWords@4");
	if(! guEndianSwapWords) return FALSE;
	
  guFogGenerateExp = (guFogGenerateExp_fpt)GetProcAddress(glideDLLInst,"_guFogGenerateExp@8");
	if(! guFogGenerateExp) return FALSE;
	
  guFogGenerateExp2 = (guFogGenerateExp2_fpt)GetProcAddress(glideDLLInst,"_guFogGenerateExp2@8");
	if(! guFogGenerateExp2) return FALSE;
	
  guFogGenerateLinear = (guFogGenerateLinear_fpt)GetProcAddress(glideDLLInst,"_guFogGenerateLinear@12");
	if(! guFogGenerateLinear) return FALSE;
	
  guFogTableIndexToW = (guFogTableIndexToW_fpt)GetProcAddress(glideDLLInst,"_guFogTableIndexToW@4");
	if(! guFogTableIndexToW) return FALSE;
	
  guTexAllocateMemory = (guTexAllocateMemory_fpt)GetProcAddress(glideDLLInst,"_guTexAllocateMemory@60");
	if(! guTexAllocateMemory) return FALSE;
	
  guTexChangeAttributes = (guTexChangeAttributes_fpt)GetProcAddress(glideDLLInst,"_guTexChangeAttributes@48");
	if(! guTexChangeAttributes) return FALSE;
	
  guTexCombineFunction = (guTexCombineFunction_fpt)GetProcAddress(glideDLLInst,"_guTexCombineFunction@8");
	if(! guTexCombineFunction) return FALSE;
	
  guTexCreateColorMipMap = (guTexCreateColorMipMap_fpt)GetProcAddress(glideDLLInst,"_guTexCreateColorMipMap@0");
	if(! guTexCreateColorMipMap) return FALSE;
	
  guTexDownloadMipMap = (guTexDownloadMipMap_fpt)GetProcAddress(glideDLLInst,"_guTexDownloadMipMap@12");
	if(! guTexDownloadMipMap) return FALSE;
	
  guTexDownloadMipMapLevel = (guTexDownloadMipMapLevel_fpt)GetProcAddress(glideDLLInst,"_guTexDownloadMipMapLevel@12");
	if(! guTexDownloadMipMapLevel) return FALSE;
	
  guTexGetCurrentMipMap = (guTexGetCurrentMipMap_fpt)GetProcAddress(glideDLLInst,"_guTexGetCurrentMipMap@4");
	if(! guTexGetCurrentMipMap) return FALSE;
	
  guTexGetMipMapInfo = (guTexGetMipMapInfo_fpt)GetProcAddress(glideDLLInst,"_guTexGetMipMapInfo@4");
	if(! guTexGetMipMapInfo) return FALSE;
	
  guTexMemQueryAvail = (guTexMemQueryAvail_fpt)GetProcAddress(glideDLLInst,"_guTexMemQueryAvail@4");
	if(! guTexMemQueryAvail) return FALSE;
	
  guTexMemReset = (guTexMemReset_fpt)GetProcAddress(glideDLLInst,"_guTexMemReset@0");
	if(! guTexMemReset) return FALSE;
	
  guTexSource = (guTexSource_fpt)GetProcAddress(glideDLLInst,"_guTexSource@4");
	if(! guTexSource) return FALSE;

	return TRUE;
}


ConvertAndDownloadRle_fpt         ConvertAndDownloadRle=0;
grAADrawLine_fpt                  grAADrawLine=0;
grAADrawPoint_fpt                 grAADrawPoint=0;
grAADrawPolygon_fpt               grAADrawPolygon=0;
grAADrawPolygonVertexList_fpt     grAADrawPolygonVertexList=0;
grAADrawTriangle_fpt              grAADrawTriangle=0;
grAlphaBlendFunction_fpt          grAlphaBlendFunction=0;
grAlphaCombine_fpt                grAlphaCombine=0;
grAlphaControlsITRGBLighting_fpt  grAlphaControlsITRGBLighting=0;
grAlphaTestFunction_fpt           grAlphaTestFunction=0;
grAlphaTestReferenceValue_fpt     grAlphaTestReferenceValue=0;
grBufferClear_fpt                 grBufferClear=0;
grBufferNumPending_fpt            grBufferNumPending=0;
grBufferSwap_fpt                  grBufferSwap=0;
grCheckForRoom_fpt                grCheckForRoom=0;
grChromakeyMode_fpt               grChromakeyMode=0;
grChromakeyValue_fpt              grChromakeyValue=0;
grClipWindow_fpt                  grClipWindow=0;
grColorCombine_fpt                grColorCombine=0;
grColorMask_fpt                   grColorMask=0;
grConstantColorValue_fpt          grConstantColorValue=0;
grConstantColorValue4_fpt         grConstantColorValue4=0;
grCullMode_fpt                    grCullMode=0;
grDepthBiasLevel_fpt              grDepthBiasLevel=0;
grDepthBufferFunction_fpt         grDepthBufferFunction=0;
grDepthBufferMode_fpt             grDepthBufferMode=0;
grDepthMask_fpt                   grDepthMask=0;
grDisableAllEffects_fpt           grDisableAllEffects=0;
grDitherMode_fpt                  grDitherMode=0;
grDrawLine_fpt                    grDrawLine=0;
grDrawPlanarPolygon_fpt           grDrawPlanarPolygon=0;
grDrawPlanarPolygonVertexList_fpt grDrawPlanarPolygonVertexList=0;
grDrawPoint_fpt                   grDrawPoint=0;
grDrawPolygon_fpt                 grDrawPolygon=0;
grDrawPolygonVertexList_fpt       grDrawPolygonVertexList=0;
grDrawTriangle_fpt                grDrawTriangle=0;
grErrorSetCallback_fpt            grErrorSetCallback=0;
grFogColorValue_fpt               grFogColorValue=0;
grFogMode_fpt                     grFogMode=0;
grFogTable_fpt                    grFogTable=0;
grGammaCorrectionValue_fpt        grGammaCorrectionValue=0;
grGlideGetState_fpt               grGlideGetState=0;
grGlideGetVersion_fpt             grGlideGetVersion=0;
grGlideInit_fpt                   grGlideInit=0;
grGlideSetState_fpt               grGlideSetState=0;
grGlideShamelessPlug_fpt          grGlideShamelessPlug=0;
grGlideShutdown_fpt               grGlideShutdown=0;
grHints_fpt                       grHints=0;
grLfbConstantAlpha_fpt            grLfbConstantAlpha=0;
grLfbConstantDepth_fpt            grLfbConstantDepth=0;
grLfbLock_fpt                     grLfbLock=0;
grLfbReadRegion_fpt               grLfbReadRegion=0;
grLfbUnlock_fpt                   grLfbUnlock=0;
grLfbWriteColorFormat_fpt         grLfbWriteColorFormat=0;
grLfbWriteColorSwizzle_fpt        grLfbWriteColorSwizzle=0;
grLfbWriteRegion_fpt              grLfbWriteRegion=0;
grRenderBuffer_fpt                grRenderBuffer=0;
grResetTriStats_fpt               grResetTriStats=0;
grSplash_fpt                      grSplash=0;
grSstControl_fpt                  grSstControl=0;
grSstIdle_fpt                     grSstIdle=0;
grSstIsBusy_fpt                   grSstIsBusy=0;
grSstOrigin_fpt                   grSstOrigin=0;
grSstPerfStats_fpt                grSstPerfStats=0;
grSstQueryBoards_fpt              grSstQueryBoards=0;
grSstQueryHardware_fpt            grSstQueryHardware=0;
grSstResetPerfStats_fpt           grSstResetPerfStats=0;
grSstScreenHeight_fpt             grSstScreenHeight=0;
grSstScreenWidth_fpt              grSstScreenWidth=0;
grSstSelect_fpt                   grSstSelect=0;
grSstStatus_fpt                   grSstStatus=0;
grSstVRetraceOn_fpt               grSstVRetraceOn=0;
grSstVideoLine_fpt                grSstVideoLine=0;
grSstWinClose_fpt                 grSstWinClose=0;
grSstWinOpen_fpt                  grSstWinOpen=0;
grTexCalcMemRequired_fpt          grTexCalcMemRequired=0;
grTexClampMode_fpt                grTexClampMode=0;
grTexCombine_fpt                  grTexCombine=0;
grTexCombineFunction_fpt          grTexCombineFunction=0;
grTexDetailControl_fpt              grTexDetailControl=0;
grTexDownloadMipMap_fpt             grTexDownloadMipMap=0;
grTexDownloadMipMapLevel_fpt        grTexDownloadMipMapLevel=0;
grTexDownloadMipMapLevelPartial_fpt grTexDownloadMipMapLevelPartial=0;
grTexDownloadTable_fpt              grTexDownloadTable=0;
grTexDownloadTablePartial_fpt       grTexDownloadTablePartial=0;
grTexFilterMode_fpt                 grTexFilterMode=0;
grTexLodBiasValue_fpt               grTexLodBiasValue=0;
grTexMaxAddress_fpt                 grTexMaxAddress=0;
grTexMinAddress_fpt                 grTexMinAddress=0;
grTexMipMapMode_fpt                 grTexMipMapMode=0;
grTexMultibase_fpt                  grTexMultibase=0;
grTexMultibaseAddress_fpt           grTexMultibaseAddress=0;
grTexNCCTable_fpt                   grTexNCCTable=0;
grTexSource_fpt                     grTexSource=0;
grTexTextureMemRequired_fpt         grTexTextureMemRequired=0;
grTriStats_fpt                      grTriStats=0;
gu3dfGetInfo_fpt                    gu3dfGetInfo=0;
gu3dfLoad_fpt                       gu3dfLoad=0;
guAADrawTriangleWithClip_fpt        guAADrawTriangleWithClip=0;
guAlphaSource_fpt                   guAlphaSource=0;
guColorCombineFunction_fpt          guColorCombineFunction=0;
guDrawPolygonVertexListWithClip_fpt guDrawPolygonVertexListWithClip=0;
guDrawTriangleWithClip_fpt          guDrawTriangleWithClip=0;
guEncodeRLE16_fpt                   guEncodeRLE16=0;
guEndianSwapBytes_fpt               guEndianSwapBytes=0;
guEndianSwapWords_fpt               guEndianSwapWords=0;
guFogGenerateExp_fpt                guFogGenerateExp=0;
guFogGenerateExp2_fpt               guFogGenerateExp2=0;
guFogGenerateLinear_fpt             guFogGenerateLinear=0;
guFogTableIndexToW_fpt              guFogTableIndexToW=0;
guTexAllocateMemory_fpt             guTexAllocateMemory=0;
guTexChangeAttributes_fpt           guTexChangeAttributes=0;
guTexCombineFunction_fpt            guTexCombineFunction=0;
guTexCreateColorMipMap_fpt          guTexCreateColorMipMap=0;
guTexDownloadMipMap_fpt             guTexDownloadMipMap=0;
guTexDownloadMipMapLevel_fpt        guTexDownloadMipMapLevel=0;
guTexGetCurrentMipMap_fpt           guTexGetCurrentMipMap=0;
guTexGetMipMapInfo_fpt              guTexGetMipMapInfo=0;
guTexMemQueryAvail_fpt              guTexMemQueryAvail=0;
guTexMemReset_fpt                   guTexMemReset=0;
guTexSource_fpt                     guTexSource=0;

