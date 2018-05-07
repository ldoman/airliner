from os import path, sys
from pyliner import Pyliner
from time import sleep
from datetime import datetime
from flufl.enum import Enum

# Enums
class FlightMode(Enum):
    Manual                     = 1
    AltCtl                     = 2
    PosCtl                     = 3
    RTL                        = 4

class Direction(Enum):
    Forward                     = 1
    Backward                    = 2
    Left                        = 3
    Right                       = 4
    North                       = 5
    South                       = 6
    East                        = 7
    West                        = 8    
    Up                          = 9    

def atp(airliner, txt):
    print "%s requires authorization for: %s" % (airliner.script_name, txt)
    raw_input("Press enter to proceed >>>")
    airliner.log("%s requires authorization to proceed. Requesting: %s" % (airliner.script_name, txt))

# Flight control commands
def vehicle_move(airliner, direction, speed = 0.5, time = 1, stop = True, stop_wait = 0):
    """ Router for move commands. Note: speed is stick position percentage, not velocity. """

    # Move in the specified direction
    if direction == Direction.Forward:
        vehicle_move_forward(airliner, speed, time)
    elif direction == Direction.Backward:
        vehicle_move_backward(airliner, speed, time)
    elif direction == Direction.Right:
        vehicle_move_right(airliner, speed, time)
    elif direction == Direction.Left:
        vehicle_move_left(airliner, speed, time)
    elif direction == Direction.Up:
        vehicle_move_up(airliner, speed, time)
    else:
        airliner.log("Unknown move direction specified", LogLevel.Error)

    # Execute manuevor for specified durationm4
    if time:
        sleep(time)

    # If user wants vehicle to stop vehicle after executing manuevor return to stable hover
    if stop:
        vehicle_stable_hover(airliner)
        if stop_wait:
            sleep(stop_wait)

def vehicle_move_dist(airliner, direction, dist, timeout = 30):
    """ Router for move commands. Note: speed is stick position percentage, not velocity. """

    # Move in the specified direction
    if direction == Direction.Up:
        vehicle_move_cl_up(airliner, dist, timeout)
    elif direction == Direction.Down:
        vehicle_move_cl_down(airliner, dist, timeout)
#    elif direction == Direction.Forward:
#        vehicle_move_cl_forward(airliner, dist, timeout)
#    elif direction == Direction.Backward:
#        vehicle_move_cl_backward(airliner, dist, timeout)
#    elif direction == Direction.Right:
#        vehicle_move_cl_right(airliner, dist, timeout)
#    elif direction == Direction.Left:
#        vehicle_move_cl_left(airliner, dist, timeout)
    else:
        airliner.log("Unknown move direction specified", LogLevel.Error)

def vehicle_move_cl_up(airliner, deltaZ, timeout = 30):
    print "%s: Move up %s meters" % (airliner.script_name, deltaZ)
    airliner.log("%s: Move up %s meters" % (airliner.script_name, deltaZ))
    
    initial_alt = airliner.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
    print "initial_alt: " + str(initial_alt)
    intial_time = datetime.now()
    
    current_alt = initial_alt
    current_time = intial_time
    
    while((current_alt < (initial_alt + deltaZ)) or (current_time - intial_time) > timeout):
        vehicle_move_up(airliner, .75, -1)
        time.sleep(1)
        current_alt = airliner.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
        current_time = datetime.now()
        print "alt: " + str(current_alt)
        print "time: " + str(current_time - intial_time)
        
    vehicle_stable_hover()

def vehicle_move_cl_down(airliner, deltaZ, timeout = 30):
    print "%s: Move down %s meters" % (airliner.script_name, deltaZ)
    airliner.log("%s: Move down %s meters" % (airliner.script_name, deltaZ))
    
    initial_alt = airliner.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
    print "initial_alt: " + str(initial_alt)
    intial_time = datetime.now()
    
    current_alt = initial_alt
    current_time = intial_time
    
    while((current_alt > (initial_alt - deltaZ)) or (current_time - intial_time) > timeout):
        vehicle_move_down(airliner, .75, -1)
        time.sleep(1)
        current_alt = airliner.get_tlm_value('/Airliner/CNTL/VehicleGlobalPosition/Alt')
        current_time = datetime.now()
        print "alt: " + str(current_alt)
        print "time: " + str(current_time - intial_time)
        
    vehicle_stable_hover()

