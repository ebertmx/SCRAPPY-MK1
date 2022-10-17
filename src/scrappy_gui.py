import PySimpleGUI as sg
from scrappy_server import *
from serial_reader import *
import traceback
import serial.tools.list_ports
WIN_CLOSED = sg.WIN_CLOSED

STEP = 10

ZERO_POS = "0-position"
ONE_POS = "1-position"
TWO_POS = "2-position"

ZERO_MOTOR = 0
ONE_MOTOR = 1
TWO_MOTOR = 2

POS = 1
NEG = -1

default_serial_string = \
    """Serial Port: None
Connected: False
    """


class Window(sg.Window):

    def __init__(self):
        super(Window, self).__init__("SCRAPPY-MK1", layout, finalize=True, resizable=True)
        self.serial = None
        self.server = None


def move_0(args, window):
    pass


def move_1(args, window):
    pass


def move_2(args, window):
    pass


def calibrate(args, window):
    pass


def submit(args, window):
    pass


def test(args, window):
    pass


def start_serial(args, window):
    if window.serial is None:
        window.serial = SerialReader(args["port_input"], window)
    window.serial.start()


def stop_serial(args, window):
    window.serial.stop()


sg.theme('Dark')
control_column = [
    [sg.Text("Controls", font=("Arial", 20))],
    [sg.HSeparator(pad=(5, (3, 20)))],
    [sg.Text("Motor 0", font=("Arial", 15))],
    [sg.Text("Position"), sg.InputText(key=ZERO_POS, default_text="0"), sg.Text("Speed"),
     sg.InputText(key="0-speed", default_text="50")],
    [sg.Button("+", size=(15, 3), pad=((5, 40), 3), key="0-forward"),
     sg.Button("-", size=(15, 3), key='0-backward')],
    [sg.HorizontalSeparator(pad=(5, (3, 30)))],
    [sg.Text("Motor B", font=("Arial", 15))],
    [sg.Text("Position"), sg.InputText(key=ONE_POS, default_text="0"), sg.Text("Speed"),
     sg.InputText(key="1-speed", default_text="50")],
    [sg.Button("+", size=(15, 3), pad=((5, 40), 3), key="1-forward"),
     sg.Button("-", size=(15, 3), key='1-backward')],
    [sg.HorizontalSeparator(pad=(5, (3, 30)))],
    [sg.Text("Motor C", font=("Arial", 15))],
    [sg.Text("Position"), sg.InputText(key=TWO_POS, default_text="0"), sg.Text("Speed"),
     sg.InputText(key="2-speed", default_text="50")],
    [sg.Button("+", size=(15, 3), pad=((5, 40), 3), key="2-forward"),
     sg.Button("-", size=(15, 3), key='2-backward')],
    [sg.Button("Submit", key="motor-submit"), sg.Button("Calibrate", key="calibrate"), sg.Button("Test", key="test")],
    [sg.HorizontalSeparator(pad=(5, (3, 30)))],
    [sg.Text(size=(43, 15), key='cmd-output', background_color='black', text_color='green')],
    [sg.InputText(key='cmd-input', size=(49, 3), do_not_clear=False)]
]
data_column = [
    [sg.Output(size=(40, 100), key='cmd-display', background_color='black', text_color='green')]
    # [sg.Output(size=(40, 100), key='cmd-display', background_color='black', text_color='green')]
]

network_column = [
    [sg.Text("Network", font=("Arial", 20))],
    [sg.Text(size=(43, 15), key='network-display', background_color='black', text_color='green')],
    [sg.Button("Reconnect", size=(15, 3), pad=((5, 46), (1, 50)), key="connect"),
     sg.Button("Disconnect", size=(15, 3), pad=((46, 5), (1, 50)), key='disconnect')],
    [sg.Text("Network", font=("Arial", 20))],
    [sg.Text(size=(43, 15), key='serial-display', background_color='black', text_color='green')],
    [sg.Text("Port:"), sg.InputText(key='port_input', size=(43, 3))],
    [sg.Button("Start Serial", size=(15, 3), pad=((5, 46), (1, 50)), key=start_serial),
     sg.Button("Stop Serial", size=(15, 3), pad=((46, 5), (1, 50)), key=stop_serial)],

]

layout = [
    [
        sg.Column(control_column, justification='left', vertical_alignment='top'),
        sg.Column(network_column, vertical_alignment='top'),
        sg.Column(data_column, vertical_alignment='top')
    ]
]


def setup_window(window, server):
    window.maximize()
    window['cmd-input'].bind("<Return>", "_Enter")
    window.server = server
    server.print_network_status()
    window['serial-display'].update(default_serial_string)


def send_move_command(command, values, server):
    command = f"{command}:{values['0-position']},{values['1-position']},{values['2-position']},{values['0-speed']},{values['1-speed']},{values['2-speed']}"
    if ",," in command or len(values['0-position']) == 0 or len(values["2-speed"]) == 0:
        server.print_to_element(f"Invalid move arguments {command}", "cmd-output")
        return False
    server.send_command(command)
    return True


def handle_event(event, values, window):
    print(event)
    server = window.server
    if callable(event):
        event(values, window)
    elif event == "serial-event":
        server.send_command(values['serial-event'])
    elif event == "cmd-input_Enter":
        server.send_command(values['cmd-input'])
    elif event == 'connect':
        server.start()
    elif event == 'disconnect':
        server.disconnect()
    elif event == "calibrate":
        server.arm_calibrated = send_move_command("C", values, server)
    elif event == 'motor-submit':
        if not server.arm_calibrated:
            server.print_to_element("Not calibrated", "cmd-output")
            return
        send_move_command("M", values, server)
    elif event == "test":
        command = "M:100,100,100,30,30,30"
        server.send_command(command)
    elif "-forward" in event or "-backward" in event:
        zero_pos = int(values['0-position'])
        one_pos = int(values['1-position'])
        two_pos = int(values['2-position'])
        if "0-" in event:
            values['0-position'] = str(zero_pos + STEP if "forward" in event else zero_pos - STEP)
            window[ZERO_POS].update(values['0-position'])
        elif "1-" in event:
            values['1-position'] = str(one_pos + STEP if "forward" in event else one_pos - STEP)
            window[ONE_POS].update(values['1-position'])
        elif "2-" in event:
            values['2-position'] = str(two_pos + STEP if "forward" in event else two_pos - STEP)
            window[TWO_POS].update(values['2-position'])
        send_move_command("M", values, server)


def main():
    window = Window()
    with ArmServer(ADDRESS, window) as server:
        setup_window(window, server)
        server.start()
        while True:
            event, values = window.read()
            if event == WIN_CLOSED:
                break
            try:
                handle_event(event, values, window)
            except Exception as e:
                print(e)
                traceback.print_exc()
            server.print_network_status()

        window.close()


if __name__ == '__main__':
    main()
