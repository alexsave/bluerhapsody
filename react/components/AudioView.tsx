"use client";

import {useState} from 'react';
//import { promises as fs } from 'fs';
export const AudioView = () => {
    const [raw, setRaw] = useState('hi');
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
