import PySimpleGUI as sg
from scrappy_server import *

WIN_CLOSED = sg.WIN_CLOSED

sg.theme('Dark')
control_column = [
    [sg.Text("Controls", font=("Arial", 20))],
    [sg.HSeparator(pad=(5, (3, 20)))],
    [sg.Text("X", font=("Arial", 15))],
    [sg.Button("Forward", size=(15, 3), pad=((5, 40), 3), key="x-forward"),
     sg.Button("Backward", size=(15, 3), key='x-backward')],
    [sg.HorizontalSeparator(pad=(5, (3, 30)))],
    [sg.Text("Y", font=("Arial", 15))],
    [sg.Button("Forward", size=(15, 3), pad=((5, 40), 3), key="y-forward"),
     sg.Button("Backward", size=(15, 3), key='y-backward')],
    [sg.HorizontalSeparator(pad=(5, (3, 30)))],
    [sg.Text("Z", font=("Arial", 15))],
    [sg.Button("Forward", size=(15, 3), pad=((5, 40), 3), key="z-forward"),
     sg.Button("Backward", size=(15, 3), key='z-backward')],
    [sg.HorizontalSeparator(pad=(5, (3, 30)))],
    [sg.Text(size=(43, 30), key='cmd-output', background_color='black', text_color='green')],
    [sg.InputText(key='cmd-input', size=(49, 3), do_not_clear=False)]
]
data_column = [
    [sg.Output(size=(40, 100), key='cmd-display', background_color='black', text_color='green')]
    # [sg.Output(size=(40, 100), key='cmd-display', background_color='black', text_color='green')]
]

network_column = [
    [sg.Text("Network", font=("Arial", 20))],
    [sg.Text(size=(43, 30), key='network-display', background_color='black', text_color='green')],
    [sg.Button("Reconnect", size=(15, 3), pad=((5, 46), (1, 50)), key="connect"),
     sg.Button("Disconnect", size=(15, 3), pad=((46, 5), (1, 50)), key='disconnect')]
]

layout = [
    [
        sg.Column(control_column, justification='left', vertical_alignment='top'),
        sg.Text(pad=((400, 0), 0)),
        sg.Column(network_column, vertical_alignment='bottom'),
        sg.Text(pad=((400, 0), 0)),
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
        server.start()
    elif ev == 'disconnect':
        server.disconnect()
    else:
        s.send_command(ev)


if __name__ == '__main__':
    window = sg.Window("SCRAPPY-MK1", layout, finalize=True)
    with ArmServer(ADDRESS, window) as server:
        setup_window(window, server)
        server.start()
        while True:
            event, values = window.read()
            if event == WIN_CLOSED:
                break
            handle_event(event, values, server)
            server.print_network_status()

        window.close()
