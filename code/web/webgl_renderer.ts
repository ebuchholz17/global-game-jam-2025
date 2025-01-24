/// <reference path="shaders\test_vertex.ts"/>

class ShaderProgram {
    public vertexShader: WebGLShader = null;
    public fragmentShader: WebGLShader = null;
    public program: WebGLProgram = null;
}

class WebGLRenderer {
    public canvas: HTMLCanvasElement = null;
    public glContext: WebGLRenderingContext = null;

    public memoryView: DataView = null;

    public spriteShader: ShaderProgram = null;
    public spriteVertexBuffer: WebGLBuffer = null;
    public spriteIndexBuffer: WebGLBuffer = null;

    public windowWidth: number = 0;
    public windowHeight: number = 0;

    public textures: WebGLTexture[] = [];
    public textureSizes: [number, number][] = [];

    public numSpritesDrawnThisFrame: number = 0;
    public spriteBatchNumSpritesDrawn: number = 0;
    public spriteBatchSeenTextures: {[key: number]: boolean} = {};
    public spriteBatchTextureList: number[] = []; // order that textures were drawn
}

let wglr: WebGLRenderer = new WebGLRenderer();
let MAX_NUM_SPRITES: number = 10000;

function compileAndLinkShader (gl: WebGLRenderingContext, 
                               vertexShaderSource: string, 
                               fragmentShaderSource: string): ShaderProgram 
{
    var result = new ShaderProgram();

    result.vertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(result.vertexShader, vertexShaderSource);
    gl.compileShader(result.vertexShader);
    if (!gl.getShaderParameter(result.vertexShader, gl.COMPILE_STATUS)) {
        console.log(gl.getShaderInfoLog(result.vertexShader));
        return;
    }

    result.fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(result.fragmentShader, fragmentShaderSource);
    gl.compileShader(result.fragmentShader);
    if (!gl.getShaderParameter(result.fragmentShader, gl.COMPILE_STATUS)) {
        console.log(gl.getShaderInfoLog(result.fragmentShader));
        return;
    }

    result.program = gl.createProgram();
    gl.attachShader(result.program, result.vertexShader);
    gl.attachShader(result.program, result.fragmentShader);
    gl.linkProgram(result.program);

    if (!gl.getProgramParameter(result.program, gl.LINK_STATUS)) {
        console.log(gl.getProgramInfoLog(result.program));
        return;
    }

    return result;
}

function createStaticBuffer (gl: WebGLRenderingContext, data: number[]): WebGLBuffer {
    let floatArray = new Float32Array(data);
    let result: WebGLBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, result);
    gl.bufferData(gl.ARRAY_BUFFER, floatArray, gl.STATIC_DRAW);
    return result;
}

function createIndexBuffer (gl: WebGLRenderingContext, data: number[]): WebGLBuffer {
    let uintArray = new Uint16Array(data);
    let result: WebGLBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, result);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, uintArray, gl.STATIC_DRAW);
    return result;
}

function initWebGL (canvas: HTMLCanvasElement, memory: Uint8Array): void {
    wglr.canvas = canvas;
    wglr.glContext = <WebGLRenderingContext>canvas.getContext("webgl", { alpha: false, antialias: false});

    wglr.memoryView = new DataView(memory.buffer);

    let gl: WebGLRenderingContext = wglr.glContext;
    //gl.getExtension("OES_element_index_uint");
    gl.getExtension("OES_standard_derivatives");
    wglr.spriteShader = compileAndLinkShader(gl, sprite_vertex_source, sprite_fragment_source);

    wglr.spriteVertexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, wglr.spriteVertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, 4 * MAX_NUM_SPRITES * 40, gl.STATIC_DRAW);

    let indices: number[] = [];
    for (let spriteIndex = 0; spriteIndex < MAX_NUM_SPRITES; spriteIndex++) {
        indices[spriteIndex * 6 + 0] = (spriteIndex * 4) + 0;
        indices[spriteIndex * 6 + 1] = (spriteIndex * 4) + 2;
        indices[spriteIndex * 6 + 2] = (spriteIndex * 4) + 1;
        indices[spriteIndex * 6 + 3] = (spriteIndex * 4) + 1;
        indices[spriteIndex * 6 + 4] = (spriteIndex * 4) + 2;
        indices[spriteIndex * 6 + 5] = (spriteIndex * 4) + 3;
    }
    wglr.spriteIndexBuffer = createIndexBuffer(gl, indices);
}

