@echo off

set SSID=FMG_SERVER
set PASSWORD=AIIMS_FMG_SERVER

netsh wlan set hostednetwork mode=allow ssid=%SSID% key=%PASSWORD%
netsh wlan start hopstednetwork

echo Hotspot created with SSID: %SSID%
pause
