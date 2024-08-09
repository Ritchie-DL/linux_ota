/*
Created by Ritchie TangWei on 2024/8/9.
*/

#include "rk_ab_meta.h"

#include <stdint.h>
#include "my_debug.h"
#include "rk_flash.h"
#include "my_types.h"

#define AB_META_BLOCK_PARTITION_NAME "/dev/block/by-name/misc"
#define META_DATA_OFFSET 2048

static uint32_t g_ab_misc_crc32_tab[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/* Converts a 32-bit unsigned integer from host to big-endian byte order. */
static uint32_t ab_meta_htonl(uint32_t in)
{
    union {
        uint32_t word;
        uint8_t bytes[4];
    } ret;
    
    ret.bytes[0] = (in >> 24) & 0xff;
    ret.bytes[1] = (in >> 16) & 0xff;
    ret.bytes[2] = (in >> 8) & 0xff;
    ret.bytes[3] = in & 0xff;
    
    return ret.word;
}

static uint32_t ab_meta_crc32(uint32_t crc_in, const uint8_t *buf, uint32_t size)
{
    const uint8_t *p = buf;
    uint32_t crc = 0;

    crc = crc_in ^ ~0U;
    while (size--) {
        crc = g_ab_misc_crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }

    return crc ^ ~0U;
}

static uint32_t avb_crc32(const uint8_t *buf, uint32_t size)
{
    return ab_meta_crc32(0, buf, size);
}

static int ab_meta_calc_crc(rk_ab_meta_data_t *data)
{
    uint32_t crc32 = 0;

    if (assert_ptr(data)) {
        return -1;
    }
    crc32 = avb_crc32((const uint8_t *)data, sizeof(rk_ab_meta_data_t) - sizeof(uint32_t));
    crc32 = ab_meta_htonl(crc32);

    return (int)crc32;
}

static int ab_meta_read_data(rk_ab_meta_data_t *data)
{
    int ret = 0;

    if (assert_ptr(data)) {
        return -1;
    }

    int fd = rk_block_open(AB_META_BLOCK_PARTITION_NAME);
    if (fd < 0) {
        dbg_err("rk_block_open failed\n");
        return -1;
    }
    ret = rk_block_read(fd, META_DATA_OFFSET, (uint8_t *) data, sizeof(rk_ab_meta_data_t));
    if (ret < 0) {
        dbg_err("rk_block_read failed\n");
        rk_block_close(fd);
        return -1;
    }
    rk_block_close(fd);

    return ret;
}

static int ab_meta_write_data(rk_ab_meta_data_t *data)
{
    int ret = 0;

    if (assert_ptr(data)) {
        return -1;
    }

    int fd = rk_block_open(AB_META_BLOCK_PARTITION_NAME);
    if (fd < 0) {
        dbg_err("rk_block_open failed\n");
        return -1;
    }
    ret = rk_block_write(fd, META_DATA_OFFSET, (uint8_t *) data, sizeof(rk_ab_meta_data_t));
    if (ret < 0) {
        dbg_err("rk_block_write failed\n");
        rk_block_close(fd);
        return -1;
    }
    rk_block_close(fd);

    return ret;
}

int rk_ab_meta_read_data(rk_ab_meta_data_t *data)
{
    int ret = 0;

    if (assert_ptr(data)) {
        return -1;
    }

    ret = ab_meta_read_data(data);

    return ret;
}

static bool slot_is_bootable(const rk_ab_slot_data_t *slot)
{
    return slot->priority > 0 && (slot->successful_boot || (slot->tries_remaining > 0));
}

static int get_lastboot(const rk_ab_meta_data_t *ab_data)
{
    return ab_data->last_boot;
}

static int get_current_slot(const rk_ab_meta_data_t *ab_data)
{
    static int last_slot_index = -1;
    int slot_index_to_boot;
    int ret = 0;

    if (slot_is_bootable(&ab_data->slots[0]) && slot_is_bootable(&ab_data->slots[1])) {
        if (ab_data->slots[1].priority > ab_data->slots[0].priority) {
            slot_index_to_boot = 1;
        } else {
            slot_index_to_boot = 0;
        }
    } else if (slot_is_bootable(&ab_data->slots[0])) {
        slot_index_to_boot = 0;
    } else if (slot_is_bootable(&ab_data->slots[1])) {
        slot_index_to_boot = 1;
    } else {
        dbg_warn("No bootable slots found, use last boot slot.\n");
        if (get_lastboot(ab_data) == 0) {
            ret = 0;
            goto out;
        } else if (get_lastboot(ab_data) == 1) {
            ret = 1;
            goto out;
        } else {
            return -1;
        }
    }

    if (slot_index_to_boot == 0) {
        ret = 0;
    } else if (slot_index_to_boot == 1) {
        ret = 1;
    }

    if (last_slot_index != slot_index_to_boot) {
        last_slot_index = slot_index_to_boot;
        dbg_info("A/B-slot: %c, successful: %d, tries-remain: %d\n", (ret == 0) ? 'A' : 'B',
               ab_data->slots[slot_index_to_boot].successful_boot, ab_data->slots[slot_index_to_boot].tries_remaining);
    }
out:
    return ret;
}

int rk_ab_meta_get_current_slot(void)
{
    int ret = 0;

    rk_ab_meta_data_t ab_meta_data = {0};
    
    ret = ab_meta_read_data(&ab_meta_data);
    if (ret < 0) {
        dbg_err("ab_meta_read_data failed\n");
        return -1;
    }
    ret = get_current_slot(&ab_meta_data);
    if (ret < 0) {
        dbg_err("get_current_slot failed\n");
        return -1;
    }

    return ret;
}

int rk_ab_meta_active_current_slot(void)
{
    int ret = 0;

    rk_ab_meta_data_t ab_meta_data = {0};

    ret = ab_meta_read_data(&ab_meta_data);
    if (ret < 0) {
        dbg_err("ab_meta_read_data failed\n");
        return -1;
    }
    ab_meta_data.last_boot = ret;
    ab_meta_data.crc32 = ab_meta_calc_crc(&ab_meta_data);
    ret = ab_meta_write_data(&ab_meta_data);
    if (ret < 0) {
        dbg_err("ab_meta_write_data failed\n");
        return -1;
    }

    return ret;
}

int rk_ab_meta_active_another_slot(void)
{
    int ret = 0;
    int current_slot = 0;

    rk_ab_meta_data_t ab_meta_data = {0};

    ret = ab_meta_read_data(&ab_meta_data);
    if (ret < 0) {
        dbg_err("ab_meta_read_data failed\n");
        return -1;
    } else if (ret >= ARRAY_SIZE(ab_meta_data.slots)) {
        dbg_err("invalid slot(%d)\n", ret);
        return -1;
    }
    current_slot = ret;

    ab_meta_data.slots[current_slot].priority = AB_META_MAX_PRIORITY - 1;
    ab_meta_data.slots[1 - current_slot].priority = AB_META_MAX_PRIORITY;
    ab_meta_data.slots[1 - current_slot].tries_remaining = AB_META_MAX_TRIES_REMAINING;
    /* When switch to another slot, assume it is not success boot, try it out! */
    ab_meta_data.slots[1 - current_slot].successful_boot = 0;

    ab_meta_data.last_boot = ret;
    ab_meta_data.crc32 = ab_meta_calc_crc(&ab_meta_data);
    ret = ab_meta_write_data(&ab_meta_data);
    if (ret < 0) {
        dbg_err("ab_meta_write_data failed\n");
        return -1;
    }

    return ret;
}
