#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

// Function to change the adapter name
void set_adapter_name(GDBusConnection *connection, const char *adapter_path, const char *new_name) {
    GVariant *result;
    GError *error = NULL;

    // The Set method requires the interface name, property name, and value as parameters
    result = g_dbus_connection_call_sync(connection,
                                         "org.bluez",
                                         adapter_path,
                                         "org.freedesktop.DBus.Properties",
                                         "Set",
                                         g_variant_new("(ssv)", "org.bluez.Adapter1", "Alias", g_variant_new_variant(g_variant_new_string(new_name))),
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);
    if (error) {
        g_printerr("Failed to set adapter name: %s\n", error->message);
        g_error_free(error);
    } else {
        g_print("Adapter name set to %s\n", new_name);
    }
}

// Function to make the adapter discoverable and pairable
void set_discoverable_and_pairable(GDBusConnection *connection, const char *adapter_path) {
    GError *error = NULL;

    // Make the adapter discoverable
    g_dbus_connection_call_sync(connection,
                                "org.bluez",
                                adapter_path,
                                "org.freedesktop.DBus.Properties",
                                "Set",
                                g_variant_new("(ssv)", "org.bluez.Adapter1", "Discoverable", g_variant_new_variant(g_variant_new_boolean(TRUE))),
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    if (error) {
        g_printerr("Failed to make adapter discoverable: %s\n", error->message);
        g_error_free(error);
    } else {
        g_print("Adapter is discoverable\n");
    }

    // Make the adapter pairable
    g_dbus_connection_call_sync(connection,
                                "org.bluez",
                                adapter_path,
                                "org.freedesktop.DBus.Properties",
                                "Set",
                                g_variant_new("(ssv)", "org.bluez.Adapter1", "Pairable", g_variant_new_variant(g_variant_new_boolean(TRUE))),
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    if (error) {
        g_printerr("Failed to make adapter pairable: %s\n", error->message);
        g_error_free(error);
    } else {
        g_print("Adapter is pairable\n");
    }
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GDBusConnection *connection;
    GError *error = NULL;

    connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (!connection) {
        g_printerr("Failed to connect to system bus: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    // Change adapter name to "test-rs9116"
    set_adapter_name(connection, "/org/bluez/hci0", "test-rs9116");

    // Make the device discoverable and pairable
    set_discoverable_and_pairable(connection, "/org/bluez/hci0");

    // Your additional GATT server and advertisement code goes here...

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
