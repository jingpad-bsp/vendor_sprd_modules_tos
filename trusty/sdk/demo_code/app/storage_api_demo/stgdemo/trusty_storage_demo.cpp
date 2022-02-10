/*
 * Copyright 2015 The Android Open Source Project
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
#include <lib/storage/storage.h>

#include "trusty_storage_demo.h"


namespace storage_demo {


StorageDemo::StorageDemo() {
    TLOGI("StorageDemo init\n");
}


uint8_t StorageDemo::Decrease(uint8_t counter) {
    TLOGI("StorageDemo Decrease counter: %d\n",counter);

    uint8_t ret = counter - 1;
    TLOGI("StorageDemo Decrease ret: %d\n",ret);
    return ret;
}


uint8_t StorageDemo::Increase(uint8_t counter) {
    TLOGI("StorageDemo Increase counter:%d\n",counter);

    uint8_t ret = counter + 1;
    TLOGI("StorageDemo Increase ret:%d\n",ret);
    return ret;
}

stgdemo_error_t StorageDemo::WriteRpmb(const void *src, uint32_t size) {
    TLOGI("StorageDemo WriteRpmb size: %u\n", size);

    storage_session_t session_write;
    int rc;

    TLOGI("test_secure_storage write begin \n");
    rc = storage_open_session_async(&session_write, STORAGE_CLIENT_TP_PORT, 100);
    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage session\n", rc);
        return ERROR_UNKNOWN;
    }

    file_handle_t handle_w;
    rc = storage_open_file(session_write, &handle_w, "file_test",
            STORAGE_FILE_OPEN_CREATE, 0);
    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage object\n", rc);
        storage_close_session(session_write);
        return ERROR_UNKNOWN;
    }

    const uint8_t *in_buf = reinterpret_cast<const uint8_t *>(src);
    TLOGE("test_secure_storage write in_buf size: %d\n", size);
    for (uint32_t i = 0; i < size; i++) {
        TLOGI("test_secure_storage write in_buf:%x\n", in_buf[i]);
    }
    rc = storage_write(handle_w, 0, in_buf, size, 0);

    storage_close_file(handle_w);
    storage_end_transaction(session_write, true);
    storage_close_session(session_write);

    TLOGI("test_secure_storage write end \n");
    return ERROR_NONE;

}

stgdemo_error_t StorageDemo::ReadRpmb(void *dest, uint32_t *size) {
    TLOGI("StorageDemo ReadRpmb size: %u\n", *size);

    storage_session_t session_read;
    int rc;

    TLOGI("test_secure_storage read begin \n");
    rc = storage_open_session_async(&session_read, STORAGE_CLIENT_TP_PORT, 100);
    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage session\n", rc);
        return ERROR_UNKNOWN;
    }

    file_handle_t handle_r;
    rc = storage_open_file(session_read, &handle_r, "file_test",
            STORAGE_FILE_OPEN_CREATE, 0);
    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage object\n", rc);
        storage_close_session(session_read);
        return ERROR_UNKNOWN;
    }

    uint8_t *out_buf = reinterpret_cast<uint8_t*>(dest);
    *size = 16;
    TLOGI("test_secure_storage read out_buf size: %d\n", *size);
    rc = storage_read(handle_r, 0, out_buf, *size);
    for (uint32_t i = 0; i < *size; i++) {
        TLOGI("test_secure_storage read out_buf:%x\n", out_buf[i]);
    }

    storage_close_file(handle_r);
    storage_end_transaction(session_read, true);
    storage_close_session(session_read);

    TLOGE("test_secure_storage read end \n");
    return ERROR_NONE;
}

}