def vehicle_arm(airliner):
    print "%s: Arming vehicle" % airliner.script_name
    airliner.log("Arming vehicle")
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.0},
        {'name':'R', 'value':0.0},
        {'name':'Flaps', 'value':0.0},
        {'name':'Aux1', 'value':0.0},
        {'name':'Aux2', 'value':0.0},
        {'name':'Aux3', 'value':0.0},
        {'name':'Aux4', 'value':0.0},
        {'name':'Aux5', 'value':0.0},
        {'name':'ModeSwitch', 'value':0},
        {'name':'ReturnSwitch', 'value':0},
        {'name':'RattitudeSwitch', 'value':0},
        {'name':'PosctlSwitch', 'value':0},
        {'name':'LoiterSwitch', 'value':0},
        {'name':'AcroSwitch', 'value':0},
        {'name':'OffboardSwitch', 'value':0},
        {'name':'KillSwitch', 'value':0},
        {'name':'TransitionSwitch', 'value':0},
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':3}, # OFF
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})
    sleep(1)
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.0},
        {'name':'R', 'value':0.0},
        {'name':'Flaps', 'value':0.0},
        {'name':'Aux1', 'value':0.0},
        {'name':'Aux2', 'value':0.0},
        {'name':'Aux3', 'value':0.0},
        {'name':'Aux4', 'value':0.0},
        {'name':'Aux5', 'value':0.0},
        {'name':'ModeSwitch', 'value':0},
        {'name':'ReturnSwitch', 'value':0},
        {'name':'RattitudeSwitch', 'value':0},
        {'name':'PosctlSwitch', 'value':0},
        {'name':'LoiterSwitch', 'value':0},
        {'name':'AcroSwitch', 'value':0},
        {'name':'OffboardSwitch', 'value':0},
        {'name':'KillSwitch', 'value':0},
        {'name':'TransitionSwitch', 'value':0},
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1}, # ON
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

def vehicle_disarm(airliner):
    print "%s: Disarming vehicle" % airliner.script_name
    airliner.log("Disarming vehicle")
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.0},
        {'name':'R', 'value':0.0},
        {'name':'Flaps', 'value':0.0},
        {'name':'Aux1', 'value':0.0},
        {'name':'Aux2', 'value':0.0},
        {'name':'Aux3', 'value':0.0},
        {'name':'Aux4', 'value':0.0},
        {'name':'Aux5', 'value':0.0},
        {'name':'ModeSwitch', 'value':0},
        {'name':'ReturnSwitch', 'value':0},
        {'name':'RattitudeSwitch', 'value':0},
        {'name':'PosctlSwitch', 'value':0},
        {'name':'LoiterSwitch', 'value':0},
        {'name':'AcroSwitch', 'value':0},
        {'name':'OffboardSwitch', 'value':0},
        {'name':'KillSwitch', 'value':0},
        {'name':'TransitionSwitch', 'value':0},
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':3}, # OFF
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})


def vehicle_takeoff(airliner):
    print "%s: Auto takeoff" % airliner.script_name
    airliner.log("Auto takeoff")
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.0},
        {'name':'R', 'value':0.0},
        {'name':'Flaps', 'value':0.0},
        {'name':'Aux1', 'value':0.0},
        {'name':'Aux2', 'value':0.0},
        {'name':'Aux3', 'value':0.0},
        {'name':'Aux4', 'value':0.0},
        {'name':'Aux5', 'value':0.0},
        {'name':'ModeSwitch', 'value':0},
        {'name':'ReturnSwitch', 'value':0},
        {'name':'RattitudeSwitch', 'value':0},
        {'name':'PosctlSwitch', 'value':0},
        {'name':'LoiterSwitch', 'value':0},
        {'name':'AcroSwitch', 'value':0},
        {'name':'OffboardSwitch', 'value':0},
        {'name':'KillSwitch', 'value':0},
        {'name':'TransitionSwitch', 'value':1}, # ON
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})
    sleep(5)

def vehicle_flight_mode(airliner, mode):
    if not mode:
        airliner.log("Mode transition requires a passed mode.", LogLevel.Error)
    if mode == FlightMode.Manual:
        raise NotImplemented()
    elif mode == FlightMode.AltCtl:
        raise NotImplemented()
    elif mode == FlightMode.PosCtl:
        vehicle_mode_posctl(airliner)

