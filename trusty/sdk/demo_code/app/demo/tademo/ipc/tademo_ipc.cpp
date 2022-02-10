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

// TODO: add guard in header
extern "C" {
#include <stdlib.h>
}

#include <string.h>
#include <stdio.h>

#include <trusty_std.h>
#include <trusty_ipc.h>

#include <err.h>

#include "trusty_tademo.h"
#include "tademo_ipc.h"
#include "../crypto/rsa_demo.h"

#include <sprd_pal_fp_default.h>
#include <io_device_def.h>

#include <lib/storage/storage.h>
#include <math.h>


using namespace tademo;
TaDemo *device;


uuid_t allow_uuid = {
    0xd01afa05, 0x6021, 0x4a02, {0xa6, 0x47, 0x86, 0xe9, 0xc5, 0x9a, 0xeb, 0x1f}};

typedef void (*event_handler_proc_t)(const uevent_t* ev, void* ctx);
struct tipc_event_handler {
    event_handler_proc_t proc;
    void* priv;
};

struct tademo_chan_ctx {
    struct tipc_event_handler handler;
    uuid_t uuid;
    handle_t chan;
    tademo_error_t (*dispatch)(tademo_chan_ctx*, tademo_message*, uint32_t, UniquePtr<uint8_t[]>*,
                     uint32_t*);
};

struct tademo_srv_ctx {
    handle_t port_secure;
    handle_t port_non_secure;
};


static void tademo_port_handler_secure(const uevent_t* ev, void* priv);
static void tademo_port_handler_non_secure(const uevent_t* ev, void* priv);
static void tademo_port_handler(const uevent_t* ev, void* priv, bool secure);
static void tademo_chan_handler(const uevent_t* ev, void* priv);



static tipc_event_handler tademo_port_evt_handler_secure = {
    .proc = tademo_port_handler_secure, .priv = NULL,
};

static tipc_event_handler tademo_port_evt_handler_non_secure = {
    .proc = tademo_port_handler_non_secure, .priv = NULL,
};



class MessageDeleter {
public:
    explicit MessageDeleter(handle_t chan, int id) {
        chan_ = chan;
        id_ = id;
    }

    ~MessageDeleter() {
        put_msg(chan_, id_);
    }

private:
    handle_t chan_;
    int id_;
};

static long handle_port_errors(const uevent_t* ev) {
    if ((ev->event & IPC_HANDLE_POLL_ERROR) || (ev->event & IPC_HANDLE_POLL_HUP) ||
        (ev->event & IPC_HANDLE_POLL_MSG) || (ev->event & IPC_HANDLE_POLL_SEND_UNBLOCKED)) {
        /* should never happen with port handles */
        TLOGE("error event (0x%x) for port (%d)", ev->event, ev->handle);
        return ERR_BAD_STATE;
    }

    return NO_ERROR;
}


static tademo_error_t tipc_err_to_tademo_err(long tipc_err) {
    switch (tipc_err) {
        case NO_ERROR:
            return ERROR_NONE;
        case ERR_BAD_LEN:
        case ERR_NOT_VALID:
        case ERR_NOT_IMPLEMENTED:
        case ERR_NOT_SUPPORTED:
        default:
            return ERROR_UNKNOWN;
    }
}


bool test_secure_storage_write() {
    storage_session_t session;
    uint32_t i;
    uint8_t record[5000];
    TLOGE("test_secure_storage write begin \n");

    for(i=0;i<5000;i++)
         record[i] = 88;


    int rc = storage_open_session(&session, STORAGE_CLIENT_TD_PORT);

    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage session\n", rc);
        return false;
    }


    file_handle_t handle;
    rc = storage_open_file(session, &handle, "file_test", STORAGE_FILE_OPEN_CREATE, 0);
    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage object\n", rc);
        storage_close_session(session);
        return false;
    }

    rc = storage_write(handle, 0, record, 5000*sizeof(uint8_t), 0);

    storage_close_file(handle);

    storage_end_transaction(session,true);

    storage_close_session(session);

    if (rc < 0) {
        TLOGE("Error:[%d] writing storage object.\n", rc);
        return false;
    }

    TLOGE("test_secure_storage write end \n");
    return true;
}


