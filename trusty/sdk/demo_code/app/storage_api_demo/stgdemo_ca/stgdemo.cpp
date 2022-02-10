/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <type_traits>
#include <stdlib.h>
#include <string.h>

#include "stgdemo.h"
#include "stgdemo_ipc.h"


#define LOG_TAG "stgdemo"

const uint32_t SEND_BUF_SIZE = 16;
const uint32_t RECV_BUF_SIZE = 32;


int main(int argc, char *argv[])
{
    printf("--- test STGDEMO_INCREASE start ---\n");

    uint32_t command = STGDEMO_INCREASE;
    uint8_t recv_buf[RECV_BUF_SIZE];
    uint32_t recv_size = RECV_BUF_SIZE;
    uint8_t send_buf[SEND_BUF_SIZE];
    uint32_t send_size = SEND_BUF_SIZE;

    int rc = trusty_stgdemo_connect();
    if (rc < 0) {
        printf("Error initializing trusty session: %d\n", rc);
        return ERROR_UNKNOWN;
    }

    send_buf[0] = 99;
    printf("Before  counter: %d\n", send_buf[0]);
    rc = trusty_stgdemo_call(command, send_buf, send_size, recv_buf, &recv_size);
    if (rc < 0) {
        printf("error (%d) calling  STGDEMO_INCREASE \n", rc);
        return ERROR_UNKNOWN;
    }
    const stgdemo_message *msg = reinterpret_cast<stgdemo_message *>(recv_buf);
    const uint8_t *payload = msg->payload;
    printf("Invoking TA to increment  counter: %d\n", payload[0]);
    trusty_stgdemo_disconnect();
    printf("--- test STGDEMO_INCREASE end ---\n\n");
    

    printf("--- test TA write start ---\n");
    int ret;
    uint32_t i = 0;
    ret = trusty_stgdemo_connect();
    if (ret < 0) {
        printf("Error initializing trusty session: %d\n", rc);
        return ERROR_UNKNOWN;
    }

    for (i = 0; i < SEND_BUF_SIZE; i++) {
         send_buf[i] = i;
         printf("send_buf[%u]: %x\n", i, send_buf[i]);
    }
    command = STGDEMO_DATA_SAVE;
    recv_size = RECV_BUF_SIZE;
    ret = trusty_stgdemo_call(command, send_buf, SEND_BUF_SIZE, recv_buf, &recv_size);
    if (ret < 0) {
        printf("error (%d) calling   TA encypt save \n", ret);
        return ERROR_UNKNOWN;
    }
    trusty_stgdemo_disconnect();
    printf("--- test TA write end ---\n\n");



    printf("--- test TA read start ---\n");
    ret = trusty_stgdemo_connect();
    if (ret < 0) {
        printf("Error initializing trusty session: %d\n", rc);
        return ERROR_UNKNOWN;
    }
    
    command = STGDEMO_DATA_READ;
    recv_size = RECV_BUF_SIZE;
    ret = trusty_stgdemo_call(command, NULL, 0, recv_buf, &recv_size);
    if (ret < 0) {
        printf("error (%d) calling TA encypt read \n", ret);
        return ERROR_UNKNOWN;
    }
    const stgdemo_message *msg_r = reinterpret_cast<stgdemo_message *>(recv_buf);
    const uint8_t *payload_r = msg_r->payload;
    for (i = 0; i < recv_size; i++) {
        printf("read from TA [%u]: %x\n", i, payload_r[i]);
    }

    trusty_stgdemo_disconnect();
    printf("--- test TA read end ---\n");

    return 0;
}

