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
#include <stddef.h>
#include <trusty_app_manifest.h>

// TODO: generate my own UUID
trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{

    /* UUID : {4304bef6-36e5-4d90-94b0-1ea4cd51d40b} */
    { 0x4304bef6, 0x36e5, 0x4d90,
        { 0x94, 0xb0, 0x1e, 0xa4, 0xcd, 0x51, 0xd4, 0x0b } },

        /* optional configuration options here */
        {
            TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(20 * 4096),
            TRUSTY_APP_CONFIG_MIN_STACK_SIZE(10 * 4096),
        },
};
