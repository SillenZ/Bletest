from pydbus import SystemBus
from gi.repository import GLib
from dbus.mainloop.glib import DBusGMainLoop
import os

# Configuration
PASSKEY = "123456"  # Replace with your passkey
SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef1"
CHARACTERISTIC_UUID = "12345678-1234-5678-1234-56789abcdef2"

# Variable to store the value written to the characteristic
stored_value = 0

def list_adapters():
    """List all available Bluetooth adapters."""
    bus = SystemBus()
    manager = bus.get(".org.bluez", "/org/bluez")

    adapters = manager.GetManagedObjects()
    adapter_list = []

    print("Available Bluetooth Adapters:")
    for path, ifaces in adapters.items():
        adapter = ifaces.get("org.bluez.Adapter1")
        if adapter:
            adapter_list.append((path, adapter["Address"], adapter.get("Name", "Unknown")))
            print(f"{len(adapter_list)}. {adapter['Address']} ({adapter.get('Name', 'Unknown')}) at {path}")

    return adapter_list

def select_adapter(adapter_list):
    """Prompt the user to select a Bluetooth adapter."""
    if not adapter_list:
        print("No Bluetooth adapters found!")
        return None
    
    while True:
        try:
            choice = int(input("Select the Bluetooth adapter (by number): "))
            if 1 <= choice <= len(adapter_list):
                selected_adapter = adapter_list[choice - 1]
                print(f"Selected Adapter: {selected_adapter[1]} ({selected_adapter[2]})")
                return selected_adapter[0]  # Return the adapter path
            else:
                print(f"Please enter a number between 1 and {len(adapter_list)}.")
        except ValueError:
            print("Invalid input. Please enter a number.")

def setup_adapter(adapter_path):
    """Set up the selected Bluetooth adapter."""
    adapter_name = adapter_path.split('/')[-1]
    print(f"Configuring Bluetooth adapter {adapter_name}...")
    os.system(f"sudo hciconfig {adapter_name} up")
    os.system(f"sudo bluetoothctl -- {adapter_name} << EOF\n"
              "power on\n"
              "discoverable on\n"
              "pairable on\n"
              "agent NoInputNoOutput\n"
              "default-agent\n"
              "EOF")

def run_pairing_agent():
    """Run a custom pairing agent that uses a fixed passkey."""
    bus = SystemBus()
    manager = bus.get(".AgentManager1")
    agent_path = "/test/agent"

    class PairingAgent(dbus.service.Object):
        def __init__(self, bus, path):
            dbus.service.Object.__init__(self, bus, path)

        @dbus.service.method("org.bluez.Agent1", in_signature="o", out_signature="")
        def RequestPinCode(self, device):
            print(f"Pairing request from {device}, providing passkey: {PASSKEY}")
            return PASSKEY

        @dbus.service.method("org.bluez.Agent1", in_signature="ouq", out_signature="")
        def DisplayPasskey(self, device, passkey, entered):
            print(f"Displaying passkey: {PASSKEY} to {device}")

        @dbus.service.method("org.bluez.Agent1", in_signature="os", out_signature="")
        def RequestConfirmation(self, device, passkey):
            print(f"RequestConfirmation for {device}, passkey: {PASSKEY}")
            return True

    # Register the agent
    agent = PairingAgent(bus, agent_path)
    manager.RegisterAgent(agent_path, "NoInputNoOutput")
    manager.RequestDefaultAgent(agent_path)
    print("Pairing agent registered and running...")

def create_gatt_server():
    """Create and run the GATT server."""
    DBusGMainLoop(set_as_default=True)
    bus = SystemBus()

    class Characteristic(dbus.service.Object):
        def __init__(self, bus):
            self.path = "/org/bluez/example/characteristic0"
            self.interface = "org.bluez.GattCharacteristic1"
            dbus.service.Object.__init__(self, bus, self.path)

        @dbus.service.method(self.interface, in_signature="", out_signature="ay")
        def ReadValue(self, options):
            global stored_value
            squared_value = stored_value ** 2
            print(f"Read request, returning squared value: {squared_value}")
            return dbus.Array([dbus.Byte(b) for b in squared_value.to_bytes(4, byteorder='little')], signature='y')

        @dbus.service.method(self.interface, in_signature="aya{sv}", out_signature="")
        def WriteValue(self, value, options):
            global stored_value
            stored_value = int.from_bytes(bytearray(value), byteorder='little')
            print(f"Write request, received value: {stored_value}")

        def get_properties(self):
            return {
                "UUID": CHARACTERISTIC_UUID,
                "Service": f"/org/bluez/example/service0",
                "Flags": ["read", "write"],
                "Value": [],
            }

    class Service(dbus.service.Object):
        def __init__(self, bus):
            self.path = "/org/bluez/example/service0"
            self.characteristics = [Characteristic(bus)]
            dbus.service.Object.__init__(self, bus, self.path)

        def get_properties(self):
            return {
                "UUID": SERVICE_UUID,
                "Primary": True,
            }

        def get_characteristics(self):
            return self.characteristics

    class Application(dbus.service.Object):
        def __init__(self, bus):
            self.path = "/"
            self.services = [Service(bus)]
            dbus.service.Object.__init__(self, bus, self.path)

        @dbus.service.method("org.freedesktop.DBus.ObjectManager", out_signature="a{oa{sa{sv}}}")
        def GetManagedObjects(self):
            response = {}
            for service in self.services:
                response[service.path] = service.get_properties()
                for chrc in service.get_characteristics():
                    response[chrc.path] = chrc.get_properties()
            return response

    # Register the GATT application
    app = Application(bus)
    manager = bus.get(".GattManager1")
    print("Registering GATT application...")
    manager.RegisterApplication("/", {}, reply_handler=print, error_handler=print)

    # Run the GLib main loop
    loop = GLib.MainLoop()
    loop.run()

def main():
    # List all available Bluetooth adapters
    adapter_list = list_adapters()

    # Select a Bluetooth adapter
    selected_adapter_path = select_adapter(adapter_list)

    # Set up the selected adapter and run the pairing agent
    if selected_adapter_path:
        setup_adapter(selected_adapter_path)
        run_pairing_agent()

        # Create and run the GATT server
        create_gatt_server()

if __name__ == "__main__":
    main()
