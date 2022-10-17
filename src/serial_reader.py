import threading
from serial import *


class SerialReader:

    def __init__(self, port, window=None):
        self.port = port
        self.started = False
        self.read_thread = None
        self.serial = Serial(self.port, 115200)
        self.window = window

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.serial.close()

    def start(self):
        if not self.started:
            self.serial.flush()
            self.read_thread = threading.Thread(target=self._start, daemon=True)
            self.read_thread.start()
            self.started = True

    def _start(self):
        while self.serial.isOpen():
            command = self.serial.readline().decode().strip()
            self.window.write_event_value("serial", command)
        self.read_thread.join()
        self.started = False

    def test(self):
        while self.serial.isOpen():
            command = self.serial.readline().decode().strip()
            command = command.split(":")
            print(command)


if __name__ == '__main__':
    with SerialReader("COM4") as reader:
        reader.test()
