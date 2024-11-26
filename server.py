from flask import Flask, request, jsonify
from flask_socketio import SocketIO, emit
import os
import logging
from datetime import datetime
from marshmallow import Schema, fields, ValidationError
from queue import Queue
from threading import Thread

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

# Schema for sensor data
class SensorDataSchema(Schema):
    sensor_read_time = fields.DateTime(required=True)
    data_send_time = fields.DateTime(required=True)
    AIN0 = fields.Float(required=True)
    AIN1 = fields.Float(required=True)
    AIN2 = fields.Float(required=True)
    AIN3 = fields.Float(required=True)
    AIN4 = fields.Float(required=True)
    AIN5 = fields.Float(required=True)
    AIN6 = fields.Float(required=True)
    AIN7 = fields.Float(required=True)
    Acceleration_x = fields.Float(required=True)
    Acceleration_y = fields.Float(required=True)
    Acceleration_z = fields.Float(required=True)
    Rotation_x = fields.Float(required=True)
    Rotation_y = fields.Float(required=True)
    Rotation_z = fields.Float(required=True)

# Instance for Schema class
sensor_data_schema = SensorDataSchema()

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
        logging.info(f"Received data: {data}")
        
        # Data validation
        validated_data = sensor_data_schema.load(data)

        # Append data to text file
        data_to_write = (f"{validated_data['sensor_read_time']}, {validated_data['data_send_time']}, "
                       f"{validated_data['AIN0']}, {validated_data['AIN1']}, {validated_data['AIN2']}, {validated_data['AIN3']}, {validated_data['AIN4']}, {validated_data['AIN5']}, {validated_data['AIN6']}, {validated_data['AIN7']}, "
                       f"{validated_data['Acceleration_x']}, {validated_data['Acceleration_y']}, {validated_data['Acceleration_z']}, "
                       f"{validated_data['Rotation_x']}, {validated_data['Rotation_y']}, {validated_data['Rotation_z']}\n")

        # Add data to the queue
        write_queue.put(data_to_write)

        # Send data to the web client
        socketio.emit('sensor_update', validated_data, namespace='/sensor')

        return jsonify({"status": "success", "data": validated_data}), 200
    except ValidationError as err:
        return jsonify({"status": "error", "message": err.messages}), 400
    except Exception as e:
        logging.exception("An error occurred while processing the request")
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    try:
        socketio.run(app, host='0.0.0.0', port=5000)
    finally:
        write_queue.put(None)
        writer_thread.join()
