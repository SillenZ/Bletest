#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

static int squared_value = 0;

static const char *CHARACTERISTIC_UUID = "12345678-1234-5678-1234-56789abcdef0";
static const char *SERVICE_PATH = "/org/bluez/example/service0";
static const char *CHARACTERISTIC_PATH = "/org/bluez/example/service0/char0";

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