bool test_secure_storage_read() {
    storage_session_t session;
    uint32_t i;
    uint8_t record[5000];
    TLOGE("test_secure_storage read begin \n");



    int rc = storage_open_session(&session, STORAGE_CLIENT_TD_PORT);
    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage session\n", rc);
        return false;
    }


    file_handle_t handle;
    rc = storage_open_file(session, &handle, "file_test", STORAGE_FILE_OPEN_CREATE, 0);
    if (rc < 0) {
        TLOGE("Error: [%d] failed to open storage object\n", rc);
        storage_close_session(session);
        return false;
    }

    rc = storage_read(handle, 0, record, 5000*sizeof(uint8_t));
    storage_close_file(handle);
    storage_close_session(session);

    if (rc < 0) {
        TLOGE("Error:[%d] read storage object.\n", rc);
        return false;
    }

    TLOGE("test_secure_storage read end \n");
    return true;
}



static tademo_error_t handle_request(uint32_t cmd, uint8_t *in_buf, uint32_t in_buf_size,
                                         UniquePtr<uint8_t[]> *out_buf, uint32_t *out_buf_size) {
    switch (cmd) {
        case TA_INCREASE:
        {
            TLOGE("handle_request \n");
#if 1
            uint8_t temp[8];
            temp[0] = device->Increase(in_buf[0]);
            *out_buf_size = 1;
            out_buf->reset(new uint8_t[*out_buf_size]);
            memcpy(out_buf->get(), temp, *out_buf_size);
#endif

#if 0
            int32_t ret;
            struct WRITE_THEN_READ_STR wr;

            TLOGE("spi write read ====================================\n");

            wr.max_speed_hz = 6000000;
            wr.chip_select = 0;
            wr.mode = 0;
            wr.bits_per_word = 8;
            wr.number = 1;
            wr.len = 26*1024;
            wr.rxbuf = 0;
            wr.txbuf = 0;
            wr.debug = 1;
            TLOGE("spi before ioctl number = %d====================================\n", wr.number);
            ret = ioctl(IO_DEVICE_FP, SPI_WRITE_AND_READ, &wr);
            TLOGE("spi after ioctl number = %d====================================\n", wr.number);


#endif

#if 0
            test_secure_storage_write();
            test_secure_storage_read();

#endif

#if 0
            Rsa_Demo rsa_test;
            rsa_test.rsa_test_enc();
            rsa_test.rsa_test_dec();
#endif

            return ERROR_NONE;
        }
        default:
            return ERROR_UNKNOWN;
    }
}

static tademo_error_t handle_secure_request(uint32_t cmd, uint8_t *in_buf, uint32_t in_buf_size,
                                         UniquePtr<uint8_t[]> *out_buf, uint32_t *out_buf_size) {

    switch (cmd) {
        case TA_CALL_FROME_OTHER_TA:;
            TLOGE("handle_secure_request TA_CALL_FROME_OTHER_TA \n");
            uint8_t temp[8];
            temp[0] = device->Decrease(in_buf[0]);
            *out_buf_size = 1;
            out_buf->reset(new uint8_t[*out_buf_size]);
            memcpy(out_buf->get(), temp, *out_buf_size);

            return ERROR_NONE;
        default:
            TLOGE("handle_secure_request default cmd:%d\n",cmd);
            return ERROR_UNKNOWN;
    }
}

static tademo_error_t send_response(handle_t chan,
        uint32_t cmd, uint8_t *out_buf, uint32_t out_buf_size) {
    struct tademo_message ta_msg = { cmd | TA_RESP_BIT, {}};
    iovec_t iov[2] = {
        { &ta_msg, sizeof(ta_msg) },
        { out_buf, out_buf_size },
    };
    ipc_msg_t msg = { 2, iov, 0, NULL };


    /* send message back to the caller */
    long rc = send_msg(chan, &msg);

    // fatal error
    if (rc < 0) {
        TLOGE("failed (%ld) to send_msg for chan (%d)\n", rc, chan);
        return tipc_err_to_tademo_err(rc);
    }

    return ERROR_NONE;
}




static tademo_error_t tademo_dispatch_secure(tademo_chan_ctx* ctx, tademo_message* msg,
                                      uint32_t payload_size, UniquePtr<uint8_t[]>* out,
                                      uint32_t* out_size) {

	return handle_secure_request(msg->cmd, msg->payload, payload_size,out,out_size);
}




