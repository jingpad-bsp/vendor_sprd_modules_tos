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

#include "cademo.h"
#include "cademo_ipc.h"


#define LOG_TAG "cademo"
#include <cutils/log.h>



const uint32_t SEND_BUF_SIZE = 512;
const uint32_t RECV_BUF_SIZE = 512;



int main(int argc, char *argv[])
{

    int rc = trusty_cademo_connect();
    if (rc < 0) {
		printf("Error initializing trusty session: %d\n", rc);
        return ERROR_UNKNOWN;
    }
	printf("argc: %d  argv[0]:%s\n", argc,argv[0]);

	uint32_t command = TA_INCREASE;
    uint8_t recv_buf[RECV_BUF_SIZE];
    uint32_t response_size = RECV_BUF_SIZE;
    uint8_t send_buf[SEND_BUF_SIZE];
    uint32_t request_size = SEND_BUF_SIZE;

    send_buf[0] = 99;
	printf("Before  counter: %d\n", send_buf[0]);
    rc = trusty_cademo_call(command, send_buf, request_size, recv_buf, &response_size);
    if (rc < 0) {
		printf("error (%d) calling  TA\n", rc);
        return ERROR_UNKNOWN;
    }
    const tademo_message *msg = reinterpret_cast<tademo_message *>(recv_buf);
    const uint8_t *payload = msg->payload;

	printf("Invoking TA to increment  counter: %d\n", payload[0]);
	
	trusty_cademo_disconnect();

	return 0;
}

