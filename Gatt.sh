#!/bin/bash

# Set up Bluetooth interface hci0
echo "Setting up Bluetooth adapter hci0..."
bluetoothctl <<EOF
power on
agent on
default-agent
EOF

# Remove existing pairing information
echo "Removing existing Bluetooth pairings..."
bluetoothctl <<EOF
remove *
EOF

# Set the Bluetooth device name
echo "Setting Bluetooth alias to rs9116-test..."
bluetoothctl <<EOF
select hci0
system-alias rs9116-test
EOF

# Make the device discoverable and pairable
echo "Making device discoverable and pairable..."
bluetoothctl <<EOF
discoverable on
pairable on
EOF

# Start advertising
echo "Starting advertising..."
bluetoothctl <<EOF
advertise on
EOF

# Run the GATT server Python script
echo "Starting the GATT server..."
sudo python3 gatt_server.py
