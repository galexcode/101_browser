#ifndef _Fold_h
#define _Fold_h

#include <limits>
#include <cassert>

namespace OZO
{
	namespace HigherOrderFunctions
	{
		template <typename State> State foldl(
			State (*in_foldFunction)(State, uint8_t), State in_state, 
			void *in_buffer, size_t in_bufferSize)
		{
			assert(in_bufferSize < std::numeric_limits<size_t>::max());

			for (size_t bufferIndex=0; bufferIndex<in_bufferSize; bufferIndex++)
			{
				in_state = (*in_foldFunction)(in_state, ((uint8_t*) in_buffer)[bufferIndex]);
			}

			return in_state;
		}
	}
}

#endif