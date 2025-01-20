/// <reference path="webgl_renderer.ts"/>

let textDecoder: TextDecoder = new TextDecoder("utf-8");
let textEncoder: TextEncoder = new TextEncoder();

class GameAPI {
    initGame: CallableFunction = null;
    initAudio: CallableFunction = null;

    gameSetControllerConnected: CallableFunction = null;
    gameSetControllerIndexDPad: CallableFunction = null;
    gameSetControllerIndexStartBack: CallableFunction = null;
    gameSetControllerIndexStickButtons: CallableFunction = null;
    gameSetControllerIndexBumpers: CallableFunction = null;
    gameSetControllerIndexFaceButtons: CallableFunction = null;
    gameSetControllerIndexTriggers: CallableFunction = null;
    gameSetControllerIndexSticks: CallableFunction = null;
    gameSetControllerIndexStickDirections: CallableFunction = null;

    setTouchInput: CallableFunction = null;
    setVirtualInputEnabled: CallableFunction = null;

    onFrameStart: CallableFunction = null;
    updateGame: CallableFunction = null;
    renderGame: CallableFunction = null;
    getGameSoundSamples: CallableFunction = null;
    onFrameEnd: CallableFunction = null;

    onResize: CallableFunction = null;
    onMouseMove: CallableFunction = null;
    onMouseUp: CallableFunction = null;
    onMouseDown: CallableFunction = null;

    onLetterKeyDown: CallableFunction = null;
    onLetterKeyUp: CallableFunction = null;
    onArrowKeyDown: CallableFunction = null;
    onArrowKeyUp: CallableFunction = null;

    onFileFetched: CallableFunction = null;

    webAllocMemory: CallableFunction = null;
    webAllocTempMemory: CallableFunction = null;
}

enum GameInputEventType {
    MOUSE_DOWN,
    MOUSE_UP,
    MOUSE_MOVE,
    KEY_DOWN,
    KEY_UP
}

enum GameInputArrowKeyDir {
    UP,
    DOWN,
    LEFT,
    RIGHT
}

let gameInputKeyboardLetters = "abcdefghijklmnopqrstuvwxyz";

class GameInputEvent {
    public type: GameInputEventType;
    public x: number = 0;
    public y: number = 0;

    public key: string = null;
}

class WebInputKey {
    public down: boolean = false;
    public justPressed: boolean = false;
}

class WebTouch {
    public active: boolean = false;
    public identifier: number = -1;

    public touchState: WebInputKey = new WebInputKey();
    public x: number = -1;
    public y: number = -1;

    public radiusX: number = -1;
    public radiusY: number = -1;
}

class WebGamepad {
    public connected: boolean = false;
    public gamePadRef: Gamepad = null;

    public dPadUp: WebInputKey = new WebInputKey();
    public dPadDown: WebInputKey = new WebInputKey();
    public dPadLeft: WebInputKey = new WebInputKey();
    public dPadRight: WebInputKey = new WebInputKey();

    public start: WebInputKey = new WebInputKey();
    public back: WebInputKey = new WebInputKey();

    public leftStick: WebInputKey = new WebInputKey();
    public rightStick: WebInputKey = new WebInputKey();

    public leftBumper: WebInputKey = new WebInputKey();
    public rightBumper: WebInputKey = new WebInputKey();

    public aButton: WebInputKey = new WebInputKey();
    public bButton: WebInputKey = new WebInputKey();
    public xButton: WebInputKey = new WebInputKey();
    public yButton: WebInputKey = new WebInputKey();

    public leftTrigger: number = 0.0;
    public leftTriggerButton: WebInputKey = new WebInputKey();
    public rightTrigger: number = 0.0;
    public rightTriggerButton: WebInputKey = new WebInputKey();

    public leftStickX: number = 0.0;
    public leftStickY: number = 0.0;
    public rightStickX: number = 0.0;
    public rightStickY: number = 0.0;

    public leftStickUp: WebInputKey = new WebInputKey();
    public leftStickDown: WebInputKey = new WebInputKey();
    public leftStickLeft: WebInputKey = new WebInputKey();
    public leftStickRight: WebInputKey = new WebInputKey();

    public rightStickUp: WebInputKey = new WebInputKey();
    public rightStickDown: WebInputKey = new WebInputKey();
    public rightStickLeft: WebInputKey = new WebInputKey();
    public rightStickRight: WebInputKey = new WebInputKey();
}

