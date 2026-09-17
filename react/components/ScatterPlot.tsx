import * as d3 from "d3";
import {useRef, useEffect} from 'react';

type ScatterPlotProps = {
    width: number;
    height: number;
    data: { x: number; y: number }[];
};

export const ScatterPlot = ({ width, height, data }: ScatterPlotProps) => {
    const marginTop = 20;
    const marginRight = 20;
    const marginBottom = 30;
    const marginLeft = 40;

    // read the data
    // do some stuff with d3
    // compute all the <circle>
    const gx = useRef();
    const gy = useRef();
    const x = d3.scaleLinear(d3.extent(data.map(d=>d.x)), [marginLeft, width - marginRight]);
    const y = d3.scaleLinear(d3.extent(data.map(d=>d.y)), [height - marginBottom, marginTop]);
    //const line = d3.line((d, i) => x(d.x), y);
    console.log(data[10]);
    useEffect(() => void d3.select(gx.current).call(d3.axisBottom(x)), [gx, x]);
    useEffect(() => void d3.select(gy.current).call(d3.axisLeft(y)), [gy, y]);


    return (
        <div>
            <svg width={width} height={height}>
                  <g ref={gx} transform={`translate(0,${height - marginBottom})`} />
                  <g ref={gy} transform={`translate(${marginLeft},0)`} />
                  <g fill="white" stroke="currentColor" strokeWidth="1.5">
                    {data.map((d, i) => (<circle key={i} cx={x(d.x)} cy={y(d.y)} r="2.5" />))}
                  </g>
    </svg>
        </div>
   );
};

