"""
Basic server which demonstrates hosting an image which constantly reloads, creating a real-time feed
"""

from flask import Flask, render_template, jsonify, send_from_directory
import os
import random
import time

app = Flask(__name__)

# Directory to store images
IMAGE_DIR = 'static/images'

# Ensure the image directory exists
os.makedirs(IMAGE_DIR, exist_ok=True)

# A function to generate a random image name
def get_random_image_name():
    image_names = [f'image_{i}.png' for i in range(1, 6)]  # Example images
    return random.choice(image_names)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/image')
def image():
    image_name = get_random_image_name()
    return send_from_directory(IMAGE_DIR, image_name)

@app.route('/update')
def update():
    # Simulate new data by returning the current timestamp
    return jsonify({'timestamp': time.time()})

if __name__ == '__main__':
    app.run(debug=True)
