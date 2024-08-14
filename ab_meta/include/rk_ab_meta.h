/*
Created by Ritchie TangWei on 2024/8/9.
*/

#ifndef __RK_AB_META_H
#define __RK_AB_META_H

#if defined (__cplusplus)
extern "C" {
#endif
    
#define AB_META_MAGIC "\0AB0"
#define AB_META_MAGIC_LEN 4
#define AB_META_MAX_PRIORITY 15
#define AB_META_MAX_TRIES_REMAINING 7

typedef struct {
    /* Slot priority. Valid values range from 0 to AVB_AB_MAX_PRIORITY,
     * both inclusive with 1 being the lowest and AVB_AB_MAX_PRIORITY
     * being the highest. The special value 0 is used to indicate the
     * slot is not able to boot.
     */
    unsigned char priority;//0,14,15

    /* Number of times left attempting to boot this slot ranging from 0
     * to AVB_AB_MAX_TRIES_REMAINING.
     */
    unsigned char tries_remaining;//7--,成功启动，设为0

    /* Non-zero if this slot has booted successfully, 0 otherwise. */
    unsigned char successful_boot;//0,1

    /* Reserved for future use. */
    unsigned char reserved[1];
} rk_ab_slot_data_t;

/* Struct used for recording A/B metadata.
 *
 * When serialized, data is stored in network byte-order.
 */
typedef struct {
    /* Magic number used for identification - see AB_META_MAGIC. */
    unsigned char magic[AB_META_MAGIC_LEN];

    /* Version of on-disk struct - see AVB_AB_{MAJOR,MINOR}_VERSION. */
    unsigned char version_major; //AVB_AB_MAJOR_VERSION
    unsigned char version_minor; //AVB_AB_MINOR_VERSION

    /* Padding to ensure |slots| field start eight bytes in. */
    unsigned char reserved1[2];

    /* Per-slot metadata. */
    rk_ab_slot_data_t slots[2];

    /* Reserved for future use. */
    unsigned char last_boot;//默认a，上一次成功启动slot的标志位，0-->a，1-->b
    unsigned char reserved2[11];

    /* CRC32 of all 28 bytes preceding this field. */
    unsigned int crc32;
} rk_ab_meta_data_t;

int rk_ab_meta_read_data(rk_ab_meta_data_t *data);
int rk_ab_meta_get_running_slot(void);
int rk_ab_meta_get_current_slot(void);
char rk_ab_meta_get_suffix_by_slot(int slot);
int rk_ab_meta_active_current_slot(void);
int rk_ab_meta_active_another_slot(void);

#if defined (__cplusplus)
}
#endif

#endif //__RK_AB_META_H
