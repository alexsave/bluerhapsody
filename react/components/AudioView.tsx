"use client";

import { useState, useEffect, useWasm } from 'react';

import createModule from "../wasm/bluerhapsody.mjs";


//import { promises as fs } from 'fs';
export const AudioView = () => {
    const [raw, setRaw] = useState('hi');

    useEffect(() => {
        createModule().then((Module) => {
            console.log("Wasm ready", Module);
            let int_sqrt = Module.cwrap('int_sqrt', 'number', ['number'])
            console.log(int_sqrt(12));
            console.log(int_sqrt(28));
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
                    setRaw(e.target.result);
                }

                reader.readAsArrayBuffer(path);
            }}
        />
        <p>{raw.byteLength}</p>

    </div>;

}
