from os import path, sys
sys.path.append( path.dirname( path.dirname( path.abspath(__file__) ) ) )
from pyliner import Pyliner
import time
import math
from px4_msg_enums import *

# Initialize pyliner object
airliner = Pyliner(**{"airliner_map": "cookiecutter.json", 
                      "ci_port": 5009,
                      "to_port": 5012,
                      "script_name": "FT5_Onboard",
                      "log_dir": "./logs/"})

airliner.subscribe({'tlm': ['/Airliner/SENS/HK/Acc',
                            '/Airliner/SENS/HK/BaroAlt',
                            '/Airliner/SENS/HK/Mag',
                            '/Airliner/GPS/HK/Lat',
                            '/Airliner/GPS/HK/Lon',
                            '/Airliner/ES/HK/CmdCounter']})

def vehicle_arm():
    print "Arming vehicle"
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
    time.sleep(2)
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
    time.sleep(1)


def vehicle_disarm():
    print "Disarming vehicle"
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


def vehicle_takeoff():
    print "Auto takeoff"
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
    time.sleep(5)


def vehicle_posctl_mode():
    print "Position control"
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
        {'name':'PosctlSwitch', 'value':1}, # ON
        {'name':'LoiterSwitch', 'value':0},
        {'name':'AcroSwitch', 'value':0},
        {'name':'OffboardSwitch', 'value':0},
        {'name':'KillSwitch', 'value':0},
        {'name':'TransitionSwitch', 'value':0},
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':0},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})
    time.sleep(0.1)


def vehicle_stable_hover():
    # 50% Throttle
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


def vehicle_forward(power = 0.5):
    "Move forward " + str(power)
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':power},
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
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})


def vehicle_left(power = 0.5):
    print "Move left " + str(power)
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':-(power)},
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
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})


def vehicle_reverse(power = 0.5):
    print "Move backwards " + str(power)
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':-(power)},
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
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})


def vehicle_right(power = 0.5):
    print "Move right " + str(power)
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':power},
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
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})


def vehicle_down(power = 0.0):
    print "Move down " + str(power)
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':power},
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
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})


def vehicle_up(power = 0.65):
    "Move Up" + str(power)
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.0},
        {'name':'Z', 'value':power},
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
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})



def vehicle_land():
    print "RTL and land"
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
        {'name':'GearSwitch', 'value':0},
        {'name':'ArmSwitch', 'value':1},
        {'name':'StabSwitch', 'value':0},
        {'name':'ManSwitch', 'value':0},
        {'name':'ModeSlot', 'value':0},
        {'name':'DataSource', 'value':0}]})


def vehicle_orbit_cw():
    print "Orbit CW"
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':-0.5},
        {'name':'Z', 'value':0.5},
        {'name':'R', 'value':0.25},
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


def vehicle_orbit_ccw():
    print "Orbit CCW"
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.5},
        {'name':'Z', 'value':0.5},
        {'name':'R', 'value':-0.25},
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


def vehicle_spiral_ccw():
    print "Sprial CCW"
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':0.5},
        {'name':'Z', 'value':0.65},
        {'name':'R', 'value':-0.5},
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


