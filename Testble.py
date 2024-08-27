#!/usr/bin/env python3

from pydbus import SystemBus
from gi.repository import GLib
import dbus
import dbus.exceptions
import dbus.service
import dbus.mainloop.glib
from dbus.mainloop.glib import DBusGMainLoop

GATT_CHRC_IFACE = 'org.bluez.GattCharacteristic1'
GATT_SERVICE_IFACE = 'org.bluez.GattService1'
LE_ADVERT_IFACE = 'org.bluez.LEAdvertisement1'

class Application(dbus.service.Object):
    PATH_BASE = '/org/bluez/example/service'

    def __init__(self, bus):
        self.path = '/'
        self.services = []
        dbus.service.Object.__init__(self, bus, self.path)

    def get_path(self):
        return dbus.ObjectPath(self.path)

    def add_service(self, service):
        self.services.append(service)

    def get_service(self, index):
        return self.services[index]

    @dbus.service.method('org.freedesktop.DBus.ObjectManager',
                         out_signature='a{oa{sa{sv}}}')
    def GetManagedObjects(self):
        response = {}
        for service in self.services:
            response[service.get_path()] = service.get_properties()
            chrcs = service.get_characteristics()
            for chrc in chrcs:
                response[chrc.get_path()] = chrc.get_properties()

        return response


class Service(dbus.service.Object):
    def __init__(self, bus, index, uuid, primary):
        self.path = self.PATH_BASE + str(index)
        self.bus = bus
        self.uuid = uuid
        self.primary = primary
        self.characteristics = []
        dbus.service.Object.__init__(self, bus, self.path)

    def get_properties(self):
        return {
            GATT_SERVICE_IFACE: {
                'UUID': self.uuid,
                'Primary': self.primary,
                'Characteristics': dbus.Array(self.get_characteristic_paths(),
                                              signature='o')
            }
        }

    def get_path(self):
        return dbus.ObjectPath(self.path)

    def add_characteristic(self, characteristic):
        self.characteristics.append(characteristic)

    def get_characteristics(self):
        return self.characteristics

    def get_characteristic_paths(self):
        result = []
        for chrc in self.get_characteristics():
            result.append(chrc.get_path())
        return result


class Characteristic(dbus.service.Object):
    def __init__(self, bus, index, uuid, flags, service):
        self.path = service.path + '/char' + str(index)
        self.bus = bus
        self.uuid = uuid
        self.flags = flags
        self.service = service
        dbus.service.Object.__init__(self, bus, self.path)
        self.value = 0

    def ReadValue(self, options):
        print('Read: {}'.format(self.value))
        squared_value = self.value * self.value
        return dbus.Array([dbus.Byte(squared_value)], signature='y')

    def WriteValue(self, value, options):
        self.value = int(value[0])
        print('Write: {}'.format(self.value))

    def get_properties(self):
        return {
            GATT_CHRC_IFACE: {
                'Service': self.service.get_path(),
                'UUID': self.uuid,
                'Flags': self.flags,
            }
        }

    def get_path(self):
        return dbus.ObjectPath(self.path)


def main():
    DBusGMainLoop(set_as_default=True)

    bus = SystemBus()
    adapter = bus.get('org.bluez', '/org/bluez/hci0')
    adapter.Set('org.bluez.Adapter1', 'Powered', dbus.Boolean(1))

    app = Application(bus)
    service = Service(bus, 0, '12345678-1234-5678-1234-56789abcdef0', True)
    app.add_service(service)

    characteristic = Characteristic(bus, 0, '12345678-1234-5678-1234-56789abcdef1',
                                    ['read', 'write'], service)
    service.add_characteristic(characteristic)

    bus_name = dbus.service.BusName('org.bluez', bus)
    app_path = '/org/bluez/example/service0'
    app.bus.request_name('org.bluez.example.service0')
    bus.export_object(app_path, app)

    mainloop = GLib.MainLoop()
    print('GATT server running...')
    mainloop.run()


if __name__ == '__main__':
    main()
