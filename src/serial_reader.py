import threading
from serial import *


class SerialReader:

    def __init__(self, port, window):
        self.port = port
        self.started = False
        self.read_thread = None
        self.serial = Serial(self.port)
        self.window = window

    def start(self):
        if not self.started:
            self.serial.flush()
            self.read_thread = threading.Thread(target=self._start, daemon=True)
            self.read_thread.start()
            self.started = True

    def _start(self):
        while self.serial.isOpen():
            command = self.serial.readline().decode()
            command = command.split(":")
            self.window.write_event_value(command[0], command[1])
        self.read_thread.join()
        self.started = False
