import PySimpleGUI as sg
from scrappy_server import *

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


def move_0(direction, event_vars):
    pass


def move_1(direction, event_vars):
    pass


def move_2(direction, event_vars):
    pass


def calibrate(event_vars):
    pass


def submit(event_vars):
    pass


def test(event_vars):
    pass



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
     sg.Button("Disconnect", size=(15, 3), pad=((46, 5), (1, 50)), key='disconnect')]
]

layout = [
    [
        sg.Column(control_column, justification='left', vertical_alignment='top'),
        sg.Column(network_column, vertical_alignment='top'),
        sg.Column(data_column, vertical_alignment='top')
    ]
]


def setup_window(w, s):
    w.maximize()
    w['cmd-input'].bind("<Return>", "_Enter")
    s.print_network_status()


def send_move_command(v, s):
    command = f"move:{v['0-position']},{v['1-position']},{v['2-position']},{v['0-speed']},{v['1-speed']},{v['2-speed']}"
    if ",," in command or len(v['0-position']) == 0 or len(v["2-speed"]) == 0:
        s.print_to_element(f"Invalid move arguments {command}", "cmd-output")
        return False
    s.send_command(command)
    return True


def handle_event(ev, v, s, w):
    if ev == "cmd-input_Enter":
        s.send_command(v['cmd-input'])
    elif ev == 'connect':
        s.start()
    elif ev == 'disconnect':
        s.disconnect()
    elif ev == "calibrate":
        s.arm_calibrated = send_move_command(v, s)
    elif ev == 'motor-submit':
        if not s.arm_calibrated:
            s.print_to_element("Not calibrated", "cmd-output")
            return
        send_move_command(v, s)
    elif ev == "test":
        command = "move:100,100,100,30,30,30"
        s.send_command(command)
    elif "-forward" in ev or "-backward" in ev:
        zero_pos = int(v['0-position'])
        one_pos = int(v['1-position'])
        two_pos = int(v['2-position'])
        if "0-" in ev:
            v['0-position'] = str(zero_pos + STEP if "forward" in ev else zero_pos - STEP)
            w[ZERO_POS].update(v['0-position'])
        elif "1-" in ev:
            v['1-position'] = str(one_pos + STEP if "forward" in ev else one_pos - STEP)
            w[ONE_POS].update(v['1-position'])
        elif "2-" in ev:
            v['2-position'] = str(two_pos + STEP if "forward" in ev else two_pos - STEP)
            w[TWO_POS].update(v['2-position'])
        send_move_command(v, s)
    else:
        s.send_command(ev)


if __name__ == '__main__':
    window = sg.Window("SCRAPP1-MK1", layout, finalize=True, )
    with ArmServer(ADDRESS, window) as server:
        setup_window(window, server)
        server.start()
        while True:
            event, values = window.read()
            print(event, values)
            if event == WIN_CLOSED:
                break
            handle_event(event, values, server, window)
            server.print_network_status()

        window.close()
