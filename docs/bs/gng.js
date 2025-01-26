"use strict";
class WebAudio {
    constructor() {
        this.samplesPerSecond = 0;
        this.audioCtx = null;
        this.audioBufferSize = 512;
        this.started = false;
        this.playTime = 0.0;
        this.nextAudioBufferPlayTime = 0.0;
    }
    init() {
        this.audioCtx = new AudioContext();
        this.samplesPerSecond = this.audioCtx.sampleRate;
        this.playTime = this.audioCtx.currentTime;
        this.started = true;
    }
    getSampleRate() {
        let dummyAudioCtx = new AudioContext();
        return dummyAudioCtx.sampleRate;
    }
    updateAudio(gameAPI, memory) {
        let currentAudioTime = this.audioCtx.currentTime;
        let bufferingDelay = 50.0 / 1000.0;
        let bufferTime = this.audioBufferSize / this.samplesPerSecond;
        let numSamples = this.audioBufferSize;
        let maxNumQueuedBuffers = 5;
        for (let i = 0; i < maxNumQueuedBuffers; ++i) {
            let secsLeftTilNextAudio = this.nextAudioBufferPlayTime - currentAudioTime;
            if (secsLeftTilNextAudio < bufferingDelay + bufferTime * maxNumQueuedBuffers) {
                let newSamples = gameAPI.getGameSoundSamples(numSamples);
                if (newSamples == 0) {
                    return;
                }
                let floatBuffer = new Float32Array(memory.buffer, newSamples, numSamples);
                let buffer = this.audioCtx.createBuffer(1, numSamples, this.samplesPerSecond);
                let output = buffer.getChannelData(0);
                for (let sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
                    output[sampleIndex] = floatBuffer[sampleIndex];
                }
                let bufferSourceNode = this.audioCtx.createBufferSource();
                bufferSourceNode.connect(this.audioCtx.destination);
                bufferSourceNode.buffer = buffer;
                let playTime = Math.max(currentAudioTime + bufferingDelay, this.nextAudioBufferPlayTime);
                bufferSourceNode.start(playTime);
                this.nextAudioBufferPlayTime = playTime + bufferTime;
            }
            else {
                break;
            }
        }
    }
    tryUnlockAudioContext() {
        if (this.audioCtx.state == "suspended") {
            this.audioCtx.resume();
        }
    }
}
let test_vertex_source = `
struct tri_transform {
    vec2 pos;
    float rotation;
};

uniform tri_transform triTransform;

attribute vec3 position;
attribute vec3 color;

varying vec3 vColor;

void main() {
    mat3 transform = mat3(cos(triTransform.rotation), sin(triTransform.rotation), 0.0, 
                          -sin(triTransform.rotation), cos(triTransform.rotation), 0.0, 
                          triTransform.pos.x, triTransform.pos.y, 1.0);
    vec3 pos = vec3(position.x, position.y, 1.0);
    pos = vec3(transform * pos);
    gl_Position = vec4(pos.x, pos.y, -2.0 * (position.z - 0.5), 1.0); // use 0-1 for z like dx
    gl_Position.y = -gl_Position.y;
    vColor = color;
}`;
let test_fragment_source = `
precision highp float;

varying vec3 vColor;

void main() {
    gl_FragColor = vec4(vColor, 1.0);
}`;
/// <reference path="shaders\test_vertex.ts"/>
class ShaderProgram {
    constructor() {
        this.vertexShader = null;
        this.fragmentShader = null;
        this.program = null;
    }
}
class WebGLRenderer {
    constructor() {
        this.canvas = null;
        this.glContext = null;
        this.memoryView = null;
        this.spriteShader = null;
        this.spriteVertexBuffer = null;
        this.spriteIndexBuffer = null;
        this.windowWidth = 0;
        this.windowHeight = 0;
        this.textures = [];
        this.textureSizes = [];
        this.numSpritesDrawnThisFrame = 0;
        this.spriteBatchNumSpritesDrawn = 0;
        this.spriteBatchSeenTextures = {};
        this.spriteBatchTextureList = []; // order that textures were drawn
    }
}
let wglr = new WebGLRenderer();
let MAX_NUM_SPRITES = 10000;
function compileAndLinkShader(gl, vertexShaderSource, fragmentShaderSource) {
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
function createStaticBuffer(gl, data) {
    let floatArray = new Float32Array(data);
    let result = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, result);
    gl.bufferData(gl.ARRAY_BUFFER, floatArray, gl.STATIC_DRAW);
    return result;
}
function createIndexBuffer(gl, data) {
    let uintArray = new Uint16Array(data);
    let result = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, result);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, uintArray, gl.STATIC_DRAW);
    return result;
}
function initWebGL(canvas, memory) {
    wglr.canvas = canvas;
    wglr.glContext = canvas.getContext("webgl", { alpha: false, antialias: false });
    wglr.memoryView = new DataView(memory.buffer);
    let gl = wglr.glContext;
    //gl.getExtension("OES_element_index_uint");
    gl.getExtension("OES_standard_derivatives");
    wglr.spriteShader = compileAndLinkShader(gl, sprite_vertex_source, sprite_fragment_source);
    wglr.spriteVertexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, wglr.spriteVertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, 4 * MAX_NUM_SPRITES * 40, gl.STATIC_DRAW);
    let indices = [];
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
function resizeWebGL(windowWidth, windowHeight) {
    wglr.windowWidth = windowWidth;
    wglr.windowHeight = windowHeight;
}
function webglOnRenderStart() {
    let gl = wglr.glContext;
    gl.viewport(0, 0, wglr.windowWidth, wglr.windowHeight);
    //gl.enable(gl.DEPTH_TEST);
    //gl.enable(gl.CULL_FACE);
    gl.clearColor(0.05, 0.07, 0.16, 0.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
}
function webglLoadTexture(id, width, height, pixelDataPtr) {
    let gl = wglr.glContext;
    let texture = gl.createTexture();
    if (wglr.textures.length != id) {
        throw 1;
    }
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
function webglSpriteBatchStart() {
    let gl = wglr.glContext;
    let program = wglr.spriteShader.program;
    gl.useProgram(program);
    let screenWidthLoc = gl.getUniformLocation(program, "screenDims.width");
    gl.uniform1f(screenWidthLoc, wglr.windowWidth);
    let screenHeightLoc = gl.getUniformLocation(program, "screenDims.height");
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
function webglSpriteBatchFlush(spriteDataPtr, numSpritesBatched, vertexSize, numSpritesDrawn, textureIDs, numTextures) {
    let gl = wglr.glContext;
    let program = wglr.spriteShader.program;
    let dataSize = numSpritesBatched * (vertexSize / 4) * 4;
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
/// <reference path="webgl_renderer.ts"/>
let textDecoder = new TextDecoder("utf-8");
let textEncoder = new TextEncoder();
class GameAPI {
    constructor() {
        this.initGame = null;
        this.initAudio = null;
        this.gameSetControllerConnected = null;
        this.gameSetControllerIndexDPad = null;
        this.gameSetControllerIndexStartBack = null;
        this.gameSetControllerIndexStickButtons = null;
        this.gameSetControllerIndexBumpers = null;
        this.gameSetControllerIndexFaceButtons = null;
        this.gameSetControllerIndexTriggers = null;
        this.gameSetControllerIndexSticks = null;
        this.gameSetControllerIndexStickDirections = null;
        this.setTouchInput = null;
        this.setVirtualInputEnabled = null;
        this.onFrameStart = null;
        this.updateGame = null;
        this.renderGame = null;
        this.getGameSoundSamples = null;
        this.onFrameEnd = null;
        this.onResize = null;
        this.onMouseMove = null;
        this.onMouseUp = null;
        this.onMouseDown = null;
        this.onLetterKeyDown = null;
        this.onLetterKeyUp = null;
        this.onArrowKeyDown = null;
        this.onArrowKeyUp = null;
        this.onFileFetched = null;
        this.webAllocMemory = null;
        this.webAllocTempMemory = null;
    }
}
var GameInputEventType;
(function (GameInputEventType) {
    GameInputEventType[GameInputEventType["MOUSE_DOWN"] = 0] = "MOUSE_DOWN";
    GameInputEventType[GameInputEventType["MOUSE_UP"] = 1] = "MOUSE_UP";
    GameInputEventType[GameInputEventType["MOUSE_MOVE"] = 2] = "MOUSE_MOVE";
    GameInputEventType[GameInputEventType["KEY_DOWN"] = 3] = "KEY_DOWN";
    GameInputEventType[GameInputEventType["KEY_UP"] = 4] = "KEY_UP";
})(GameInputEventType || (GameInputEventType = {}));
var GameInputArrowKeyDir;
(function (GameInputArrowKeyDir) {
    GameInputArrowKeyDir[GameInputArrowKeyDir["UP"] = 0] = "UP";
    GameInputArrowKeyDir[GameInputArrowKeyDir["DOWN"] = 1] = "DOWN";
    GameInputArrowKeyDir[GameInputArrowKeyDir["LEFT"] = 2] = "LEFT";
    GameInputArrowKeyDir[GameInputArrowKeyDir["RIGHT"] = 3] = "RIGHT";
})(GameInputArrowKeyDir || (GameInputArrowKeyDir = {}));
let gameInputKeyboardLetters = "abcdefghijklmnopqrstuvwxyz";
class GameInputEvent {
    constructor() {
        this.x = 0;
        this.y = 0;
        this.key = null;
    }
}
class WebInputKey {
    constructor() {
        this.down = false;
        this.justPressed = false;
    }
}
class WebTouch {
    constructor() {
        this.active = false;
        this.identifier = -1;
        this.touchState = new WebInputKey();
        this.x = -1;
        this.y = -1;
        this.radiusX = -1;
        this.radiusY = -1;
    }
}
class WebGamepad {
    constructor() {
        this.connected = false;
        this.gamePadRef = null;
        this.dPadUp = new WebInputKey();
        this.dPadDown = new WebInputKey();
        this.dPadLeft = new WebInputKey();
        this.dPadRight = new WebInputKey();
        this.start = new WebInputKey();
        this.back = new WebInputKey();
        this.leftStick = new WebInputKey();
        this.rightStick = new WebInputKey();
        this.leftBumper = new WebInputKey();
        this.rightBumper = new WebInputKey();
        this.aButton = new WebInputKey();
        this.bButton = new WebInputKey();
        this.xButton = new WebInputKey();
        this.yButton = new WebInputKey();
        this.leftTrigger = 0.0;
        this.leftTriggerButton = new WebInputKey();
        this.rightTrigger = 0.0;
        this.rightTriggerButton = new WebInputKey();
        this.leftStickX = 0.0;
        this.leftStickY = 0.0;
        this.rightStickX = 0.0;
        this.rightStickY = 0.0;
        this.leftStickUp = new WebInputKey();
        this.leftStickDown = new WebInputKey();
        this.leftStickLeft = new WebInputKey();
        this.leftStickRight = new WebInputKey();
        this.rightStickUp = new WebInputKey();
        this.rightStickDown = new WebInputKey();
        this.rightStickLeft = new WebInputKey();
        this.rightStickRight = new WebInputKey();
    }
}
function bytesToString(memory, msgPointer) {
    let msgStart = msgPointer;
    let msgEnd = msgPointer;
    while (memory[msgEnd] != 0) {
        ++msgEnd;
    }
    let msgBuffer = memory.subarray(msgStart, msgEnd);
    let text = textDecoder.decode(msgBuffer);
    return text;
}
function processInputEvents(gameAPI, events, memory) {
    for (let eventIndex = 0; eventIndex < events.length; ++eventIndex) {
        let event = events[eventIndex];
        let letter = "";
        let arrowDir = -1;
        if (event.type == GameInputEventType.KEY_UP || event.type == GameInputEventType.KEY_DOWN) {
            if (event.key.length == 1) {
                letter = event.key.toLowerCase();
            }
            else if (event.key == "ArrowUp") {
                arrowDir = GameInputArrowKeyDir.UP;
            }
            else if (event.key == "ArrowDown") {
                arrowDir = GameInputArrowKeyDir.DOWN;
            }
            else if (event.key == "ArrowLeft") {
                arrowDir = GameInputArrowKeyDir.LEFT;
            }
            else if (event.key == "ArrowRight") {
                arrowDir = GameInputArrowKeyDir.RIGHT;
            }
        }
        switch (event.type) {
            case GameInputEventType.MOUSE_MOVE:
                {
                    gameAPI.onMouseMove(event.x, event.y);
                }
                break;
            case GameInputEventType.MOUSE_DOWN:
                {
                    gameAPI.onMouseDown();
                }
                break;
            case GameInputEventType.MOUSE_UP:
                {
                    gameAPI.onMouseUp();
                }
                break;
            case GameInputEventType.KEY_DOWN:
                {
                    if (letter != "") {
                        gameAPI.onLetterKeyDown(letter.charCodeAt(0));
                    }
                    else if (arrowDir != -1) {
                        gameAPI.onArrowKeyDown(arrowDir);
                    }
                }
                break;
            case GameInputEventType.KEY_UP:
                {
                    if (letter != "") {
                        gameAPI.onLetterKeyUp(letter.charCodeAt(0));
                    }
                    else if (arrowDir != -1) {
                        gameAPI.onArrowKeyUp(arrowDir);
                    }
                }
                break;
        }
    }
}
function encodeTempString(str, memory, gameAPI) {
    let strBytes = textEncoder.encode(str);
    let tempGameSideString = gameAPI.webAllocTempMemory(strBytes.length);
    let gameSideStringView = new Uint8Array(memory.buffer, tempGameSideString, strBytes.length);
    return gameSideStringView.byteOffset;
}
function updateControllerButton(gameController, buttonProp, gamepad, buttonID) {
    let inputKey = (gameController[buttonProp]);
    inputKey.justPressed = false;
    if (gamepad.buttons[buttonID].pressed) {
        if (!inputKey.down) {
            inputKey.justPressed = true;
        }
        inputKey.down = true;
    }
    else {
        inputKey.down = false;
    }
}
function updateControllerTrigger(gameController, triggerProp, triggerButtonProp, value) {
    let inputKey = (gameController[triggerButtonProp]);
    inputKey.justPressed = false;
    let deadZone = 0.11764705882352941176470588235294; // attempt to match xinput default deadzone
    if (value >= deadZone) {
        value -= deadZone;
        //@ts-ignore
        gameController[triggerProp] = value / (1.0 - deadZone);
        if (!inputKey.down) {
            inputKey.justPressed = true;
        }
        inputKey.down = true;
    }
    else {
        //@ts-ignore
        gameController[triggerProp] = 0.0;
        inputKey.down = false;
    }
}
function updateControllerStick(gameController, xProperty, xValue, yProperty, yValue, deadZone) {
    let magnitude = Math.sqrt(xValue * xValue + yValue * yValue);
    let normalizedX = xValue / magnitude;
    let normalizedY = yValue / magnitude;
    if (magnitude >= deadZone) {
        if (magnitude > 1.0) {
            magnitude = 1.0;
        }
        magnitude -= deadZone;
        let normalizedMagnitude = magnitude / (1.0 - deadZone);
        //@ts-ignore
        gameController[xProperty] = normalizedX * normalizedMagnitude;
        //@ts-ignore
        gameController[yProperty] = normalizedY * normalizedMagnitude;
    }
    else {
        //@ts-ignore
        gameController[xProperty] = 0.0;
        //@ts-ignore
        gameController[yProperty] = 0.0;
    }
}
function updateControllerStickDirection(gameController, dir0Prop, dir1Prop, value, deadZone) {
    //@ts-ignore
    gameController[dir0Prop].justPressed = false;
    if (value <= -deadZone) {
        //@ts-ignore
        if (!gameController[dir0Prop].down) {
            //@ts-ignore
            gameController[dir0Prop].justPressed = true;
        }
        //@ts-ignore
        gameController[dir0Prop].down = true;
    }
    else {
        //@ts-ignore
        gameController[dir0Prop].down = false;
    }
    //@ts-ignore
    gameController[dir1Prop].justPressed = false;
    if (value >= deadZone) {
        //@ts-ignore
        if (!gameController[dir1Prop].down) {
            //@ts-ignore
            gameController[dir1Prop].justPressed = true;
        }
        //@ts-ignore
        gameController[dir1Prop].down = true;
    }
    else {
        //@ts-ignore
        gameController[dir1Prop].down = false;
    }
}
function updateController(gameController, gamepad) {
    updateControllerButton(gameController, "dPadUp", gamepad, 12);
    updateControllerButton(gameController, "dPadDown", gamepad, 13);
    updateControllerButton(gameController, "dPadLeft", gamepad, 14);
    updateControllerButton(gameController, "dPadRight", gamepad, 15);
    updateControllerButton(gameController, "start", gamepad, 9);
    updateControllerButton(gameController, "back", gamepad, 8);
    updateControllerButton(gameController, "leftStick", gamepad, 10);
    updateControllerButton(gameController, "rightStick", gamepad, 11);
    updateControllerButton(gameController, "leftBumper", gamepad, 4);
    updateControllerButton(gameController, "rightBumper", gamepad, 5);
    updateControllerButton(gameController, "aButton", gamepad, 0);
    updateControllerButton(gameController, "bButton", gamepad, 1);
    updateControllerButton(gameController, "xButton", gamepad, 2);
    updateControllerButton(gameController, "yButton", gamepad, 3);
    updateControllerTrigger(gameController, "leftTrigger", "leftTriggerButton", gamepad.buttons[6].value);
    updateControllerTrigger(gameController, "rightTrigger", "rightTriggerButton", gamepad.buttons[7].value);
    updateControllerStick(gameController, "leftStickX", gamepad.axes[0], "leftStickY", gamepad.axes[1], 0.23953978087710196234015930661946);
    updateControllerStick(gameController, "rightStickX", gamepad.axes[2], "rightStickY", gamepad.axes[3], 0.23953978087710196234015930661946);
    updateControllerStickDirection(gameController, "leftStickUp", "leftStickDown", gamepad.axes[1], 0.61037018951994384594256416516617);
    updateControllerStickDirection(gameController, "leftStickLeft", "leftStickRight", gamepad.axes[0], 0.61037018951994384594256416516617);
    updateControllerStickDirection(gameController, "rightStickUp", "rightStickDown", gamepad.axes[3], 0.26517532883693960386974700155644);
    updateControllerStickDirection(gameController, "rightStickLeft", "rightStickRight", gamepad.axes[2], 0.26517532883693960386974700155644);
}
function clientPosToScreenPos(clientPos, canvasDim, clientDim, clientStartPos) {
    let scale = canvasDim / clientDim;
    return (clientPos - clientStartPos) * scale;
}
async function main() {
    let pageSize = 64 * 1024;
    let numPages = 10000;
    let wasmMemory = new WebAssembly.Memory({ initial: numPages });
    let memory = new Uint8Array(wasmMemory.buffer);
    let wglr;
    // TODO(ebuchholz): try instantiateStreaming again, with fixed mime-type
    let wasmResponse = await fetch("./gng.wasm");
    let wasmBytes = await wasmResponse.arrayBuffer();
    let wasmObj = {
        env: {
            memory: wasmMemory,
            getMemCapacity: () => {
                return pageSize * numPages;
            },
            consoleLog: (msgPointer) => {
                console.log(bytesToString(memory, msgPointer));
            },
            rngSeedFromTime: () => {
                return BigInt(Date.now());
            },
            readFile: (assetNamePtr, assetPathPtr) => {
                let assetName = bytesToString(memory, assetNamePtr);
                let assetPath = bytesToString(memory, assetPathPtr);
                fetch(assetPath).then((response) => {
                    response.arrayBuffer().then((data) => {
                        // Copy file data into wasm memory
                        let dataView = new Uint8Array(data);
                        let numBytes = data.byteLength;
                        // TODO: better management of temporary loaded file data
                        let gameSideData = gameAPI.webAllocTempMemory(numBytes);
                        let gameSideDataView = new Uint8Array(memory.buffer, gameSideData, numBytes);
                        gameSideDataView.set(dataView, 0);
                        gameAPI.onFileFetched(assetNamePtr, gameSideDataView.byteOffset, gameSideDataView.length);
                    });
                });
            },
            onError: (msgPointer) => {
                console.error(bytesToString(memory, msgPointer));
            },
            rendererResize: (windowWidth, windowHeight) => {
                resizeWebGL(windowWidth, windowHeight);
            },
            webglOnRenderStart: () => { webglOnRenderStart(); },
            webglLoadTexture: (id, width, height, pixelDataPtr) => {
                webglLoadTexture(id, width, height, pixelDataPtr);
            },
            webglSpriteBatchStart: () => { webglSpriteBatchStart(); },
            webglSpriteBatchFlush: (spriteDataPtr, numSprites, vertexSize, totalNumSpritesDrawn, textureIDs, numTextures) => {
                webglSpriteBatchFlush(spriteDataPtr, numSprites, vertexSize, totalNumSpritesDrawn, textureIDs, numTextures);
            }
        }
    };
    let result = await WebAssembly.instantiate(wasmBytes, wasmObj);
    let wasmInstance = result.instance;
    let gameAPI = {
        initGame: wasmInstance.exports.initGame,
        initAudio: wasmInstance.exports.initAudio,
        onFrameStart: wasmInstance.exports.onFrameStart,
        updateGame: wasmInstance.exports.updateGame,
        renderGame: wasmInstance.exports.renderGame,
        getGameSoundSamples: wasmInstance.exports.getGameSoundSamples,
        onFrameEnd: wasmInstance.exports.onFrameEnd,
        gameSetControllerConnected: wasmInstance.exports.gameSetControllerConnected,
        gameSetControllerIndexDPad: wasmInstance.exports.gameSetControllerIndexDPad,
        gameSetControllerIndexStartBack: wasmInstance.exports.gameSetControllerIndexStartBack,
        gameSetControllerIndexStickButtons: wasmInstance.exports.gameSetControllerIndexStickButtons,
        gameSetControllerIndexBumpers: wasmInstance.exports.gameSetControllerIndexBumpers,
        gameSetControllerIndexFaceButtons: wasmInstance.exports.gameSetControllerIndexFaceButtons,
        gameSetControllerIndexTriggers: wasmInstance.exports.gameSetControllerIndexTriggers,
        gameSetControllerIndexSticks: wasmInstance.exports.gameSetControllerIndexSticks,
        gameSetControllerIndexStickDirections: wasmInstance.exports.gameSetControllerIndexStickDirections,
        setTouchInput: wasmInstance.exports.setTouchInput,
        setVirtualInputEnabled: wasmInstance.exports.setVirtualInputEnabled,
        onFileFetched: wasmInstance.exports.onFileFetched,
        webAllocMemory: wasmInstance.exports.webAllocMemory,
        webAllocTempMemory: wasmInstance.exports.webAllocTempMemory,
        onResize: wasmInstance.exports.onResize,
        onMouseMove: wasmInstance.exports.onMouseMove,
        onMouseDown: wasmInstance.exports.onMouseDown,
        onMouseUp: wasmInstance.exports.onMouseUp,
        onLetterKeyDown: wasmInstance.exports.onLetterKeyDown,
        onLetterKeyUp: wasmInstance.exports.onLetterKeyUp,
        onArrowKeyDown: wasmInstance.exports.onArrowKeyDown,
        onArrowKeyUp: wasmInstance.exports.onArrowKeyUp
    };
    let justResized = false;
    let resize = () => {
        justResized = true;
        canvas.width = mainDiv.clientWidth;
        canvas.height = mainDiv.clientHeight;
        canvas.style.width = mainDiv.clientWidth + "px";
        canvas.style.height = mainDiv.clientHeight + "px";
        gameAPI.onResize(canvas.width, canvas.height);
    };
    let mainDiv = document.getElementById("gng");
    //@ts-ignore
    mainDiv.style["touch-action"] = "none";
    mainDiv.style["overflow"] = "hidden";
    //@ts-ignore
    mainDiv.style["touch-action"] = "none";
    //@ts-ignore
    mainDiv.style["user-select"] = "none";
    //@ts-ignore
    mainDiv.style["-moz-user-select"] = "none";
    //@ts-ignore
    mainDiv.style["-ms-user-select"] = "none";
    //@ts-ignore
    mainDiv.style["-khtml-user-select"] = "none";
    //@ts-ignore
    mainDiv.style["-webkit-user-select"] = "none";
    //@ts-ignore
    mainDiv.style["-webkit-touch-callout"] = "none";
    mainDiv.draggable = false;
    let canvas = document.createElement("canvas");
    //@ts-ignore
    canvas.style["touch-action"] = "none";
    //@ts-ignore
    canvas.style["user-select"] = "none";
    //@ts-ignore
    canvas.style["-moz-user-select"] = "none";
    //@ts-ignore
    canvas.style["-ms-user-select"] = "none";
    //@ts-ignore
    canvas.style["-khtml-user-select"] = "none";
    //@ts-ignore
    canvas.style["-webkit-user-select"] = "none";
    //@ts-ignore
    canvas.style["-webkit-touch-callout"] = "none";
    canvas.style["outline"] = "none";
    canvas.style["overflow"] = "hidden";
    canvas.draggable = false;
    mainDiv.insertAdjacentElement("afterbegin", canvas);
    let webAudio = new WebAudio();
    let inputEvents = [];
    canvas.addEventListener("mousedown", (e) => {
        let inputEvent = {
            type: GameInputEventType.MOUSE_DOWN
        };
        inputEvents.push(inputEvent);
        if (!webAudio.started) {
            webAudio.init();
        }
        // for ios/chrome/etc, whatever browers need touch input to start audio
        webAudio.tryUnlockAudioContext();
    });
    canvas.addEventListener("mouseup", (e) => {
        let inputEvent = {
            type: GameInputEventType.MOUSE_UP
        };
        inputEvents.push(inputEvent);
    });
    canvas.addEventListener("mousemove", (e) => {
        let clientX = e.clientX;
        let clientY = e.clientY;
        let scaleX = canvas.width / canvas.clientWidth;
        let scaleY = canvas.height / canvas.clientHeight;
        let inputEvent = {
            type: GameInputEventType.MOUSE_MOVE,
            x: (clientX - canvas.clientLeft) * scaleX,
            y: (clientY - canvas.clientTop) * scaleY
        };
        inputEvents.push(inputEvent);
    });
    document.addEventListener("keydown", (e) => {
        inputEvents.push({
            type: GameInputEventType.KEY_DOWN,
            key: e.key
        });
    });
    document.addEventListener("keyup", (e) => {
        inputEvents.push({
            type: GameInputEventType.KEY_UP,
            key: e.key
        });
    });
    let touches = [];
    const MAX_NUM_TOUCHES = 4;
    for (let i = 0; i < MAX_NUM_TOUCHES; i++) {
        touches[i] = new WebTouch();
    }
    canvas.addEventListener("touchstart", (e) => {
        let changedTouches = e.changedTouches;
        for (let changedTouchIndex = 0; changedTouchIndex < changedTouches.length; changedTouchIndex++) {
            let touch = changedTouches.item(changedTouchIndex);
            for (let touchIndex = 0; touchIndex < MAX_NUM_TOUCHES; touchIndex++) {
                let webTouch = touches[touchIndex];
                if (!webTouch.active) {
                    webTouch.active = true;
                    webTouch.identifier = touch.identifier;
                    webTouch.touchState.justPressed = true;
                    webTouch.touchState.down = true;
                    webTouch.x = clientPosToScreenPos(touch.clientX, canvas.width, canvas.clientWidth, canvas.clientLeft);
                    webTouch.y = clientPosToScreenPos(touch.clientY, canvas.height, canvas.clientHeight, canvas.clientTop);
                    webTouch.radiusX = touch.radiusX;
                    webTouch.radiusY = touch.radiusY;
                    break;
                }
            }
        }
        if (!webAudio.started) {
            webAudio.init();
        }
        // for ios/chrome/etc, whatever browers need touch input to start audio
        webAudio.tryUnlockAudioContext();
        e.preventDefault();
    });
    canvas.addEventListener("touchmove", (e) => {
        let changedTouches = e.changedTouches;
        for (let changedTouchIndex = 0; changedTouchIndex < changedTouches.length; changedTouchIndex++) {
            let touch = changedTouches.item(changedTouchIndex);
            for (let touchIndex = 0; touchIndex < MAX_NUM_TOUCHES; touchIndex++) {
                let webTouch = touches[touchIndex];
                if (webTouch.active && webTouch.identifier == touch.identifier) {
                    webTouch.x = clientPosToScreenPos(touch.clientX, canvas.width, canvas.clientWidth, canvas.clientLeft);
                    webTouch.y = clientPosToScreenPos(touch.clientY, canvas.height, canvas.clientHeight, canvas.clientTop);
                    webTouch.radiusX = touch.radiusX;
                    webTouch.radiusY = touch.radiusY;
                    break;
                }
            }
        }
    });
    canvas.addEventListener("touchend", (e) => {
        let changedTouches = e.changedTouches;
        for (let changedTouchIndex = 0; changedTouchIndex < changedTouches.length; changedTouchIndex++) {
            let touch = changedTouches.item(changedTouchIndex);
            for (let touchIndex = 0; touchIndex < MAX_NUM_TOUCHES; touchIndex++) {
                let webTouch = touches[touchIndex];
                if (webTouch.active && webTouch.identifier == touch.identifier) {
                    webTouch.active = false;
                    webTouch.touchState.down = false;
                    break;
                }
            }
        }
        // for ios/chrome/etc, whatever browers need touch input to start audio
        webAudio.tryUnlockAudioContext();
    });
    canvas.addEventListener("touchcancel", (e) => {
        let changedTouches = e.changedTouches;
        for (let changedTouchIndex = 0; changedTouchIndex < changedTouches.length; changedTouchIndex++) {
            let touch = changedTouches.item(changedTouchIndex);
            for (let touchIndex = 0; touchIndex < MAX_NUM_TOUCHES; touchIndex++) {
                let webTouch = touches[touchIndex];
                if (webTouch.active && webTouch.identifier == touch.identifier) {
                    webTouch.active = false;
                    webTouch.touchState.down = false;
                    break;
                }
            }
        }
    });
    let controllers = [];
    const MAX_NUM_CONTROLLERS = 4;
    for (let i = 0; i < MAX_NUM_CONTROLLERS; i++) {
        controllers[i] = new WebGamepad();
    }
    window.addEventListener("gamepadconnected", (e) => {
        controllers[e.gamepad.index].connected = true;
        controllers[e.gamepad.index].gamePadRef = e.gamepad;
    });
    window.addEventListener("gamepaddisconnected", (e) => {
        controllers[e.gamepad.index].connected = false;
        controllers[e.gamepad.index].gamePadRef = null;
    });
    window.addEventListener("resize", resize);
    initWebGL(canvas, memory);
    gameAPI.initGame();
    gameAPI.initAudio(webAudio.getSampleRate());
    if (/Android|webOS|iPhone|iPad|iPod|BlackBerry|BB|PlayBook|IEMobile|Windows Phone|Kindle|Silk|Opera Mini/i.test(navigator.userAgent)) {
        gameAPI.setVirtualInputEnabled(true);
    }
    let lastTime = new Date().getTime() / 1000;
    let update = () => {
        let currentTime = new Date().getTime() / 1000;
        let dt = currentTime - lastTime;
        lastTime = currentTime;
        processInputEvents(gameAPI, inputEvents, memory);
        inputEvents.length = 0;
        let gamepads = navigator.getGamepads();
        for (let i = 0; i < gamepads.length && i < MAX_NUM_CONTROLLERS; i++) {
            if (gamepads[i] == null) {
                controllers[i].connected = false;
                controllers[i].gamePadRef = null;
            }
            else {
                updateController(controllers[i], gamepads[i]);
                gameAPI.gameSetControllerConnected(i, controllers[i].connected);
                gameAPI.gameSetControllerIndexDPad(i, controllers[i].dPadUp.down, controllers[i].dPadUp.justPressed, controllers[i].dPadDown.down, controllers[i].dPadDown.justPressed, controllers[i].dPadLeft.down, controllers[i].dPadLeft.justPressed, controllers[i].dPadRight.down, controllers[i].dPadRight.justPressed);
                gameAPI.gameSetControllerIndexStartBack(i, controllers[i].start.down, controllers[i].start.justPressed, controllers[i].back.down, controllers[i].back.justPressed);
                gameAPI.gameSetControllerIndexStickButtons(i, controllers[i].leftStick.down, controllers[i].leftStick.justPressed, controllers[i].rightStick.down, controllers[i].rightStick.justPressed);
                gameAPI.gameSetControllerIndexBumpers(i, controllers[i].leftBumper.down, controllers[i].leftBumper.justPressed, controllers[i].rightBumper.down, controllers[i].rightBumper.justPressed);
                gameAPI.gameSetControllerIndexFaceButtons(i, controllers[i].aButton.down, controllers[i].aButton.justPressed, controllers[i].bButton.down, controllers[i].bButton.justPressed, controllers[i].xButton.down, controllers[i].xButton.justPressed, controllers[i].yButton.down, controllers[i].yButton.justPressed);
                gameAPI.gameSetControllerIndexTriggers(i, controllers[i].leftTrigger, controllers[i].leftTriggerButton.down, controllers[i].leftTriggerButton.justPressed, controllers[i].rightTrigger, controllers[i].rightTriggerButton.down, controllers[i].rightTriggerButton.justPressed);
                gameAPI.gameSetControllerIndexSticks(i, controllers[i].leftStickX, controllers[i].leftStickY, controllers[i].rightStickX, controllers[i].rightStickY);
                gameAPI.gameSetControllerIndexStickDirections(i, controllers[i].leftStickUp.down, controllers[i].leftStickUp.justPressed, controllers[i].leftStickDown.down, controllers[i].leftStickDown.justPressed, controllers[i].leftStickLeft.down, controllers[i].leftStickLeft.justPressed, controllers[i].leftStickRight.down, controllers[i].leftStickRight.justPressed, controllers[i].rightStickUp.down, controllers[i].rightStickUp.justPressed, controllers[i].rightStickDown.down, controllers[i].rightStickDown.justPressed, controllers[i].rightStickLeft.down, controllers[i].rightStickLeft.justPressed, controllers[i].rightStickRight.down, controllers[i].rightStickRight.justPressed);
            }
        }
        for (let touchIndex = 0; touchIndex < MAX_NUM_TOUCHES; touchIndex++) {
            let touch = touches[touchIndex];
            gameAPI.setTouchInput(touchIndex, touch.active, touch.touchState.down, touch.touchState.justPressed, touch.x, touch.y, touch.radiusX, touch.radiusY);
        }
        gameAPI.onFrameStart();
        gameAPI.updateGame(dt);
        gameAPI.renderGame();
        if (webAudio.started) {
            webAudio.updateAudio(gameAPI, memory);
        }
        gameAPI.onFrameEnd();
        requestAnimationFrame(update);
    };
    resize();
    update();
}
main();
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
//# sourceMappingURL=gng.js.map