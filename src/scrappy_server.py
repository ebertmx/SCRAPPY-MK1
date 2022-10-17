import socket
import threading


CMD_PORT = 9999
CAMERA_PORT = 9998
HOST = socket.gethostbyname(socket.gethostname())
ADDRESS = (HOST, CMD_PORT)


class ArmServer:

    def __init__(self, address, window):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind(address)
        self.window = window
        self.connected = False
        self.started = False
        self.client = None
        self.addr = None
        self.server_thread = None
        self.arm_calibrated = True

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.socket.close()

    def disconnect(self):
        if not self.connected:
            return
        self.client.close()

    def start(self):
        if not self.started:
            self.server_thread = threading.Thread(target=self._start, daemon=True)
            self.server_thread.start()
            self.started = True

    def _start(self):
        self.socket.listen()
        self.client, self.addr = self.socket.accept()
        self.connected = True
        self.print_network_status()
        with self.client:
            while self.connected:
                try:
                    data = self.client.recv(1024).decode()
                    self.print_to_element(data, "scrappy-display")
                except Exception as e:
                    print(e)
                    self.connected = False
                    self.print_network_status()
        self.started = False

    def print_to_element(self, data, element_id):
        self.window[element_id].print(data)

    def send_command(self, command):
        self.print_to_element(command, 'scrappy-output')
        if not self.connected:
            self.print_to_element("No connection", 'scrappy-display')
            return
        self.client.sendall(command.encode())

    def print_network_status(self):
        network_string = \
            f"""Host IP: {HOST}
Host Port: {CMD_PORT}
Connected: {self.connected}
"""
        if self.connected:
            network_string += \
                f"""Client IP: {self.addr[0]}
Client Port: {self.addr[1]}
    """
        self.window['network-display'].update(network_string)