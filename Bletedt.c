#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

// Global variable to store squared value
static int squared_value = 0;

// UUIDs and Paths
static const char *SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0";
static const char *SERVICE_PATH = "/org/bluez/example/service0";
static const char *CHARACTERISTIC_PATH = "/org/bluez/example/service0/char0";

// Function to change the adapter name
void set_adapter_name(GDBusConnection *connection, const char *adapter_path, const char *new_name) {
    GVariant *result;
    GError *error = NULL;

    result = g_dbus_connection_call_sync(connection,
                                         "org.bluez",
                                         adapter_path,
                                         "org.freedesktop.DBus.Properties",
                                         "Set",
                                         g_variant_new("(ssv)", "org.bluez.Adapter1", "Alias", g_variant_new_string(new_name)),
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
                                g_variant_new("(ssv)", "org.bluez.Adapter1", "Discoverable", g_variant_new_boolean(TRUE)),
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
                                g_variant_new("(ssv)", "org.bluez.Adapter1", "Pairable", g_variant_new_boolean(TRUE)),
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

// Function to register advertisement
void register_advertisement(GDBusConnection *connection) {
    GVariant *result;
    GError *error = NULL;

    GVariantBuilder *service_uuids_builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
    g_variant_builder_add(service_uuids_builder, "s", SERVICE_UUID);

    GVariantBuilder *properties_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(properties_builder, "{sv}", "Type", g_variant_new_string("peripheral"));
    g_variant_builder_add(properties_builder, "{sv}", "ServiceUUIDs", g_variant_new("as", service_uuids_builder));

    result = g_dbus_connection_call_sync(connection,
                                         "org.bluez",
                                         "/org/bluez/hci1",
                                         "org.bluez.LEAdvertisingManager1",
                                         "RegisterAdvertisement",
                                         g_variant_new("(oa{sv})", "/org/bluez/advertisement0", g_variant_builder_end(properties_builder)),
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);
    if (error) {
        g_printerr("Failed to register advertisement: %s\n", error->message);
        g_error_free(error);
    } else {
        g_print("Advertisement registered successfully\n");
    }

    g_variant_builder_unref(service_uuids_builder);
    g_variant_builder_unref(properties_builder);
}

// Handle write requests
static void handle_characteristic_write(GDBusConnection *connection, GVariant *value) {
    const gchar *str_value = g_variant_get_string(value, NULL);
    int input_value = atoi(str_value);
    squared_value = input_value * input_value;
    printf("Received value: %d, squared value: %d\n", input_value, squared_value);
}

// Handle read requests
static GVariant *handle_characteristic_read(GDBusConnection *connection) {
    gchar *value_str = g_strdup_printf("%d", squared_value);
    GVariant *response = g_variant_new_string(value_str);
    g_free(value_str);
    printf("Read request: returned value %d\n", squared_value);
    return response;
}

// Method call handler
static void on_method_call(GDBusConnection *connection, const gchar *sender, const gchar *object_path,
                           const gchar *interface_name, const gchar *method_name, GVariant *parameters,
                           GDBusMethodInvocation *invocation, gpointer user_data) {
    if (g_strcmp0(method_name, "ReadValue") == 0) {
        GVariant *result = handle_characteristic_read(connection);
        g_dbus_method_invocation_return_value(invocation, g_variant_new_tuple(&result, 1));
    } else if (g_strcmp0(method_name, "WriteValue") == 0) {
        GVariantIter *iter;
        GVariant *value;
        g_variant_get(parameters, "(ay)", &iter);
        while (g_variant_iter_loop(iter, "y", &value)) {
            handle_characteristic_write(connection, g_variant_ref(value));
        }
        g_dbus_method_invocation_return_value(invocation, NULL);
    }
}

// Main loop and setup
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
    set_adapter_name(connection, "/org/bluez/hci1", "test-rs9116");

    // Make the device discoverable and pairable
    set_discoverable_and_pairable(connection, "/org/bluez/hci1");

    // Register the advertisement
    register_advertisement(connection);

    // Set up GATT characteristic handling
    GDBusNodeInfo *introspection_data = g_dbus_node_info_new_for_xml(
        "<node>"
        "  <interface name='org.bluez.GattCharacteristic1'>"
        "    <method name='ReadValue'>"
        "      <arg type='ay' name='value' direction='out'/>"
        "    </method>"
        "    <method name='WriteValue'>"
        "      <arg type='ay' name='value' direction='in'/>"
        "    </method>"
        "  </interface>"
        "</node>", NULL);

    g_dbus_connection_register_object(connection, CHARACTERISTIC_PATH,
                                      introspection_data->interfaces[0],
                                      &(GDBusInterfaceVTable){.method_call = on_method_call},
                                      NULL, NULL, &error);

    if (error) {
        g_printerr("Failed to register GATT characteristic: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    g_dbus_node_info_unref(introspection_data);
    return 0;
}
