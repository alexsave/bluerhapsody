"use client";

import { useState, useEffect, useWasm } from 'react';
import { ScatterPlot } from './ScatterPlot.tsx';

import createModule from "../wasm/bluerhapsody.mjs";


//import { promises as fs } from 'fs';
export const AudioView = () => {
    const [raw, setRaw] = useState('hi');

    const [lValues, setLValues] = useState([]);
    const [frequency, setFrequency] = useState(1);
    const [sampleCount, setSampleCount] = useState(1);

    const [Module, setModule] = useState(null);

    const [data, setData] = useState([]);

    useEffect(() => {
        createModule().then((Module) => {
            setModule(Module);
        });
    }, []);
    // use ref?

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
                    setSampleCount(sampleCount);
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


                    const freqMagPtr = Module.ccall("wasm_fft", "number", ["number"], [inBuffer]);
                    
                    console.log(Module.HEAPF64[freqMagPtr/8]);
                    console.log(Module.HEAPF64[freqMagPtr/8 + 1]);

                    console.log(Module.HEAPF64[freqMagPtr/8 + 2]);
                    console.log(Module.HEAPF64[freqMagPtr/8 + 3]);

                    
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
        <ScatterPlot width={1000} height={600} data={data.slice(frequency*2,frequency*2+100)}/>

    </div>;

}
