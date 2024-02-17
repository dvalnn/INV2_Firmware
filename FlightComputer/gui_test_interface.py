import PySimpleGUI as sg
import time
import serial

#mac
ser = serial.Serial('/dev/cu.usbserial-0001', 115200, timeout=0.015) #set read timeout of 1s
#windows
#ser = serial.Serial('COM3', 115200, timeout=0.015) #set read timeout of 1s

command_map = {
    "STATUS" : 0,
    "READY" : 1,
    "ARM" : 2,
    "ABORT" : 3,
    "LED_ON" : 4,
    "LED_OFF" : 5,
    "IMU_CALIB" : 6, 
    "STATUS_ACK" : 8,
    "ARM_ACK" : 10
}

state_map = {
    "IDLE" : 0,
    "READY" : 1,
    "ARMED" : 2,
    "LAUNCH": 3,
    "ABORT": 4,
    "IMU_CALIB": 5
}

state_map_to_string = {
    0: "IDLE",
    1: "READY",
    2: "ARMED",
    3: "LAUNCH",
    4: "ABORT",
    5: "IMU_CALIB"
}

sg.theme('DarkAmber')    # Keep things interesting for your users

layout = [[sg.Button('LED_ON', key = '_LED_ON_', size = (10,5)), sg.Button('LED_OFF', key = '_LED_OFF_', size = (10,5)), sg.Text("Ax  Ay  Az  Gx  Gy  Gz", key = '_IMU_OUT_', size = (40, 5), auto_size_text=True, font=('Arial Bold', 18))],      
          [sg.Button('Ready', key = '_READY_', size = (10,5)),
           sg.Button('Arm', key = '_ARM_', size = (10,5)), 
           sg.Button('Abort', key = '_ABORT_', size = (10,5)), 
           sg.Button('Log', key = "_LOG_", size = (10,5)),
           sg.Button('IMU_calib', key = "_IMU_calib_", size = (10,5)),
           sg.Button('Toggle log', key = '_LOG_TOGGLE_', size = (10,5)), sg.Exit()]]      

window = sg.Window('Window that stays open', layout)      


log_status = 0
def toggle_status():
    global log_status
    if log_status == 0: log_status = 1
    elif log_status == 1: log_status = 0

def arm():
    cmd = bytearray([0x55, command_map['ARM'], 1, 1, 2, 3])
    ser.write(cmd)
    
    rep = ser.read(6)
    if(len(rep) == 6 and (int(rep[1]) != command_map['ARM_ACK'] or int(rep[3]) != 1)):
        print("ARM1 error", rep)
        return
    else:
        print("ARM 1 ok")

    cmd = bytearray([0x55, command_map['ARM'], 1, 2, 2, 3])
    ser.write(cmd)
    
    rep = ser.read(6)
    if(len(rep) == 6 and (int(rep[1]) != command_map['ARM_ACK'] or int(rep[3]) != 2)):
        print("ARM2 error", rep)
        return
    else:
        print("ARM 2 ok")

    cmd = bytearray([0x55, command_map['ARM'], 1, 3, 2, 3])
    ser.write(cmd)
    
    rep = ser.read(6)
    if(len(rep) == 6 and (int(rep[1]) != command_map['ARM_ACK'] or int(rep[3]) != 3)):
        print("ARM3 error", rep)
        return
    else:
        print("ARM 3 ok")



sync_state = 1
cmd_state = 2
size_state = 3
data_state = 4
crc_1_state = 5
crc_2_state = 6
end_state = 7

comm_state = sync_state
data_total = 0
data_recv = 0
begin = time.perf_counter()
end = time.perf_counter()
buff = bytearray()

def print_status():
    state = int.from_bytes(buff[3:4], byteorder='big', signed=True)

    ax = int.from_bytes(buff[4:6], byteorder='big', signed=True)
    ay = int.from_bytes(buff[6:8], byteorder='big', signed=True)
    az = int.from_bytes(buff[8:10], byteorder='big', signed=True)

    ax = (ax * 0.244) / 1000 * 9.80665
    ay = (ay * 0.244) / 1000 * 9.80665
    az = (az * 0.244) / 1000 * 9.80665

    gx = int.from_bytes(buff[10:12], byteorder='big', signed=True)
    gy = int.from_bytes(buff[12:14], byteorder='big', signed=True)
    gz = int.from_bytes(buff[14:16], byteorder='big', signed=True)


    s1 = "State: " + state_map_to_string[state] + "\n"
    s = s1 + "Ax: " + str(round(ax, 2)) + " Ay: " + str(round(ay, 2)) + " Az: " + str(round(az, 2)) + "\nGx: " + str(round(gx, 2)) + " Gy: " + str(round(gy, 2)) + " Gz: " + str(round(gz, 2))
    window['_IMU_OUT_'].update(s)

    print("ax:", ax, "ay:", ay, "az:", az)
    print("gx:", gx, "gy:", gy, "gz:", gz)

