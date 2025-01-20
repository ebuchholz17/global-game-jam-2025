class WebAudio {

    public samplesPerSecond: number = 0;
    public audioCtx: AudioContext = null;
    public audioBufferSize: number = 512;

    public started: boolean = false;

    public playTime: number = 0.0;
    public nextAudioBufferPlayTime: number = 0.0;

    public init (): void {
        this.audioCtx = new AudioContext();
        this.samplesPerSecond = this.audioCtx.sampleRate;
        this.playTime = this.audioCtx.currentTime;
        this.started = true;
    }

    public getSampleRate (): number {
        let dummyAudioCtx = new AudioContext();
        return dummyAudioCtx.sampleRate;
    }

    public updateAudio (gameAPI: GameAPI, memory: Uint8Array): void {
        let currentAudioTime: number = this.audioCtx.currentTime;
        let bufferingDelay = 50.0 /1000.0;
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

    public tryUnlockAudioContext (): void {
        if (this.audioCtx.state == "suspended") {
            this.audioCtx.resume();
        }
    }
}
