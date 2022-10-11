import PySimpleGUI as sg
from scrappy_server import *
"""
    [sg.Button("Forward", size=(15, 3), pad=((5, 40), 3), key="x-forward"),
     sg.Button("Backward", size=(15, 3), key='x-backward')]
     [sg.Button("Forward", size=(15, 3), pad=((5, 40), 3), key="y-forward"),
     sg.Button("Backward", size=(15, 3), key='y-backward')],
         [sg.Button("Forward", size=(15, 3), pad=((5, 40), 3), key="z-forward"),
     sg.Button("Backward", size=(15, 3), key='z-backward')],
"""
WIN_CLOSED = sg.WIN_CLOSED

sg.theme('Dark')
control_column = [
    [sg.Text("Controls", font=("Arial", 20))],
    [sg.HSeparator(pad=(5, (3, 20)))],
    [sg.Text("Motor A", font=("Arial", 15))],
    [sg.Text("Position"), sg.Input(key="a-position", do_not_clear=False), sg.Text("Speed"), sg.Input(key="a-speed", do_not_clear=False)],
    [sg.HorizontalSeparator(pad=(5, (3, 30)))],
    [sg.Text("Motor B", font=("Arial", 15))],
    [sg.Text("Position"), sg.Input(key="b-position", do_not_clear=False), sg.Text("Speed"), sg.Input(key="b-speed", do_not_clear=False)],
    [sg.HorizontalSeparator(pad=(5, (3, 30)))],
    [sg.Text("Motor C", font=("Arial", 15))],
    [sg.Text("Position"), sg.Input(key="c-position", do_not_clear=False), sg.Text("Speed"), sg.Input(key="c-speed", do_not_clear=False)],
    [sg.Button("Submit", key="motor-submit")],
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


def handle_event(ev, v, s):
    if ev == "cmd-input_Enter":
        s.send_command(v['cmd-input'])
    elif ev == 'connect':
        s.start()
    elif ev == 'disconnect':
        s.disconnect()
    elif ev == 'motor-submit':
        command = f"move:{v['a-position']},{v['b-position']},{v['c-position']},{v['a-speed']},{v['b-speed']},{v['c-speed']}"
        print(command)
        s.send_command(command)

    else:
        s.send_command(ev)


if __name__ == '__main__':
    window = sg.Window("SCRAPPY-MK1", layout, finalize=True)
    with ArmServer(ADDRESS, window) as server:
        setup_window(window, server)
        server.start()
        while True:
            event, values = window.read()
            print(event, values)
            if event == WIN_CLOSED:
                break
            handle_event(event, values, server)
            server.print_network_status()

        window.close()
