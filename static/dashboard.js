const checkUrl = '/check'; // URL to check for data
const submitUrl = '/register_final'; // URL to send form data

/*
function fetchData() {
    fetch(checkUrl)
        .then(response => response.json())
        .then(data => {
            if (data && data.title) {
                addFormIfUnique(data.title);
            }
        })
        .catch(error => console.error('Error fetching data:', error));
}
*/

function fetchData() {
    fetch(checkUrl)
        .then(response => response.json())
        .then(data => {
            if (Array.isArray(data)) {
                data.forEach(item => {
                    if (typeof item === 'string') {
                        addFormIfUnique(item);
                    }
                });
            } else {
                console.error('Expected an array but received:', data);
            }
        })
        .catch(error => console.error('Error fetching data:', error));
}

function addFormIfUnique(title) {
    const container = document.getElementById('add-nodes-container');

    // Check if the title already exists
    const existingForms = container.getElementsByTagName('h2');
    for (let i = 0; i < existingForms.length; i++) {
        if (existingForms[i].textContent === title) {
            return; // Title already exists, do nothing
        }
    }

    // Create the new form
    const form = document.createElement('form');
    const titleElement = document.createElement('h2');
    const inputElement = document.createElement('input');
    const submitButton = document.createElement('button');

    titleElement.textContent = title;
    inputElement.type = 'text';
    inputElement.placeholder = 'Enter your data here';
    submitButton.textContent = 'Submit';

    // Set the ID for the form based on the title
    form.id = "form_id_"+title.toLowerCase();

    form.appendChild(titleElement);
    form.appendChild(inputElement);
    form.appendChild(submitButton);
    
    form.addEventListener('submit', function(event) {
        event.preventDefault();
        const inputData = inputElement.value;

        // Send input data to another URL
        fetch(submitUrl, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ data: inputData, id: title.toLowerCase()})
        })
        .then(response => response.json())
        .then(result => {
            console.log('Data submitted successfully:', result);
            container.removeChild(form); // Remove the form from the page
        })
        .catch(error => console.error('Error submitting data:', error));

        form.remove();

    });

    container.appendChild(form);
}

setInterval(fetchData, 1000); // Check every 1 second


function toggleForm(formId) {
    const forms = document.querySelectorAll('.edit-form');
    forms.forEach(form => {
        form.classList.remove('active');
    });
    document.getElementById(formId).classList.toggle('active');
    document.querySelectorAll('.top-right').forEach(el => el.style.height = '70vh');
    document.querySelectorAll('.bottom-right').forEach(el => el.style.height = '30vh');

    document.getElementById(formId).style.height = '70vh'
}

let zoomLevel = 1;
const image = document.getElementById('dynamicImage');
const imageContainer = document.getElementById('imageContainer');
let isDragging = false;
let startX, startY, initialX, initialY;
let initialDistance = 0;

// Function to reload the image
function reloadImage() {
    const timestamp = new Date().getTime();
    image.src = `/static/images/image.png?timestamp=${timestamp}`;
}

// Calculate distance between two touch points
function getDistance(touches) {
    const dx = touches[0].clientX - touches[1].clientX;
    const dy = touches[0].clientY - touches[1].clientY;
    return Math.sqrt(dx * dx + dy * dy);
}

// Zoom with mouse wheel
imageContainer.addEventListener('wheel', (event) => {
    event.preventDefault();
    const scaleAmount = 0.1;
    const mouseX = event.offsetX / imageContainer.clientWidth;
    const mouseY = event.offsetY / imageContainer.clientHeight;

    if (event.deltaY < 0) {
        zoomLevel += scaleAmount; // Zoom in
    } else {
        zoomLevel -= scaleAmount; // Zoom out
    }

    zoomLevel = Math.max(1, zoomLevel); // Prevent zooming out too much
    image.style.transform = `scale(${zoomLevel}) translate(-${mouseX * (zoomLevel - 1)}px, -${mouseY * (zoomLevel - 1)}px)`;
});

// Panning functionality for mouse
const startDragging = (clientX, clientY) => {
    isDragging = true;
    startX = clientX;
    startY = clientY;
    initialX = image.offsetLeft;
    initialY = image.offsetTop;
    imageContainer.style.cursor = 'grabbing'; // Change cursor to indicate dragging
};

const dragImage = (clientX, clientY) => {
    if (isDragging) {
        const dx = clientX - startX;
        const dy = clientY - startY;
        image.style.left = `${initialX + dx}px`;
        image.style.top = `${initialY + dy}px`;
    }
};

const stopDragging = () => {
    isDragging = false;
    imageContainer.style.cursor = 'grab'; // Reset cursor
};

// Mouse event listeners
imageContainer.addEventListener('mousedown', (event) => {
    event.preventDefault(); // Prevent default dragging behavior
    startDragging(event.clientX, event.clientY);
});

imageContainer.addEventListener('mousemove', (event) => {
    event.preventDefault(); // Prevent default dragging behavior
    dragImage(event.clientX, event.clientY);
});

imageContainer.addEventListener('mouseup', stopDragging);
imageContainer.addEventListener('mouseleave', stopDragging);

// Touch event listeners
imageContainer.addEventListener('touchstart', (event) => {
    event.preventDefault(); // Prevent default dragging behavior
    if (event.touches.length === 1) {
        const touch = event.touches[0];
        startDragging(touch.clientX, touch.clientY);
    } else if (event.touches.length === 2) {
        initialDistance = getDistance(event.touches); // Get initial distance between two touches
    }
});

imageContainer.addEventListener('touchmove', (event) => {
    event.preventDefault(); // Prevent default dragging behavior
    if (event.touches.length === 1) {
        const touch = event.touches[0];
        dragImage(touch.clientX, touch.clientY);
    } else if (event.touches.length === 2) {
        const currentDistance = getDistance(event.touches);
        const scaleAmount = (currentDistance / initialDistance) - 1; // Calculate scale amount
        zoomLevel *= 1 + scaleAmount; // Update zoom level
        zoomLevel = Math.max(1, zoomLevel); // Prevent zooming out too much
        image.style.transform = `scale(${zoomLevel})`; // Apply zoom transformation

        // Update initial distance for next calculation
        initialDistance = currentDistance;
    }
});

imageContainer.addEventListener('touchend', (event) => {
    if (event.touches.length === 0) {
        stopDragging();
    }
});

// Reload the image at a set interval
setInterval(reloadImage, 1000);
