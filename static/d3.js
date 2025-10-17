import * as d3 from "https://cdn.jsdelivr.net/npm/d3@7/+esm";

// set the dimensions and margins of the graph
const margin = {top: 20, right: 20, bottom: 30, left: 40},
        width = 640 - margin.left - margin.right,
        height = 400 - margin.top - margin.bottom;

// append the svg object to the body of the page
const svg = d3.select("#container")
    .append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
    .append("g")
    .attr("transform", `translate(${margin.left}, ${margin.top})`);

svg.append("image")
    .attr("href", "/static/images/dark_map.png")
    .attr("x", 0)
    .attr("y", 0)
    .attr("width", width)
    .attr("height", height);

//Read the data
d3.csv("/data").then( function(data) {

    // Add X axis
    const x = d3.scaleLinear()
    .domain([0, 100])
    .range([ 0, width ]);
    svg.append("g")
    .attr("transform", `translate(0, ${height})`)
    .call(d3.axisBottom(x));

    // Add Y axis
    const y = d3.scaleLinear()
    .domain([0, 100])
    .range([ height, 0]);
    svg.append("g")
    .call(d3.axisLeft(y));


    // Add dots
svg.append('g')
    .selectAll("dot")
    .data(data)
    .join("circle")
        .attr("cx", function (d) { return x(d.x); } )
        .attr("cy", function (d) { return y(d.y); } )
        .attr("r", 2.5)
        .style("fill", "#b36969ff")

})

