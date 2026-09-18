"use client";

import { useRef, useState, useEffect, useWasm } from 'react';
import { ScatterPlot } from './ScatterPlot.tsx';

import createModule from "../wasm/bluerhapsody.mjs";


//import { promises as fs } from 'fs';
export const AudioView = () => {
    const [raw, setRaw] = useState('hi');

    const [lValues, setLValues] = useState([]);
    const [frequency, setFrequency] = useState(1);
    //const [sampleCount, setSampleCount] = useState(1);

    const [Module, setModule] = useState(null);

    const [data, setData] = useState([]);

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

                    const spectraPtr = Module.ccall("wasm_spectra", "number", ["number", "number"], [inBuffer, 8192n]);
                    

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

                            const scaled_mag = mag/maxMag * 256;
                            //console.log(scaled_mag);

                            context.fillStyle = `rgb(${scaled_mag}, ${scaled_mag}, ${scaled_mag})`;
                            //context.fillStyle = 'red';
//'rgb(0.18873989692110288,0.18873989692110288,0.18873989692110288)';

                            
                            // flip f*dy sign later 
                            context.fillRect(w*dx, f*dy, dx, dy);
                        }
                        console.log('calling window done')
                    }

                    //Our first draw
                    //context.fillStyle = '#00FF00'
                    //context.fillRect(20, 20, 20, 20);

                    //const access = new Float64Array(Module.HEAPF64.buffer, spectraPtr/8 + 4, (numWindows+1)*windowSamples)
                    //console.log(access[0]);
                    //console.log(access[1]);


                    
                    // pray
                    //setLValues(resultFlatArray);;

                    Module._free(inBuffer);
                    Module._free(lPtr);
                    Module._free(rPtr);
                    Module._free(spectraPtr);

                }

                reader.readAsArrayBuffer(path);
            }}
        />
        <p>{raw.byteLength}</p>
        <p>{JSON.stringify(lValues.slice(0,100))}</p>
        <p>{data.length}</p>
        <canvas width={1000} height={1000} ref={canvasRef}/>
        <ScatterPlot width={600} height={600} data={data.slice(frequency*0,frequency*0+100)}/>
  
// very useful, take a look https://medium.com/@pdx.lucasm/canvas-with-react-js-32e133c05258
//also -new Float64Array(Module.HEAPF64.buffer, ptr, count)


    </div>;

}
