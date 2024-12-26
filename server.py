from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import socket
from threading import Thread, Event
import os
import time

app = Flask(__name__)
socketio = SocketIO(app)

UDP_IP = "0.0.0.0"
UDP_PORT = 5000  # Use a different port for UDP to avoid conflicts
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

# Event to signal thread shutdown
stop_event = Event()

# File setup
if not os.path.exists("data"):
    os.makedirs("data")

filename = f"data/udp_data_{int(time.time())}.txt"

def udp_listener():
    with open(filename, 'a') as file:  # Open file in append mode
        while not stop_event.is_set():
            try:
                data, addr = sock.recvfrom(1024)
                data = data.decode('utf-8').strip()
                print(data)

                # Save raw data to file
                file.write(data + "\n")

                values = data.split(',')
                
                # Ensure we have the correct number of values
                if len(values) == 15:
                    counter = int(values[0])
                    adc_values = [float(values[i]) for i in range(1, 9)]
                    accel_values = [float(values[i]) for i in range(9, 12)]
                    rotation_values = [float(values[i]) for i in range(12, 15)]
                    
                    # Emit categorized data
                    socketio.emit('new_data', {
                        'counter': counter,
                        'adc': adc_values,
                        'acceleration': accel_values,
                        'rotation': rotation_values
                    })
            except Exception as e:
                print(f"Error in UDP listener: {e}")

udp_thread = Thread(target=udp_listener, daemon=True)
udp_thread.start()

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    try:
        socketio.run(app, host='0.0.0.0', port=5000)
    except KeyboardInterrupt:
        print("Shutting down server.")
    finally:
        stop_event.set()
        udp_thread.join()
