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

extern "C" {
#include <trusty_std.h>
}

#include "trusty_tademo.h"



namespace tademo {



TaDemo::TaDemo() {
    TLOGE("wjp TaDemo init \n");

}


uint8_t TaDemo::Decrease(uint8_t counter) {
	TLOGE("wjp TaDemo Decrease counter:%d\n",counter);
	uint8_t ret = counter-1;
	TLOGE("wjp TaDemo Decrease ret:%d\n",ret);
    return ret;
}



uint8_t TaDemo::Increase(uint8_t counter) {
	TLOGE("wjp TaDemo Increase counter:%d\n",counter);
	uint8_t ret = counter+1;
	TLOGE("wjp TaDemo Increase ret:%d\n",ret);
    return ret;
}



}
