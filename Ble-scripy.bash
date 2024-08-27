#!/bin/bash

# Set up Bluetooth interface hci0
bluetoothctl <<EOF
power on
agent on
default-agent
EOF

# Remove existing pairing information
bluetoothctl <<EOF
remove *
EOF

# Start advertising with specific device
bluetoothctl <<EOF
select hci0
discoverable on
pairable on
advertise on
advertise name rs9116-test
EOF

# Function to simulate reading and writing values
gatt_simulation() {
    local value=0
    while true; do
        echo "Waiting for input..."
        read input
        if [[ $input =~ ^[0-9]+$ ]]; then
            value=$input
            squared_value=$((value * value))
            echo "Received value: $value"
            echo "Squared value: $squared_value"
        fi
    done
}

# Run the GATT server simulation
gatt_simulation
