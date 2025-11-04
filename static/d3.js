import * as d3 from "https://cdn.jsdelivr.net/npm/d3@7/+esm";

const margin = {top: 20, right: 20, bottom: 30, left: 40};

const container = d3.select("#d3-graph");

var nodeColor = "#eb3131ff";
var kiwiBotColor = "#476cc1ff";
var tooltipColor = "#3a4e7de0";

var scaleX = 851; // Scale based on image in feet
var scaleY = 517;

// Ensure the container exists
if (container.empty()) {
    throw new Error("#d3-graph element not found");
}

// Use an existing SVG if the container is an <svg>, otherwise create one that fills the container
let svgRoot;
if (container.node().nodeName.toLowerCase() === "svg") {
    svgRoot = container;
    svgRoot.style("width", "100%").style("height", "100%");
} else {
    svgRoot = container.append("svg")
        .attr("id", "d3-graph-svg")
        .style("width", "100%")
        .style("height", "100%")
        .style("font-size", "20px");
}

// Group that will be transformed by zoom & where we draw content
let g = svgRoot.append("g").attr("class", "app-group");

// Make sure image/tooltips aren't duplicated if file is reloaded
d3.select("#tooltip").remove();
d3.select("#kiwitooltip").remove();

// Create tooltips (singletons)
const tooltip = d3.select("body").append("div")
    .attr("id", "tooltip")
    .style("position", "absolute")
    .style("opacity", "0")
    .style("background", tooltipColor)
    .style("border", "1px solid #ccc")
    .style("padding", "6px")
    .style("border-radius", "4px")
    .style("pointer-events", "none")
    .style("color", "#ffffff")
    .style("transition", "opacity 0.1s ease-out");

const kiwiTooltip = d3.select("body").append("div")
    .attr("id", "kiwitooltip")
    .style("position", "absolute")
    .style("opacity", "0")
    .style("background", tooltipColor)
    .style("border", "1px solid #ccc")
    .style("padding", "6px")
    .style("border-radius", "4px")
    .style("pointer-events", "none")
    .style("color", "#ffffff")
    .style("transition", "opacity 0.1s ease-out");

// Keep references to current drawn elements so we can redraw on resize
let xScale, yScale, innerWidth, innerHeight;

// Responsive render function
function render() {
    // measure container size
    const rect = container.node().getBoundingClientRect();
    const fullW = Math.max(200, rect.width);
    const fullH = Math.max(200, rect.height);

    // set viewBox to make svg content scale with container
    svgRoot.attr("viewBox", `0 0 ${fullW} ${fullH}`)
            .attr("preserveAspectRatio", "xMidYMid meet");

    // compute inner usable size
    innerWidth = Math.max(50, fullW - margin.left - margin.right);
    innerHeight = Math.max(50, fullH - margin.top - margin.bottom);

    // clear previous drawing group and recreate (keeps DOM simpler)
    svgRoot.selectAll(".app-group").remove();
    g = svgRoot.append("g").attr("class", "app-group").attr("transform", `translate(${margin.left},${margin.top})`);

    // append background image sized to inner drawing area so it always fits perfectly
    g.append("image")
        .attr("href", "/static/images/satellite851x517.png") // Current image spans about 851x517 feet
        .attr("x", 0)
        .attr("y", 0)
        .attr("width", innerWidth)
        .attr("height", innerHeight)
        .attr("preserveAspectRatio", "xMidYMid slice");

    // create scales based on inner dimensions
    xScale = d3.scaleLinear().domain([0, scaleX]).range([0, innerWidth]); 
    yScale = d3.scaleLinear().domain([0, scaleY]).range([innerHeight, 0]);

    // axes
    const xAxis = d3.axisBottom(xScale).ticks(Math.round(scaleX/100)).tickSizeOuter(0);
    const yAxis = d3.axisLeft(yScale).ticks(Math.round(scaleY/100)).tickSizeOuter(0);

    // axes groups (positioned relative to g)
    g.append("g")
        .attr("class", "x-axis")
        .attr("transform", `translate(0, ${innerHeight})`)
        .style("font-size", "20px")
        .call(xAxis);

    g.append("g")
        .attr("class", "y-axis")
        .style("font-size", "20px")
        .call(yAxis);

    // load and draw nodes and tracked data
    drawNodes();
    drawTracked();
}