def vehicle_mode_posctl(airliner):
    print "%s: Position control" % airliner.script_name
    airliner.log("Position control")
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.5},
        {'name':'R', 'value':0.0},
        {'name':'Flaps', 'value':0.0},
        {'name':'Aux1', 'value':0.0},
        {'name':'Aux2', 'value':0.0},
        {'name':'Aux3', 'value':0.0},
        {'name':'Aux4', 'value':0.0},
        {'name':'Aux5', 'value':0.0},
        {'name':'ModeSwitch', 'value':0},
        {'name':'ReturnSwitch', 'value':0},
        {'name':'RattitudeSwitch', 'value':0},
        {'name':'PosctlSwitch', 'value':1}, # ON
        {'name':'LoiterSwitch', 'value':0},
        {'name':'AcroSwitch', 'value':0},
        {'name':'OffboardSwitch', 'value':0},
        {'name':'KillSwitch', 'value':0},
        {'name':'TransitionSwitch', 'value':0},
        {'name':'GearSwitch', 'value':1},
        {'name':'ArmSwitch', 'value':0},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

def vehicle_stable_hover(airliner):
    airliner.log("Stable hover")
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.5}, # 50% Throttle
        {'name':'R', 'value':0.0},
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
        {'name':'GearSwitch', 'value':1},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

def vehicle_move_forward(airliner, percent_speed, time):
    print "%s: moving forward at %s%% for %s seconds" % (airliner.script_name, percent_speed * 100, time)
    airliner.log("Moving forward at %s%% for %s seconds" % (percent_speed, time))
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':percent_speed},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.5},
        {'name':'R', 'value':0.0},
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
        {'name':'GearSwitch', 'value':1},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

def vehicle_move_left(airliner, percent_speed, time):
    print "%s: moving left at %s%% for %s seconds" % (airliner.script_name, percent_speed * 100, time)
    airliner.log("Moving left at %s%% for %s seconds" % (percent_speed, time))
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':-percent_speed},
        {'name':'Z', 'value':0.5},
        {'name':'R', 'value':0.0},
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
        {'name':'GearSwitch', 'value':1},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

def vehicle_move_backward(airliner, percent_speed, time):
    print "%s: moving backward at %s%% for %s seconds" % (airliner.script_name, percent_speed * 100, time)
    airliner.log("Moving backward at %s%% for %s seconds" % (percent_speed, time))
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':-percent_speed},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.5},
        {'name':'R', 'value':0.0},
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
        {'name':'GearSwitch', 'value':1},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

def vehicle_move_right(airliner, percent_speed, time):
    print "%s: moving right at %s%% for %s seconds" % (airliner.script_name, percent_speed * 100, time)
    airliner.log("Moving right at %s%% for %s seconds" % (percent_speed, time))
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':percent_speed},
        {'name':'Z', 'value':0.5},
        {'name':'R', 'value':0.0},
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
        {'name':'GearSwitch', 'value':1},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

def vehicle_move_up(airliner, percent_speed, time):
    print "%s: moving up at %s%% for %s seconds" % (airliner.script_name, percent_speed * 100, time)
    airliner.log("Moving up at %s%% for %s seconds" % (percent_speed, time))
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.5 + (percent_speed * .5)},
        {'name':'R', 'value':0.0},
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
        {'name':'GearSwitch', 'value':1},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

def vehicle_rtl(airliner):
    print "%s: RTL" % airliner.script_name
    airliner.log("RTL")
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':0.0},
        {'name':'R', 'value':0.0},
        {'name':'Flaps', 'value':0.0},
        {'name':'Aux1', 'value':0.0},
        {'name':'Aux2', 'value':0.0},
        {'name':'Aux3', 'value':0.0},
        {'name':'Aux4', 'value':0.0},
        {'name':'Aux5', 'value':0.0},
        {'name':'ModeSwitch', 'value':0},
        {'name':'ReturnSwitch', 'value':1}, # ON
        {'name':'RattitudeSwitch', 'value':0},
        {'name':'PosctlSwitch', 'value':0},
        {'name':'LoiterSwitch', 'value':0},
        {'name':'AcroSwitch', 'value':0},
        {'name':'OffboardSwitch', 'value':0},
        {'name':'KillSwitch', 'value':0},
        {'name':'TransitionSwitch', 'value':0},
        {'name':'GearSwitch', 'value':3},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})

#move = {Direction.Forward: vehicle_move_forward(airliner, speed, time),
#        Direction.Backward: 
#        Direction.Left:
#        Direction.Right:
#        Direction.Up:
#        Direction.Down:
#}

def move(airliner, direction, speed, time):
    # Move in the specified direction
    if direction == Direction.Forward:
        vehicle_move_forward(airliner, speed, time)
    elif direction == Direction.Backward:
        vehicle_move_backward(airliner, speed, time)
    elif direction == Direction.Right:
        vehicle_move_right(airliner, speed, time)
    elif direction == Direction.Left:
        vehicle_move_left(airliner, speed, time)
    elif direction == Direction.Up:
        vehicle_move_up(airliner, speed, time)
    else:
        airliner.log("Unknown move direction specified", LogLevel.Error)
