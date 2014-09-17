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

#include <stdio.h> // printf

#include "angel.h"
#include "sentence.h"

namespace AngelCommunication
{

// http://en.wikipedia.org/wiki/Interrogative_word
const char *interrogativeWords[] = {
	"which", "what",
	"whose",
	"who", "whom",
	"where",
	"whence",
	"whither",
	"when",
	"how",
	"why", "wherefore",
	"whether",
	NULL
};

// http://en.wikipedia.org/wiki/English_modal_verbs
const char *modalVerbs[] = {
	"can", "could", "may", "might", "must", "shall", "should", "will", "would", NULL
};

// http://en.wikipedia.org/wiki/Copula_(linguistics)
const char *linkingVerbs[] = {
	"is", "are", "was", "am", "were",
	"be", // FIXME: confirm this is a consistered a linking verb
	NULL
};

// like isn't always a verb
const char *miscVerbs[] = {
	"like", NULL
};

#if 0
// http://www.scientificpsychic.com/grammar/enggramg.html
const char *prepositionWords[] = {
	"from", "toward", "in", "about", "over", "above", "under", "at", "below", NULL
};
#endif

const char *punctuationMarks[] = {
	".", "!", "?", ",", ";", "\"", "\'", NULL
};

bool inWordList( const String &original, const char **wordList ) {
	for ( int i = 0; wordList[i] != NULL; ++i ) {
		if ( !original.icompareTo( wordList[i] ) ) {
			return true;
		}
	}
	return false;
}

Sentence::Sentence() {
}

Sentence::Sentence( const char *text ) {
	parse( text );
}

void Sentence::clear() {
	//addressee = "";
	parts.clear();
}

void Sentence::parse( const char *text ) {
	SentencePart newPart;

	Lexer tokens( text );

	enum TokenType {
		TT_NONE,
		TT_QUESTWORD,
		TT_LINKVERB,
		TT_MODALVERB,
		TT_MISCVERB,
		TT_PUNCTUATION,
		TT_OTHER
	};
	TokenType tokenTypes[1024] = {TT_NONE}; // FIXME: use dynamic length

	// part of sentence tagging.
	for ( int i = 0; i < tokens.getNumTokens(); ++i ) {
		if ( inWordList( tokens[i], interrogativeWords ) ) {
			tokenTypes[i] = TT_QUESTWORD;
		}
		else if ( inWordList( tokens[i], linkingVerbs ) ) {
			tokenTypes[i] = TT_LINKVERB;
		}
		else if ( inWordList( tokens[i], modalVerbs ) ) {
			tokenTypes[i] = TT_MODALVERB;
		}
		else if ( inWordList( tokens[i], miscVerbs ) ) {
			tokenTypes[i] = TT_MISCVERB;
		}
		else if ( inWordList( tokens[i], punctuationMarks ) ) {
			tokenTypes[i] = TT_PUNCTUATION;
		}
		else {
			// adjective, adverbs, noun, etc
			tokenTypes[i] = TT_OTHER;
		}
	}

	newPart.clear();

	int questTokenNum = -1;
	int linkTokenNum = -1;
	bool readSubject = false, readPredicate = false;

	for ( int i = 0; i < tokens.getNumTokens(); ++i ) {
		// add sentence part after processing all adjacent punctuation tokens
		if ( i > 0 && tokenTypes[i-1] == TT_PUNCTUATION && tokenTypes[i] != TT_PUNCTUATION ) {
			if ( newPart.function != SentencePart::SF_UNKNOWN ) {
				parts.push_back( newPart );
			}

			newPart.clear();

			questTokenNum = linkTokenNum = -1;
		}

		if ( tokenTypes[i] == TT_QUESTWORD ) {
			if ( !newPart.interrogative.isEmpty() ) {
				printf("  WARNING: Two interrogative words found in one sentence part\n");
			}
			newPart.function = SentencePart::SF_QUESTION;
			newPart.interrogative = tokens[i];
			questTokenNum = i;

			// begin subject reading
			readSubject = true;
			readPredicate = false;
		}
		// modal verb acts like a interrogative if at beginning of sentence
		// otherwise acts like a linking verb (at least, that's my conclusion)
		else if ( tokenTypes[i] == TT_MODALVERB || tokenTypes[i] == TT_LINKVERB ) {
			// try to form a link
			TokenType perviousType = ( i > 0 ) ? tokenTypes[i-1] : TT_NONE;

			// if start of sentence
			if ( perviousType == TT_NONE ) {
				if ( !newPart.interrogative.isEmpty() ) {
					printf("  WARNING: Two interrogative words found in one sentence part\n");
				}
				newPart.function = SentencePart::SF_QUESTION;
				newPart.interrogative = tokens[i];
				questTokenNum = i;

				// begin subject reading
				readSubject = true;
				readPredicate = false;
			}
			// right after an interrogative word
			else if ( perviousType == TT_QUESTWORD ) {
				//newPart.function = SentencePart::SF_QUESTION;

				newPart.linkingVerb = tokens[i];
				linkTokenNum = i;

				// begin subject reading
				readSubject = true;
				readPredicate = false;
			}
			// after something else
			else {
				if ( !newPart.linkingVerb.isEmpty() ) {
					printf("  WARNING: Two linking verbs found in one sentence part\n");
				}

				// hmm. why does this force statement? might of already parsed a question word
				// hmm.. maybe because link is between (presumably) two things?
				newPart.function = SentencePart::SF_STATEMENT;
				newPart.linkingVerb = tokens[i];
				linkTokenNum = i;

#if 0
				// FIXME: doesn't work correct :/
				int startToken = 0;
				for ( int s = i-1; s >= 0; --s ) {
					if ( tokenTypes[s] != TT_OTHER ) {
						startToken = s+1;
						break;
					}
				}
				newPart.subject = tokens.toString( startToken, i-1 );
#else
				// everything between the interrogative and here or beginning of sentence till here.
				// NOTE: this doesn't always work corrent with multiple sentence parts and should be replaced.
				newPart.subject = tokens.toString( questTokenNum+1, i-1 );
#endif

				// begin predicate reading
				readSubject = false;
				readPredicate = true;
			}
		}
		else if ( tokenTypes[i] == TT_PUNCTUATION ) {
			readSubject = false;
			readPredicate = false;

			if ( tokens[i] == "?" && newPart.function != SentencePart::SF_QUESTION ) {
				// it just became a question!
				printf( "  NOTICE: Forcing %s to question!\n", newPart.getFunctionName() );
				newPart.function = SentencePart::SF_QUESTION;
			}
			// Avoid "May The Force be with you." being a question. Might not be the best method as "May The Force be with you" will be treated as a question.
			// See "The auxiliary verbs may and let are also used often in the subjunctive mood." http://en.wikipedia.org/wiki/English_modal_verbs
			// This could be bad if someone writes "What.", "What is that.", etc. Then again, this whole thing asuming mostly proper english with optional punctuation.
			else if ( tokens[i] == "." && newPart.function == SentencePart::SF_QUESTION ) {
				// it just became a question!
				printf( "  NOTICE: Forcing question to statement!\n" );
				newPart.function = SentencePart::SF_STATEMENT;
			}
		} else {
			if ( readSubject ) {
				if ( !newPart.subject.isEmpty() )
					newPart.subject.append(" ");
				newPart.subject.append(tokens[i]);

				// HACK: should be if any noun? or if this is a noun and next is a verb?
				if ( tokens[i] == "you" || tokens[i] == "your" ) {
					// begin predicate reading
					readSubject = false;
					readPredicate = true;
				}
			}
			else if ( readPredicate ) {
				if ( !newPart.predicate.isEmpty() )
					newPart.predicate.append(" ");
				newPart.predicate.append(tokens[i]);
			}
		}
	}

#if 0
	if ( newPart.function == SentencePart::SF_UNKNOWN ) {
		newPart.function = SentencePart::SF_STATEMENT;
		newPart.incomplete = true;
	}
#endif

	if ( newPart.function != SentencePart::SF_UNKNOWN ) {
		parts.push_back( newPart );
	}
}

void SentencePart::clear() {
	function = SF_UNKNOWN;
	interrogative = "";
	linkingVerb = "";
	subject = "";
	predicate = "";
}

const char *SentencePart::getFunctionName() const {
	const char *name = "unknown";

	switch ( function ) {
		case SF_STATEMENT:
			name = "statement";
			break;
		case SF_QUESTION:
			name = "question";
			break;
		case SF_EXCLAMATION:
			name = "exclamation";
			break;
		case SF_COMMAND:
			name = "command";
			break;
		case SF_CONDITION:
			name = "condition";
			break;
		default:
			break;
	}

	return name;
}

}
