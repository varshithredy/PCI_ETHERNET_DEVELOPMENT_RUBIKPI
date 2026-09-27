#include "my_pci_stage7.h"

/* These values are part of the RTL8168H firmware image format. */
enum action_type {
    ACTION_READ            = 0x0,
    ACTION_OR              = 0x1,
    ACTION_AND             = 0x2,
    ACTION_BACK             = 0x3,
    ACTION_ACCESS           = 0x4,
    ACTION_CLEAR_COUNT     = 0x7,
    ACTION_WRITE           = 0x8,
    ACTION_COUNT_SKIP      = 0x9,
    ACTION_EQUAL_SKIP      = 0xa,
    ACTION_NOT_EQUAL_SKIP  = 0xb,
    ACTION_WRITE_PREVIOUS  = 0xc,
    ACTION_SKIP             = 0xd,
    ACTION_DELAY            = 0xe,
};

struct image_header {
    u32 magic;
    char version[FW_VERSION_SIZE];
    __le32 start;
    __le32 length;
    u8 checksum;
} __packed;

static int image_info(const struct firmware *fw, const u8 **code,
                      size_t *count, char *version, size_t version_size)
{
    const struct image_header *h;
    u8 sum = 0;
    size_t i;
    u32 start, length;

    if (!fw || fw->size < sizeof(__le32))
        return -1;

    h = (const struct image_header *)fw->data;

    if (h->magic == 0) {
        if (fw->size < sizeof(*h))
            return -1;

        for (i = 0; i < fw->size; i++)
            sum += fw->data[i];
        if (sum != 0)
            return -1;

        start = le32_to_cpu(h->start);
        length = le32_to_cpu(h->length);

        if (start > fw->size)
            return -1;
        if (length > (fw->size - start) / sizeof(__le32))
            return -1;

        if (version && version_size)
            strscpy(version, h->version, version_size);

        *code = fw->data + start;
        *count = length;
    } else {
        if (fw->size % sizeof(__le32))
            return -1;

        if (version && version_size)
            strscpy(version, FW_NAME, version_size);

        *code = fw->data;
        *count = fw->size / sizeof(__le32);
    }

    if (*count > FW_MAX_ACTIONS)
        return -1;

    return 0;
}

static int image_actions_ok(const u8 *code, size_t count)
{
    size_t i;

    for (i = 0; i < count; i++) {
        u32 action = le32_to_cpup((const __le32 *)(code + i * 4));
        u32 data = action & 0xffff;
        u32 reg = (action >> 16) & 0x0fff;
        u32 op = action >> 28;

        switch (op) {
        case ACTION_READ:
        case ACTION_OR:
        case ACTION_AND:
        case ACTION_CLEAR_COUNT:
        case ACTION_WRITE:
        case ACTION_WRITE_PREVIOUS:
        case ACTION_DELAY:
            break;
        case ACTION_ACCESS:
            if (data > 1)
                return -1;
            break;
        case ACTION_BACK:
            if (reg > i)
                return -1;
            break;
        case ACTION_COUNT_SKIP:
            if (i + 2 >= count)
                return -1;
            break;
        case ACTION_EQUAL_SKIP:
        case ACTION_NOT_EQUAL_SKIP:
        case ACTION_SKIP:
            if (i + 1 + reg >= count)
                return -1;
            break;
        default:
            return -1;
        }
    }

    return 0;
}

int firmware_load(struct my_nic *nic, const char *name,
                  const struct firmware **fw, char *version,
                  size_t version_size, u32 *actions)
{
    const u8 *code;
    size_t count;

    if (request_firmware(fw, name, &nic->pdev->dev))
        return -1;

    if (image_info(*fw, &code, &count, version, version_size))
        goto bad;

    if (image_actions_ok(code, count))
        goto bad;

    if (actions)
        *actions = count;

    return 0;

bad:
    release_firmware(*fw);
    *fw = NULL;
    return -1;
}

static const char *access_name(bool mac_mode)
{
    return mac_mode ? "mac" : "phy";
}

