#ifndef WEBGL_RENDERER_H
#define WEBGL_RENDERER_H

typedef struct wglr_texture {
    u32 id;
    //ID3D12Resource *resource;
    //D3D12_CPU_DESCRIPTOR_HANDLE descriptor;          
} wglr_texture;
#define MAX_NUM_WGLR_TEXTURES 100

typedef struct wglr_loaded_texture {
    wglr_texture *texture;
} wglr_loaded_texture;

#define UNUSED_TEXTURE_VAL ((u32)(-1))

// webgl format vertex data
#pragma pack(push, 1)
typedef struct wglr_vertex {
    f32 pos[3];
    f32 texCoords[2];
    f32 color[4];
    float textureID; // NOTE: webgl only uses floats
} wglr_vertex;
#pragma pack(pop)

#define MAX_NUM_SPRITES 10000

typedef struct webgl_renderer {
    wglr_texture textures[MAX_NUM_WGLR_TEXTURES];
    u32 numTextures;

    u16 numSpritesDrawnThisFrame;

    u32 spriteBatchSeenTextures[MAX_NUM_WGLR_TEXTURES];
    u32 spriteBatchNumSeenTextures;
    u16 spriteBatchNumSpritesDrawn;
    wglr_vertex *batchStart;
    wglr_vertex *currentVertex;
} webgl_renderer;


void webglOnRenderStart (void);
void webglLoadTexture (u32 id, u32 width, u32 height, u8 *pixels);
void webglSpriteBatchStart (void);
void webglSpriteBatchFlush (u8 *spriteDataPtr, u32 numSprites, u32 vertexSize, u32 totalNumSpritesDrawn, u32 *textureIDs, u32 numTextures);

#endif
