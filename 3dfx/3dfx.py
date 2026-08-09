#!/usr/bin/python3

import socket
import struct

from dataclasses import dataclass

@dataclass
class Hyro:
    device_id: int
    temp: float
    x_roll: float
    y_pitch: float
    z_yaw: float
    base_x: float
    base_y: float
    base_z: float

UDP_IP = "0.0.0.0"
UDP_PORT = 0x3d3d

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening {UDP_IP}:{UDP_PORT}...")

packet_format = '<Bffff'
packet_size = struct.calcsize(packet_format)

file = open("3dfx.csv", "w", encoding="utf-8")
print("X roll,Y pitch,Z yaw", file=file)

dev1 = Hyro(1, 0.0, 0.0, 0.0, 0.0, 361.0, 361.0, 361.0)
dev2 = Hyro(2, 0.0, 0.0, 0.0, 0.0, 361.0, 361.0, 361.0)

count = 0
try:
    while True:
        data, addr = sock.recvfrom(1024)
        if len(data) == packet_size:
            device_id, temp, x_roll, y_pitch, z_yaw = struct.unpack(packet_format, data)
            if (device_id == 1):
                dev1.device_id = device_id
                dev1.temp = temp
                dev1.x_roll = x_roll
                dev1.y_pitch = y_pitch
                dev1.z_yaw = z_yaw

                if (dev1.base_x == 361.0):
                    dev1.base_x = x_roll
                    dev1.base_y = y_pitch
                    dev1.base_z = z_yaw

            if (device_id == 2):
                dev2.device_id = device_id
                dev2.temp = temp
                dev2.x_roll = x_roll
                dev2.y_pitch = y_pitch
                dev2.z_yaw = z_yaw

                if (dev2.base_x == 361.0):
                    dev2.base_x = x_roll
                    dev2.base_y = y_pitch
                    dev2.base_z = z_yaw

            print("\033[H\033[2J", end=""); # Clear screen
            print(f"Device ID     = {dev1.device_id:7d}  {dev2.device_id:7d}")
            print(f"Temperature   = {dev1.temp:7.0f}°C {dev2.temp:6.0f}°C")

            print("") # Newline
            print(f"X roll        = {dev1.x_roll:7.2f}° {dev2.x_roll:7.2f}°")
            print(f"Y pitch       = {dev1.y_pitch:7.2f}° {dev2.y_pitch:7.2f}°")
            print(f"Z yaw         = {dev1.z_yaw:7.2f}° {dev2.z_yaw:7.2f}°");

            if (dev1.base_x != 361.0) and (dev2.base_x != 361.0):
                dev1.offset_x = dev1.base_x - dev1.x_roll
                dev1.offset_y = dev1.base_y - dev1.y_pitch
                dev1.offset_z = dev1.base_z - dev1.z_yaw

                dev2.offset_x = dev2.base_x - dev2.x_roll
                dev2.offset_y = dev2.base_y - dev2.y_pitch
                dev2.offset_z = dev2.base_z - dev2.z_yaw

                print("") # Newline
                print(f"Base X offset = {dev1.offset_x:7.2f}° {dev2.offset_x:7.2f}°")
                print(f"Base Y offset = {dev1.offset_y:7.2f}° {dev2.offset_y:7.2f}°")
                print(f"Base Z offset = {dev1.offset_z:7.2f}° {dev2.offset_z:7.2f}°");

                delta_x = dev1.offset_x - dev2.offset_x
                delta_y = dev1.offset_y - dev2.offset_y
                delta_z = dev1.offset_z - dev2.offset_z
                print("") # Newline
                print(f"X delta       = {delta_x:7.2f}°");
                print(f"Y delta       = {delta_y:7.2f}°");
                print(f"Z delta       = {delta_z:7.2f}°");

                print(f"{delta_x:.2f},{delta_y:.2f},{delta_z:.2f}", file=file)
                count = count + 1
                print("") # Newline
                print(f"{file.name}:{count}")
        else:
            print(f"Received a corrupted packet from {addr[0]}.")
except KeyboardInterrupt:
    print("KeyboardInterrupt")
finally:
    sock.close()
    file.close()

