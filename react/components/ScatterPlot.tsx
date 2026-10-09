import {useRef, useEffect} from 'react';

type ScatterPlotProps = {
    width: number;
    height: number;
    data: { x: number; y: number }[];
};

// TODO actually make this draggable and zoomable
export const ScatterPlot = ({ width, height, data }: ScatterPlotProps) => {

    const canvasRef = useRef(null);

    const marginTop = 20;
    const marginRight = 20;
    const marginBottom = 30;
    const marginLeft = 40;

    // read the data
    // do some stuff with d3
    // compute all the <circle>
    const gx = useRef();
    const gy = useRef();
    //const x = d3.scaleLinear(d3.extent(data.map(d=>d.x)), [marginLeft, width - marginRight]);
    //const y = d3.scaleLinear(d3.extent(data.map(d=>d.y)), [height - marginBottom, marginTop]);
    //const line = d3.line((d, i) => x(d.x), y);
    console.log(data[10]);
    //useEffect(() => void d3.select(gx.current).call(d3.axisBottom(x)), [gx, x]);
    //useEffect(() => void d3.select(gy.current).call(d3.axisLeft(y)), [gy, y]);

    useEffect(() => {
        const canvas = canvasRef.current
        const context = canvas.getContext('2d');
        //Our first draw
        context.fillStyle = 'black';
        context.fillRect(0, 0, context.canvas.width, context.canvas.height);

        

        let yMax = data[0]?.y;
        let yMin = data[0]?.y;
        for (let i = 0; i < data.length; i++) {
            if(data[i].y > yMax)
                yMax = data[i].y;
            if(data[i].y < yMin)
                yMin = data[i].y;
        }

        console.log("found min and max");

        context.fillStyle = 'white';
        context.strokeStyle = 'white';
        const xScale = context.canvas.width / data.length;

        const yScale = context.canvas.height / (yMax - yMin);

        context.beginPath();

        let firstX = 0;
        let firstY = (data[0]?.y - yMin) * yScale;
        context.moveTo(firstX, firstY); // Move the pen to (30, 50)

        for (let i = 1; i < data.length; i++) {
            
            let x = i * xScale;
            let y = (data[i].y - yMin) * yScale; 
            //context.lineTo(x, context.canvas.height - y);//, 1, 1);
            context.fillRect(x, context.canvas.height - y, 2, 2);

            //ctx.lineTo(150, 100); 
        }
        context.lineWidth = 1;

        context.stroke();


        console.log("filled canvas");

        context.fillText(yMax, 0, 10);
        context.fillText(yMin, 0, context.canvas.height);

        
    }, [data]);

    return (
        <div>
            <canvas width={1000} height={600} ref={canvasRef}/>
            {/*<svg width={width} height={height}>
                  <g ref={gx} transform={`translate(0,${height - marginBottom})`} />
                  <g ref={gy} transform={`translate(${marginLeft},0)`} />
                  <g fill="white">
                    {data.map((d, i) => (<circle key={i} cx={x(d.x)} cy={y(d.y)} r="1.0" />))}
                  </g>
    </svg>*/}
        </div>
   );
};

