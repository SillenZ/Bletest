import asyncio
from bless import (
    BlessServerBlueZDBus,
    GATTCharacteristicProperties,
    GATTCharacteristic,
    GATTService,
)

# UUIDs for the service and characteristic
SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef1"
CHARACTERISTIC_UUID = "12345678-1234-5678-1234-56789abcdef2"

# Variable to store the value written to the characteristic
stored_value = 0

async def main():
    # Step 1: Initialize the Bless server with BlueZ backend
    server = BlessServerBlueZDBus(name="BLE Test Server")

    # Step 2: Define a read handler for the characteristic
    def read_handler() -> bytearray:
        global stored_value
        squared_value = stored_value ** 2
        print(f"Characteristic read, returning squared value: {squared_value}")
        return squared_value.to_bytes(4, byteorder='little')

    # Step 3: Define a write handler for the characteristic
    def write_handler(value: bytearray):
        global stored_value
        stored_value = int.from_bytes(value, byteorder='little')
        print(f"Characteristic written with value: {stored_value}")

    # Step 4: Create the GATT characteristic with read and write properties
    characteristic = GATTCharacteristic(
        CHARACTERISTIC_UUID,
        properties=(
            GATTCharacteristicProperties.READ |
            GATTCharacteristicProperties.WRITE
        ),
        read_handler=read_handler,
        write_handler=write_handler,
    )

    # Step 5: Create the GATT service and add the characteristic to it
    service = GATTService(SERVICE_UUID, [characteristic])

    # Step 6: Add the service to the server
    server.add_service(service)

    # Step 7: Start the GATT server
    await server.start()
    print("GATT server started. Waiting for connections...")

    # Keep the server running indefinitely
    await asyncio.get_event_loop().run_forever()

if __name__ == "__main__":
    asyncio.run(main())