static tademo_error_t tademo_dispatch_non_secure(tademo_chan_ctx* ctx, tademo_message* msg,
                                          uint32_t payload_size, UniquePtr<uint8_t[]>* out,
                                          uint32_t* out_size) {

    return handle_request(msg->cmd, msg->payload, payload_size,out,out_size);

}

static bool tademo_port_accessible(uuid_t* uuid, bool secure) {
    return !secure || memcmp(uuid, &allow_uuid, sizeof(allow_uuid)) == 0;
}


static long handle_msg(tademo_chan_ctx* ctx) {
    handle_t chan = ctx->chan;

    /* get message info */
    ipc_msg_info_t msg_inf;
    int rc = get_msg(chan, &msg_inf);
    if (rc == ERR_NO_MSG)
        return NO_ERROR; /* no new messages */

    // fatal error
    if (rc != NO_ERROR) {
        TLOGE("failed (%d) to get_msg for chan (%d), closing connection", rc, chan);
        return rc;
    }

    MessageDeleter md(chan, msg_inf.id);

    // allocate msg_buf, with one extra byte for null-terminator
    UniquePtr<uint8_t[]> msg_buf(new uint8_t[msg_inf.len + 1]);
    msg_buf[msg_inf.len] = 0;

    /* read msg content */
    iovec_t iov = {msg_buf.get(), msg_inf.len};
    ipc_msg_t msg = {1, &iov, 0, NULL};

    rc = read_msg(chan, msg_inf.id, 0, &msg);

    // fatal error
    if (rc < 0) {
        TLOGE("failed to read msg (%d)", rc, chan);
        return rc;
    }


    if (((unsigned long)rc) < sizeof(tademo_message)) {
        TLOGE("invalid message of size (%d)", rc, chan);
        return ERR_NOT_VALID;
    }

    UniquePtr<uint8_t[]> out_buf;
    uint32_t out_buf_size = 0;
    tademo_message* in_msg = reinterpret_cast<tademo_message*>(msg_buf.get());


    rc = ctx->dispatch(ctx, in_msg, msg_inf.len - sizeof(*in_msg), &out_buf, &out_buf_size);


    if (rc < 0) {
        TLOGE("error handling message (%d)", rc);
        //TODO
        //return send_error_response(chan, in_msg->cmd, KM_ERROR_UNKNOWN_ERROR);
        return ERROR_UNKNOWN;
    }

    //TLOGD("Sending %d-byte response", out_buf_size);
    return send_response(chan, in_msg->cmd, out_buf.get(), out_buf_size);
}






static void tademo_port_handler_secure(const uevent_t* ev, void* priv) {
    tademo_port_handler(ev, priv, true);
}

static void tademo_port_handler_non_secure(const uevent_t* ev, void* priv) {
    tademo_port_handler(ev, priv, false);
}


static tademo_chan_ctx* tademo_ctx_open(handle_t chan, uuid_t* uuid, bool secure) {

    if (!tademo_port_accessible(uuid, secure)) {

        TLOGI("access denied for client uuid \n", 0);
        return NULL;
    }


    tademo_chan_ctx* ctx = new tademo_chan_ctx;
    if (ctx == NULL) {
        return ctx;
    }

    ctx->handler.proc = &tademo_chan_handler;
    ctx->handler.priv = ctx;
    ctx->uuid = *uuid;
    ctx->chan = chan;
    ctx->dispatch = secure ? &tademo_dispatch_secure : &tademo_dispatch_non_secure;

    return ctx;
}

static void tademo_ctx_close(tademo_chan_ctx* ctx) {
    close(ctx->chan);
    delete ctx;
}


static void tademo_chan_handler(const uevent_t* ev, void* priv) {
    tademo_chan_ctx* ctx = reinterpret_cast<tademo_chan_ctx*>(priv);

    if (ctx == NULL) {
        TLOGE("error: no context on channel %d", ev->handle);
        close(ev->handle);
        return;
    }

    if ((ev->event & IPC_HANDLE_POLL_ERROR) || (ev->event & IPC_HANDLE_POLL_READY)) {
        /* close it as it is in an error state */
        TLOGE("error event (0x%x) for chan (%d)", ev->event, ev->handle);
        close(ev->handle);
        return;
    }

    if (ev->event & IPC_HANDLE_POLL_MSG) {
        long rc = handle_msg(ctx);
        if (rc != NO_ERROR) {
            /* report an error and close channel */
            TLOGE("failed (%d) to handle event on channel %d", rc, ev->handle);
            tademo_ctx_close(ctx);
            return;
        }
    }

    if (ev->event & IPC_HANDLE_POLL_HUP) {
        /* closed by peer. */
        tademo_ctx_close(ctx);
        return;
    }
}


