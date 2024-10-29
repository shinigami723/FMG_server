#!/bin/bash

SSID = "FMG_SERVER"
PASSWORD = "AIIMS_FMG_SERVER"

nmcli dev wifi hotspot ifname wlano ssid "$SSID" pasword "$PASSWORD"

echo "Hotspot created with SSID: $SSID"
