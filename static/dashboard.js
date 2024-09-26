const checkUrl = '/check'; // URL to check for data
const submitUrl = '/submit'; // URL to send form data

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
            body: JSON.stringify({ data: inputData }),
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
    const forms = document.querySelectorAll('.form');
    forms.forEach(form => {
        form.classList.remove('active');
    });
    document.getElementById(formId).classList.toggle('active');
}

function reloadImage() {
    const image = document.getElementById('dynamic-image');
    const timestamp = new Date().getTime(); // Current timestamp to avoid caching
    image.src = `/static/images/image.png?${timestamp}`;
}

setInterval(reloadImage, 1000); // Reload image every 1 second
