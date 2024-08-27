#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

static const char *SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0";
static const char *ADAPTER_PATH = "/org/bluez/hci0";

// Define a structure to hold the advertisement details
typedef struct {
    GDBusConnection *connection;
    char *path;
} Advertisement;

// Function to register the advertisement
void register_advertisement(Advertisement *ad) {
    GError *error = NULL;

    GVariantBuilder *service_uuids_builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
    g_variant_builder_add(service_uuids_builder, "s", SERVICE_UUID);

    GVariantBuilder *properties_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(properties_builder, "{sv}", "Type", g_variant_new_string("peripheral"));
    g_variant_builder_add(properties_builder, "{sv}", "ServiceUUIDs", g_variant_new("as", service_uuids_builder));
    g_variant_builder_add(properties_builder, "{sv}", "LocalName", g_variant_new_string("test-rs9116"));

    g_dbus_connection_call_sync(ad->connection,
                                "org.bluez",
                                ADAPTER_PATH,
                                "org.bluez.LEAdvertisingManager1",
                                "RegisterAdvertisement",
                                g_variant_new("(oa{sv})", ad->path, g_variant_builder_end(properties_builder)),
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (error) {
        g_printerr("Failed to register advertisement: %s\n", error->message);
        g_error_free(error);
    } else {
        g_print("Advertisement registered successfully with name 'test-rs9116'\n");
    }

    g_variant_builder_unref(service_uuids_builder);
    g_variant_builder_unref(properties_builder);
}

// Function to create and initialize an advertisement object
Advertisement* create_advertisement(GDBusConnection *connection) {
    Advertisement *ad = g_new0(Advertisement, 1);
    ad->connection = connection;
    ad->path = g_strdup("/org/bluez/advertisement0");  // Unique path for the advertisement object
    return ad;
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

    // Create the advertisement object
    Advertisement *ad = create_advertisement(connection);

    // Register the advertisement with the desired local name
    register_advertisement(ad);

    // Start the main loop
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    // Clean up
    g_free(ad->path);
    g_free(ad);

    return 0;
}
