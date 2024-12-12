from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import socket
from threading import Thread

app = Flask(__name__)
socketio = SocketIO(app)

UDP_IP = "0.0.0.0"
UDP_PORT = 5000
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

def udp_listener():
    while True:
        data, addr = sock.recvfrom(1024)
        data = data.decode('utf-8').strip()
        print(f"{data}")
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

udp_thread = Thread(target=udp_listener)
udp_thread.start()

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
