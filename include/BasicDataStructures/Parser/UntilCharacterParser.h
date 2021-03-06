/*
 * Copyright 2008-2011 Wolfgang Keller
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

#ifndef _UntilCharacterParser_h
#define _UntilCharacterParser_h

#include "BasicDataStructures/Parser/AbstractParser.h"
#include <list>

class UntilCharacterParser : public AbstractParser
{
protected:
	bool mReachedCharacter;
	char mUntilCharacter;

	virtual bool parseTokenIfNotDone(char in_token);

public:
	std::list<char> mReadCharacters;

	inline UntilCharacterParser(char in_untilCharacter) : mUntilCharacter(in_untilCharacter)
	{
		reset();
	}

	virtual void reset();
	virtual bool isDone() const;
	virtual bool isOK() const;
};

#endif
