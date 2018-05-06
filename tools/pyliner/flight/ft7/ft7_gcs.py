from os import sys
from os.path import join, dirname, abspath
sys.path.append(dirname(dirname(dirname(abspath(__file__)))))
from pyliner import Pyliner
from flight_control_lib import *
import math
import time

# Initialize pyliner object
OcPoC = Pyliner(**{"airliner_map": join(dirname(abspath(__file__)), "cookiecutter.json"), 
                   "address": "192.168.1.2",
                   "ci_port": 5009,
                   "to_port": 5012,
                   "script_name": "FT7_GCS",
                   "log_dir": join(dirname(abspath(__file__)), "logs")})

# Subscribe to desired telemetry
OcPoC.subscribe({'tlm': ['/Airliner/CNTL/VehicleGlobalPosition/Lat',
                         '/Airliner/CNTL/VehicleGlobalPosition/Lon',
                         '/Airliner/CNTL/VehicleGlobalPosition/Alt',
                         '/Airliner/CNTL/VehicleGlobalPosition/Yaw',
                         '/Airliner/CNTL/VehicleGlobalPosition/VelN',
                         '/Airliner/CNTL/VehicleGlobalPosition/VelE',
                         '/Airliner/CNTL/VehicleGlobalPosition/VelD']})

# Wait for pyliner data dictionary to populate with initial values
while OcPoC.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt') == 'NULL':
    print "Waiting for telemetry downlink..."
    time.sleep(1)

alt = OcPoC.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
print "Alt: " + str(alt)

atp(OcPoC, "Arm")
vehicle_arm(OcPoC)
atp(OcPoC, "Takeoff")
vehicle_takeoff(OcPoC)
vehicle_flight_mode(OcPoC, FlightMode.PosCtl)

alt = OcPoC.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
print "Alt: " + str(alt)

atp(OcPoC, "Move up")
vehicle_move(OcPoC, Direction.Up, speed = 1.0, time = 2, stop = True, stop_wait = 3)

alt = OcPoC.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
print "Alt: " + str(alt)

atp(OcPoC, "Move forward")
vehicle_move(OcPoC, Direction.Forward, speed = .75, time = 2, stop = True, stop_wait = 3)
atp(OcPoC, "Move left")
vehicle_move(OcPoC, Direction.Left, speed = .75, time = 2, stop = True, stop_wait = 3)
atp(OcPoC, "Move backward")
vehicle_move(OcPoC, Direction.Backward, speed = .75, time = 2, stop = True, stop_wait = 3)
atp(OcPoC, "Move right")
vehicle_move(OcPoC, Direction.Right, speed = .75, time = 2, stop = True, stop_wait = 3)

atp(OcPoC, "RTL")
vehicle_rtl(OcPoC)
