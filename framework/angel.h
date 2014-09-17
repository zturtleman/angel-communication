/*
Angel Communication
Copyright (C) 2013-2014 Zack Middleton <zturtleman@gmail.com>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef ANGEL_COMMUNICATION_INCLUDED
#define ANGEL_COMMUNICATION_INCLUDED

#include "string.h"
#include "lexer.h"
#include "sentence.h"
#include "persona.h"
#include "conversation.h"

// functions that must exist outside the framework (aka imported functions)
void ANGELC_PrintMessage( const AngelCommunication::Conversation *con, const AngelCommunication::Persona *speaker, const char *message );
void ANGELC_PersonaRename( const char *oldname, const char *newname );

#endif // ANGEL_COMMUNICATION_INCLUDED

