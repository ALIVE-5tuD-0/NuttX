/****************************************************************************
 * include/nuttx/mbox/mbox.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __INCLUDE_NUTTX_MBOX_MBOX_H
#define __INCLUDE_NUTTX_MBOX_MBOX_H

#ifdef CONFIG_MBOX

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include <nuttx/fs/ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Command:      MBOXIOC_SEND
 * Description:  Perform an mailbox send
 * Argument:     A reference to an instance of struct mbox_send_s.
 * Dependencies: CONFIG_MBOX_DRIVER
 */

#define MBOXIOC_SEND          _MBOXIOC(0x0001)
#define MBOXIOC_SEND_WAIT     _MBOXIOC(0x0002)
#define MBOXIOC_GET_SEC_INFO  _MBOXIOC(0x0003)
#define MBOXIOC_GET_SUM       _MBOXIOC(0x0004)

typedef enum {
  Trans_To_TargetAddr = 0,
  Trans_uint8,
  Trans_uint16,
  Trans_uint32,
  Trans_uint64,
  Trans_int8,
  Trans_int16,
  Trans_int32,
  Trans_int64,
  Trans_float,
  Trans_double,
} mailbox_transtype_e;

typedef struct {
  uint8_t cmd_id;
  uint8_t sec_id;
  uint8_t trans_type;

  union {
    struct {
      uint32_t addr;
      uint32_t size;
    } tar;
  
    struct {
      uint8_t u8_v;
      uint16_t u16_v;
      uint32_t u32_v;
      uint64_t u64_v;

      int8_t i8_v;
      int16_t i16_v;
      int32_t i32_v;
      int64_t i64_v;

      float f_v;
      double d_v;
    } val;

    uint8_t buf[8];
  }
} mailbox_send_struture_t;

/* Access macros ************************************************************/

/****************************************************************************
 * Name: MBOX_SEND
 *
 * Description:
 *   Send a 32bits message to remote core from specific channel
 *
 * Input Parameters:
 *   dev   - Device-specific state data
 *   ch    - Mbox specific channel
 *   msg   - Message to send
 *
 * Returned Value:
 *   OK unless an error occurs.  Then a negated errno value is returned
 *
 ****************************************************************************/

#define MBOX_SEND(d,c,m) ((d)->ops->send(d,c,m))

/****************************************************************************
 * Name: MBOX_REGISTER_CALLBACK
 *
 * Description:
 *   Attach to receive a callback when something is received on MBOX
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   ch       - Mbox specific channel
 *   callback - The function to be called when something has been received
 *   arg      - A caller provided value to return with the callback
 *
 * Returned Value:
 *   OK unless an error occurs.  Then a negated errno value is returned
 *
 ****************************************************************************/

#define MBOX_REGISTER_CALLBACK(d,c,cb,a) \
  ((d)->ops->registercallback(d,c,cb,a))

/****************************************************************************
 * Name: MBOX_UNREGISTER_CALLBACK
 *
 * Description:
 *   Detach MBOX callback
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   ch       - Mbox specific channel
 *
 * Returned Value:
 *   OK unless an error occurs.  Then a negated errno value is returned
 *
 ****************************************************************************/

#define MBOX_UNREGISTER_CALLBACK(d,c) \
  ((d)->ops->registercallback(d,c,NULL,NULL))

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct mbox_transfer_s {
  uint8_t ip_id;
  uint8_t cmd_id;
  uint32_t param_ptr;
};

struct mbox_dev_s;
typedef CODE int (*mbox_receive_t)(FAR void *arg, uintptr_t msg);

struct mbox_ops_s
{
  CODE int (*send)(FAR struct mbox_dev_s *dev, uint32_t ch, uintptr_t msg);
  CODE int (*registercallback)(FAR struct mbox_dev_s *dev, uint32_t ch,
                               mbox_receive_t callback, FAR void *arg);
  CODE const uint32_t (*get_sec_sum)(void);
  CODE const uint32_t (*get_sec_start_addr)(uint8_t index);
  CODE const uint32_t (*get_sec_size)(uint8_t index);
};

struct mbox_dev_s
{
  FAR const struct mbox_ops_s *ops;
};

int mbox_register(FAR struct mbox_dev_s *mbox);

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_MBOX */
#endif /* __INCLUDE_NUTTX_MBOX_MBOX_H */
