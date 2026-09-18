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
        context.fillStyle = '#FF0000'
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

                    const totalSampleCount = Module.HEAPF64[spectraPtr/8];
                    const sampleRate = Module.HEAPF64[spectraPtr/8 + 1];
                    const windowSamples = Module.HEAPF64[spectraPtr/8 + 2];
                    const numWindows = Module.HEAPF64[spectraPtr/8 + 3];

                    // should we just assume that each window is at i * .5 * window duraiton?

                    console.log(Module.HEAPU64[spectraPtr/8]);
                    console.log(Module.HEAPU64[spectraPtr/8 + 1]);
                    console.log(Module.HEAPU64[spectraPtr/8 + 2]);
                    console.log(Module.HEAPU64[spectraPtr/8 + 3]);

                    const acccess = new Float64Array(Module.HEAPF64.buffer, spectraPtr/8 + 4, (numWindows+1)*windowSamples)
                    console.log(access[0]);
                    console.log(access[1]);


                    
                    // pray
                    //setLValues(resultFlatArray);;

                    Module._free(inBuffer);
                    Module._free(lPtr);
                    Module._free(rPtr);
                    Module._free(freqMagPtr);

                }

                reader.readAsArrayBuffer(path);
            }}
        />
        <p>{raw.byteLength}</p>
        <p>{JSON.stringify(lValues.slice(0,100))}</p>
        <p>{data.length}</p>
        <ScatterPlot width={600} height={600} data={data.slice(frequency*0,frequency*0+100)}/>
  
// very useful, take a look https://medium.com/@pdx.lucasm/canvas-with-react-js-32e133c05258
//also -new Float64Array(Module.HEAPF64.buffer, ptr, count)
        <canvas ref={canvasRef}/>


    </div>;

}