int firmware_execute(struct my_nic *nic, const struct firmware *fw,
                     char *version, size_t version_size, u32 *actions)
{
    const u8 *code;
    size_t count, index = 0;
    int previous = 0;
    int read_count = 0;
    bool mac_mode = false;
    int (*read_fn)(struct my_nic *, u16, u16 *) = phy_read;
    int (*write_fn)(struct my_nic *, u16, u16) = phy_write;

    if (image_info(fw, &code, &count, version, version_size))
        return -1;
    if (image_actions_ok(code, count))
        return -1;

    while (index < count) {
        size_t action_index = index;
        u32 action = le32_to_cpup((const __le32 *)(code + index * 4));
        u32 data = action & 0xffff;
        u32 reg = (action >> 16) & 0x0fff;
        u32 op = action >> 28;
        int ret;

        switch (op) {
        case ACTION_READ: {
            u16 value;

            ret = read_fn(nic, reg, &value);
            if (ret) {
                dev_err(&nic->pdev->dev,
                        "Stage 6: action %zu READ failed, mode=%s reg=0x%x\n",
                        action_index, access_name(mac_mode), reg);
                return -1;
            }
            previous = value;
            read_count++;
            index++;
            break;
        }

        case ACTION_OR:
            previous |= data;
            index++;
            break;

        case ACTION_AND:
            previous &= data;
            index++;
            break;

        case ACTION_BACK:
            index -= reg + 1;
            break;

        case ACTION_ACCESS:
            mac_mode = !!data;
            if (mac_mode) {
                read_fn = mac_read;
                write_fn = mac_write;
            } else {
                read_fn = phy_read;
                write_fn = phy_write;
            }
            dev_dbg(&nic->pdev->dev,
                    "Stage 6: action %zu access=%s\n",
                    action_index, access_name(mac_mode));
            index++;
            break;

        case ACTION_CLEAR_COUNT:
            read_count = 0;
            index++;
            break;

        case ACTION_WRITE:
            ret = write_fn(nic, reg, (u16)data);
            if (ret) {
                dev_err(&nic->pdev->dev,
                        "Stage 6: action %zu WRITE failed, mode=%s reg=0x%x data=0x%04x\n",
                        action_index, access_name(mac_mode), reg, data);
                return -1;
            }
            index++;
            break;

        case ACTION_COUNT_SKIP:
            index += (read_count == data) ? 2 : 1;
            break;

        case ACTION_EQUAL_SKIP:
            if (previous == (int)data)
                index += reg + 1;
            else
                index++;
            break;

        case ACTION_NOT_EQUAL_SKIP:
            if (previous != (int)data)
                index += reg + 1;
            else
                index++;
            break;

        case ACTION_WRITE_PREVIOUS:
            ret = write_fn(nic, reg, (u16)previous);
            if (ret) {
                dev_err(&nic->pdev->dev,
                        "Stage 6: action %zu WRITE_PREVIOUS failed, mode=%s reg=0x%x value=0x%04x\n",
                        action_index, access_name(mac_mode), reg,
                        (u16)previous);
                return -1;
            }
            index++;
            break;

        case ACTION_SKIP:
            index += reg + 1;
            break;

        case ACTION_DELAY:
            msleep(data);
            index++;
            break;

        default:
            dev_err(&nic->pdev->dev,
                    "Stage 6: action %zu invalid opcode=0x%x\n",
                    action_index, op);
            return -1;
        }
    }

    if (actions)
        *actions = count;

    return 0;
}

int firmware_finish(struct my_nic *nic)
{
    u16 value;
    int i;

    /* Firmware may leave the access window on a different page. */
    nic->ocp_base = OCP_STD_PHY_BASE;

    /* The hardware reference flow waits for a possible PHY soft reset. */
    for (i = 0; i < 600; i++) {
        if (phy_read(nic, 0x00, &value)) {
            dev_err(&nic->pdev->dev,
                    "Stage 6: PHY status read failed during completion check\n");
            return -1;
        }

        if (!(value & PHY_RESET_BIT)) {
            dev_info(&nic->pdev->dev,
                     "Stage 6: PHY completion check passed, BMCR=0x%04x\n",
                     value);
            return 0;
        }

        usleep_range(900, 1100);
    }

    dev_err(&nic->pdev->dev,
            "Stage 6: PHY reset did not complete, BMCR=0x%04x\n", value);
    return -1;
}
