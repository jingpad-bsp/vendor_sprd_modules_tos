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

#ifndef TRUSTY_TADEMO_H_
#define TRUSTY_TADEMO_H_

#include <trusty_std.h>
#include <stdio.h>
#include <UniquePtr.h>



#define LOG_TAG "trusty_tademo"
#define TLOGE(fmt, ...) \
    fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

#define TLOGI(fmt, ...) \
    fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)


namespace tademo {

class TaDemo{
public:
    TaDemo();

    uint8_t Increase(uint8_t counter);
    uint8_t Decrease(uint8_t counter);

};

}





#endif // TRUSTY_TADEMO_H_
