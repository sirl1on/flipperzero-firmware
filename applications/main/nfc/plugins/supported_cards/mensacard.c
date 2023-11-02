/*
 * mensacard.c - Parser for Mensa payment cards used by a lot of german universities.
 *
 * Tested with cards from:
 * - Technische Universität Braunschweig
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>
#include <applications/services/locale/locale.h>
#include <furi_hal_rtc.h>

#include <lib/nfc/protocols/mf_desfire/mf_desfire.h>

static const MfDesfireApplicationId mensacard_app_id = {.data = {0x5f, 0x84, 0x15}};

static const MfDesfireFileId mensacard_file_id = 0x01;

static double rawDataToDouble(const char* data) {
    if(data == 0) {
        return 0.0;
    }
    unsigned int value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    return (double)value / 1000;
}

static bool mensacard_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfDesfireData* data = nfc_device_get_data(device, NfcProtocolMfDesfire);
    bool parsed = false;

    do {
        const MfDesfireApplication* app = mf_desfire_get_application(data, &mensacard_app_id);
        if(app == NULL) break;

        const MfDesfireFileSettings* file_settings =
            mf_desfire_get_file_settings(app, &mensacard_file_id);
        if(file_settings == NULL || file_settings->type != MfDesfireFileTypeValue) {
            break;
        }

        const MfDesfireFileData* file_data = mf_desfire_get_file_data(app, &mensacard_file_id);
        if(file_data == NULL) break;

        const char* data = simple_array_cget_data(file_data->data);

        furi_string_printf(parsed_data, "Balance: %.2f EUR", rawDataToDouble(data));
        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin mensacard_plugin = {
    .protocol = NfcProtocolMfDesfire,
    .verify = NULL,
    .read = NULL,
    .parse = mensacard_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor mensacard_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &mensacard_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* mensacard_plugin_ep() {
    return &mensacard_plugin_descriptor;
}
