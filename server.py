from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO, emit
import os
import time
from datetime import datetime
import logging

app = Flask(__name__)
socketio = SocketIO(app)

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# Ensure the directory for the text file exists
if not os.path.exists("data"):
    os.makedirs("data")

# Path to the text file
filename = input("Enter the file name: ")
txt_file_path = f"data/{filename}.txt"

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/sensor', methods=['POST'])
def sensor_data():
    try:
        data = request.json
        logging.info(f"Received data: {data}")

        # Check if all required fields are present
        required_fields = ["sensor_read_time", "data_send_time", "AIN0", "Acceleration_x", "Acceleration_y", "Acceleration_z", "Rotation_x", "Rotation_y", "Rotation_z"]
        for field in required_fields:
            if field not in data:
                raise ValueError(f"Missing field: {field}")

        # Timestamp for data receive time
        receive_time = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]

        # Append data to text file
        with open(txt_file_path, mode='a') as file:
            file.write(f"{data['sensor_read_time']}, {data['data_send_time']}, {receive_time}, "
                       f"{data['AIN0']}, {data['Acceleration_x']}, {data['Acceleration_y']}, {data['Acceleration_z']}, "
                       f"{data['Rotation_x']}, {data['Rotation_y']}, {data['Rotation_z']}\n")

        # Prepare data for web client
        data['data_receive_time'] = receive_time

        # Send data to the web client
        socketio.emit('sensor_update', data, broadcast=True)

        return jsonify({"status": "success", "data": data}), 200
    except Exception as e:
        logging.error(f"Error: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
