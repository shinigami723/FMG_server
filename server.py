from flask import Flask, request, jsonify
from flask_socketio import SocketIO, emit
import os
from datetime import datetime
from queue import Queue
from threading import Thread
import logging
import socket

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
        try:
            with open(txt_file_path, mode='a') as file:
                file.write(data)
        except IOError as e:
            print(f"Error writing to file: {e}")
        write_queue.task_done()

# Start a thread for writing to the file
writer_thread = Thread(target=write_to_file)
writer_thread.start()

# UDP server setup
UDP_IP = "0.0.0.0"
UDP_PORT = 5000
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

def udp_listener():
    while True:
        data, addr = sock.recvfrom(1024)
        data = data.decode('utf-8')

        # Display the numeric values on the terminal
        print(f"{data}")

        # Append data to text file
        data_to_write = data + "\n"
        
        # Add data to the queue
        write_queue.put(data_to_write)
        
# Start the UDP listener thread
udp_thread = Thread(target=udp_listener)
udp_thread.start()

if __name__ == '__main__':
    try:
        socketio.run(app, host='0.0.0.0', port=5000)
    finally:
        write_queue.put(None)
        writer_thread.join()
