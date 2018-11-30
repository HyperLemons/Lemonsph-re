/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <stdint.h>
#include <atmosphere/version.h>

#include "bootconfig.h"
#include "configitem.h"
#include "interrupt.h"
#include "package2.h"
#include "se.h"
#include "fuse.h"
#include "utils.h"
#include "masterkey.h"
#include "exocfg.h"

static bool g_battery_profile = false;
static bool g_debugmode_override_user = false, g_debugmode_override_priv = false;

uint32_t configitem_set(bool privileged, ConfigItem item, uint64_t value) {
    if (item != CONFIGITEM_BATTERYPROFILE) {
        return 2;
    }
    
    g_battery_profile = (value != 0);
    return 0;
}

bool configitem_is_recovery_boot(void) {
    uint64_t is_recovery_boot;
    if (configitem_get(true, CONFIGITEM_ISRECOVERYBOOT, &is_recovery_boot) != 0) {
        generic_panic();
    }

    return is_recovery_boot != 0;
}

bool configitem_is_retail(void) {
    uint64_t is_retail;
    if (configitem_get(true, CONFIGITEM_ISRETAIL, &is_retail) != 0) {
        generic_panic();
    }

    return is_retail != 0;
}

bool configitem_should_profile_battery(void) {
    return g_battery_profile;
}

uint64_t configitem_get_hardware_type(void) {
    uint64_t hardware_type;
    if (configitem_get(true, CONFIGITEM_HARDWARETYPE, &hardware_type) != 0) {
        generic_panic();
    }
    return hardware_type;
}

void configitem_set_debugmode_override(bool user, bool priv) {
    g_debugmode_override_user = user;
    g_debugmode_override_priv = priv;
}

uint32_t configitem_get(bool privileged, ConfigItem item, uint64_t *p_outvalue) {
    uint32_t result = 0;
    switch (item) {
        case CONFIGITEM_DISABLEPROGRAMVERIFICATION:
            *p_outvalue = (int)(bootconfig_disable_program_verification());
            break;
        case CONFIGITEM_DRAMID:
            *p_outvalue = fuse_get_dram_id();
            break;
        case CONFIGITEM_SECURITYENGINEIRQ:
            /* SE is interrupt #0x2C. */
            *p_outvalue = INTERRUPT_ID_USER_SECURITY_ENGINE;
            break;
        case CONFIGITEM_VERSION:
            /* Always returns maxver - 1 on hardware. */
            *p_outvalue = PACKAGE2_MAXVER_400_410 - 1;
            break;
        case CONFIGITEM_HARDWARETYPE:
            *p_outvalue = fuse_get_hardware_type();
            break;
        case CONFIGITEM_ISRETAIL:
            *p_outvalue = fuse_get_retail_type();
            break;
        case CONFIGITEM_ISRECOVERYBOOT:
            *p_outvalue = (int)(bootconfig_is_recovery_boot());
            break;
        case CONFIGITEM_DEVICEID:
            *p_outvalue = fuse_get_device_id();
            break;
        case CONFIGITEM_BOOTREASON:
            /* For some reason, Nintendo removed it on 4.0 */
            if (exosphere_get_target_firmware() < EXOSPHERE_TARGET_FIRMWARE_400) {
                *p_outvalue = bootconfig_get_boot_reason();
            } else {
                result = 2;
            }
            break;
        case CONFIGITEM_MEMORYARRANGE:
            *p_outvalue = bootconfig_get_memory_arrangement();
            break;
        case CONFIGITEM_ISDEBUGMODE:
            if ((privileged && g_debugmode_override_priv) || (!privileged && g_debugmode_override_user)) {
                *p_outvalue = 1;
            } else {
                *p_outvalue = (int)(bootconfig_is_debug_mode());
            }
            break;
        case CONFIGITEM_KERNELMEMORYCONFIGURATION:
            *p_outvalue = bootconfig_get_kernel_memory_configuration();
            break;
        case CONFIGITEM_BATTERYPROFILE:
            *p_outvalue = (int)g_battery_profile;
            break;
        case CONFIGITEM_ISQUESTUNIT:
            /* Added on 3.0, used to determine whether console is a kiosk unit. */
            if (exosphere_get_target_firmware() >= EXOSPHERE_TARGET_FIRMWARE_300) {
                *p_outvalue = (fuse_get_reserved_odm(4) >> 10) & 1;
            } else {
                result = 2;
            }
            break;
        case CONFIGITEM_NEWHARDWARETYPE_5X:
            /* Added in 5.x, currently hardcoded to 0. */
            if (exosphere_get_target_firmware() >= EXOSPHERE_TARGET_FIRMWARE_500) {
                *p_outvalue = 0;
            } else {
                result = 2;
            }
            break;
        case CONFIGITEM_NEWKEYGENERATION_5X:
            /* Added in 5.x. */
            if (exosphere_get_target_firmware() >= EXOSPHERE_TARGET_FIRMWARE_500) {
                *p_outvalue = fuse_get_5x_key_generation();
            } else {
                result = 2;
            }
            break;
        case CONFIGITEM_PACKAGE2HASH_5X:
            /* Added in 5.x. */
            if (exosphere_get_target_firmware() >= EXOSPHERE_TARGET_FIRMWARE_500 && bootconfig_is_recovery_boot()) {
                bootconfig_get_package2_hash_for_recovery(p_outvalue);
            } else {
                result = 2;
            }
            break;
        case CONFIGITEM_EXOSPHERE_VERSION:
            /* UNOFFICIAL: Gets information about the current exosphere version. */
            *p_outvalue = ((uint64_t)(ATMOSPHERE_RELEASE_VERSION_MAJOR & 0xFF) << 32ull) | 
                          ((uint64_t)(ATMOSPHERE_RELEASE_VERSION_MINOR & 0xFF) << 24ull) |
                          ((uint64_t)(ATMOSPHERE_RELEASE_VERSION_MICRO & 0xFF) << 16ull) |
                          ((uint64_t)(exosphere_get_target_firmware() & 0xFF) << 8ull) |
                          ((uint64_t)(mkey_get_revision() & 0xFF) << 0ull);
            break;
        default:
            result = 2;
            break;
    }
    return result;
}
