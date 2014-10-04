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

#ifndef ANGEL_SENTENCE_INCLUDED
#define ANGEL_SENTENCE_INCLUDED

#include <vector>

#include "string.h"
#include "lexer.h"

namespace AngelCommunication
{

/*
Examples:

SF_STATEMENT:
	Pizza		is				good		.
	^subject	^linkingVerb	^predicate

	My name		is				Zack		.
	^subject	^linkingVerb	^predicate

SF_QUESTION:
	Pizza		is 				good		?
	^subject	^linkingVerb	^predicate

	What 			is 				pizza	?
	^interrogative	^subjectVerb	^subject

	What			pizza		is 				good		?
	^interrogative	^subject	^linkingVerb	^predicate

SF_COMMAND

	Go				get				a pizza		.
	^command		^subjectVerb	^subject

	Go				to				the pizza store		.
	^command		^subjectVerb	^subject

*/
class SentencePart {
	public:

	// http://en.wikipedia.org/wiki/Sentence_function
	// http://www.scientificpsychic.com/grammar/enggram2.html
	enum SentenceFunction {
		SF_UNKNOWN,
		SF_STATEMENT,	// declarative
		SF_QUESTION,	// interrogative
		SF_EXCLAMATION,	// exclamative	// Note: Not impletemented! How is it different from statement (besides exclamation mark)?
		SF_COMMAND,		// imperative
		SF_CONDITION, 	// conditional	// Not impletemented yet. Example: if something
	};

	SentenceFunction	function;
	//bool				incomplete; // aka sentence fragment
	//String conjunction;	// word that connects this sentence part to an earlier part

	// Word causing a question. either a http://en.wikipedia.org/wiki/Interrogative_word
	// or (only at the beginning of a sentence) http://en.wikipedia.org/wiki/English_modal_verbs or a linking verb.
	// "suppose to" only be present for questions, but currently if a question ends with a period it's changed to a statement.
	String interrogative;

	// Word causing a command.
	String command;

	String subjectVerb;	// verb after interrogative/command and before subject
	String subject;		// Subject of sentence
	String linkingVerb;	// Word that connects subject and predicate.
	String predicate;	// Optional. Something about the subject.

	void clear();
	const char *getFunctionName() const;
};

class Sentence {
	public:
		//String addressee;		// Ex: Bob what is.., What is it, bob?
		std::vector<SentencePart> parts; // Ex: If you jump, I will too.

		Sentence();
		Sentence( const char *text );

		void parse( const char *text );
		void clear();
};

}

#endif // ANGEL_SENTENCE_INCLUDED
