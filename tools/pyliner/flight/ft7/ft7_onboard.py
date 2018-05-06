from os import sys
from os.path import join, dirname, abspath
sys.path.append(dirname(dirname(dirname(abspath(__file__)))))
from pyliner import Pyliner
from flight_control_lib import *
import math
import time

# Initialize pyliner object
OcPoC = Pyliner(**{"airliner_map": join(dirname(abspath(__file__)), "cookiecutter.json"), 
                   "ci_port": 5009,
                   "to_port": 5012,
                   "script_name": "FT7_Onboard",
                   "log_dir": join(dirname(abspath(__file__)), "logs")})

vehicle_arm(OcPoC)
vehicle_takeoff(OcPoC)
vehicle_flight_mode(OcPoC, FlightMode.PosCtl)

vehicle_move_cl_up(OcPoC, 10)
#vehicle_move(OcPoC, Direction.Up, speed = 1.0, time = 2, stop = True, stop_wait = 3)
#vehicle_move(OcPoC, Direction.Forward, speed = .75, time = 2, stop = True, stop_wait = 3)
#vehicle_move(OcPoC, Direction.Left, speed = .75, time = 2, stop = True, stop_wait = 3)
#vehicle_move(OcPoC, Direction.Backward, speed = .75, time = 2, stop = True, stop_wait = 3)
#vehicle_move(OcPoC, Direction.Right, speed = .75, time = 2, stop = True, stop_wait = 3)
vehicle_rtl(OcPoC)