function bytesToString (memory: Uint8Array, msgPointer: number): string {
    let msgStart: number = msgPointer;
    let msgEnd: number = msgPointer;
    while (memory[msgEnd] != 0) {
        ++msgEnd;
    }

    let msgBuffer: Uint8Array = memory.subarray(msgStart, msgEnd);
    let text: string = textDecoder.decode(msgBuffer);
    return text;
}

function processInputEvents (gameAPI: GameAPI, events: GameInputEvent[], memory: Uint8Array) {
    for (let eventIndex: number = 0; eventIndex < events.length; ++eventIndex) {
        let event: GameInputEvent = events[eventIndex];

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
            case GameInputEventType.MOUSE_MOVE: {
                gameAPI.onMouseMove(event.x, event.y);
            } break;
            case GameInputEventType.MOUSE_DOWN: {
                gameAPI.onMouseDown();
            } break;
            case GameInputEventType.MOUSE_UP: {
                gameAPI.onMouseUp();
            } break;
            case GameInputEventType.KEY_DOWN: {
                if (letter != "") {
                    gameAPI.onLetterKeyDown(letter.charCodeAt(0));
                }
                else if (arrowDir != -1) {
                    gameAPI.onArrowKeyDown(arrowDir);
                }
            } break;
            case GameInputEventType.KEY_UP: {
                if (letter != "") {
                    gameAPI.onLetterKeyUp(letter.charCodeAt(0));
                }
                else if (arrowDir != -1) {
                    gameAPI.onArrowKeyUp(arrowDir);
                }
            } break;
        }
    }
}

function encodeTempString (str: string, memory: Uint8Array, gameAPI: GameAPI): number {
    let strBytes: Uint8Array = textEncoder.encode(str);
    let tempGameSideString: number = gameAPI.webAllocTempMemory(strBytes.length);
    let gameSideStringView = new Uint8Array(memory.buffer, tempGameSideString, strBytes.length);
    return gameSideStringView.byteOffset;
}

