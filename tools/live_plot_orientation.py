#!/usr/bin/env python3
"""
Live IMU orientation plotter.

Reads CSV data from STM32 over UART and plots roll/pitch live.

Expected simple firmware format:

START
counter,roll,pitch
0,123,-45
1,124,-44
...

Angle values are assumed to be scaled by 100:
    123 -> 1.23 deg

Also supports headers such as:
    counter,roll_comp,roll_kalman,pitch_comp,pitch_kalman

In that case it uses:
    roll_comp  as roll
    pitch_comp as pitch
"""

import argparse
import math
import time
from collections import deque

import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.patches import Rectangle
from matplotlib.transforms import Affine2D


SCALE = 100.0


def parse_args():
    parser = argparse.ArgumentParser(description="Live roll/pitch plot from STM32 UART CSV")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="Serial port, default: /dev/ttyUSB0")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate, default: 115200")
    parser.add_argument("--window", type=float, default=10.0, help="Plot time window in seconds")
    parser.add_argument("--fs", type=float, default=100.0, help="Sampling frequency in Hz")
    return parser.parse_args()


class OrientationSerialReader:
    def __init__(self, port, baud):
        self.ser = serial.Serial(
            port=port,
            baudrate=baud,
            timeout=0.02,
        )

        self.header = None
        self.roll_index = None
        self.pitch_index = None
        self.counter_index = None
        self.started = False

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    def _decode_line(self):
        raw = self.ser.readline()

        if not raw:
            return None

        try:
            return raw.decode("utf-8", errors="ignore").strip()
        except UnicodeDecodeError:
            return None

    def _process_header(self, line):
        fields = [x.strip() for x in line.split(",")]
        self.header = fields

        if "counter" in fields:
            self.counter_index = fields.index("counter")
        else:
            self.counter_index = 0

        # Preferred simple demo format
        if "roll" in fields and "pitch" in fields:
            self.roll_index = fields.index("roll")
            self.pitch_index = fields.index("pitch")
            return True

        # Week 3 comparison format
        if "roll_comp" in fields and "pitch_comp" in fields:
            self.roll_index = fields.index("roll_comp")
            self.pitch_index = fields.index("pitch_comp")
            return True

        # Alternative output naming
        if "roll_out" in fields and "pitch_out" in fields:
            self.roll_index = fields.index("roll_out")
            self.pitch_index = fields.index("pitch_out")
            return True

        return False

    def read_sample(self):
        """
        Returns:
            (counter, roll_deg, pitch_deg)
        or:
            None if no valid sample is available.
        """

        line = self._decode_line()

        if line is None or line == "":
            return None

        if line == "START":
            self.started = True
            self.header = None
            self.roll_index = None
            self.pitch_index = None
            return None

        if not self.started:
            return None

        # First non-empty line after START should be the header
        if self.header is None:
            self._process_header(line)
            return None

        parts = [x.strip() for x in line.split(",")]

        if self.roll_index is None or self.pitch_index is None:
            return None

        if len(parts) <= max(self.counter_index, self.roll_index, self.pitch_index):
            return None

        try:
            counter = int(parts[self.counter_index])
            roll_deg = int(parts[self.roll_index]) / SCALE
            pitch_deg = int(parts[self.pitch_index]) / SCALE
        except ValueError:
            return None

        return counter, roll_deg, pitch_deg


def main():
    args = parse_args()

    max_points = int(args.window * args.fs)

    times = deque(maxlen=max_points)
    rolls = deque(maxlen=max_points)
    pitches = deque(maxlen=max_points)

    reader = OrientationSerialReader(args.port, args.baud)

    start_time = time.time()

    fig = plt.figure(figsize=(11, 7))

    ax_plot = fig.add_subplot(2, 1, 1)
    ax_attitude = fig.add_subplot(2, 2, 3)
    ax_bar = fig.add_subplot(2, 2, 4)

    # Live line plot
    roll_line, = ax_plot.plot([], [], label="Roll")
    pitch_line, = ax_plot.plot([], [], label="Pitch")

    ax_plot.set_title("Live IMU Roll/Pitch")
    ax_plot.set_xlabel("Time [s]")
    ax_plot.set_ylabel("Angle [deg]")
    ax_plot.grid(True)
    ax_plot.legend(loc="upper right")

    # Simple 2D attitude view
    ax_attitude.set_title("2D Roll View")
    ax_attitude.set_xlim(-2, 2)
    ax_attitude.set_ylim(-1.5, 1.5)
    ax_attitude.set_aspect("equal")
    ax_attitude.grid(True)

    body = Rectangle(
        (-1.0, -0.15),
        2.0,
        0.3,
        fill=False,
        linewidth=2,
    )
    ax_attitude.add_patch(body)

    roll_text = ax_attitude.text(
        0,
        -1.2,
        "Roll: 0.00 deg",
        ha="center",
        va="center",
    )

    # Pitch bar
    ax_bar.set_title("Pitch Bar")
    ax_bar.set_xlim(-30, 30)
    ax_bar.set_ylim(0, 1)
    ax_bar.set_yticks([])
    ax_bar.grid(True)

    pitch_bar = ax_bar.barh([0.5], [0.0], height=0.3)[0]
    pitch_text = ax_bar.text(
        0,
        0.85,
        "Pitch: 0.00 deg",
        ha="center",
        va="center",
    )

    def update(_frame):
        # Read several samples per animation frame so plot stays responsive
        for _ in range(20):
            sample = reader.read_sample()

            if sample is None:
                continue

            counter, roll_deg, pitch_deg = sample

            t = time.time() - start_time

            times.append(t)
            rolls.append(roll_deg)
            pitches.append(pitch_deg)

        if len(times) < 2:
            return roll_line, pitch_line, body, pitch_bar, roll_text, pitch_text

        # Update line plot
        roll_line.set_data(times, rolls)
        pitch_line.set_data(times, pitches)

        t_max = times[-1]
        t_min = max(0.0, t_max - args.window)

        ax_plot.set_xlim(t_min, t_max)

        y_min = min(min(rolls), min(pitches)) - 5.0
        y_max = max(max(rolls), max(pitches)) + 5.0

        if y_min == y_max:
            y_min -= 1.0
            y_max += 1.0

        ax_plot.set_ylim(y_min, y_max)

        # Latest values
        roll_now = rolls[-1]
        pitch_now = pitches[-1]

        # Update 2D roll view
        transform = (
            Affine2D()
            .rotate_deg(roll_now)
            + ax_attitude.transData
        )
        body.set_transform(transform)

        roll_text.set_text(f"Roll: {roll_now:.2f} deg")

        # Update pitch bar
        pitch_bar.set_width(pitch_now)
        pitch_text.set_text(f"Pitch: {pitch_now:.2f} deg")

        return roll_line, pitch_line, body, pitch_bar, roll_text, pitch_text

    ani = FuncAnimation(
        fig,
        update,
        interval=50,
        blit=False,
        cache_frame_data=False,
    )

    try:
        plt.tight_layout()
        plt.show()
    finally:
        reader.close()


if __name__ == "__main__":
    main()