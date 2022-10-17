import threading
from serial import *


class SerialReader:

    def __init__(self, window=None):
        self.port = None
        self.started = False
        self.serial = None
        self.read_thread = None
        self.window = window

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.serial.close()

    def start(self, port):
        if not self.started:
            self.serial = Serial(port, 115200)
            self.port = port
            self.serial.flush()
            self.read_thread = threading.Thread(target=self._start, daemon=True)
            self.read_thread.start()
            self.started = True
        self.print_serial_status()

    def stop(self):
        if self.started:
            self.serial.close()
            self.started = False
        self.print_serial_status()

    def _start(self):
        while self.serial.isOpen():
            command = self.serial.readline().decode().strip()
            self.window.write_event_value("serial-event", command)
        self.started = False

    def print_serial_status(self):
        serial_string = \
    f"""Serial Port: {self.port}
Connected: {self.started}
    """
        self.window['serial-display'].update(serial_string)


    def test(self):
        while self.serial.isOpen():
            command = self.serial.readline().decode().strip()
            command = command.split(":")
            print(command)


def main():
    with SerialReader("COM4") as reader:
        reader.test()


if __name__ == '__main__':
    main()
