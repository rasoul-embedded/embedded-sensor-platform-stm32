#!/usr/bin/env python3
"""
Live 3D IMU orientation viewer.

Reads roll and pitch from STM32 UART CSV and displays a 3D board.

Expected UART format:

START
counter,roll,pitch
0,123,-45
1,124,-44

Angles are scaled by 100:
    123 -> 1.23 degrees

Also tolerates direct numeric data if the script starts after START:
    1250,123,-45
    1251,124,-44
"""

import argparse
import time
from collections import deque

import numpy as np
import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d.art3d import Poly3DCollection


SCALE = 100.0


def parse_args():
    parser = argparse.ArgumentParser(description="Live 3D roll/pitch viewer from STM32 UART")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="Serial port")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument("--debug", action="store_true", help="Print received UART lines")
    return parser.parse_args()


class OrientationSerialReader:
    def __init__(self, port, baud, debug=False):
        self.ser = serial.Serial(port=port, baudrate=baud, timeout=0.02)
        self.debug = debug

        self.started = False
        self.header = None

        self.counter_index = 0
        self.roll_index = 1
        self.pitch_index = 2

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    def _read_line(self):
        raw = self.ser.readline()

        if not raw:
            return None

        try:
            line = raw.decode("utf-8", errors="ignore").strip()
        except UnicodeDecodeError:
            return None

        if self.debug:
            print("RX:", line)

        return line

    def _process_header(self, line):
        fields = [x.strip() for x in line.split(",")]
        self.header = fields

        if "counter" in fields:
            self.counter_index = fields.index("counter")
        else:
            self.counter_index = 0

        if "roll" in fields and "pitch" in fields:
            self.roll_index = fields.index("roll")
            self.pitch_index = fields.index("pitch")
            return True

        if "roll_comp" in fields and "pitch_comp" in fields:
            self.roll_index = fields.index("roll_comp")
            self.pitch_index = fields.index("pitch_comp")
            return True

        if "roll_kalman" in fields and "pitch_kalman" in fields:
            self.roll_index = fields.index("roll_kalman")
            self.pitch_index = fields.index("pitch_kalman")
            return True

        return False

    def read_sample(self):
        """
        Return:
            counter, roll_deg, pitch_deg

        or:
            None
        """

        line = self._read_line()

        if line is None or line == "":
            return None

        if line == "START":
            self.started = True
            self.header = None
            return None

        if "counter" in line and "," in line:
            self.started = True
            self._process_header(line)
            return None

        parts = [x.strip() for x in line.split(",")]

        if self.header is None:
            self.started = True
            self.counter_index = 0
            self.roll_index = 1
            self.pitch_index = 2

        if len(parts) <= max(self.counter_index, self.roll_index, self.pitch_index):
            return None

        try:
            counter = int(parts[self.counter_index])
            roll_deg = int(parts[self.roll_index]) / SCALE
            pitch_deg = int(parts[self.pitch_index]) / SCALE
        except ValueError:
            return None

        return counter, roll_deg, pitch_deg


def rotation_matrix_roll_pitch(roll_deg, pitch_deg):
    """
    Create rotation matrix from roll and pitch.

    Coordinate convention used for visualization:

    x-axis: board length direction
    y-axis: board width direction
    z-axis: upward

    roll:
        rotation around x-axis

    pitch:
        rotation around y-axis
    """

    roll = np.deg2rad(roll_deg)
    pitch = np.deg2rad(pitch_deg)

    Rx = np.array([
        [1.0, 0.0, 0.0],
        [0.0, np.cos(roll), -np.sin(roll)],
        [0.0, np.sin(roll),  np.cos(roll)]
    ])

    Ry = np.array([
        [ np.cos(pitch), 0.0, np.sin(pitch)],
        [0.0,            1.0, 0.0],
        [-np.sin(pitch), 0.0, np.cos(pitch)]
    ])

    return Ry @ Rx


def create_board_vertices():
    """
    Create a simple rectangular board centered at origin.
    """

    length = 2.4
    width = 1.4
    thickness = 0.08

    x = length / 2.0
    y = width / 2.0
    z = thickness / 2.0

    vertices = np.array([
        [-x, -y, -z],
        [ x, -y, -z],
        [ x,  y, -z],
        [-x,  y, -z],
        [-x, -y,  z],
        [ x, -y,  z],
        [ x,  y,  z],
        [-x,  y,  z],
    ])

    faces = [
        [0, 1, 2, 3],  # bottom
        [4, 5, 6, 7],  # top
        [0, 1, 5, 4],  # front
        [1, 2, 6, 5],  # right
        [2, 3, 7, 6],  # back
        [3, 0, 4, 7],  # left
    ]

    return vertices, faces


def transform_vertices(vertices, roll_deg, pitch_deg):
    R = rotation_matrix_roll_pitch(roll_deg, pitch_deg)
    return vertices @ R.T


