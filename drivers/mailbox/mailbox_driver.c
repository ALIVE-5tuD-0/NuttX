#include <nuttx/config.h>

#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/kmalloc.h>
#include <nuttx/fs/fs.h>
#include <nuttx/mbox/mbox.h>
#include <nuttx/semaphore.h>

#ifdef CONFIG_MBOX

// #define Mailbox_TraceOut(fmt, ...) sg2002_trace_dirout(fmt, ##__VA_ARGS__)
#define Mailbox_TraceOut(fmt, ...)

#define DEVNAME_FMT    "/dev/mailbox"
#define DEVNAME_FMTLEN (strlen(DEVNAME_FMT) + 1)

static int     mboxdrvr_open(FAR struct file *filep);
static int     mboxdrvr_close(FAR struct file *filep);
static ssize_t mboxdrvr_read(FAR struct file *filep, FAR char *buffer, size_t buflen);
static ssize_t mboxdrvr_write(FAR struct file *filep, FAR const char *buffer, size_t buflen);
static int     mboxdrvr_ioctl(FAR struct file *filep, int cmd, unsigned long arg);

struct mbox_driver_s {
    FAR struct mbox_dev_s *mbox;
    sem_t exclsem;                 /* Mutual exclusion */
};

static const struct file_operations mboxdrvr_fops =
{
  mboxdrvr_open,    /* open     */
  mboxdrvr_close,   /* close    */
  mboxdrvr_read,    /* read     */
  mboxdrvr_write,   /* write    */
  NULL,             /* seek     */
  mboxdrvr_ioctl,   /* ioctl    */
  NULL,             /* poll     */
  NULL              /* unlink   */
};

static int mboxdrvr_open(FAR struct file *filep) {
    UNUSED(filep);
    Mailbox_TraceOut("open mailbox\n");
    return OK;
}

static int mboxdrvr_close(FAR struct file *filep) {
    UNUSED(filep);
    return OK;
}

static ssize_t mboxdrvr_read(FAR struct file *filep, FAR char *buffer, size_t buflen) {
    return 0;
}

static ssize_t mboxdrvr_write(FAR struct file *filep, FAR const char *buffer, size_t buflen) {
    Mailbox_TraceOut("mailbox write\n");
    return 0;
}

static int mboxdrvr_ioctl(FAR struct file *filep, int cmd, unsigned long arg) {
    FAR struct inode *inode = NULL;
    FAR struct mbox_driver_s *priv = NULL;
    int ret = -1;

    DEBUGASSERT(filep != NULL && filep->f_inode != NULL);
    inode = filep->f_inode;

    priv = (FAR struct mbox_driver_s *)inode->i_private;
    DEBUGASSERT(priv);
    
    ret = nxsem_wait(&priv->exclsem);
    if (ret < 0)
        return ret;

    Mailbox_TraceOut("mailbox driver ioctl\n");
    switch (cmd) {
        case MBOXIOC_SEND: {
            mailbox_send_struture_t *p_mb_tx = (mailbox_send_struture_t *)arg;
            uint32_t mb_mem_end_addr = 0;

            /* check arg */
            /* check sec id */
            if (p_mb_tx->sec_id > priv->mbox->get_sec_sum())
                return -1;

            /* check trans type */
            if (p_mb_tx->trans_type > Trans_double)
                return -1;

            if (p_mb_tx->trans_type == Trans_To_TargetAddr) {
                /* TO DO */
                /* check selected section area avaliable */

                /* check target memory addr */
#if (CONFIG_ARCH_CHIP_SG2002)
                if ((priv->mbox->ops->get_sec_start_addr == NULL) || \
                    (priv->mbox->ops->get_sec_size == NULL))
                    return -1;

                mb_mem_end_addr = priv->mbox->ops->get_sec_start_addr(p_mb_tx->sec_id);
                mb_mem_end_addr += priv->mbox->ops->get_sec_size(p_mb_tx->sec_id);
            
                if (p_mb_tx->tar.addr >= mb_mem_end_addr)
                    return -1;
#endif
            }

            ret = MBOX_SEND(priv->mbox, 0, arg);
            break;
        }

        default: ret = -ENOTTY; break;
    }

    nxsem_post(&priv->exclsem);

    return ret;
}

int mbox_register(FAR struct mbox_dev_s *mbox) {
    FAR struct mbox_driver_s *priv = NULL;
    char devname[DEVNAME_FMTLEN];
    int ret = 0;

    /* Allocate a mailbox device structure */

    priv = (FAR struct mbox_driver_s *)kmm_zalloc(sizeof(struct mbox_driver_s));
    if (priv) {
        /* Initialize the mailbox device structure */

        priv->mbox = mbox;
        nxsem_init(&priv->exclsem, 0, 1);

        /* Create the character device name */

        memset(devname, '\0', DEVNAME_FMTLEN);
        snprintf(devname, DEVNAME_FMTLEN, DEVNAME_FMT);
        ret = register_driver(devname, &mboxdrvr_fops, 0666, priv);
        if (ret < 0) {
            /* Free the device structure if we failed to create the character
            * device.
            */

            kmm_free(priv);
            return ret;
        }

        /* Return the result of the registration */

        return OK;
    }

    return -ENOMEM;
}

#endif