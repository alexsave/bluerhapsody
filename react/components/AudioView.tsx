"use client";

import { useRef, useState, useEffect, useWasm } from 'react';
import { ScatterPlot } from './ScatterPlot.tsx';

import createModule from "../wasm/bluerhapsody.mjs";


//import { promises as fs } from 'fs';
export const AudioView = () => {
    const [raw, setRaw] = useState(null);
    const [reload, setReload] = useState(0);

    const [lValues, setLValues] = useState([]);
    const [frequency, setFrequency] = useState(1);

    const [sampleCount, setSampleCount] = useState(100);
    const [sampleStart, setSampleStart] = useState(100);

    const [Module, setModule] = useState(null);

    const [data, setData] = useState([]);

    const [binCount, setBinCount] = useState(8192n);

    useEffect(() => {
        createModule().then((Module) => {
            setModule(Module);
        });
    }, []);
    // use ref?

    const canvasRef = useRef(null)
  
    useEffect(() => {
        const canvas = canvasRef.current
        const context = canvas.getContext('2d')
        //Our first draw
        context.fillStyle = '#000000'
        context.fillRect(0, 0, 200, context.canvas.height)
    }, [])

    useEffect(() => {
        if (Number(binCount) != 2**Math.floor(Math.log2(Number(binCount))))
            return;

        // buggy
        if (binCount == 1)
            return;

        if (!raw)
            return;

        const canvas = canvasRef.current
        const context = canvas.getContext('2d')
        context.clearRect(0, 0, canvas.width, canvas.height);
        drawSpectrogram();
    }, [reload, raw, binCount]);

    const drawSpectrogram = () => {
        const localRaw = raw;
        const length = localRaw.byteLength;

        // in
        const inBuffer = Module._malloc(length);
        Module.HEAPU8.set(new Uint8Array(localRaw), inBuffer);

        // out
        // maybe instead of sample count, we should allocate WavMetadata followed by samples, and return a pointer to that
        const resultPointer = Module.ccall(
            "wasm_get_channels",
            "number",
            ["number"],
            [inBuffer]);

        const sampleCount = Module.HEAPU32[resultPointer/4];
        //setSampleCount(sampleCount);
        const frequency = Module.HEAPU32[resultPointer/4 + 1];
        setFrequency(frequency);
        const lPtr = Module.HEAPU32[resultPointer/4 + 2];
        const rPtr = Module.HEAPU32[resultPointer/4 + 3];

        const left = Module.HEAPF64[lPtr/8];
        const right = Module.HEAPF64[lPtr/8];

        // its passed as just length of memory, but we will treat it as a f64*
        const resultFlatArray = [];
        for (let i = 0; i < sampleCount; i++) {
            resultFlatArray.push(Module.HEAPF64[lPtr/8 + i]);
        }

        setData(resultFlatArray.map((s,i) => ({x: i/frequency, y: s})));


        //Spectra * wasm_spectra(u8* file_buffer, u64 window_samples)
        //const binCount = binCount;

        const spectraPtr = Module.ccall("wasm_spectra", "number", ["number", "number"], [inBuffer, binCount]);
        

        // now plot it in a canvas
        const totalSampleCount = Number(Module.HEAPU64[spectraPtr/8]);
        const sampleRate = Number(Module.HEAPU64[spectraPtr/8 + 1]);
        const windowSamples = Number(Module.HEAPU64[spectraPtr/8 + 2]);
        const numWindows = Number(Module.HEAPU64[spectraPtr/8 + 3]);

        // should we just assume that each window is at i * .5 * window duraiton?

        console.log(Module.HEAPU64[spectraPtr/8]);
        console.log(Module.HEAPU64[spectraPtr/8 + 1]);
        console.log(Module.HEAPU64[spectraPtr/8 + 2]);
        console.log(Module.HEAPU64[spectraPtr/8 + 3]);

        console.log(Module.HEAPF64[spectraPtr/8 + 5]);

        const canvas = canvasRef.current
        const context = canvas.getContext('2d')

        console.log(canvas.width);

        // ok now we are going to space out the windows evenly from 0 to canvas.width

        const dx = canvas.width/numWindows;
        // windowSamples is bin count
        const dy = canvas.height/windowSamples;


        // at this point, we now have enough to kinda build somehting in canvas
        
        const freqStartIdx = spectraPtr/8 + 4;

        for (let f = 0; f < windowSamples; f++) {
            const freq = Module.HEAPU64[freqStartIdx + f];
        }

        const magStartIdx = freqStartIdx + windowSamples;
        let maxMag = 0n;
        for (let w = 0; w < numWindows; w++) {
            const windowStartIdx = magStartIdx + w * windowSamples;
            for (let f = 0; f < windowSamples; f++) {
                const mag = Module.HEAPF64[windowStartIdx + f];
                if (mag > maxMag)
                    maxMag = mag;
            }
        }
        console.log("found max mag" + maxMag);

        for (let w = 0; w < numWindows; w++) {
            const windowStartIdx = magStartIdx + w * windowSamples;
            for (let f = 0; f < windowSamples; f++) {
                const mag = Module.HEAPF64[windowStartIdx + f];

                //console.log(scaled_mag);
                //context.fillStyle = 'red';
                
                // flip f*dy sign later 
            }
            //console.log('calling window done')
        }

        // this works for sure, but the better way might be going pixel by pixel and figuring out what is in that bucket
        // also log

        new Float32Array(canvas.width * canvas.height);

        const height = canvas.height;
        

        for (let w = 0; w < numWindows; w++) {
            const windowStartIdx = magStartIdx + w * windowSamples;
            for (let f = 0; f < windowSamples; f++) {
                const mag = Module.HEAPF64[windowStartIdx + f];
                //const scaled_mag = mag/maxMag * 256;
                let scaled_mag = 256*(10*Math.log10(mag/maxMag) + 60)/60;
                //let scaled_mag = 256*(Math.pow(mag/maxMag, 1/6));
                if (mag == 0)scaled_mag = 0;
                // now the question - what pixel does this belong in?

                let x = Math.floor(w * canvas.width / numWindows);
                // make it log sacle to be more interesting
                let y = Math.floor(Math.log2(f) * canvas.height / Math.log2(windowSamples));
                const t = scaled_mag / 255.0;
                context.fillStyle = `rgb(${255*Math.min(1, t*1.8)}, ${255*Math.max(0, t*1.6-0.6)}, ${255*Math.max(0, Math.min(2*t, 1.2-2.2*t))})`

                //context.fillStyle = `rgb(${255*t**3}, ${255*t**1.2}, ${255*t**0.6})`

                //context.fillStyle = `hsl(${280 - 280*t}, 100%, ${60*t}%)`
                //context.fillStyle = `rgb(${scaled_mag}, ${scaled_mag/4}, ${scaled_mag/2})`;
                context.fillRect(x, height-y, dx, 1);
            }
        }
        context.font = "10px Arial";
        context.fillStyle = "white";
        context.strokeStyle = "white";      

        // not quite

        // ok what do we know
        // the 
        // this should probably be computed int eh damn C
        // but wasm isnt set correctly
        // we have frequency, and we have windowSamples (count of them)

        // if we have 8192 samples at frequency 48khz, we can detect... what exactly
        // its a simple solution
        // but the index 0 is zero frequency, index 1 is the slowest frequency we can detect, taking the entire span of 8192 samples
        // thus 1/6 second ish? so 48khz/8192
        // ok so thats wher ethe first frequency lies
        // second is then 2 * 48khz / 8192
        // all  so i * 48khz / 8192
        // so these things are plotted at  Math.log2(i) / Math.log2(windowSamples)

        // so to get the "i", we want to do 10 = i * 48khz/ 8192
        // i = 10 / 48khz * 8192

        
        const add = (text, freq) => context.fillText(text, 0, height + 10/2 - Math.floor(Math.log2(freq * windowSamples / frequency ) * canvas.height / Math.log2(windowSamples)));
        for (let i = 1; i < 20; i++ ){
            add(i + "Hz", i);
            add(i + "0Hz", i*10);
            add(i + "00Hz", i*100);
            add(i + "kHz", i*1000);
            add(i + "0kHz", i*10000);
        }

        Module._free(inBuffer);
        Module._free(lPtr);
        Module._free(rPtr);
        Module._free(spectraPtr);


    }


    return <div>
        <p>Blue Rhapsody UI</p> 
        <input 
        type="file" 
        accept="audio/*"
        onChange={async e => {
            const path = e.target.files[0];

            const reader = new FileReader();
            reader.onload = async e => {
                let localRaw = e.target.result;
                setRaw(e.target.result);
                setReload(prev=> prev+1);
            }

            reader.readAsArrayBuffer(path);
        }}
    />
        <p>{JSON.stringify(lValues.slice(0,100))}</p>
        <p>{data.length}</p>
        <input value={''+binCount} type="text" onChange={e => setBinCount(BigInt(e.target.value))} style={{border: '1px solid blue'}}/>
        <input value={''+sampleStart} type="text" onChange={e => setSampleStart(+e.target.value)} style={{border: '1px solid blue'}}/>
        <input value={''+sampleCount} type="text" onChange={e => setSampleCount(+e.target.value)} style={{border: '1px solid blue'}}/>
        <canvas width={1000} height={1000} ref={canvasRef}/>
        <ScatterPlot width={600} height={600} data={data.slice(sampleStart, sampleStart + sampleCount)}/>

        // very useful, take a look https://medium.com/@pdx.lucasm/canvas-with-react-js-32e133c05258
        //also -new Float64Array(Module.HEAPF64.buffer, ptr, count)


        </div>;

}
