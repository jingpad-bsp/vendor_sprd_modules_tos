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

#include <err.h>
#include <trusty_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "tademo_ipc.h"



#define LOG_TAG "native_ta_demo"
#define TLOGE(fmt, ...) \
	    fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)
#define TLOGI(fmt, ...) \
		fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,	## __VA_ARGS__)


typedef handle_t tademo_session_t;

#define MSEC 1000000UL


static long await_response(tademo_session_t session, struct ipc_msg_info *inf)
{
	uevent_t uevt;
	long rc = wait(session, &uevt, -1);
	if (rc != NO_ERROR) {
		TLOGE("%s: interrupted waiting for response (%ld)\n",
		      __func__, rc);
		return rc;
	}

	rc = get_msg(session, inf);
	if (rc != NO_ERROR) {
		TLOGE("%s: failed to get_msg (%ld)\n", __func__, rc);
	}

	return rc;
}

static long read_response(tademo_session_t session, uint32_t msg_id,
                          uint32_t cmd, uint8_t *buf, uint32_t size)
{
	struct tademo_message msg;

	iovec_t rx_iov[2] = {
		{
			.base = &msg,
			.len = sizeof(msg)
		},
		{
			.base = buf,
			.len = size
		}
	};
	struct ipc_msg rx_msg = {
		.iov = rx_iov,
		.num_iov = 2,
	};

	long rc = read_msg(session, msg_id, 0, &rx_msg);
	put_msg(session, msg_id);

	if (msg.cmd != (cmd | TA_RESP_BIT)) {
		TLOGE("%s: invalid response (0x%x) for cmd (0x%x)\n",
		      __func__, msg.cmd, cmd);
		return ERR_NOT_VALID;
	}

	return rc;
}


int tademo_open(void)
{
	return connect(TADEMO_SECURE_PORT, IPC_CONNECT_WAIT_FOR_PORT);
}

void tademo_close(tademo_session_t session)
{
	close(session);
}


long tademo_increse(int session, uint8_t* value){
    /*send msg*/
    uint8_t msg_size = sizeof(uint8_t) + sizeof(struct tademo_message);
    struct tademo_message *msg = malloc(msg_size);

    msg->cmd = TA_CALL_FROME_OTHER_TA;
    memcpy(msg->payload, value, sizeof(uint8_t));

	iovec_t tx_iov = {
		.base = msg,
		.len = msg_size,
	};
	ipc_msg_t tx_msg = {
		.iov = &tx_iov,
		.num_iov = 1,
	};


	long rc = send_msg(session, &tx_msg);
	if (rc < 0) {
		TLOGE("%s: failed (%ld) to send_msg\n", __func__, rc);
		return rc;
	}

	if(((size_t) rc) != msg_size) {
		TLOGE("%s: msg invalid size (%zu != %zu)",
		      __func__, (size_t)rc, sizeof(msg));
		return ERR_IO;
	}

    free(msg);


    /*wait response*/
	struct ipc_msg_info inf;
	rc = await_response(session, &inf);

	if (rc < 0) {
		TLOGE("%s: failed (%ld) to await response\n", __func__, rc);
		return rc;
	}

	if (inf.len <= sizeof(struct tademo_message)) {
		TLOGE("%s: invalid  ret len (%zu)\n", __func__, inf.len);
		put_msg(session, inf.id);
		return ERR_NOT_FOUND;
	}


	size_t size = inf.len - sizeof(struct tademo_message);
	uint8_t *ret_buf = malloc(size);
	if (ret_buf == NULL) {
		TLOGE("%s: out of memory (%zu)\n", __func__, inf.len);
		put_msg(session, inf.id);
		return ERR_NO_MEMORY;
	}


	rc = read_response(session, inf.id, TA_CALL_FROME_OTHER_TA,
					   ret_buf, size);

	if (rc < 0) {
		goto err_bad_read;
	}


	size_t read_len = (size_t) rc;
	if (read_len != inf.len){
		// data read in does not match message length
		TLOGE("%s: invalid read length: (%zu != %zu)\n",
			  __func__, read_len, inf.len);
		rc = ERR_IO;
		goto err_bad_read;
	}


	*value = ret_buf[0];
	return NO_ERROR;

err_bad_read:
	free(ret_buf);
	TLOGE("%s: failed read_msg (%ld)", __func__, rc);
	return rc;


}




int main(void)
{


    long rc = tademo_open();
    if (rc < 0) {
		TLOGI("native ta demo eeeee rc:%d \n",rc);        		
        return false;
    }

    tademo_session_t session = (tademo_session_t) rc;


    uint8_t value = 100;




	TLOGI("native ta demo before call another ta value:%d \n",value);
	
	tademo_increse(session, &value);
	
	TLOGI("native ta demo after call another ta value:%d \n",value);



	tademo_close(session);

	return 0;
}
