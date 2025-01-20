#ifndef WIN_DX_RENDERER_H
#define WIN_DX_RENDERER_H

#include <dxgi1_6.h>
#include <d3d12.h>

#define DX_TRY(func) \
{\
    HRESULT result = (func);\
    if (FAILED(result)) {\
        ASSERT(0);\
    }\
}

// shaders
#include "shaders/test_vertex.h"
#include "shaders/test_pixel.h"

static const u32 numSwapChainFrames = 2;

typedef struct vertex_pos_tex_coords {
    f32 pos[3];
    f32 texCoords[2];
} vertex_pos_tex_coords;

typedef struct vertex_sprite {
    f32 pos[3];
    f32 texCoords[2];
    f32 color[4];
    u8 textureID;
} vertex_sprite;

typedef struct screen_dims {
    f32 width;
    f32 height;
} screen_dims;

typedef struct dxr_texture {
    u32 id;
    ID3D12Resource *resource;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor;          
} dxr_texture;
#define MAX_NUM_DXR_TEXTURES 100

typedef struct dxr_loaded_texture {
    dxr_texture *texture;
} dxr_loaded_texture;

#define MAX_NUM_SPRITES 10000
#define UNUSED_TEXTURE_VAL ((u32)(-1))

typedef struct dx_renderer {
    ID3D12Device2 *device;

    ID3D12DescriptorHeap *rtvDescriptorHeap;
    u32 rtvDescriptorSize;

    ID3D12DescriptorHeap *permDescriptorHeap;
    u32 numPermHeapDescriptors;
    ID3D12DescriptorHeap *perFrameDescriptorHeap;
    u32 numPerFrameDescriptors;
    u32 cbvSRVUAVDescriptorSize;

    ID3D12DescriptorHeap *dsvHeap;
    ID3D12Resource *depthBuffer;

    IDXGISwapChain4 *swapChain;
    ID3D12Resource *backBuffers[numSwapChainFrames];
    u32 currBackBufferIndex;

    ID3D12CommandQueue *queue;
    ID3D12CommandAllocator *cmdAllocators[numSwapChainFrames];
    ID3D12GraphicsCommandList *cmdList;

    ID3D12Fence *fence;
    HANDLE fenceEvent;
    u64 fenceValue;

    b32 allowTearing;

    u32 windowWidth;
    u32 windowHeight;

    dxr_texture textures[MAX_NUM_DXR_TEXTURES];
    u32 numTextures;

    ID3D12PipelineState *spriteBatchPipelineState;
    ID3D12RootSignature *spriteBatchRootSignature;
    ID3D12Resource *spriteVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW spriteVBufferView;
    u8 *spriteVBufferData;
    ID3D12Resource *spriteIndexBuffer;
    D3D12_INDEX_BUFFER_VIEW spriteIBufferView;
    u16 numSpritesRenderedThisFrame;

    u32 spriteBatchSeenTextures[MAX_NUM_DXR_TEXTURES];
    u32 spriteBatchNumSeenTextures;
    u16 spriteBatchNumSpritesDrawn;
} dx_renderer;

#endif
