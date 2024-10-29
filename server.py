from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO, emit
import csv
import os
import time
from datetime import datetime

app = Flask(__name__)
socketio = SocketIO(app)

# Ensure the directory for the CSV file exists
if not os.path.exists("data"):
    os.makedirs("data")

# Path to the CSV file
filename = input("Enter the file name: ")
csv_file_path = f"data/{filename}.csv"

# Write header if file doesn't exist
if not os.path.isfile(csv_file_path):
    with open(csv_file_path, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["sensor_read_time", "data_send_time", "data_receive_time", "ADC_0", "Acceleration_x", "Acceleration_y", "Acceleration_z", "Rotation_x", "Rotation_y", "Rotation_z"])  # Header

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/sensor', methods=['POST'])
def sensor_data():
    try:
        data = request.json
        print(f"Received data: {data}")

        # Check if all required fields are present
        required_fields = ["sensor_read_time", "data_send_time", "AIN0", "Acceleration_x", "Acceleration_y", "Acceleration_z", "Rotation_x", "Rotation_y", "Rotation_z"]
        for field in required_fields:
            if field not in data:
                raise ValueError(f"Missing field: {field}")

        # Timestamp for data receive time
        receive_time = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]

        # Append data to CSV file
        with open(csv_file_path, mode='a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([data['sensor_read_time'], data['data_send_time'], receive_time, 
                             data['AIN0'], data['Acceleration_x'], data['Acceleration_y'], data['Acceleration_z'], 
                             data['Rotation_x'], data['Rotation_y'], data['Rotation_z']])

        # Prepare data for web client
        data['data_receive_time'] = receive_time

        # Send data to the web client
        socketio.emit('sensor_update', data, broadcast=True)

        return jsonify({"status": "success", "data": data}), 200
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
