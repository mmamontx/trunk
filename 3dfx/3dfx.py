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
    align_amp: float
    acce_x: int
    acce_y: int
    acce_z: int
    gyro_x: int
    gyro_y: int
    gyro_z: int
    rpm: float
    loop_time_ms: int

UDP_IP = "0.0.0.0"
UDP_PORT = 0x3d3d

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening {UDP_IP}:{UDP_PORT}...")

packet_format = '<BffffffffffffI'
packet_size = struct.calcsize(packet_format)

file = open("3dfx.csv", "w", encoding="utf-8")
print("X roll,Y pitch,Z yaw", file=file)

dev1 = Hyro(1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0)
dev2 = Hyro(2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0)

count = 0
try:
    while True:
        data, addr = sock.recvfrom(1024)
        if len(data) == packet_size:
            device_id, temp, x_roll, y_pitch, z_yaw, align_amp, acce_x, acce_y, acce_z, gyro_x, gyro_y, gyro_z, rpm, loop_time = struct.unpack(packet_format, data)
            if (device_id == 1):
                dev1.device_id = device_id
                dev1.temp = temp
                dev1.x_roll = x_roll
                dev1.y_pitch = y_pitch
                dev1.z_yaw = z_yaw
                dev1.align_amp = align_amp
                dev1.acce_x = acce_x
                dev1.acce_y = acce_y
                dev1.acce_z = acce_z
                dev1.gyro_x = gyro_x
                dev1.gyro_y = gyro_y
                dev1.gyro_z = gyro_z

            if (device_id == 2):
                dev2.device_id = device_id
                dev2.temp = temp
                dev2.x_roll = x_roll
                dev2.y_pitch = y_pitch
                dev2.z_yaw = z_yaw
                dev2.align_amp = align_amp
                dev2.acce_x = acce_x
                dev2.acce_y = acce_y
                dev2.acce_z = acce_z
                dev2.gyro_x = gyro_x
                dev2.gyro_y = gyro_y
                dev2.gyro_z = gyro_z

            print("\033[H\033[2J", end="") # Clear screen
            #print(f"Loop time     = {loop_time:7d}ms")
            print(f"RPM           = {rpm:7.2f}")

            print("") # Newline
            print(f"Device ID     = {dev2.device_id:6d}L   {dev1.device_id:6d}R")
            #print(f"Temperature   = {dev2.temp:7.0f}°C   {dev1.temp:6.0f}°C")

            print("") # Newline
            print(f"X gyro        = {dev2.gyro_x:7.2f}°/s {dev1.gyro_x:7.2f}°/s")
            print(f"Y gyro        = {dev2.gyro_y:7.2f}°/s {dev1.gyro_y:7.2f}°/s")
            print(f"Z gyro        = {dev2.gyro_z:7.2f}°/s {dev1.gyro_z:7.2f}°/s")

            print("") # Newline
            print(f"X acce        = {dev2.acce_x:7.2f}g   {dev1.acce_x:7.2f}g")
            print(f"Y acce        = {dev2.acce_y:7.2f}g   {dev1.acce_y:7.2f}g")
            print(f"Z acce        = {dev2.acce_z:7.2f}g   {dev1.acce_z:7.2f}g")

            print("") # Newline
            #print(f"X roll        = {dev2.x_roll:7.2f}°   {dev1.x_roll:7.2f}°")
            #print(f"Y pitch       = {dev2.y_pitch:7.2f}°   {dev1.y_pitch:7.2f}°")
            print(f"Z yaw         = {dev2.z_yaw:7.2f}°   {dev1.z_yaw:7.2f}°")

            print("") # Newline
            print(f"Align amp     = {dev2.align_amp:7.2f}°   {dev1.align_amp:7.2f}°")
            if (dev2.align_amp != 0.0) and (dev2.align_amp != 0.0):
                align_amp_sum = dev1.align_amp - dev2.align_amp
                print("") # Newline
                if (dev2.align_amp > 0):
                    print("2L << left (outward) <<        ", end="")
                else:
                    print("2L >> right (inward) >>        ", end="")
                if (dev1.align_amp > 0):
                    print("1R << left (inward) <<")
                else:
                    print("1R >> right (outward) >>")

                print("") # Newline
                print(f"Align amp dt  = {align_amp_sum:7.2f}°")

                print(f"{dev2.align_amp:.2f},{dev1.align_amp:.2f},{align_amp_sum:.2f}", file=file)
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