static void tademo_port_handler(const uevent_t* ev, void* priv, bool secure) {
    long rc = handle_port_errors(ev);
    if (rc != NO_ERROR) {
        abort();
    }


    uuid_t peer_uuid;
    if (ev->event & IPC_HANDLE_POLL_READY) {
        /* incoming connection: accept it */


        int rc = accept(ev->handle, &peer_uuid);

        if (rc < 0) {

            TLOGE("failed (%d) to accept on port %d", rc, ev->handle);
            return;
        }


        handle_t chan = (handle_t)rc;

        tademo_chan_ctx* ctx = tademo_ctx_open(chan, &peer_uuid, secure);

        if (ctx == NULL) {
            TLOGE("failed to allocate context on chan %d", chan);
            close(chan);
            return;
        }

        rc = set_cookie(chan, ctx);
        if (rc < 0) {
            TLOGE("failed (%d) to set_cookie on chan %d", rc, chan);
            tademo_ctx_close(ctx);
            return;
        }
    }

}


static void dispatch_event(const uevent_t* ev) {
    if (ev == NULL)
        return;

    if (ev->event == IPC_HANDLE_POLL_NONE) {
        /* not really an event, do nothing */
        TLOGE("got an empty event", 0);
        return;
    }

    /* check if we have handler */
    tipc_event_handler* handler = reinterpret_cast<tipc_event_handler*>(ev->cookie);
    if (handler && handler->proc) {
        /* invoke it */

        handler->proc(ev, handler->priv);
        return;
    }

    /* no handler? close it */
    TLOGE("no handler for event (0x%x) with handle %d", ev->event, ev->handle);

    close(ev->handle);

    return;
}



static long tademo_ipc_init(tademo_srv_ctx* ctx) {
    int rc;
#if 0
    /* Initialize secure service , other TA will use this*/
    rc = port_create(TADEMO_SECURE_PORT, 1, TADEMO_MAX_BUFFER_LENGTH,
            IPC_PORT_ALLOW_TA_CONNECT);
    if (rc < 0) {
        TLOGE("Failed (%d) to create port %s\n", rc, TADEMO_SECURE_PORT);
    }

    ctx->port_secure = (handle_t)rc;

    rc = set_cookie(ctx->port_secure, &tademo_port_evt_handler_secure);
    if (rc) {
        TLOGE("failed (%d) to set_cookie on port %d", rc, ctx->port_secure);
        close(ctx->port_secure);
        return rc;
    }
#endif


    /* initialize non-secure side service , CA will use this*/
    rc = port_create(TADEMO_PORT, 1, TADEMO_MAX_BUFFER_LENGTH, IPC_PORT_ALLOW_NS_CONNECT);
    if (rc < 0) {
        TLOGE("Failed (%d) to create port %s", rc, TADEMO_PORT);
        return rc;
    }

    ctx->port_non_secure = (handle_t)rc;

    rc = set_cookie(ctx->port_non_secure, &tademo_port_evt_handler_non_secure);
    if (rc) {
        TLOGE("failed (%d) to set_cookie on port %d", rc, ctx->port_non_secure);
        close(ctx->port_non_secure);
        return rc;
    }

    return NO_ERROR;
}


int main(void) {
    long rc;
    uevent_t event;

    TLOGI("Initializing\n");

    device = new TaDemo();

    tademo_srv_ctx ctx;



    rc = tademo_ipc_init(&ctx);
    if (rc < 0) {
        TLOGE("failed (%ld) to initialize tademo", rc);
        return rc;
    }

    handle_t port = (handle_t) rc;

    /* enter main event loop */
    while (true) {
        event.handle = INVALID_IPC_HANDLE;
        event.event  = 0;
        event.cookie = NULL;


        rc = wait_any(&event, -1);
        if (rc < 0) {
            TLOGE("wait_any failed (%ld)\n", rc);
            break;
        }

        if (rc == NO_ERROR) { /* got an event */
            dispatch_event(&event);
        }
    }

    return 0;
}
