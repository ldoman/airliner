from os import sys
from os.path import join, dirname, abspath
sys.path.append(dirname(dirname(dirname(abspath(__file__)))))
from pyliner import Pyliner
from flight_control_lib import *
import math
import time

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
vehicle_move(rocky, Direction.Up, speed = 1.0, time = 2, stop = True, stop_wait = 3)

alt = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
print "Alt: " + str(alt)

atp(rocky, "Move forward")
vehicle_move(rocky, Direction.Forward, speed = .75, time = 2, stop = True, stop_wait = 3)
atp(rocky, "Move left")
vehicle_move(rocky, Direction.Left, speed = .75, time = 2, stop = True, stop_wait = 3)
atp(rocky, "Move backward")
vehicle_move(rocky, Direction.Backward, speed = .75, time = 2, stop = True, stop_wait = 3)
atp(rocky, "Move right")
vehicle_move(rocky, Direction.Right, speed = .75, time = 2, stop = True, stop_wait = 3)

atp(rocky, "RTL")
vehicle_rtl(rocky)