function updateControllerButton (gameController: WebGamepad, buttonProp: keyof WebGamepad, 
                                 gamepad: Gamepad, buttonID: number): void 
{
    let inputKey = <WebInputKey>(gameController[buttonProp]);
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

function updateControllerTrigger (gameController: WebGamepad, triggerProp: keyof WebGamepad, 
                                  triggerButtonProp: keyof WebGamepad, value: number) 
{
    let inputKey = <WebInputKey>(gameController[triggerButtonProp]);
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

function updateControllerStick (gameController: WebGamepad, 
                                xProperty: string, xValue: number, 
                                yProperty: string, yValue: number, deadZone: number): void 
{
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

function updateControllerStickDirection (gameController: WebGamepad, dir0Prop: string, dir1Prop: string, 
                                         value: number, deadZone: number) 
{
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

function updateController (gameController: WebGamepad, gamepad: Gamepad): void {
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

function clientPosToScreenPos (clientPos: number, canvasDim: number, clientDim: number, clientStartPos: number): number {
    let scale = canvasDim / clientDim;
    return (clientPos - clientStartPos) * scale;
}

async function main (): Promise<void> {
    let pageSize: number = 64 * 1024;
    let numPages: number = 10000;
    let wasmMemory: WebAssembly.Memory = new WebAssembly.Memory({ initial: numPages });
    let memory: Uint8Array = new Uint8Array(wasmMemory.buffer);

    let wglr: WebGLRenderer;

    // TODO(ebuchholz): try instantiateStreaming again, with fixed mime-type
    let wasmResponse = await fetch("./gng.wasm");
    let wasmBytes = await wasmResponse.arrayBuffer();

    let wasmObj = {
        env: {
            memory: wasmMemory,
            getMemCapacity: (): number => {
                return pageSize * numPages;
            },
            consoleLog: (msgPointer: number): void => {
                console.log(bytesToString(memory, msgPointer));
            },
            rngSeedFromTime: (): BigInt => {
                return BigInt(Date.now());
            },
            readFile: (assetNamePtr: number, assetPathPtr: number): void => {
                let assetName: string = bytesToString(memory, assetNamePtr);
                let assetPath: string = bytesToString(memory, assetPathPtr);

                fetch(assetPath).then((response: Response): void => {
                    response.arrayBuffer().then((data: ArrayBuffer): void => {
                        // Copy file data into wasm memory
                        let dataView = new Uint8Array(data);
                        let numBytes: number = data.byteLength;
                        // TODO: better management of temporary loaded file data
                        let gameSideData: number = gameAPI.webAllocTempMemory(numBytes);
                        let gameSideDataView = new Uint8Array(memory.buffer, gameSideData, numBytes);
                        gameSideDataView.set(dataView, 0);

                        gameAPI.onFileFetched(assetNamePtr, gameSideDataView.byteOffset, gameSideDataView.length);
                    });
                });
            },
            onError: (msgPointer: number): void => {
                console.error(bytesToString(memory, msgPointer));
            },
            rendererResize: (windowWidth: number, windowHeight: number): void => {
                resizeWebGL(windowWidth, windowHeight);
            },
            webglOnRenderStart: (): void => { webglOnRenderStart(); },
            webglLoadTexture: (id: number, width: number, height: number, pixelDataPtr: number): void => {
                webglLoadTexture(id, width, height, pixelDataPtr); 
            },
            webglSpriteBatchStart: (): void => { webglSpriteBatchStart(); },
            webglSpriteBatchFlush: (spriteDataPtr: number, numSprites: number, vertexSize: number, totalNumSpritesDrawn: number, textureIDs: number, numTextures: number): void => {
                webglSpriteBatchFlush(spriteDataPtr, numSprites, vertexSize, totalNumSpritesDrawn, textureIDs, numTextures);
            }
        }
    };
    let result = await WebAssembly.instantiate(wasmBytes, wasmObj);
    let wasmInstance: WebAssembly.Instance = result.instance;

    let gameAPI = <GameAPI>{
        initGame: (wasmInstance.exports.initGame as CallableFunction),
        initAudio: (wasmInstance.exports.initAudio as CallableFunction),

        onFrameStart: (wasmInstance.exports.onFrameStart as CallableFunction),
        updateGame: (wasmInstance.exports.updateGame as CallableFunction),
        renderGame: (wasmInstance.exports.renderGame as CallableFunction),
        getGameSoundSamples: (wasmInstance.exports.getGameSoundSamples as CallableFunction),
        onFrameEnd: (wasmInstance.exports.onFrameEnd as CallableFunction),

        gameSetControllerConnected: (wasmInstance.exports.gameSetControllerConnected as CallableFunction),
        gameSetControllerIndexDPad: (wasmInstance.exports.gameSetControllerIndexDPad as CallableFunction),
        gameSetControllerIndexStartBack: (wasmInstance.exports.gameSetControllerIndexStartBack as CallableFunction),
        gameSetControllerIndexStickButtons: (wasmInstance.exports.gameSetControllerIndexStickButtons as CallableFunction),
        gameSetControllerIndexBumpers: (wasmInstance.exports.gameSetControllerIndexBumpers as CallableFunction),
        gameSetControllerIndexFaceButtons: (wasmInstance.exports.gameSetControllerIndexFaceButtons as CallableFunction),
        gameSetControllerIndexTriggers: (wasmInstance.exports.gameSetControllerIndexTriggers as CallableFunction),
        gameSetControllerIndexSticks: (wasmInstance.exports.gameSetControllerIndexSticks as CallableFunction),
        gameSetControllerIndexStickDirections: (wasmInstance.exports.gameSetControllerIndexStickDirections as CallableFunction),

        setTouchInput: (wasmInstance.exports.setTouchInput as CallableFunction),
        setVirtualInputEnabled: (wasmInstance.exports.setVirtualInputEnabled as CallableFunction),

        onFileFetched: (wasmInstance.exports.onFileFetched as CallableFunction),
        webAllocMemory: (wasmInstance.exports.webAllocMemory as CallableFunction),
        webAllocTempMemory: (wasmInstance.exports.webAllocTempMemory as CallableFunction),
        onResize: (wasmInstance.exports.onResize as CallableFunction),

        onMouseMove: (wasmInstance.exports.onMouseMove as CallableFunction),
        onMouseDown: (wasmInstance.exports.onMouseDown as CallableFunction),
        onMouseUp: (wasmInstance.exports.onMouseUp as CallableFunction),
        onLetterKeyDown: (wasmInstance.exports.onLetterKeyDown as CallableFunction),
        onLetterKeyUp: (wasmInstance.exports.onLetterKeyUp as CallableFunction),
        onArrowKeyDown: (wasmInstance.exports.onArrowKeyDown as CallableFunction),
        onArrowKeyUp: (wasmInstance.exports.onArrowKeyUp as CallableFunction)
    };

    let justResized = false;
    let resize = (): void => {
        justResized = true;

        canvas.width = mainDiv.clientWidth;
        canvas.height = mainDiv.clientHeight;
        canvas.style.width = mainDiv.clientWidth + "px";
        canvas.style.height = mainDiv.clientHeight + "px";

        gameAPI.onResize(canvas.width, canvas.height);
    }

    let mainDiv: HTMLElement = <HTMLElement>document.getElementById("gng");
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

    let canvas: HTMLCanvasElement = document.createElement("canvas");
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

    let inputEvents: GameInputEvent[] = [];
    canvas.addEventListener("mousedown", (e: MouseEvent): void => {
        let inputEvent = <GameInputEvent>{
            type: GameInputEventType.MOUSE_DOWN
        };
        inputEvents.push(inputEvent);

        if (!webAudio.started) {
            webAudio.init();
        }
        // for ios/chrome/etc, whatever browers need touch input to start audio
        webAudio.tryUnlockAudioContext();
    });
    canvas.addEventListener("mouseup", (e: MouseEvent): void => {
        let inputEvent = <GameInputEvent>{
            type: GameInputEventType.MOUSE_UP
        };
        inputEvents.push(inputEvent);
    });
    canvas.addEventListener("mousemove", (e: MouseEvent): void => {
        let clientX = e.clientX;
        let clientY = e.clientY;
        let scaleX = canvas.width / canvas.clientWidth;
        let scaleY = canvas.height / canvas.clientHeight;
        let inputEvent = <GameInputEvent>{
            type: GameInputEventType.MOUSE_MOVE,
            x: (clientX - canvas.clientLeft) * scaleX,
            y: (clientY - canvas.clientTop) * scaleY
        };
        inputEvents.push(inputEvent);
    });
    document.addEventListener("keydown", (e: KeyboardEvent): void => {
        inputEvents.push(<GameInputEvent>{
            type: GameInputEventType.KEY_DOWN,
            key: e.key
        });
    });
    document.addEventListener("keyup", (e: KeyboardEvent): void => {
        inputEvents.push(<GameInputEvent>{
            type: GameInputEventType.KEY_UP,
            key: e.key
        });
    });


    let touches: WebTouch[] = [];
    const MAX_NUM_TOUCHES = 4;
    for (let i = 0; i < MAX_NUM_TOUCHES; i++) {
        touches[i] = new WebTouch();
    }
    canvas.addEventListener("touchstart", (e: TouchEvent) => {
        let changedTouches: TouchList = e.changedTouches;
        for (let changedTouchIndex = 0; changedTouchIndex < changedTouches.length; changedTouchIndex++) {
            let touch: Touch = changedTouches.item(changedTouchIndex);

            for (let touchIndex = 0; touchIndex < MAX_NUM_TOUCHES; touchIndex++) {
                let webTouch: WebTouch = touches[touchIndex];
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
    canvas.addEventListener("touchmove", (e: TouchEvent) => {
        let changedTouches: TouchList = e.changedTouches;
        for (let changedTouchIndex = 0; changedTouchIndex < changedTouches.length; changedTouchIndex++) {
            let touch: Touch = changedTouches.item(changedTouchIndex);

            for (let touchIndex = 0; touchIndex < MAX_NUM_TOUCHES; touchIndex++) {
                let webTouch: WebTouch = touches[touchIndex];
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
    canvas.addEventListener("touchend", (e: TouchEvent) => {
        let changedTouches: TouchList = e.changedTouches;
        for (let changedTouchIndex = 0; changedTouchIndex < changedTouches.length; changedTouchIndex++) {
            let touch: Touch = changedTouches.item(changedTouchIndex);

            for (let touchIndex = 0; touchIndex < MAX_NUM_TOUCHES; touchIndex++) {
                let webTouch: WebTouch = touches[touchIndex];
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
    canvas.addEventListener("touchcancel", (e: TouchEvent) => {
        let changedTouches: TouchList = e.changedTouches;
        for (let changedTouchIndex = 0; changedTouchIndex < changedTouches.length; changedTouchIndex++) {
            let touch: Touch = changedTouches.item(changedTouchIndex);

            for (let touchIndex = 0; touchIndex < MAX_NUM_TOUCHES; touchIndex++) {
                let webTouch: WebTouch = touches[touchIndex];
                if (webTouch.active && webTouch.identifier == touch.identifier) {
                    webTouch.active = false;
                    webTouch.touchState.down = false;
                    break;
                }
            }
        }
    });
    

    let controllers: WebGamepad[] = [];
    const MAX_NUM_CONTROLLERS = 4;
    for (let i = 0; i < MAX_NUM_CONTROLLERS; i++) {
        controllers[i] = new WebGamepad();
    }

    window.addEventListener("gamepadconnected", (e: GamepadEvent) => {
        controllers[e.gamepad.index].connected = true;
        controllers[e.gamepad.index].gamePadRef = e.gamepad;
    });
    window.addEventListener("gamepaddisconnected", (e: GamepadEvent) => {
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

    let update = (): void => {
        let currentTime = new Date().getTime() / 1000;
        let dt = currentTime - lastTime;
        lastTime = currentTime;

        processInputEvents(gameAPI, inputEvents, memory);
        inputEvents.length = 0;

        let gamepads: Gamepad[] = navigator.getGamepads();
        for (let i = 0; i < gamepads.length && i < MAX_NUM_CONTROLLERS; i++) {
            if (gamepads[i] == null) {
                controllers[i].connected = false;
                controllers[i].gamePadRef = null;
            }
            else {
                updateController(controllers[i], gamepads[i]);
                gameAPI.gameSetControllerConnected(i, controllers[i].connected);
                gameAPI.gameSetControllerIndexDPad(i, 
                    controllers[i].dPadUp.down, controllers[i].dPadUp.justPressed,
                    controllers[i].dPadDown.down, controllers[i].dPadDown.justPressed,
                    controllers[i].dPadLeft.down, controllers[i].dPadLeft.justPressed,
                    controllers[i].dPadRight.down, controllers[i].dPadRight.justPressed);
                gameAPI.gameSetControllerIndexStartBack(i, 
                    controllers[i].start.down, controllers[i].start.justPressed,
                    controllers[i].back.down, controllers[i].back.justPressed)
                gameAPI.gameSetControllerIndexStickButtons(i, 
                    controllers[i].leftStick.down, controllers[i].leftStick.justPressed,
                    controllers[i].rightStick.down, controllers[i].rightStick.justPressed);
                gameAPI.gameSetControllerIndexBumpers(i, 
                    controllers[i].leftBumper.down, controllers[i].leftBumper.justPressed,
                    controllers[i].rightBumper.down, controllers[i].rightBumper.justPressed);
                gameAPI.gameSetControllerIndexFaceButtons(i, 
                    controllers[i].aButton.down, controllers[i].aButton.justPressed,
                    controllers[i].bButton.down, controllers[i].bButton.justPressed,
                    controllers[i].xButton.down, controllers[i].xButton.justPressed,
                    controllers[i].yButton.down, controllers[i].yButton.justPressed);
                gameAPI.gameSetControllerIndexTriggers(i, 
                    controllers[i].leftTrigger, 
                    controllers[i].leftTriggerButton.down, controllers[i].leftTriggerButton.justPressed,
                    controllers[i].rightTrigger, 
                    controllers[i].rightTriggerButton.down, controllers[i].rightTriggerButton.justPressed);
                gameAPI.gameSetControllerIndexSticks(i,                     
                    controllers[i].leftStickX,  
                    controllers[i].leftStickY,  
                    controllers[i].rightStickX,  
                    controllers[i].rightStickY);
                gameAPI.gameSetControllerIndexStickDirections(i, 
                    controllers[i].leftStickUp.down, controllers[i].leftStickUp.justPressed,
                    controllers[i].leftStickDown.down, controllers[i].leftStickDown.justPressed,
                    controllers[i].leftStickLeft.down, controllers[i].leftStickLeft.justPressed,
                    controllers[i].leftStickRight.down, controllers[i].leftStickRight.justPressed,
                    controllers[i].rightStickUp.down, controllers[i].rightStickUp.justPressed,
                    controllers[i].rightStickDown.down, controllers[i].rightStickDown.justPressed,
                    controllers[i].rightStickLeft.down, controllers[i].rightStickLeft.justPressed,
                    controllers[i].rightStickRight.down, controllers[i].rightStickRight.justPressed);
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
    }

    resize();
    update();
}

main();
