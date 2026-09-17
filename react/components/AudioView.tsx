"use client";

import { useState, useEffect, useWasm } from 'react';

import createModule from "../wasm/bluerhapsody.mjs";


//import { promises as fs } from 'fs';
export const AudioView = () => {
    const [raw, setRaw] = useState('hi');

    const [Module, setModule] = useState(null);

    useEffect(() => {
        createModule().then((Module) => {
            setModule(Module);
            //console.log("Wasm ready", Module);
            //let int_sqrt = Module.cwrap('int_sqrt', 'number', ['number'])
            //console.log(int_sqrt(12));
            //console.log(int_sqrt(28));
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

                    console.log(length);
                    console.log((new Uint8Array(localRaw))[0]);

                    // in
                    const inBuffer = Module._malloc(length);
                    Module.HEAPU8.set(new Uint8Array(localRaw), inBuffer);

                    //result. size idk
                    const lBuffer = Module._malloc(length);
                    //Module.HEAPU8.set(localRaw, lBuffer);
                    const rBuffer = Module._malloc(length);
                    //Module.HEAPU8.set(localRaw, rBuffer);

                    // out
                    const sampleCount = Module.ccall(
                        "wasm_get_channels",
                        "number",
                        ["number", "number", "number"],
                        [inBuffer, lBuffer, rBuffer]);

                    // its passed as just length of memory, but we will treat it as a f64*
                    const resultFlatArray = [];
                    for (let i = 0; i < sampleCount; i++) {
                        resultFlatArray.push(Module.HEAPF64[lBuffer/8 + i]);
                    }

                    console.log(resultFlatArray[0]);
                    console.log(resultFlatArray[1]);
                    console.log(resultFlatArray[2]);
                    console.log(resultFlatArray[3]);
                    console.log(resultFlatArray[4]);
                    console.log(resultFlatArray[5]);
                    console.log(resultFlatArray[6]);
                    console.log(resultFlatArray[7]);

                    Module._free(inBuffer);
                    Module._free(outBuffer);




                    //let int_sqrt = Module.cwrap('int_sqrt', 'number', ['number'])
                    //console.log(int_sqrt(12));
                    //console.log(int_sqrt(28));

                }

                reader.readAsArrayBuffer(path);
            }}
        />
        <p>{raw.byteLength}</p>

    </div>;

}
