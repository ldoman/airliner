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
        {'name':'Y', 'value':0.65},
        {'name':'Z', 'value':0.75},
        {'name':'R', 'value':-0.35},
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
        print "Alt: " + str(current_altitude)
        print "heading: " + str(get_heading())
    vehicle_stable_hover(rocky)
    time.sleep(1)
    
def get_heading():
    heading = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Yaw')
    heading = math.degrees(heading) + 180
    return heading

def turn_left(forward_speed, delta_heading):
   print "Turn left " + str(delta_heading) +" degrees"
   previous_heading = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Yaw')
   current_heading = previous_heading
   print "initial heading: " + str(get_heading())
   total_rotation = 0
   while(total_rotation < math.radians(delta_heading)):
        vehicle_turn_left(forward_speed)
        time.sleep(1.0)
        current_heading = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Yaw')
        delta_rotation = math.fabs(current_heading - previous_heading)
        if (delta_rotation > math.pi):
            delta_rotation -= 2*math.pi
        
        total_rotation += delta_rotation
            
        print "current heading: " + str(get_heading())
        print "current_heading (radians): " + str(current_heading)
        print "previous_heading (radians): " + str(previous_heading)
        print "delta rotation (radians): " + str(delta_rotation)
        print "total rotation (radians): " + str(total_rotation)
        print "delta rotation (radians): " + str(math.radians(delta_heading))
        previous_heading = current_heading
                
   vehicle_stable_hover(rocky)

def vehicle_turn_left(fwd):
    rocky.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':rocky.get_time()},
        {'name':'X', 'value':fwd},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.5},
        {'name':'R', 'value':-0.1},
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


# Initialize pyliner object
rocky = Pyliner(**{"airliner_map": join(dirname(abspath(__file__)), "cookiecutter.json"), 
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

#atp(rocky, "Arm")
vehicle_arm(rocky)
#atp(rocky, "Takeoff")
vehicle_takeoff(rocky)
vehicle_flight_mode(rocky, FlightMode.PosCtl)
vehicle_move(rocky, Direction.Up, speed = .85, time = 1, stop = True, stop_wait = 3)
atp(rocky, "Turn left 720")
turn_left(0,720)
atp(rocky, "Turn left 45")
turn_left(0, 45)
atp(rocky, "Turn left fly forward")
turn_left(0.35, 360)
atp(rocky, "Turn left 90")
turn_left(0, 90)
#alt = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
#print "Alt: " + str(alt)

#atp(rocky, "Move up")
#vehicle_move(rocky, Direction.Up, speed = .85, time = 1, stop = True, stop_wait = 3)

#alt = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
#heading = rocky.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Yaw')
#print "Alt: " + str(alt)
#print "Heading: " + str(heading)

#atp(rocky, "Fly spiral")
#print "Fly spiral up `0 meters"
#vehicle_fly_spiral_ccw(10)

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
