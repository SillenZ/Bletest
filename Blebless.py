import bless
import asyncio

# UUIDs for the service and characteristic
SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef1"
CHARACTERISTIC_UUID = "12345678-1234-5678-1234-56789abcdef2"

# Variable to store the value written to the characteristic
stored_value = 0

async def run_server():
    # Step 1: Initialize the Bless server
    server = bless.BlessServer(name="BLE Test Server")

    # Step 2: Define the GATT service and characteristic
    @server.characteristic(SERVICE_UUID, CHARACTERISTIC_UUID)
    def on_characteristic_write(value: bytearray):
        global stored_value
        stored_value = int.from_bytes(value, byteorder='little')
        print(f"Received value: {stored_value}")

    @server.characteristic(SERVICE_UUID, CHARACTERISTIC_UUID)
    def on_characteristic_read() -> bytearray:
        squared_value = stored_value ** 2
        print(f"Returning squared value: {squared_value}")
        return squared_value.to_bytes(4, byteorder='little')

    # Step 3: Start the GATT server
    await server.start()
    print("GATT server started. Waiting for connections...")

    # Keep the server running indefinitely
    await asyncio.get_event_loop().run_forever()

if __name__ == "__main__":
    # Run the server
    asyncio.run(run_server())
