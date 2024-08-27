#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

static const char *SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0";
static const char *ADAPTER_PATH = "/org/bluez/hci0";

typedef struct {
    GDBusConnection *connection;
    char *path;
} Advertisement;

void register_advertisement(Advertisement *ad) {
    GError *error = NULL;

    // Build the Service UUIDs array
    GVariantBuilder service_uuids_builder;
    g_variant_builder_init(&service_uuids_builder, G_VARIANT_TYPE("as"));
    g_variant_builder_add(&service_uuids_builder, "s", SERVICE_UUID);
    GVariant *service_uuids = g_variant_builder_end(&service_uuids_builder);

    // Build the properties dictionary
    GVariantBuilder properties_builder;
    g_variant_builder_init(&properties_builder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&properties_builder, "{sv}", "Type", g_variant_new_string("peripheral"));
    g_variant_builder_add(&properties_builder, "{sv}", "ServiceUUIDs", service_uuids);
    g_variant_builder_add(&properties_builder, "{sv}", "LocalName", g_variant_new_string("test-rs9116"));
    GVariant *properties = g_variant_builder_end(&properties_builder);

    // Register the advertisement
    g_dbus_connection_call_sync(ad->connection,
                                "org.bluez",
                                ADAPTER_PATH,
                                "org.bluez.LEAdvertisingManager1",
                                "RegisterAdvertisement",
                                g_variant_new("(oa{sv})", ad->path, properties),
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
}

Advertisement* create_advertisement(GDBusConnection *connection) {
    Advertisement *ad = g_new0(Advertisement, 1);
    ad->connection = connection;
    ad->path = g_strdup("/org/bluez/advertisement0");
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

    Advertisement *ad = create_advertisement(connection);
    register_advertisement(ad);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    g_free(ad->path);
    g_free(ad);

    return 0;
}