function resizeWebGL(windowWidth: number, windowHeight: number): void{
    wglr.windowWidth = windowWidth;
    wglr.windowHeight = windowHeight;
}

function webglOnRenderStart (): void {
    let gl: WebGLRenderingContext = wglr.glContext;
    gl.viewport(0, 0, wglr.windowWidth, wglr.windowHeight);
    //gl.enable(gl.DEPTH_TEST);
    //gl.enable(gl.CULL_FACE);
    gl.clearColor(0.05, 0.07, 0.16, 0.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
}

function webglLoadTexture (id: number, width: number, height: number, pixelDataPtr: number): void {
    let gl: WebGLRenderingContext = wglr.glContext;
    let texture: WebGLTexture = gl.createTexture();
    if (wglr.textures.length != id) { throw 1; }
    wglr.textures.push(texture);

    wglr.textureSizes.push([width, height]);

    let pixelView = new Uint8Array(wglr.memoryView.buffer, pixelDataPtr, width * height * 4);

    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, pixelView);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
}

function webglSpriteBatchStart (): void {
    let gl: WebGLRenderingContext = wglr.glContext;

    let program: WebGLProgram = wglr.spriteShader.program;
    gl.useProgram(program);

    let screenWidthLoc: WebGLUniformLocation = gl.getUniformLocation(program, "screenDims.width");
    gl.uniform1f(screenWidthLoc, wglr.windowWidth);
    let screenHeightLoc: WebGLUniformLocation = gl.getUniformLocation(program, "screenDims.height");
    gl.uniform1f(screenHeightLoc, wglr.windowHeight);

    gl.bindBuffer(gl.ARRAY_BUFFER, wglr.spriteVertexBuffer);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, wglr.spriteIndexBuffer);

    var positionLocation = gl.getAttribLocation(program, "pos");
    gl.enableVertexAttribArray(positionLocation);
    gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 40, 0);

    var texCoordLocation = gl.getAttribLocation(program, "texCoords");
    gl.enableVertexAttribArray(texCoordLocation);
    gl.vertexAttribPointer(texCoordLocation, 2, gl.FLOAT, false, 40, 12);

    var colorCoordLocation = gl.getAttribLocation(program, "color");
    gl.enableVertexAttribArray(colorCoordLocation);
    gl.vertexAttribPointer(colorCoordLocation, 4, gl.FLOAT, false, 40, 20);

    var texIndexLocation = gl.getAttribLocation(program, "textureID");
    gl.enableVertexAttribArray(texIndexLocation);
    gl.vertexAttribPointer(texIndexLocation, 1, gl.FLOAT, false, 40, 36);
}

function webglSpriteBatchFlush (spriteDataPtr: number, numSpritesBatched: number, vertexSize: number, 
                                numSpritesDrawn: number, textureIDs: number, numTextures: number): void 
{
    let gl: WebGLRenderingContext = wglr.glContext;
    let program: WebGLProgram = wglr.spriteShader.program;

    let dataSize = numSpritesBatched * (vertexSize/4) * 4;
    let bufferOffset = numSpritesDrawn * vertexSize * 4;

    let newSpriteDataView = new Float32Array(wglr.memoryView.buffer, spriteDataPtr, dataSize);
    gl.bufferSubData(gl.ARRAY_BUFFER, bufferOffset, newSpriteDataView);

    let textureIDArrayView = new Uint32Array(wglr.memoryView.buffer, textureIDs, numTextures);
    for (let i = 0; i < numTextures; i++) {
        let texture = wglr.textures[textureIDArrayView[i]];

        gl.activeTexture(gl.TEXTURE0 + i);
        gl.bindTexture(gl.TEXTURE_2D, texture);

        var textureLocation = gl.getUniformLocation(program, "texture" + i);
        gl.uniform1i(textureLocation, i);

        var textureSizeLocation = gl.getUniformLocation(program, "texture" + i + "Size");
        gl.uniform2fv(textureSizeLocation, wglr.textureSizes[textureIDArrayView[i]]);
    }
    gl.drawElements(gl.TRIANGLES, numSpritesBatched * 6, gl.UNSIGNED_SHORT, numSpritesDrawn * 6);
}

