/*
* Copyright 2011 Wolfgang Keller
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "MiniStdlib/memset.h"
#include "MiniStdlib/cstdint.h"

void* memset2(void * in_out_ptr, int in_value, size_t in_num)
{
	uint16_t *ptr = (uint16_t *) in_out_ptr;

	while (in_num--)
	{
		*ptr++ = in_value;
	}

	return in_out_ptr;
}