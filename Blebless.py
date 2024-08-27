import asyncio
from bless import BlessServerBlueZDBus, GATTCharacteristicProperties, GATTCharacteristic

# UUIDs for the service and characteristic
SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef1"
CHARACTERISTIC_UUID = "12345678-1234-5678-1234-56789abcdef2"

# Variable to store the value written to the characteristic
stored_value = 0

async def main():
    # Step 1: Initialize the Bless server with BlueZ backend
    server = BlessServerBlueZDBus(name="BLE Test Server")

    # Step 2: Define a characteristic with read and write properties
    def read_handler():
        squared_value = stored_value ** 2
        print(f"Characteristic read, returning squared value: {squared_value}")
        return squared_value.to_bytes(4, byteorder='little')

    def write_handler(value):
        global stored_value
        stored_value = int.from_bytes(value, byteorder='little')
        print(f"Characteristic written with value: {stored_value}")

    characteristic = GATTCharacteristic(
        CHARACTERISTIC_UUID,
        GATTCharacteristicProperties.read | GATTCharacteristicProperties.write,
        read_handler=read_handler,
        write_handler=write_handler
    )

    # Step 3: Add the characteristic to the server's service
    server.add_service(SERVICE_UUID, [characteristic])

    # Step 4: Start the GATT server
    await server.start()
    print("GATT server started. Waiting for connections...")

    # Keep the server running indefinitely
    await asyncio.get_event_loop().run_forever()

if __name__ == "__main__":
    asyncio.run(main())
