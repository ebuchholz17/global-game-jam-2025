
let sprite_vertex_source = `
struct screen_dims {
    float width;
    float height;
};
uniform screen_dims screenDims;

attribute vec3 pos;
attribute vec2 texCoords;
attribute vec4 color;
attribute float textureID;

varying vec2 vTexCoords;
varying vec4 vColor;
varying float vTextureID;

void main() {
    gl_Position = vec4(pos.x * (1.0 / screenDims.width) * 2.0 - 1.0,
                    pos.y * (1.0 / screenDims.height) * 2.0 - 1.0,
                    -1.0, 1.0);
    gl_Position.y = -gl_Position.y;
    vTexCoords = texCoords;
    vColor = color;
    vTextureID = textureID;
}`;


let sprite_fragment_source = `
#extension GL_OES_standard_derivatives : enable

precision highp float;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform sampler2D texture6;
uniform sampler2D texture7;

uniform vec2 texture0Size;
uniform vec2 texture1Size;
uniform vec2 texture2Size;
uniform vec2 texture3Size;
uniform vec2 texture4Size;
uniform vec2 texture5Size;
uniform vec2 texture6Size;
uniform vec2 texture7Size;

varying vec2 vTexCoords;
varying vec4 vColor;
varying float vTextureID;

void main() {
    vec4 baseColor;

    vec2 texSize;
    vec2 uv = vTexCoords;

    if (vTextureID < 0.5) {
        texSize = texture0Size;
    }
    else if (vTextureID >= 0.5 && vTextureID < 1.5) {
        texSize = texture1Size;
    }
    else if (vTextureID >= 1.5 && vTextureID < 2.5) {
        texSize = texture2Size;
    }
    else if (vTextureID >= 2.5 && vTextureID < 3.5) {
        texSize = texture3Size;
    }
    else if (vTextureID >= 3.5 && vTextureID < 4.5) {
        texSize = texture4Size;
    }
    else if (vTextureID >= 4.5 && vTextureID < 5.5) {
        texSize = texture5Size;
    }
    else if (vTextureID >= 5.5 && vTextureID < 6.5) {
        texSize = texture6Size;
    }
    else {
        texSize = texture7Size;
    }

    uv *= texSize;
    vec2 seam = floor(uv + 0.5);
    vec2 dudv = fwidth(uv);
    uv = seam + clamp((uv - seam) / dudv, -0.5, 0.5);
    uv /= texSize;

    if (vTextureID < 0.5) {
        baseColor = texture2D(texture0, uv);
    }
    else if (vTextureID >= 0.5 && vTextureID < 1.5) {
        baseColor = texture2D(texture1, uv);
    }
    else if (vTextureID >= 1.5 && vTextureID < 2.5) {
        baseColor = texture2D(texture2, uv);
    }
    else if (vTextureID >= 2.5 && vTextureID < 3.5) {
        baseColor = texture2D(texture3, uv);
    }
    else if (vTextureID >= 3.5 && vTextureID < 4.5) {
        baseColor = texture2D(texture4, uv);
    }
    else if (vTextureID >= 4.5 && vTextureID < 5.5) {
        baseColor = texture2D(texture5, uv);
    }
    else if (vTextureID >= 5.5 && vTextureID < 6.5) {
        baseColor = texture2D(texture6, uv);
    }
    else {
        baseColor = texture2D(texture7, uv);
    }

    gl_FragColor = baseColor * vColor;
}`;

