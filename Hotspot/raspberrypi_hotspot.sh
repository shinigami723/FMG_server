#!/bin/bash

SSID="FMG_SERVER"
PASSWORD="AIIMS_FMG_SERVER"

# Install necessary packages
sudo apt-get update
sudo apt-get install -y hostapd dnsmasq

# Configure DHCP
echo -e "interface wlan0\nstatic ip_address=192.168.4.1/24" | sudo tee -a /etc/dhcpcd.conf
sudo service dhcpcd restart

# Configure Hostapd
cat <<EOT | sudo tee /etc/hostapd/hostapd.conf
interface=wlan0
driver=nl80211
ssid=$SSID
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=$PASSWORD
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOT

# Update Hostapd defaults
sudo sed -i 's|#DAEMON_CONF=""|DAEMON_CONF="/etc/hostapd/hostapd.conf"|' /etc/default/hostapd

# Configure Dnsmasq
cat <<EOT | sudo tee /etc/dnsmasq.conf
interface=wlan0
dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h
EOT

# Enable and start services
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd
sudo systemctl restart dnsmasq

echo "Hotspot created with SSID: $SSID"
