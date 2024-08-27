import asyncio
from bless import BlessServer, GATTCharacteristic, GATTService, GATTCharacteristicProperties

# UUIDs for the service and characteristic
SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef1"
CHARACTERISTIC_UUID = "12345678-1234-5678-1234-56789abcdef2"

# Global variable to store the value written to the characteristic
stored_value = 0

async def main():
    # Initialize the Bless server
    server = BlessServer(name="BLE Test Server")

    # Define a characteristic with read and write properties
    def read_handler() -> bytearray:
        global stored_value
        squared_value = stored_value ** 2
        print(f"Characteristic read, returning squared value: {squared_value}")
        return squared_value.to_bytes(4, byteorder='little')

    def write_handler(value: bytearray):
        global stored_value
        stored_value = int.from_bytes(value, byteorder='little')
        print(f"Characteristic written with value: {stored_value}")

    characteristic = GATTCharacteristic(
        uuid=CHARACTERISTIC_UUID,
        properties=GATTCharacteristicProperties.READ | GATTCharacteristicProperties.WRITE,
        read_handler=read_handler,
        write_handler=write_handler,
    )

    # Create a GATT service and add the characteristic to it
    service = GATTService(uuid=SERVICE_UUID, characteristics=[characteristic])

    # Add the service to the server
    server.add_service(service)

    # Start the GATT server
    await server.start()
    print("GATT server started. Waiting for connections...")

    # Keep the server running indefinitely
    await asyncio.get_event_loop().run_forever()

if __name__ == "__main__":
    asyncio.run(main())
