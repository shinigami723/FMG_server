from flask import Flask, request, jsonify
from flask_socketio import SocketIO, emit
import os
from datetime import datetime
from queue import Queue
from threading import Thread
import logging

app = Flask(__name__)
socketio = SocketIO(app)

# Disable Flask's default logging of HTTP requests
log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)

# Ensure the directory for the text file exists
if not os.path.exists("data"):
    os.makedirs("data")

# Path to the text file
filename = input("Enter the file name: ")
txt_file_path = f"data/{filename}.txt"

# Queue for batch writing
write_queue = Queue()

def write_to_file():
    while True:
        data = write_queue.get()
        if data is None:
            break
        with open(txt_file_path, mode='a') as file:
            file.write(data)
        write_queue.task_done()

# Start a thread for writing to the file
writer_thread = Thread(target=write_to_file)
writer_thread.start()

@app.route('/sensor', methods=['POST'])
def sensor_data():
    try:
        data = request.json

        # Extract numeric values and format them
        numeric_values = (f"{data['counter']}, {data['AIN0']}, {data['AIN1']}, {data['AIN2']}, {data['AIN3']}, {data['AIN4']}, {data['AIN5']}, {data['AIN6']}, {data['AIN7']}, "
                          f"{data['Acceleration_x']}, {data['Acceleration_y']}, {data['Acceleration_z']}, "
                          f"{data['Rotation_x']}, {data['Rotation_y']}, {data['Rotation_z']}")

        # Display the numeric values on the terminal
        print(f"{numeric_values}")

        # Append data to text file
        data_to_write = numeric_values + "\n"
        
        # Add data to the queue
        write_queue.put(data_to_write)
        
        return jsonify({"status": "success", "data": data}), 200
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    try:
        socketio.run(app, host='0.0.0.0', port=5000)
    finally:
        write_queue.put(None)
        writer_thread.join()
