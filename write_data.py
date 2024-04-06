'''
the IPC mechanism adopted was TCP sockets, 
specifically using QTcpSocket in the C++ QT client and the standard socket library in Python. 
TCP sockets are a core part of the Internet Protocol Suite, 
allowing for reliable, ordered, and error-checked delivery of a stream of bytes 
between applications running on hosts communicating via an IP network. 
This method is widely used for client-server communication where data is exchanged over a network.
'''
import capnp
import car_data_capnp
import socket
import time
import random
import struct
from threading import Thread

# Ensure proper word alignment for Cap'n Proto messages
def pad_to_word_boundary(data):
    # Cap'n Proto expects 64-bit word alignment
    WORD_SIZE = 8
    padding_needed = (-len(data)) % WORD_SIZE
    return data + b'\x00' * padding_needed

# Send serialized data over the network socket
def send_serialized_data(client_socket):
    while True:
        # Create sample data with random speed and yaw rate
        speed = random.uniform(0, 100)  # Example speed value
        yaw_rate = random.uniform(-5, 5)  # Example yaw rate value

        # Serialize data using Cap'n Proto schema
        car_data = car_data_capnp.CarData.new_message(speed=speed, yawRate=yaw_rate)
        serialized = car_data.to_bytes()

        # Pad data to maintain word boundary alignment
        padded_serialized = pad_to_word_boundary(serialized)
        
        # Debug: Log the original and padded serialized data
        print(f"Original Serialized Data (Binary): {serialized}")
        print(f"Padded Serialized Data (Binary): {padded_serialized}")
        
        # Pack the size of the padded serialized data as a 4-byte integer
        packed_size = struct.pack('I', len(padded_serialized))

        # Debug: Log the packed size
        print(f"Packed Size (Binary): {packed_size}")

        # Send the size of the padded serialized data first
        client_socket.sendall(packed_size)

        # Then send the actual padded serialized data
        client_socket.sendall(padded_serialized)

        print(f"Sent speed: {speed}, yaw rate: {yaw_rate}")

        # Interval between data transmissions
        time.sleep(2)  # Wait for 2 seconds before sending new data

# Handle incoming client connections
def handle_client_connection(client_socket):
    try:
        send_serialized_data(client_socket)
    except Exception as e:
        print(f"An exception occurred: {e}")
    finally:
        print("Closing client socket")
        client_socket.close()

# Start the server to listen for connections
def start_server():
    host = 'localhost'
    port = 12345

    # Set up the server socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((host, port))
    server_socket.listen(1)

    print(f"Server listening on {host}:{port}")

    # Accept connections and handle them in separate threads
    try:
        while True:
            client_socket, address = server_socket.accept()
            print(f"Accepted connection from {address[0]}:{address[1]}")
            client_handler = Thread(target=handle_client_connection, args=(client_socket,))
            client_handler.start()
    finally:
        server_socket.close()

if __name__ == "__main__":
    start_server()
