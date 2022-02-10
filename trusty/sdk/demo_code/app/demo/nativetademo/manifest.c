/*
 * Copyright (C) 2012-2013 The Android Open Source Project
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

#include <trusty_app_manifest.h>
#include <stddef.h>
#include <stdio.h>

trusty_app_manifest_t TRUSTY_APP_MANIFEST_ATTRS trusty_app_manifest =
{
	/* UUID : {d01afa05-6021-4a02-a647-86e9c59aeb1f} */
	{ 0xd01afa05, 0x6021, 0x4a02,
	  { 0xa6, 0x47, 0x86, 0xe9, 0xc5, 0x9a, 0xeb, 0x1f } },

	/* optional configuration options here */
	{
		/* four pages for heap */
		TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(4 * 4096),

		/* request two I/O mappings */
		TRUSTY_APP_CONFIG_MAP_MEM(1, 0x70000000, 0x1000),
		TRUSTY_APP_CONFIG_MAP_MEM(2, 0x70000804, 0x4)
	},
};
