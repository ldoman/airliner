from os import sys
from os.path import join, dirname, abspath
sys.path.append(dirname(dirname(dirname(abspath(__file__)))))
from pyliner import Pyliner
from flight_control_lib import *
import math
import time



def vehicle_spiral_ccw():
    rocky.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':rocky.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.75},
        {'name':'Z', 'value':0.55},
        {'name':'R', 'value':-0.2},
        {'name':'Flaps', 'value':0.0},
        {'name':'Aux1', 'value':0.0},
        {'name':'Aux2', 'value':0.0},
        {'name':'Aux3', 'value':0.0},
        {'name':'Aux4', 'value':0.0},
        {'name':'Aux5', 'value':0.0},
        {'name':'ModeSwitch', 'value':0},
        {'name':'ReturnSwitch', 'value':0},
        {'name':'RattitudeSwitch', 'value':0},
        {'name':'PosctlSwitch', 'value':1},
        {'name':'LoiterSwitch', 'value':0},
        {'name':'AcroSwitch', 'value':0},
        {'name':'OffboardSwitch', 'value':0},
        {'name':'KillSwitch', 'value':0},
        {'name':'TransitionSwitch', 'value':0},
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

def  vehicle_fly_spiral_ccw(deltaZ):
    print "Fly spiral CCW and up " + str(deltaZ) +" meters"
    initial_altitude = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
    print "initial altitude: " + str(initial_altitude)
    current_altitude = initial_altitude
    while(current_altitude < (initial_altitude + deltaZ)):
        vehicle_spiral_ccw()
        time.sleep(1.0)
        current_altitude = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
        print "altitude: " + str(current_altitude)
    vehicle_stable_hover(rocky)
    time.sleep(1)


# Initialize pyliner object
rocky = Pyliner(**{"airliner_map": join(dirname(abspath(__file__)), "cookiecutter.json"), 
                   "address": "192.168.1.2",
                   "ci_port": 5009,
                   "to_port": 5012,
                   "script_name": "FT6_GCS",
                   "log_dir": join(dirname(abspath(__file__)), "logs")})

# Subscribe to desired telemetry
rocky.subscribe({'tlm': ['/Airliner/CNTL/VehicleGlobalPosition/Lat',
                         '/Airliner/CNTL/VehicleGlobalPosition/Lon',
                         '/Airliner/CNTL/VehicleGlobalPosition/Alt',
                         '/Airliner/CNTL/VehicleGlobalPosition/Yaw',
                         '/Airliner/CNTL/VehicleGlobalPosition/VelN',
                         '/Airliner/CNTL/VehicleGlobalPosition/VelE',
                         '/Airliner/CNTL/VehicleGlobalPosition/VelD']})

# Wait for pyliner data dictionary to populate with initial values
while rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt') == 'NULL':
    print "Waiting for telemetry downlink..."
    time.sleep(1)

alt = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
print "Alt: " + str(alt)

atp(rocky, "Arm")
vehicle_arm(rocky)
atp(rocky, "Takeoff")
vehicle_takeoff(rocky)
vehicle_flight_mode(rocky, FlightMode.PosCtl)

alt = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
print "Alt: " + str(alt)

atp(rocky, "Move up")
vehicle_move(rocky, Direction.Up, speed = .85, time = 1, stop = True, stop_wait = 3)

alt = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
print "Alt: " + str(alt)

atp(rocky, "Fly spiral")
print "Fly spiral up `0 meters"
vehicle_fly_spiral_ccw(10)

#atp(rocky, "Move forward")
#vehicle_move(rocky, Direction.Forward, speed = .75, time = 2, stop = True, stop_wait = 3)
#atp(rocky, "Move left")
#vehicle_move(rocky, Direction.Left, speed = .75, time = 2, stop = True, stop_wait = 3)
#atp(rocky, "Move backward")
#vehicle_move(rocky, Direction.Backward, speed = .75, time = 2, stop = True, stop_wait = 3)
#atp(rocky, "Move right")
#vehicle_move(rocky, Direction.Right, speed = .75, time = 2, stop = True, stop_wait = 3)

atp(rocky, "RTL")
vehicle_rtl(rocky)