def vehicle_spiral_cw():
    print "Spiral CW"
    airliner.send_telemetry(
        {'name':'/Airliner/CNTL/ManualSetpoint', 'args':[
        {'name':'Timestamp', 'value':airliner.get_time()},
        {'name':'X', 'value':0.0},
        {'name':'Y', 'value':-0.5},
        {'name':'Z', 'value':0.65},
        {'name':'R', 'value':0.5},
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

def vehicle_fly_vertical_square():
    print "Fly vertical sqaure"
    vehicle_fly_up(10)
    vehicle_right()
    time.sleep(3)
    vehicle_stable_hover()
    time.sleep(1)
    vehicle_fly_down(10)
    vehicle_left()
    time.sleep(3)
    vehicle_stable_hover()
    time.sleep(1)

def vehicle_fly_up(deltaZ):
    print "Move up " + str(deltaZ) + " meters"
    initial_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
    print "initial_baro_sensor_combined: " + str(initial_baro_sensor_combined)
    current_baro_sensor_combined = initial_baro_sensor_combined
    while(current_baro_sensor_combined < (initial_baro_sensor_combined + deltaZ)):
        vehicle_up()
        time.sleep(1)
        current_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
        print "baro_sensor_combined: " + str(current_baro_sensor_combined)
    vehicle_stable_hover()
    time.sleep(1)


def vehicle_fly_down(deltaZ):
    print "Move down " + str(deltaZ) + " meters"
    initial_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
    print "initial_baro_sensor_combined: " + str(initial_baro_sensor_combined)
    current_baro_sensor_combined = initial_baro_sensor_combined
    while(current_baro_sensor_combined > (initial_baro_sensor_combined - deltaZ)):
        vehicle_down()
        time.sleep(1)
        current_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
        print "baro_sensor_combined: " + str(current_baro_sensor_combined)
    vehicle_stable_hover()
    time.sleep(1)

def vehicle_fly_direction(direction, delta):
    print "Move " + direction + " " + str(delta) + " meters"
    # read initial lat long
    origin_lat = airliner.get_tlm_value('/Airliner/GPS/HK/Lat') / 10000000.0
    origin_lon = airliner.get_tlm_value('/Airliner/GPS/HK/Lon') / 10000000.0
    print "origin lat: " + str(origin_lat) + " lon: " + str(origin_lon)
    current_lat = origin_lat
    current_lon = origin_lon
    travel = distance([origin_lat, origin_lon], [current_lat, current_lon])
    while(travel < delta):
        if (direction == "forward"):
            vehicle_forward(0.25)
        elif (direction == "reverse"):
            vehicle_reverse(0.25)
        elif(direction == "right"):
            vehicle_right(0.25)
        elif (direction == "left"):
            vehicle_left(0.25)
        else:
            print "error: unkonwn direction: " + str(direction)
        time.sleep(1)
        current_lat = airliner.get_tlm_value('/Airliner/GPS/HK/Lat') / 10000000.0
        current_lon = airliner.get_tlm_value('/Airliner/GPS/HK/Lon') / 10000000.0
        travel = distance([origin_lat, origin_lon], [current_lat, current_lon])
        print "current lat: " + str(current_lat) + " lon: " + str(origin_lon) + " distance: " + str(travel)
    vehicle_stable_hover()
    time.sleep(1)

def vehicle_fly_left(deltaX):
    print "Move left " + str(deltaX) + " meters"
    initial_lat = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
    print "initial_baro_sensor_combined: " + str(initial_baro_sensor_combined)
    current_baro_sensor_combined = initial_baro_sensor_combined
    while(current_baro_sensor_combined > (initial_baro_sensor_combined - deltaZ)):
        vehicle_down()
        time.sleep(1)
        current_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
        print "baro_sensor_combined: " + str(current_baro_sensor_combined)
    vehicle_stable_hover()
    time.sleep(1)

def check_land (timeout = 60):
    print "Check landing complete with timeout: " + str(timeout) + " sec"
    # check RTL Completion
    time_spent = 0
    baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')   
    while((baro_sensor_combined > 1.0) and (time_spent < timeout)):
        time.sleep(1.0)
        time_spent +=1
        baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
        print "baro_sensor_combined: " + str(baro_sensor_combined)
        #print "time_spent: " + str(time_spent) + " timeout: " + str(timeout)

    if baro_sensor_combined > 0.0 and baro_sensor_combined  < 1.0:
        description = "passed check baro height " + str(baro_sensor_combined)
        airliner.assert_true(True, description)
        print description
    else:
        description = "failed check baro height " + str(baro_sensor_combined)
        description += " timeout(sec): " +str(timeout)
        airliner.assert_true(False, description)
        print description
        all_test_passed = False

def check_stable_hover(timeout = 5):
    print "Check stable hover with timeout: " + str(timeout) + " sec"
    # Check stable hover
    time_spent = 0
    accel_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/Acc')
    while(accel_sensor_combined[2] > -9.0 or accel_sensor_combined[2] < -11.0) and time_spent < timeout:
        print "accel_sensor_combined_z: " + str(accel_sensor_combined[2])
        time.sleep(1.0)
        time_spent +=1
        accel_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/Acc')

    # check gravity vector
    if accel_sensor_combined[2] < -9.0 and accel_sensor_combined[2] > -11.0:
        description = "passed check gravity vector " + str(accel_sensor_combined[2])
        airliner.assert_true(True, description)
        print description
    else:
        description = "failed check gravity vector " + str(accel_sensor_combined[2])
        description += " timeout(sec): " +str(timeout)
        airliner.assert_true(False,description)
        print description
        all_test_passed = False   

def vehicle_fly_square_ccw():
    print "Fly CCW square"
    vehicle_fly_direction("reverse", 4)
    vehicle_fly_direction("right", 4)
    vehicle_fly_direction("forward", 4)
    vehicle_fly_direction("left", 4)


def vehicle_fly_square_cw():
    print "Fly CW square"
    vehicle_fly_direction("forward", 4)
    vehicle_fly_direction("right", 4)
    vehicle_fly_direction("reverse", 4)
    vehicle_fly_direction("left", 4)


def vehicle_fly_orbit_cw(delay):
    print "Fly CW orbit for " + str(delay) + " sec"
    vehicle_orbit_cw()
    time.sleep(delay)
    vehicle_stable_hover()
    time.sleep(1)

def vehicle_fly_orbit_ccw(delay):
    print "Fly CCW orbit for " + str(delay) + " sec"
    vehicle_orbit_ccw()
    time.sleep(delay)
    vehicle_stable_hover()
    time.sleep(1)

def vehicle_fly_spiral_ccw(deltaZ):
    print "Fly spiral CCW and up " + str(deltaZ) +" meters"
    initial_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
    print "initial_baro_sensor_combined: " + str(initial_baro_sensor_combined)
    current_baro_sensor_combined = initial_baro_sensor_combined
    while(current_baro_sensor_combined < (initial_baro_sensor_combined + deltaZ)):
        vehicle_spiral_ccw()
        time.sleep(1.0)
        current_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
        print "baro_sensor_combined: " + str(current_baro_sensor_combined)
    vehicle_stable_hover()
    time.sleep(1)
    vehicle_fly_down(deltaZ)

def vehicle_fly_spiral_cw(deltaZ):
    print "Fly spiral CW and up " + str(deltaZ) +" meters"
    initial_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
    print "initial_baro_sensor_combined: " + str(initial_baro_sensor_combined)
    current_baro_sensor_combined = initial_baro_sensor_combined
    while(current_baro_sensor_combined < (initial_baro_sensor_combined + deltaZ)):
        vehicle_spiral_cw()
        time.sleep(1.0)
        current_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
        print "baro_sensor_combined: " + str(current_baro_sensor_combined)
    vehicle_stable_hover()
    time.sleep(1)
    vehicle_fly_down(deltaZ)

import math
def distance(origin, destination):
    """
    Calculate the Haversine distance.

    Parameters
    ----------
    origin : tuple of float
        (lat, long)
    destination : tuple of float
        (lat, long)

    Returns
    -------
    distance_in_km : float

    Examples
    --------
    >>> origin = (48.1372, 11.5756)  # Munich
    >>> destination = (52.5186, 13.4083)  # Berlin
    >>> round(distance(origin, destination), 1)
    504.2
    """
    lat1, lon1 = origin
    lat2, lon2 = destination
    radius = 6371  # km

    dlat = math.radians(lat2 - lat1)
    dlon = math.radians(lon2 - lon1)
    a = (math.sin(dlat / 2) * math.sin(dlat / 2) +
         math.cos(math.radians(lat1)) * math.cos(math.radians(lat2)) *
         math.sin(dlon / 2) * math.sin(dlon / 2))
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    d = radius * c
    return d*1000

def check_telemetry ():
    last_baro_sensor_combined = 0
    last_gps_lat = 0
    last_gps_lon = 0


    while (1):
        raw_input("Press enter to sample telemetry")
        accel_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/Acc')
        baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
        deltaAlt = baro_sensor_combined - last_baro_sensor_combined
        gps_lat = airliner.get_tlm_value('/Airliner/GPS/HK/Lat') / 10000000.0
        gps_lon = airliner.get_tlm_value('/Airliner/GPS/HK/Lon') / 10000000.0
        deltaPos = distance([last_gps_lat, last_gps_lon], [gps_lat, gps_lon]) 
        print "/Airliner/SENS/HK/Acc: " + str(accel_sensor_combined) 
        print "/Airliner/SENS/HK/BaroAlt: " + str(baro_sensor_combined)    
        print "/Airliner/ES/GPS/Lat: " + str(gps_lat)        
        print "/Airliner/ES/GPS/Lon: " + str(gps_lon)  
        print "deltaAlt: " + str(deltaAlt)
        print "deltaPos: " + str(deltaPos)  
        last_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
        last_gps_lat = airliner.get_tlm_value('/Airliner/GPS/HK/Lat') / 10000000.0
        last_gps_lon = airliner.get_tlm_value('/Airliner/GPS/HK/Lon') / 10000000.0


check_telemetry()

# vehicle control
#vehicle_arm()
#time.sleep(5)
#vehicle_takeoff()
#vehicle_posctl_mode()
#vehicle_stable_hover()
#check_stable_hover()

#i = 6
#while (i > 0):
#    i -= 1
#    vehicle_fly_direction("forward", 10)
#    vehicle_fly_direction("right", 1)
#    vehicle_fly_direction("reverse", 10)
    #vehicle_fly_direction("left", 5)
#    vehicle_fly_direction("right", 1)


#vehicle_fly_vertical_square()
#vehicle_stable_hover()
#check_stable_hover()

#vehicle_fly_up(5)
#vehicle_fly_orbit_cw(7)
#vehicle_stable_hover()
#check_stable_hover()
#vehicle_fly_orbit_ccw(7)
#vehicle_fly_down(5)

#vehicle_fly_square_ccw()
#vehicle_fly_square_cw()

#vehicle_fly_spiral_ccw(10)
#vehicle_fly_spiral_cw(10)

#vehicle_land()
#check_land()

initial_es_hk_cmdcnt = airliner.get_tlm_value('/Airliner/ES/HK/CmdCounter')
print "initial_es_hk_cmdcnt: " + str(initial_es_hk_cmdcnt)
# Send one NoOp command
airliner.send_command({'name':'/Airliner/ES/Noop'})
time.sleep(5)


# print received telemetry
es_hk_cmdcnt = airliner.get_tlm_value('/Airliner/ES/HK/CmdCounter')
print "es_hk_cmdcnt: " + str(es_hk_cmdcnt)
accel_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/Acc')
print "accel_sensor_combined_z: " + str(accel_sensor_combined[2])
baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
print "baro_sensor_combined: " + str(baro_sensor_combined)
gps_lat = airliner.get_tlm_value('/Airliner/GPS/HK/Lat') / 10000000.0
print "/Airliner/ES/GPS/Lat: " + str(gps_lat)
gps_lon = airliner.get_tlm_value('/Airliner/GPS/HK/Lon') / 10000000.0
print "/Airliner/ES/GPS/Lon: " + str(gps_lon)
mag = airliner.get_tlm_value('/Airliner/SENS/HK/Mag')
print "mag: " + str(mag)

all_test_passed = True


 
# check es command count
if es_hk_cmdcnt == initial_es_hk_cmdcnt + 1:
    description = "passed check command count"
    airliner.assert_true(True, description)
    print description
else:
    description = "failed check command count"
    airliner.assert_true(False, description)
    print description
    all_test_passed = False

# print test results to log and generate junit for jenkins
airliner.finish_test()