def make_faces(vertices, faces):
    return [[vertices[index] for index in face] for face in faces]


def main():
    args = parse_args()

    reader = OrientationSerialReader(args.port, args.baud, args.debug)

    base_vertices, face_indices = create_board_vertices()

    roll_history = deque(maxlen=500)
    pitch_history = deque(maxlen=500)
    time_history = deque(maxlen=500)

    latest_roll = 0.0
    latest_pitch = 0.0
    start_time = time.time()

    fig = plt.figure(figsize=(12, 7))

    ax3d = fig.add_subplot(1, 2, 1, projection="3d")
    ax_plot = fig.add_subplot(1, 2, 2)

    ax3d.set_title("Live 3D IMU Orientation")
    ax3d.set_xlim(-2, 2)
    ax3d.set_ylim(-2, 2)
    ax3d.set_zlim(-2, 2)
    ax3d.set_xlabel("X")
    ax3d.set_ylabel("Y")
    ax3d.set_zlabel("Z")

    ax3d.view_init(elev=25, azim=35)

    transformed = transform_vertices(base_vertices, latest_roll, latest_pitch)
    board_faces = make_faces(transformed, face_indices)

    board = Poly3DCollection(
        board_faces,
        alpha=0.75,
        edgecolor="black",
        linewidths=1.0
    )
    ax3d.add_collection3d(board)

    # Coordinate axes attached to board
    x_axis_line, = ax3d.plot([], [], [], linewidth=2, label="Board X")
    y_axis_line, = ax3d.plot([], [], [], linewidth=2, label="Board Y")
    z_axis_line, = ax3d.plot([], [], [], linewidth=2, label="Board Z")

    text_info = ax3d.text2D(
        0.05,
        0.95,
        "Roll: 0.00 deg\nPitch: 0.00 deg",
        transform=ax3d.transAxes
    )

    ax_plot.set_title("Roll / Pitch")
    ax_plot.set_xlabel("Time [s]")
    ax_plot.set_ylabel("Angle [deg]")
    ax_plot.grid(True)

    roll_line, = ax_plot.plot([], [], label="Roll")
    pitch_line, = ax_plot.plot([], [], label="Pitch")
    ax_plot.legend(loc="upper right")

    def update(_frame):
        nonlocal latest_roll, latest_pitch

        # Read several samples per animation frame
        for _ in range(30):
            sample = reader.read_sample()

            if sample is None:
                continue

            _counter, roll_deg, pitch_deg = sample

            latest_roll = roll_deg
            latest_pitch = pitch_deg

            t = time.time() - start_time

            time_history.append(t)
            roll_history.append(roll_deg)
            pitch_history.append(pitch_deg)

        # Update 3D board
        transformed_vertices = transform_vertices(base_vertices, latest_roll, latest_pitch)
        board.set_verts(make_faces(transformed_vertices, face_indices))

        # Update board coordinate axes
        R = rotation_matrix_roll_pitch(latest_roll, latest_pitch)

        origin = np.array([0.0, 0.0, 0.0])
        x_end = R @ np.array([1.5, 0.0, 0.0])
        y_end = R @ np.array([0.0, 1.5, 0.0])
        z_end = R @ np.array([0.0, 0.0, 1.5])

        x_axis_line.set_data([origin[0], x_end[0]], [origin[1], x_end[1]])
        x_axis_line.set_3d_properties([origin[2], x_end[2]])

        y_axis_line.set_data([origin[0], y_end[0]], [origin[1], y_end[1]])
        y_axis_line.set_3d_properties([origin[2], y_end[2]])

        z_axis_line.set_data([origin[0], z_end[0]], [origin[1], z_end[1]])
        z_axis_line.set_3d_properties([origin[2], z_end[2]])

        text_info.set_text(
            f"Roll: {latest_roll:.2f} deg\nPitch: {latest_pitch:.2f} deg"
        )

        # Update 2D plot
        if len(time_history) > 2:
            roll_line.set_data(time_history, roll_history)
            pitch_line.set_data(time_history, pitch_history)

            t_max = time_history[-1]
            t_min = max(0.0, t_max - 10.0)

            ax_plot.set_xlim(t_min, t_max)

            y_min = min(min(roll_history), min(pitch_history)) - 5.0
            y_max = max(max(roll_history), max(pitch_history)) + 5.0

            if y_min == y_max:
                y_min -= 1.0
                y_max += 1.0

            ax_plot.set_ylim(y_min, y_max)

        return (
            board,
            x_axis_line,
            y_axis_line,
            z_axis_line,
            text_info,
            roll_line,
            pitch_line,
        )

    ani = FuncAnimation(
        fig,
        update,
        interval=50,
        blit=False,
        cache_frame_data=False
    )

    try:
        plt.tight_layout()
        plt.show()
    finally:
        reader.close()


if __name__ == "__main__":
    main()