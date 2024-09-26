#  a test file to test sending notifications to the control center to register a new node with coordinates

from flask import Flask, render_template, jsonify, request
import threading
import time

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('notify.html')

@app.route('/notify')
def notify():
    return jsonify({"message": "Please submit your data!"})

@app.route('/submit', methods=['POST'])
def submit():
    data = request.form.get('data')
    print(f"Received data: {data}")
    return jsonify({"message": "Data submitted successfully!"})

def send_notifications():
    while True:
        time.sleep(10)  # Adjust the time as needed
        print("Notification sent!")  # Log to console for this example

threading.Thread(target=send_notifications, daemon=True).start()

if __name__ == '__main__':
    app.run(debug=True)
