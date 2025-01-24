#include "win_dx_renderer.h"
#include <d3d12.h>

void copyMemory (u8 *dest, u8 *src, u64 size) {
    for (u64 i = 0; i < size; ++i) {
        dest[i] = src[i];
    }
}

void rendererCreateRTVs (dx_renderer *dxr) {
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxr->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    for (u32 i = 0; i < numSwapChainFrames; ++i) {
        ID3D12Resource *backBuffer;
        DX_TRY(dxr->swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
        dxr->device->CreateRenderTargetView(backBuffer, 0, rtvHandle);
        dxr->backBuffers[i] = backBuffer;

        rtvHandle.ptr += dxr->rtvDescriptorSize;
    }
}

ID3D12Resource *createDynamicBuffer (dx_renderer *dxr, u64 bufferSize) {
    ID3D12Resource *buffer;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;
    bufferDesc.Width = bufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    DX_TRY(dxr->device->CreateCommittedResource(&heapProps,
                                           D3D12_HEAP_FLAG_NONE,
                                           &bufferDesc,
                                           D3D12_RESOURCE_STATE_GENERIC_READ,
                                           0,
                                           IID_PPV_ARGS(&buffer)));
    return buffer;
}

void createGPUBuffer (dx_renderer *dxr, u8 *bufferData, u64 bufferSize, 
                 ID3D12Resource **outBuffer, ID3D12Resource **outTempBuffer) 
{
    ID3D12Resource *tempBuffer;
    ID3D12Resource *buffer;

    {
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Alignment = 0;
        bufferDesc.Width = bufferSize;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.SampleDesc.Quality = 0;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        DX_TRY(dxr->device->CreateCommittedResource(&heapProps,
                                               D3D12_HEAP_FLAG_NONE,
                                               &bufferDesc,
                                               D3D12_RESOURCE_STATE_GENERIC_READ,
                                               0,
                                               IID_PPV_ARGS(&tempBuffer)));
    }
    
    {
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Alignment = 0;
        bufferDesc.Width = bufferSize;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.SampleDesc.Quality = 0;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        DX_TRY(dxr->device->CreateCommittedResource(&heapProps,
                                               D3D12_HEAP_FLAG_NONE,
                                               &bufferDesc,
                                               D3D12_RESOURCE_STATE_COPY_DEST,
                                               0,
                                               IID_PPV_ARGS(&buffer)));
    }

    u8 *tempBufferData;
    DX_TRY(tempBuffer->Map(0, 0, (void **)&tempBufferData));
    copyMemory(tempBufferData, bufferData, bufferSize);
    tempBuffer->Unmap(0, 0);

    dxr->cmdList->CopyBufferRegion(buffer, 0, tempBuffer, 0, bufferSize);

    *outBuffer = buffer;
    *outTempBuffer = tempBuffer;
}

void uploadTextureToGPU (dx_renderer *dxr, u32 width, u32 height, u8 *pixels,
                         ID3D12Resource **outTexture, ID3D12Resource **outTempTexture) 
{
    ID3D12Resource *tempTexture;
    ID3D12Resource *texture;

    D3D12_RESOURCE_ALLOCATION_INFO textureBufferSize;
    {
        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Alignment = 0;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        DX_TRY(dxr->device->CreateCommittedResource(&heapProps,
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &texDesc,
                                                    D3D12_RESOURCE_STATE_COPY_DEST,
                                                    0,
                                                    IID_PPV_ARGS(&texture)));

        textureBufferSize = dxr->device->GetResourceAllocationInfo(0, 1, &texDesc);
    }

    {
        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        texDesc.Alignment = 0;
        texDesc.Width = textureBufferSize.SizeInBytes;
        texDesc.Height = 1;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_UNKNOWN;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        DX_TRY(dxr->device->CreateCommittedResource(&heapProps,
                                               D3D12_HEAP_FLAG_NONE,
                                               &texDesc,
                                               D3D12_RESOURCE_STATE_GENERIC_READ,
                                               0,
                                               IID_PPV_ARGS(&tempTexture)));
    }

    u32 pitchAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
    u32 alignedPitch = (((width * 4 - 1) / pitchAlignment) + 1) * pitchAlignment;

    D3D12_SUBRESOURCE_FOOTPRINT pitchedDesc = {};
    pitchedDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    pitchedDesc.Width = width;
    pitchedDesc.Height = height;
    pitchedDesc.Depth = 1;
    pitchedDesc.RowPitch = alignedPitch;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D = {};
    placedTexture2D.Offset = 0;
    placedTexture2D.Footprint = pitchedDesc;

    u8 *tempTextureData;
    DX_TRY(tempTexture->Map(0, 0, (void **)&tempTextureData));
    for (u32 row = 0; row < height; row++) {
        u8 *dest = tempTextureData + row * alignedPitch;
        copyMemory(dest, &pixels[row * width * 4], width * 4);
    }
    tempTexture->Unmap(0, 0);

    D3D12_TEXTURE_COPY_LOCATION destCopyLocation = {};
    destCopyLocation.pResource = texture;
    destCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    destCopyLocation.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION srcCopyLocation = {};
    srcCopyLocation.pResource = tempTexture;
    srcCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcCopyLocation.PlacedFootprint = placedTexture2D;

    dxr->cmdList->CopyTextureRegion(&destCopyLocation, 0, 0, 0, &srcCopyLocation, 0);

    {
        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = texture;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        dxr->cmdList->ResourceBarrier(1, &barrier);
    }

    *outTexture = texture;
    *outTempTexture = tempTexture;
}

void rendererSignal (dx_renderer *dxr) {
    u64 fenceSignalledValue = ++dxr->fenceValue;
    DX_TRY(dxr->queue->Signal(dxr->fence, fenceSignalledValue)); 
}

void rendererWaitForFenceValue (dx_renderer *dxr) {
    if (dxr->fence->GetCompletedValue() < dxr->fenceValue) {
        DX_TRY(dxr->fence->SetEventOnCompletion(dxr->fenceValue, dxr->fenceEvent));
        WaitForSingleObject(dxr->fenceEvent, INFINITE);
    }
}
D3D12_BLEND_DESC getDefaultBlendDesc () {
    D3D12_BLEND_DESC result = {};

    result.AlphaToCoverageEnable = false;
    result.IndependentBlendEnable = false;
    result.RenderTarget[0].BlendEnable = true;
    result.RenderTarget[0].LogicOpEnable = false;
    result.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    result.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    result.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    result.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    result.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    result.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    result.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    result.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    return result;
}
D3D12_RASTERIZER_DESC getDefaultRasterizerState () {
    D3D12_RASTERIZER_DESC result = {};

    result.FillMode = D3D12_FILL_MODE_SOLID;
    result.CullMode = D3D12_CULL_MODE_NONE;
    result.FrontCounterClockwise = true;
    result.DepthBias = 0;
    result.DepthBiasClamp = 0.0f;
    result.SlopeScaledDepthBias = 0.0f;
    result.DepthClipEnable = true;
    result.MultisampleEnable = false;
    result.AntialiasedLineEnable = false;
    result.ForcedSampleCount = 0;
    result.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    return result;
}

D3D12_DEPTH_STENCIL_DESC getDefaultDepthStencilDesc () {
    D3D12_DEPTH_STENCIL_DESC result = {};

    result.DepthEnable = false;
    result.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    result.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    result.StencilEnable = false;
    result.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    result.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    result.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP; 
    result.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP; 
    result.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    result.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    result.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    result.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    result.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    result.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    return result;
}

void createOrResizeDepthBuffer (dx_renderer *dxr, u32 width, u32 height) {
    ASSERT(width > 0);
    ASSERT(height > 0);

    if (dxr->depthBuffer) {
        dxr->depthBuffer->Release();
    }

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil = { 1.0f, 0 };

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D ;
    textureDesc.Alignment = 0;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.MipLevels = 0;
    textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    DX_TRY(dxr->device->CreateCommittedResource(&heapProps,
                                                D3D12_HEAP_FLAG_NONE,
                                                &textureDesc,
                                                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                &clearValue,
                                                IID_PPV_ARGS(&dxr->depthBuffer)));
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    dxr->device->CreateDepthStencilView(dxr->depthBuffer, &dsvDesc,
            dxr->dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

ID3D12Resource **pushTempUploadBuffer (ID3D12Resource **tempUploadBuffers, u32 *numUploadBuffers, u32 maxUploadBuffers) {
    ASSERT(*numUploadBuffers < maxUploadBuffers);
    return &tempUploadBuffers[(*numUploadBuffers)++];
}

void freeTempUploadBuffers (ID3D12Resource **tempUploadBuffers, u32 numUploadBuffers) {
    for (u32 tempResourceIndex = 0; tempResourceIndex < numUploadBuffers; tempResourceIndex++) {
        ID3D12Resource *tempResource = tempUploadBuffers[tempResourceIndex];
        tempResource->Release();
    }
}

void initRenderer (dx_renderer *dxr, HWND window) {
    // Init debug layer if debugging
#ifdef _DEBUG
    //ID3D12Debug *debugInterface;
    //DX_TRY(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))) 
    //debugInterface->EnableDebugLayer();
    //debugInterface->Release();
#endif

    // Create DXGI factory in order to get adapter
    IDXGIFactory4 *factory;
    u32 factoryFlags = 0;
#ifdef _DEBUG
    //factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    DX_TRY(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory))); 

    // Query graphics adapters; take the one with the most memory
    IDXGIAdapter1 *adapter1 = 0;
    IDXGIAdapter4 *adapter4 = 0;
    u32 maxMemory = 0;
    // TODO: order by performance?
    for (u32 i = 0; factory->EnumAdapters1(i, &adapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 adapterDesc;
        adapter1->GetDesc1(&adapterDesc);

        if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
            SUCCEEDED(D3D12CreateDevice(adapter1, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), 0)) &&
            adapterDesc.DedicatedVideoMemory > maxMemory)
        {
            maxMemory = adapterDesc.DedicatedVideoMemory;

            DX_TRY(adapter1->QueryInterface(IID_PPV_ARGS(&adapter4))); 
        }

        adapter1->Release();
    }

    // Create device with adapter; fall back to WARP if failed
    HRESULT result = D3D12CreateDevice(adapter4, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dxr->device));
    if (adapter4) {
        adapter4->Release();
    }
    if (FAILED(result)) {
        OutputDebugStringA("Failed to create device: trying WARP\n");

        IDXGIAdapter *warpAdapter;
        DX_TRY(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        DX_TRY(D3D12CreateDevice(warpAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dxr->device)));
        warpAdapter->Release();
    }

    ID3D12Device2 *device = dxr->device;

    // Set up debug messaging
#ifdef _DEBUG
    //ID3D12InfoQueue *infoQueue;
    //DX_TRY(device->QueryInterface(IID_PPV_ARGS(&infoQueue))); 
    //infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    //infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    //infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

    //D3D12_MESSAGE_CATEGORY suppressedCategories[] = {};
    //D3D12_MESSAGE_SEVERITY suppressedSeverities[] = {
        //D3D12_MESSAGE_SEVERITY_INFO
    //};
    //D3D12_MESSAGE_ID suppressedMessages[] = {
        //D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE
    //};

    //D3D12_INFO_QUEUE_FILTER errorFilter = {};
    //errorFilter.DenyList.NumCategories = ARRAY_COUNT(suppressedCategories);
    //errorFilter.DenyList.pCategoryList = suppressedCategories;
    //errorFilter.DenyList.NumSeverities = ARRAY_COUNT(suppressedSeverities);
    //errorFilter.DenyList.pSeverityList = suppressedSeverities;
    //errorFilter.DenyList.NumIDs = ARRAY_COUNT(suppressedMessages);
    //errorFilter.DenyList.pIDList = suppressedMessages;
//
    //DX_TRY(infoQueue->PushStorageFilter(&errorFilter));

    //infoQueue->Release();
#endif

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    DX_TRY(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&dxr->queue))); 

    b32 allowTearing = false;
    IDXGIFactory5 *factory5;
    HRESULT tearingCheckResult;
    if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory5)))) {
        tearingCheckResult = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                                                           &allowTearing,
                                                           sizeof(allowTearing));
        factory5->Release();
    } 
    dxr->allowTearing = SUCCEEDED(tearingCheckResult) && allowTearing;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = 0; // Get width/height from window size
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = numSwapChainFrames;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = dxr->allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    IDXGISwapChain1 *swapChain1;
    DX_TRY(factory->CreateSwapChainForHwnd(dxr->queue, 
                                           window, 
                                           &swapChainDesc, 
                                           0,
                                           0,
                                           &swapChain1));

    DX_TRY(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));
    factory->Release();

    DX_TRY(swapChain1->QueryInterface(IID_PPV_ARGS(&dxr->swapChain))); 
    swapChain1->Release();

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    descriptorHeapDesc.NumDescriptors = numSwapChainFrames;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    DX_TRY(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&dxr->rtvDescriptorHeap)));

    dxr->rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dxr->cbvSRVUAVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    rendererCreateRTVs(dxr);

    for (i32 i = 0; i < numSwapChainFrames; ++i) {
        ID3D12CommandAllocator *cmdAllocator;
        DX_TRY(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                                    IID_PPV_ARGS(&cmdAllocator)));
        dxr->cmdAllocators[i] = cmdAllocator;
    }

    DX_TRY(device->CreateCommandList(0, 
                                     D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                     dxr->cmdAllocators[dxr->currBackBufferIndex], 
                                     0,
                                     IID_PPV_ARGS(&dxr->cmdList)));
    DX_TRY(dxr->cmdList->Close());

    DX_TRY(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dxr->fence)));

    dxr->fenceEvent = CreateEvent(0, false, false, 0);
    if (!dxr->fenceEvent) {
        OutputDebugStringA("Failed to create fence event");
        ExitProcess(1);
    }

    ID3D12CommandAllocator *currCmdAllocator = dxr->cmdAllocators[dxr->currBackBufferIndex];
    currCmdAllocator->Reset();
    dxr->cmdList->Reset(currCmdAllocator, 0);

    // Create sprite buffers
    u64 vertexBufferSize =  sizeof(vertex_sprite) * 4 * MAX_NUM_SPRITES;
    dxr->spriteVertexBuffer = createDynamicBuffer(dxr, vertexBufferSize);
    DX_TRY(dxr->spriteVertexBuffer->Map(0, 0, (void **)&dxr->spriteVBufferData));

    dxr->spriteVBufferView.BufferLocation = dxr->spriteVertexBuffer->GetGPUVirtualAddress();
    dxr->spriteVBufferView.SizeInBytes = vertexBufferSize;
    dxr->spriteVBufferView.StrideInBytes = sizeof(vertex_sprite);

    const u16 numSpriteIndices = MAX_NUM_SPRITES * 6;
    u16 indices[numSpriteIndices];
    for (u16 spriteIndex = 0; spriteIndex < MAX_NUM_SPRITES; spriteIndex++) {
        indices[spriteIndex * 6 + 0] = (spriteIndex * 4) + 0;
        indices[spriteIndex * 6 + 1] = (spriteIndex * 4) + 2;
        indices[spriteIndex * 6 + 2] = (spriteIndex * 4) + 1;
        indices[spriteIndex * 6 + 3] = (spriteIndex * 4) + 1;
        indices[spriteIndex * 6 + 4] = (spriteIndex * 4) + 2;
        indices[spriteIndex * 6 + 5] = (spriteIndex * 4) + 3;
    }
    u64 indexBufferSize = sizeof(indices);

    const int maxTempResources = 100;
    ID3D12Resource *tempResources[maxTempResources];
    unsigned int numTempResources = 0;

    ID3D12Resource **tempIndexBuffer = pushTempUploadBuffer(tempResources, &numTempResources, maxTempResources);
    createGPUBuffer(dxr, (u8 *)indices, indexBufferSize, &dxr->spriteIndexBuffer, tempIndexBuffer);

    dxr->spriteIBufferView.BufferLocation = dxr->spriteIndexBuffer->GetGPUVirtualAddress();
    dxr->spriteIBufferView.Format = DXGI_FORMAT_R16_UINT;
    dxr->spriteIBufferView.SizeInBytes = indexBufferSize;

    // load resources: execute cmd list, wait on fence
    DX_TRY(dxr->cmdList->Close());

    ID3D12CommandList *commandLists[] = {
        dxr->cmdList
    };
    dxr->queue->ExecuteCommandLists(ARRAY_COUNT(commandLists), commandLists);
    rendererSignal(dxr);
    rendererWaitForFenceValue(dxr);

    ID3D12DescriptorHeap *dsvHeap;

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DX_TRY(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dxr->dsvHeap)));

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEX_COORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEX_ID", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    D3D12_DESCRIPTOR_RANGE descRanges[1];
    descRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descRanges[0].NumDescriptors = 16;
    descRanges[0].BaseShaderRegister = 0;
    descRanges[0].RegisterSpace = 0;
    descRanges[0].OffsetInDescriptorsFromTableStart = 0;

    D3D12_ROOT_PARAMETER rootParams[2] = {};

    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[0].DescriptorTable.pDescriptorRanges = descRanges;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParams[1].Constants = { 0, 0, sizeof(screen_dims) / sizeof(f32) };
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_STATIC_SAMPLER_DESC defaultSamplerDesc = {};
    defaultSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    defaultSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    defaultSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    defaultSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    defaultSamplerDesc.MipLODBias = 0;
    defaultSamplerDesc.MaxAnisotropy = 0;
    //defaultSamplerDesc.ComparisonFunc = 0;
    //defaultSamplerDesc.BorderColor = 0;
    defaultSamplerDesc.MinLOD = 0;
    defaultSamplerDesc.MaxLOD = 0;
    defaultSamplerDesc.ShaderRegister = 0;
    defaultSamplerDesc.RegisterSpace = 0;
    defaultSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = ARRAY_COUNT(rootParams);
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.NumStaticSamplers = 1;
    rootSigDesc.pStaticSamplers = &defaultSamplerDesc;
    rootSigDesc.Flags = rootSignatureFlags;

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigVersionedDesc = {};
    rootSigVersionedDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
    rootSigVersionedDesc.Desc_1_0 = rootSigDesc;

    ID3DBlob *rootSigBlob;
    ID3DBlob *errorBlob;
    DX_TRY(D3D12SerializeVersionedRootSignature(&rootSigVersionedDesc, &rootSigBlob, &errorBlob));

    DX_TRY(device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&dxr->spriteBatchRootSignature)));

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
    pipelineDesc.pRootSignature = dxr->spriteBatchRootSignature;
    pipelineDesc.VS = { test_vertex_bytes, ARRAY_COUNT(test_vertex_bytes) };
    pipelineDesc.PS = { test_pixel_bytes, ARRAY_COUNT(test_pixel_bytes) };
    pipelineDesc.BlendState = getDefaultBlendDesc();
    pipelineDesc.SampleMask = 0xffffffff;
    pipelineDesc.RasterizerState = getDefaultRasterizerState();
    pipelineDesc.DepthStencilState = getDefaultDepthStencilDesc();
    pipelineDesc.InputLayout = { inputLayout, ARRAY_COUNT(inputLayout) };
    pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineDesc.NumRenderTargets = 1;
    pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineDesc.SampleDesc.Count = 1;
    pipelineDesc.SampleDesc.Quality = 0;

    DX_TRY(device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&dxr->spriteBatchPipelineState)));

    RECT windowRect = {};
    GetClientRect(window, &windowRect);
    dxr->windowWidth = windowRect.right - windowRect.left;
    dxr->windowHeight = windowRect.bottom - windowRect.top;
    createOrResizeDepthBuffer(dxr, dxr->windowWidth, dxr->windowHeight);

    freeTempUploadBuffers(tempResources, numTempResources);

    {
        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
        descriptorHeapDesc.NumDescriptors = 100; // TODO(ebuchholz): max textures/constant buffers define
        descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        DX_TRY(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&dxr->permDescriptorHeap)));
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
        descriptorHeapDesc.NumDescriptors = 10000; // TODO(ebuchholz): ring buffer for gpu heap? what happens when it wraps around?
        descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        DX_TRY(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&dxr->perFrameDescriptorHeap)));
    }
}