def read_cmd():
    global comm_state, sync_state, cmd_state, size_state, data_state, crc_1_state, crc_2_state, end_state
    global buff
    global data_recv, data_total
    global begin, end
    
    while ser.in_waiting > 0:
        ch = ser.read(1)
        ch = int.from_bytes(ch, 'little')

        if comm_state == sync_state:
            if ch == 0x55:
                comm_state = cmd_state
                buff = bytearray()
                buff.append(ch)
                data_recv = 0
                begin = time.perf_counter()
            else:
                print(chr(ch), end="")
                #print(str(ch).encode('utf-8'))

        elif comm_state == cmd_state:
            buff.append(ch)
            comm_state = size_state

        elif comm_state == size_state:
            buff.append(ch)
            data_total = int(ch)
            if data_total == 0:
                comm_state = crc_1_state
            else:
                comm_state = data_state

        elif comm_state == data_state:
            buff.append(ch)
            data_recv += 1
            if data_total == data_recv:
                comm_state = crc_1_state

        elif comm_state == crc_1_state:
            buff.append(ch)
            comm_state = crc_2_state

        elif comm_state == crc_2_state: 
            buff.append(ch)
            comm_state = end_state
    
    end = time.perf_counter()
    msec = (end - begin) * 1000
    #print("ms: ", msec)
    if comm_state != sync_state and msec > 15:
        comm_state = sync_state
        print("command timeout", msec)
        print("buffer recieved:", buff) 

    if comm_state == end_state:
        comm_state = sync_state
        print("ACK recieved:", len(buff))
        if(buff[1] == command_map['STATUS_ACK']):
            print_status()
        else:
            print("Cmd_ACK: [",''.join('{:02x} '.format(x) for x in buff)[:-1], "]")
        #print(buff)


log_begin = time.perf_counter()
log_end = time.perf_counter()

while True:           
    event, values = window.read(timeout=1) 
    if event != '__TIMEOUT__': print(event, values)       
    cmd_launched = 0

    if event == sg.WIN_CLOSED or event == 'Exit':
        break      
    elif event == '_LOG_TOGGLE_':
        toggle_status()    
    else:
        if event == '_ARM_':
            cmd_launched = 1
            arm()
            pass
        elif event == '_READY_':
            cmd_launched = 1
            cmd = bytearray([0x55, command_map['READY'], 0, 0x20, 0x21])
            ser.write(cmd)
            #rep = ser.read(5)
            #print(rep.decode('utf-8'))
            #print(bytearray(rep).hex())
            pass
        elif event == '_ABORT_':
            cmd_launched = 1
            cmd = bytearray([0x55, command_map['ABORT'], 0, 0x20, 0x21])
            ser.write(cmd)
            #rep = ser.read(5)
            #print(rep.decode('utf-8'))
            #print(bytearray(rep).hex())
            pass
        elif event == '_LED_ON_':
            cmd_launched = 1
            cmd = bytearray([0x55, command_map['LED_ON'], 0, 0x20, 0x21])
            ser.write(cmd)
            #rep = ser.read(5)
            #print(rep.decode('utf-8'))
            #print(bytearray(rep).hex())
        elif event == '_LED_OFF_':
            cmd_launched = 1
            cmd = bytearray([0x55, command_map['LED_OFF'], 0, 0x20, 0x21])
            ser.write(cmd)
            #rep = ser.read(5)
            #print(rep.decode('utf-8'))
            #print(bytearray(rep).hex())
        elif event == '_LOG_':
            cmd_launched = 1
            cmd = bytearray([0x55, command_map['STATUS'], 0, 0x20, 0x21])
            ser.write(cmd)
            #rep = ser.read(6)
            #print(rep.decode('utf-8'))
            #print(bytearray(rep).hex())
        elif event == '_IMU_calib_':
            print("calibrate imu")
            cmd_launched = 1
            cmd = bytearray([0x55, command_map['IMU_CALIB'], 0, 0x20, 0x21])
            ser.write(cmd)

    log_end = time.perf_counter()
    if(cmd_launched == 0 and log_status == 1 and log_end - log_begin > 0.005):
        #send status cmd
        cmd = bytearray([0x55, command_map['STATUS'], 0, 0x20, 0x21])
        ser.write(cmd)
        log_begin = time.perf_counter()
        #pass
    
    #while ser.in_waiting > 0:
        #print(ser.read(1).decode('utf-8'), end='')

    read_cmd()

    time.sleep(0.005)

window.close()