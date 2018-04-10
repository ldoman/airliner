from os import path, sys
sys.path.append(path.dirname( path.dirname( path.dirname( path.abspath(__file__) ) ) ) )
from pyliner import Pyliner
import time
import math
from flight_control_lib import *

# Initialize pyliner object
rocky = Pyliner(**{"airliner_map": "cookiecutter.json", 
                   "ci_port": 5009,
                   "to_port": 5012,
                   "script_name": "FT6_Onboard",
                   "log_dir": "./logs/"})

# Subscribe to desired telemetry
rocky.subscribe({'tlm': ['/Airliner/CNTL/VehicleGlobalPosition/Lat',
                         '/Airliner/CNTL/VehicleGlobalPosition/Lon',
                         '/Airliner/CNTL/VehicleGlobalPosition/Alt',
                         '/Airliner/CNTL/VehicleGlobalPosition/Yaw',
                         '/Airliner/CNTL/VehicleGlobalPosition/VelN',
                         '/Airliner/CNTL/VehicleGlobalPosition/VelE',
                         '/Airliner/CNTL/VehicleGlobalPosition/VelD']})

vehicle_arm(rocky)

raw_input("Press enter to takeoff>")
vehicle_takeoff(rocky)

vehicle_flight_mode(rocky, FlightMode.PosCtl)

#vehicle_fly_square_ccw()
#vehicle_fly_square_cw()
raw_input("Press enter to engage RTL>")
vehicle_rtl(rocky)

#def vehicle_fly_square_ccw():
#    print "Starting counter clockwise square pattern..."
#    raw_input("Press enter to move forward>")
#    vehicle_full_forward()
#    time.sleep(3)
#    vehicle_stable_hover()
#    raw_input("Press enter to move left>")
#    vehicle_full_left()
#    time.sleep(3)
#    vehicle_stable_hover()
#    raw_input("Press enter to move backwards>")
#    vehicle_full_reverse()
#    time.sleep(3)
#    vehicle_stable_hover()
#    raw_input("Press enter to move right>")
#    vehicle_full_right()
#    time.sleep(3)
#    vehicle_stable_hover()


#def vehicle_fly_square_cw():
#    print "Starting clockwise square pattern..."
#    raw_input("Press enter to move forward>")
#    vehicle_full_forward()
#    time.sleep(3)
#    vehicle_stable_hover()
#    raw_input("Press enter to move right>")
#    vehicle_full_right()
#    time.sleep(3)
#    vehicle_stable_hover()
#    raw_input("Press enter to move backwards>")
#    vehicle_full_reverse()
#    time.sleep(3)
#    vehicle_stable_hover()
#    raw_input("Press enter to move left>")
#    vehicle_full_left()
#    time.sleep(3)
#    vehicle_stable_hover()