// TODO: consider taking fence value as parameter, if e.g. in the future there are multiple frames
// in flight waiting for different fence values
void rendererFlush (dx_renderer *dxr) {
    rendererSignal(dxr);
    rendererWaitForFenceValue(dxr);
}

void rendererResize (dx_renderer *dxr, u32 windowWidth, u32 windowHeight) {
    // NOTE: shouldn't be necessary with current setup, shouldn't be any commands on gpu when this
    // function is called
    u64 lastCompletedFenceValue = dxr->fence->GetCompletedValue();
    ASSERT(lastCompletedFenceValue == dxr->fenceValue);

    // NOTE: alternatively, if this works more asynchronously in the future, just flush the commands
    //rendererFlush(dxr);
    // ...update bb fence values if more than one...

    for (u32 i = 0; i < numSwapChainFrames; ++i) {
        dxr->backBuffers[i]->Release();
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    DX_TRY(dxr->swapChain->GetDesc(&swapChainDesc));
    DX_TRY(dxr->swapChain->ResizeBuffers(numSwapChainFrames, 
                                               windowWidth, windowHeight, 
                                               swapChainDesc.BufferDesc.Format, 
                                               swapChainDesc.Flags));

    dxr->currBackBufferIndex = dxr->swapChain->GetCurrentBackBufferIndex();

    rendererCreateRTVs(dxr);
    createOrResizeDepthBuffer(dxr, windowWidth, windowHeight);

    dxr->windowWidth = windowWidth;
    dxr->windowHeight = windowHeight;
}

void spriteBatchFlush (dx_renderer *dxr) {
    D3D12_CPU_DESCRIPTOR_HANDLE currentPerFrameDescriptorCPUHandle = dxr->perFrameDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    currentPerFrameDescriptorCPUHandle.ptr += dxr->numPerFrameDescriptors * dxr->cbvSRVUAVDescriptorSize;

    D3D12_GPU_DESCRIPTOR_HANDLE currentPerFrameDescriptorGPUHandle = dxr->perFrameDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    currentPerFrameDescriptorGPUHandle.ptr += dxr->numPerFrameDescriptors * dxr->cbvSRVUAVDescriptorSize;

    dxr->cmdList->SetGraphicsRootDescriptorTable(0, currentPerFrameDescriptorGPUHandle);
    dxr->numPerFrameDescriptors += dxr->spriteBatchNumSeenTextures;

    D3D12_CPU_DESCRIPTOR_HANDLE textureDescriptors[16];
    u32 numTextureIDsToCopy = 0;
    for (u32 textureIndex = 0; textureIndex < MAX_NUM_DXR_TEXTURES; textureIndex++) {
        u32 shaderTextureIndex = dxr->spriteBatchSeenTextures[textureIndex];
        if (shaderTextureIndex != UNUSED_TEXTURE_VAL) {
            textureDescriptors[shaderTextureIndex] = dxr->textures[textureIndex].descriptor;
            numTextureIDsToCopy++;
        }
    }

    for (u32 textureIndex = 0; textureIndex < numTextureIDsToCopy; textureIndex++) {
        dxr->device->CopyDescriptorsSimple(1,
                currentPerFrameDescriptorCPUHandle,
                textureDescriptors[textureIndex],
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        currentPerFrameDescriptorCPUHandle.ptr += dxr->cbvSRVUAVDescriptorSize;
    }

    dxr->cmdList->DrawIndexedInstanced(6 * dxr->spriteBatchNumSpritesDrawn, 1, dxr->numSpritesRenderedThisFrame * 6, 0, 0);
    dxr->numSpritesRenderedThisFrame += dxr->spriteBatchNumSpritesDrawn;
}

void spriteBatchOnBatchStart (dx_renderer *dxr) {
    dxr->spriteBatchNumSpritesDrawn = 0;
    dxr->spriteBatchNumSeenTextures = 0;
    for (u32 textureIndex = 0; textureIndex < MAX_NUM_DXR_TEXTURES; textureIndex++) {
        dxr->spriteBatchSeenTextures[textureIndex] = UNUSED_TEXTURE_VAL;
    }
}

void spriteBatchDrawSprites (dx_renderer *dxr, render_cmd_sprite_data *sprites, u32 numSprites) {
    for (u32 spriteIndex = 0; spriteIndex < numSprites; spriteIndex++) {
        render_cmd_sprite_data *sprite = &sprites[spriteIndex];

        ASSERT(dxr->spriteBatchNumSpritesDrawn + dxr->numSpritesRenderedThisFrame < MAX_NUM_SPRITES);
        ASSERT(sprite->textureID < MAX_NUM_DXR_TEXTURES);

        u32 shaderTextureIndex = UNUSED_TEXTURE_VAL;
        if (dxr->spriteBatchSeenTextures[sprite->textureID] == UNUSED_TEXTURE_VAL) {
            if (dxr->spriteBatchNumSeenTextures >= 16) { // array of 16 textures defined in shader
                spriteBatchFlush(dxr);
                spriteBatchOnBatchStart(dxr);
            }
            dxr->spriteBatchSeenTextures[sprite->textureID] = dxr->spriteBatchNumSeenTextures;
            shaderTextureIndex = dxr->spriteBatchNumSeenTextures;
            dxr->spriteBatchNumSeenTextures++;
        }
        else {
            shaderTextureIndex = dxr->spriteBatchSeenTextures[sprite->textureID];
        }

        u8 *spriteWriteStart = dxr->spriteVBufferData + (dxr->spriteBatchNumSpritesDrawn + dxr->numSpritesRenderedThisFrame) * sizeof(vertex_sprite) * 4;
        for (u32 vIndex = 0; vIndex < 4; vIndex++) {
            vertex_sprite vertex = {};
            vertex.pos[0] = sprite->positions[vIndex * 3 + 0];
            vertex.pos[1] = sprite->positions[vIndex * 3 + 1];
            vertex.pos[2] = sprite->positions[vIndex * 3 + 2];
            vertex.texCoords[0] = sprite->texCoords[vIndex * 2 + 0];
            vertex.texCoords[1] = sprite->texCoords[vIndex * 2 + 1];
            vertex.color[0] = sprite->colors[vIndex * 4 + 0];
            vertex.color[1] = sprite->colors[vIndex * 4 + 1];
            vertex.color[2] = sprite->colors[vIndex * 4 + 2];
            vertex.color[3] = sprite->colors[vIndex * 4 + 3];
            vertex.textureID = shaderTextureIndex;
            copyMemory(spriteWriteStart, (u8 *)&vertex, sizeof(vertex_sprite));
            spriteWriteStart += sizeof(vertex_sprite);
        }

        //dxr->numSpritesRenderedThisFrame++;
        dxr->spriteBatchNumSpritesDrawn++;
    }
}

void renderGame (dx_renderer *dxr, mem_arena *renderMemory) {
    ID3D12CommandAllocator *currCmdAllocator = dxr->cmdAllocators[dxr->currBackBufferIndex];
    ID3D12GraphicsCommandList *cmdList = dxr->cmdList;
    ID3D12Resource *currBackBuffer = dxr->backBuffers[dxr->currBackBufferIndex];

    currCmdAllocator->Reset();
    cmdList->Reset(currCmdAllocator, 0);

    dxr->numPerFrameDescriptors = 0;
    dxr->numSpritesRenderedThisFrame = 0;

    {
        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = currBackBuffer;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        cmdList->ResourceBarrier(1, &barrier);
    }

    f32 clearColor[] = { 0.05f,0.07f,0.16f, 1.0f };
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxr->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += dxr->currBackBufferIndex * dxr->rtvDescriptorSize;
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, 0);

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxr->dsvHeap->GetCPUDescriptorHandleForHeapStart();
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, 0);

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (f32)dxr->windowWidth;
    viewport.Height = (f32)dxr->windowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    cmdList->RSSetViewports(1, &viewport);

    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = dxr->windowWidth;
    scissorRect.bottom = dxr->windowHeight;
    cmdList->RSSetScissorRects(1, &scissorRect);

    cmdList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

    cmdList->SetDescriptorHeaps(1, &dxr->perFrameDescriptorHeap);

    // Keep track of temp resources to upload
    const int maxTempResources = 100;
    ID3D12Resource *tempResources[maxTempResources];
    unsigned int numTempResources = 0;

    dxr_loaded_texture loadedTextures[100];
    unsigned int numLoadedTextures = 0;

    void *renderCursor = renderMemory->base;
    while (renderCursor < renderMemory->current) {
        render_cmd_header *header = (render_cmd_header *)renderCursor;
        renderCursor = (u8 *)renderCursor + sizeof(render_cmd_header);
        switch (header->type) {
            default: {
                ASSERT(false);
            } break;
            case RENDER_CMD_TYPE_LOAD_TEXTURE: {
                render_cmd_load_texture *loadTextureCommand = (render_cmd_load_texture *)renderCursor;
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_load_texture);

                ASSERT(dxr->numTextures < MAX_NUM_DXR_TEXTURES);

                dxr_texture *texture = &dxr->textures[dxr->numTextures++];
                texture->id = loadTextureCommand->id;

                ID3D12Resource **tempTextureBuffer = pushTempUploadBuffer(tempResources, &numTempResources, maxTempResources);
                uploadTextureToGPU(dxr, loadTextureCommand->width, loadTextureCommand->height, loadTextureCommand->pixels, &texture->resource, tempTextureBuffer);

                D3D12_CPU_DESCRIPTOR_HANDLE currentPermHeapHandle = dxr->permDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
                currentPermHeapHandle.ptr += dxr->cbvSRVUAVDescriptorSize * dxr->numPermHeapDescriptors++;
                texture->descriptor = currentPermHeapHandle;

                dxr_loaded_texture *loadedTexture = &loadedTextures[numLoadedTextures++];
                loadedTexture->texture = texture;
            } break;
            case RENDER_CMD_TYPE_SPRITE_BATCH_START: {
                // NOTE: no data in render cmd

                // reset counts, transition resources
                cmdList->SetPipelineState(dxr->spriteBatchPipelineState);
                cmdList->SetGraphicsRootSignature(dxr->spriteBatchRootSignature);

                cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                cmdList->IASetVertexBuffers(0, 1, &dxr->spriteVBufferView);
                cmdList->IASetIndexBuffer(&dxr->spriteIBufferView);

                screen_dims screenDims = {};
                screenDims.width = dxr->windowWidth;
                screenDims.height = dxr->windowHeight;

                cmdList->SetGraphicsRoot32BitConstants(1, sizeof(screen_dims) / sizeof(f32), &screenDims, 0);

                spriteBatchOnBatchStart(dxr);
            } break;
            case RENDER_CMD_TYPE_SPRITE_BATCH_DRAW: {
                render_cmd_sprite_batch_draw *cmd = (render_cmd_sprite_batch_draw *)renderCursor;
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_sprite_batch_draw);

                // build cmd list, tables, flush as needed
                spriteBatchDrawSprites(dxr, cmd->sprites, cmd->numSprites);

                // advance past the list of sprites
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_sprite_data) * cmd->numSprites;
            } break;
            case RENDER_CMD_TYPE_SPRITE_BATCH_END: {
                // NOTE: no data in render cmd

                // flush leftover sprites
                if (dxr->spriteBatchNumSpritesDrawn > 0) {
                    spriteBatchFlush(dxr);
                }
            } break;
        }
    }

    {
        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = currBackBuffer;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        cmdList->ResourceBarrier(1, &barrier);
    }

    DX_TRY(cmdList->Close());

    ID3D12CommandList *commandLists[] = {
        cmdList
    };
    dxr->queue->ExecuteCommandLists(ARRAY_COUNT(commandLists), commandLists);

    b32 vsyncOn = true;
    u32 syncInterval = dxr->allowTearing && !vsyncOn ? 0 : 1;
    DX_TRY(dxr->swapChain->Present(syncInterval, 0));

    rendererSignal(dxr);
    dxr->currBackBufferIndex = dxr->swapChain->GetCurrentBackBufferIndex();
    rendererWaitForFenceValue(dxr);

    freeTempUploadBuffers(tempResources, numTempResources);

    for (u32 loadedTextureIndex = 0; loadedTextureIndex < numLoadedTextures; loadedTextureIndex++) {
        dxr_loaded_texture *loadedTexture = &loadedTextures[loadedTextureIndex];

        D3D12_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
        textureViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; 
        textureViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  
        textureViewDesc.Texture2D.MostDetailedMip = 0;
        textureViewDesc.Texture2D.MipLevels = -1;
        textureViewDesc.Texture2D.PlaneSlice = 0;
        textureViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        dxr->device->CreateShaderResourceView(loadedTexture->texture->resource, &textureViewDesc, loadedTexture->texture->descriptor);
    }
}

void shutdownRenderer (dx_renderer *dxr) {
    for (u32 textureIndex = 0; textureIndex < dxr->numTextures; textureIndex++) {
        dxr->textures[textureIndex].resource->Release();
    }

    dxr->perFrameDescriptorHeap->Release();
    dxr->permDescriptorHeap->Release();
    CloseHandle(dxr->fenceEvent);
    dxr->depthBuffer->Release();
    dxr->dsvHeap->Release();
    dxr->spriteBatchPipelineState->Release();
    dxr->spriteBatchRootSignature->Release();
    dxr->spriteIndexBuffer->Release();
    dxr->spriteVertexBuffer->Release();
    dxr->fence->Release();
    dxr->cmdList->Release();
    for (i32 i = numSwapChainFrames - 1; i >= 0; --i) {
        dxr->cmdAllocators[i]->Release();
    }
    for (i32 i = numSwapChainFrames - 1; i >= 0; --i) {
        dxr->backBuffers[i]->Release();
    }
    dxr->rtvDescriptorHeap->Release();
    dxr->swapChain->Release();
    dxr->queue->Release();
    dxr->device->Release();
}
