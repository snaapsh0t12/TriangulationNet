import * as d3 from "https://cdn.jsdelivr.net/npm/d3@7/+esm";

// set the dimensions and margins of the graph
const margin = {top: 20, right: 20, bottom: 30, left: 40},
        width = 615 - margin.left - margin.right,
        height = 375 - margin.top - margin.bottom;

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
d3.csv("/nodes").then( function(data) {

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

// Tooltip stuff
const tooltip = d3.select("body").append("div")
    .attr("id", "tooltip")
    .style("position", "absolute")
    .style("opacity", "0")
    .style("background", "#4b3d5dff")
    .style("border", "1px solid #ccc")
    .style("padding", "6px")
    .style("font-size", "12px")
    .style("border-radius", "4px")
    .style("pointer-events", "none")
    .style("color", "#ffffff")
    .style("transition", "opacity 0.1s ease-out");

// Add dots
svg.append('g')
    .selectAll("dot")
    .data(data)
    .join("circle")
        .attr("class", "circle")
        .attr("cx", function (d) { return x(d.x); } )
        .attr("cy", function (d) { return y(d.y); } )
        .attr("r", 5)
        .style("fill", "#b36969ff")
        .on("mouseover", function (event, d) {
        tooltip
            .style("opacity", "1")
            .html(`Name: ${d.nickname}<br>Mac Address: ${d.mac}<br>(${d.x}, ${d.y})`);  // customize this content!
        d3.select(this)
            .transition()
            .duration(150)
            .ease(d3.easeCubicOut)
            .attr("r", 7); // grow circle
        })
        .on("mousemove", function (event) {
            tooltip
                .style("top", (event.pageY + 10) + "px")
                .style("left", (event.pageX + 10) + "px");
            
        })
        .on("mouseout", function () {
            tooltip.style("opacity", "0");
            d3.select(this)
            .transition()
            .ease(d3.easeCubicOut)
            .duration(150)
            .attr("r", 5); // grow circle
        });

})


