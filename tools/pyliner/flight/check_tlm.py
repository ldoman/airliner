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
                      "script_name": "Check Telemetry",
                      "log_dir": "./logs/"})

airliner.subscribe({'tlm': ['/Airliner/SENS/HK/Acc',
                            '/Airliner/SENS/HK/BaroAlt',
                            '/Airliner/SENS/HK/Mag',
                            '/Airliner/GPS/HK/Lat',
                            '/Airliner/GPS/HK/Lon',
                            '/Airliner/ES/HK/CmdCounter']})

def check_telemetry ():
    initial_baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
    initial_gps_lat = airliner.get_tlm_value('/Airliner/GPS/HK/Lat')
    initial_gps_lon = airliner.get_tlm_value('/Airliner/GPS/HK/Lon')
    print "initial baro: " + str(initial_baro_sensor_combined)
    print "initial lat: " + str(initial_gps_lat)
    print "initial lon: " + str(initial_gps_lon)

    while (1):
        raw_input("Press enter to sample telemetry")
        accel_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/Acc')
        baro_sensor_combined = airliner.get_tlm_value('/Airliner/SENS/HK/BaroAlt')
        gps_lat = airliner.get_tlm_value('/Airliner/GPS/HK/Lat')
        gps_lon = airliner.get_tlm_value('/Airliner/GPS/HK/Lon')
        print "/Airliner/SENS/HK/Acc: " + str(accel_sensor_combined) 
        print "/Airliner/SENS/HK/BaroAlt: " + str(baro_sensor_combined)    
        print "/Airliner/ES/GPS/Lat: " + str(gps_lat)        
        print "/Airliner/ES/GPS/Lon: " + str(gps_lon)

check_telemetry()