// draw nodes (reads from /nodes CSV)
function drawNodes() {
    d3.csv("/nodes").then(function(data) {
        // parse numeric values gently
        data.forEach(d => {
            d.x = +d.x || 0;
            d.y = +d.y || 0;
            d.range = +d.range || 5;
        });

        // range circles
        const ranges = g.selectAll("circle.range-circle").data(data, d => d.mac || d.id || JSON.stringify(d));
        ranges.join(
            enter => enter.append("circle")
                .attr("class", "range-circle")
                .attr("cx", d => xScale(d.x))
                .attr("cy", d => yScale(d.y))
                .attr("r", d => 10)
                .style("fill", nodeColor)
                .style("stroke", nodeColor)
                .style("stroke-width", 1)
                .style("opacity", 0),
            update => update
                .transition().duration(200)
                .attr("cx", d => xScale(d.x))
                .attr("cy", d => yScale(d.y))
                .attr("r", d => 10),
            exit => exit.remove()
        );

        // node dots
        const dots = g.selectAll("circle.node-dot").data(data, d => d.mac || d.id || JSON.stringify(d));
        dots.join(
            enter => enter.append("circle")
                .attr("class", "node-dot")
                .attr("cx", d => xScale(d.x))
                .attr("cy", d => yScale(d.y))
                .attr("r", 10)
                .style("fill", nodeColor)
                .on("mouseover", function(event, d) {
                    tooltip.style("opacity", "1")
                        .html(`Name: ${d.nickname || "-"}<br>Mac: ${d.mac || d.id || "-"}<br>(${d.x}, ${d.y})<br>Range: ${d.range}ft`); // Show specific information for each node
                    d3.select(this).transition().duration(120).attr("r", 15);
                    // highlight matching range circle
                    g.selectAll("circle.range-circle")
                        .filter(cd => cd === d)
                        .transition().ease(d3.easeCubic).duration(200)
                        .attr("r", d.range)
                        .style("opacity", 0.5);
                })
                .on("mousemove", function(event) {
                    tooltip.style("top", (event.pageY + 10) + "px")
                            .style("left", (event.pageX + 10) + "px");
                })
                .on("mouseout", function(event, d) {
                    tooltip.style("opacity", "0");
                    d3.select(this).transition().duration(120).attr("r", 10);
                    g.selectAll("circle.range-circle")
                        .transition().ease(d3.easeCubic).duration(200)
                        .attr("r", 10)
                        .style("opacity", 0);
                }),
            update => update
                .transition().duration(200)
                .attr("cx", d => xScale(d.x))
                .attr("cy", d => yScale(d.y)),
            exit => exit.remove()
        );
    }).catch(err => {
        console.error("Failed to load /nodes CSV:", err);
    });
}

// draw tracked kiwibot data (reads from /data CSV)
function drawTracked() {
    d3.csv("/data").then(function(data) {
        data.forEach(d => {
            d.x = +d.x || 0;
            d.y = +d.y || 0;
        });

        const dots = g.selectAll("circle.kiwi-dot").data(data, d => d.mac || JSON.stringify(d));
        dots.join(
            enter => enter.append("circle")
                .attr("class", "kiwi-dot")
                .attr("cx", d => xScale(d.x))
                .attr("cy", d => yScale(d.y))
                .attr("r", 10)
                .style("fill", kiwiBotColor)
                .on("mouseover", function(event, d) {
                    kiwiTooltip.style("opacity", "1")
                        .html(`Mac Address: ${d.mac || "-"}<br>(${d.x}, ${d.y})`);
                    d3.select(this).transition().duration(120).attr("r", 15);
                })
                .on("mousemove", function(event) {
                    kiwiTooltip.style("top", (event.pageY + 10) + "px")
                            .style("left", (event.pageX + 10) + "px");
                })
                .on("mouseout", function() {
                    kiwiTooltip.style("opacity", "0");
                    d3.select(this).transition().duration(120).attr("r", 10);
                }),
            update => update
                .transition().duration(200)
                .attr("cx", d => xScale(d.x))
                .attr("cy", d => yScale(d.y)),
            exit => exit.remove()
        );
    }).catch(err => {
        console.error("Failed to load /data CSV:", err);
    });
}

// zoom behavior: zoom transforms the inner group (g)
const zoom = d3.zoom()
    .scaleExtent([0.5, 10])
    .on("zoom", (event) => {
        // apply transform to the drawing group (translate + scale)
        svgRoot.selectAll(".app-group").attr("transform", `translate(${margin.left},${margin.top}) ${event.transform}`);
    });

// Attach zoom to the SVG root so scroll/drag works
svgRoot.call(zoom);

// initial render
render();

// re-render on window resize (debounced)
let resizeTimer;
function handleResize() {
    clearTimeout(resizeTimer);
    resizeTimer = setTimeout(() => {
        render();
    }, 120);
}
window.addEventListener("resize", handleResize);

// expose a reload function for external UI controls
window.reloadD3 = function() {
    render();
};