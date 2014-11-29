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
// these are also considered to be auxiliary verbs
const char *modalVerbs[] = {
	"can", "could", "may", "might", "must", "shall", "should", "will", "would", NULL
};

// http://en.wikipedia.org/wiki/Auxiliary_verb
// NOTE: modal verbs and auxiliary verbs are treated the same by this program
// NOTE: "have" and "do" are in the commandWords list instead of here
const char *auxiliaryVerbs[] = {
	"am", "are", "be", "been", "being", "did", "does", "had", "has", "is", "was", "were",

	// like is uber complicated and not always a verb
	"like",
	"love",

	NULL
};

// http://en.wikipedia.org/wiki/Copula_(linguistics)
// NOTE: modal verbs and linking verbs are treated the same by this program
const char *linkingVerbs[] = {
	"to", // NOTE: I don't think this is considered a linking verb
	NULL
};

// Words that are a command.
// hmm, are these all 'main verbs'? main verb means can be a predicate by itself.
const char *commandWords[] = {
	// http://ogden.basic-english.org/verbs.html
	"be", "come", "give", "make",
	"have", "go", "get", "send",
	"do", "put", "keep", "see",
	"seem", "take", "let", "say",
	// There are said to sometimes be used as operators.
	// ZTM: "because" probably needs to be a conjunction word in some cases
	"cause", "because",

	// ZTM: not part of list for Basic English verbs / operators. might not be correct handling.
	"set", "enable", "disable", "stop", "start", "restart",

	NULL
};

const char *miscVerbs[] = {
	NULL
};

#if 0
// http://www.scientificpsychic.com/grammar/enggramg.html
const char *prepositionWords[] = {
	"from", "toward", "in", "about", "over", "above", "under", "at", "below", NULL
};
#endif

const char *punctuationMarks[] = {
	".", "!", "?", ",", ";", NULL
};

const char *quoteMarks[] = {
	"\"", "\'", "`", NULL
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
		TT_AUXVERB,
		TT_MODALVERB,
		TT_LINKVERB,
		TT_MISCVERB,
		TT_COMMANDWORD,
		TT_PUNCTUATION,
		TT_QUOTE,
		TT_OTHER
	};
	TokenType tokenTypes[1024] = {TT_NONE}; // FIXME: use dynamic length

	// part of sentence tagging.
	for ( int i = 0; i < tokens.getNumTokens(); ++i ) {
		if ( inWordList( tokens[i], interrogativeWords ) ) {
			tokenTypes[i] = TT_QUESTWORD;
		}
		else if ( inWordList( tokens[i], auxiliaryVerbs ) ) {
			tokenTypes[i] = TT_AUXVERB;
		}
		else if ( inWordList( tokens[i], modalVerbs ) ) {
			tokenTypes[i] = TT_MODALVERB;
		}
		else if ( inWordList( tokens[i], linkingVerbs ) ) {
			tokenTypes[i] = TT_LINKVERB;
		}
		else if ( inWordList( tokens[i], miscVerbs ) ) {
			tokenTypes[i] = TT_MISCVERB;
		}
		else if ( inWordList( tokens[i], commandWords ) ) {
			tokenTypes[i] = TT_COMMANDWORD;
		}
		else if ( inWordList( tokens[i], punctuationMarks ) ) {
			tokenTypes[i] = TT_PUNCTUATION;
		}
		else if (  inWordList( tokens[i], quoteMarks ) ) {
			tokenTypes[i] = TT_QUOTE;
		}
		else {
			// adjective, adverbs, noun, etc
			tokenTypes[i] = TT_OTHER;
		}
	}

	newPart.clear();

	int sectencePartFirstToken = 0;
	bool readSubject = false, readPredicate = false;

	for ( int i = 0; i < tokens.getNumTokens(); ++i ) {
		// add sentence part after processing all adjacent punctuation tokens
		if ( i > 0 && tokenTypes[i-1] == TT_PUNCTUATION && tokenTypes[i] != TT_PUNCTUATION ) {
			if ( newPart.function == SentencePart::SF_UNKNOWN && ( i - sectencePartFirstToken > 2 ) ) {
#if 0
				// raw text.
				newPart.clear();
				newPart.subject = tokens.toString( sectencePartFirstToken, i-2 );
				//newPart.incomplete = true;
				if ( !newPart.subject.isEmpty() )
					parts.push_back( newPart );
#endif
			}
			else if ( newPart.function != SentencePart::SF_UNKNOWN ) {
				parts.push_back( newPart );
			}

			newPart.clear();
			sectencePartFirstToken = i;
		}

		if ( tokenTypes[i] == TT_QUESTWORD ) {
			if ( !newPart.interrogative.isEmpty() ) {
				printf("  WARNING: Two interrogative words found in one sentence part\n");
			}
			newPart.function = SentencePart::SF_QUESTION;
			newPart.interrogative = tokens[i];

			// begin subject reading or predicate if subject is already set.
			// Ex: A cat has how many legs? -- how is after has
			if ( !newPart.subject.isEmpty() ) {
				if ( !newPart.predicate.isEmpty() ) {
					printf("  WARNING: Found interrogative after subject. Swapping subject and predictate.\n");
				}
				String tmp = newPart.predicate;
				newPart.predicate = newPart.subject;
				newPart.subject = tmp;
			}
			if ( !newPart.subject.isEmpty() && !newPart.predicate.isEmpty() ) {
				printf("  WARNING: Going to append tokens after interrogative to (non empty) predicate\n");
			}
			readSubject = newPart.subject.isEmpty();
			readPredicate = !readSubject;
		}
		// modal verb acts like a interrogative if at beginning of sentence
		// otherwise acts like a linking verb (at least, that's my conclusion)
		// command words also have beginning of sentence and linking behavior
		else if ( tokenTypes[i] == TT_AUXVERB || tokenTypes[i] == TT_MODALVERB || tokenTypes[i] == TT_LINKVERB || tokenTypes[i] == TT_COMMANDWORD ) {
			// try to form a link
			TokenType perviousType = ( i > sectencePartFirstToken ) ? tokenTypes[i-1] : TT_NONE;

			// if start of sentence
			if ( perviousType == TT_NONE ) {
				if ( tokenTypes[i] == TT_COMMANDWORD ) {
					newPart.function = SentencePart::SF_COMMAND;
					newPart.command = tokens[i];
				} else {
					newPart.function = SentencePart::SF_QUESTION;
					newPart.interrogative = tokens[i];
				}

				// begin subject reading
				readSubject = true;
				readPredicate = false;
			}
			// right after an interrogative word or command word
			else if ( perviousType == TT_QUESTWORD || perviousType == TT_COMMANDWORD ) {
				if ( !newPart.subjectVerb.isEmpty() ) {
					printf("  WARNING: Two subject verbs found in one sentence part\n");
				}

				newPart.subjectVerb = tokens[i];

				// begin subject reading or predicate if subject is already set.
				// TODO: figure out what would cause subject to already be set here, because than 'subjectVerb' behavior might not fit it's description
				readSubject = newPart.subject.isEmpty();
				readPredicate = !readSubject;
			}
			// after something else
			else {
				if ( !newPart.linkingVerb.isEmpty() ) {
					printf("  WARNING: Two linking verbs found in one sentence part\n");
					// This could be a useless word that just breaks stuff.
					// In "How many legs does a cat have?" 'does' and 'have' are link words.
					// Don't need the 'have' for responding, unless nitpicking grammer.
					continue;
				}

				if ( newPart.function == SentencePart::SF_UNKNOWN ) {
					if ( tokenTypes[i] == TT_COMMANDWORD ) {
						newPart.function = SentencePart::SF_COMMAND;
					} else {
						newPart.function = SentencePart::SF_STATEMENT;
					}
				}
				newPart.linkingVerb = tokens[i];

				// search back and find unknown tokens (presumably a noun or adjective) to use as the subject
				// Ex: Say "Marvel's Ironman" is nice.
				// Extract: Marvel's Ironman to be the subject. say is in the commandWords array.
				int startToken = i-1;
				int endToken = i-1;
				for ( int s = startToken; s >= sectencePartFirstToken; --s ) {
					if ( tokenTypes[i] == TT_PUNCTUATION || tokenTypes[s] == TT_QUOTE )
						continue;
					if ( tokenTypes[s] == TT_OTHER ) {
						startToken = s;
					} else {
						break;
					}
				}

				if ( tokenTypes[endToken] == TT_QUOTE ) {
					endToken--;
				}

				if ( startToken >= sectencePartFirstToken && tokenTypes[startToken] == TT_OTHER && endToken >= sectencePartFirstToken ) {
					//printf( "  NOTICE: link subject %d to %d...\n", startToken, endToken );
					newPart.subject = tokens.toString( startToken, endToken );
				} else {
					//printf( "  NOTICE: link verb without subject before it.\n" );
				}

				// begin subject reading or predicate if subject is already set.
				readSubject = newPart.subject.isEmpty();
				readPredicate = !readSubject;
			}
		}
		else if ( tokenTypes[i] == TT_PUNCTUATION || tokenTypes[i] == TT_QUOTE ) {
			readSubject = false;
			readPredicate = false;

			if ( tokens[i] == "?" && newPart.function != SentencePart::SF_QUESTION ) {
				// it just became a question!
				//printf( "  NOTICE: Forcing %s to question!\n", newPart.getFunctionName() );
				newPart.function = SentencePart::SF_QUESTION;
			}
			// Avoid "May The Force be with you." being a question. Might not be the best method as "May The Force be with you" will be treated as a question.
			// See "The auxiliary verbs may and let are also used often in the subjunctive mood." http://en.wikipedia.org/wiki/English_modal_verbs
			// This could be bad if someone writes "What.", "What is that.", etc. Then again, this whole thing asuming mostly proper english with optional punctuation.
			else if ( tokens[i] == "." && newPart.function == SentencePart::SF_QUESTION ) {
				// it just became a question!
				//printf( "  NOTICE: Forcing question to statement!\n" );
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
	if ( newPart.function == SentencePart::SF_UNKNOWN && sectencePartFirstToken < tokens.getNumTokens() ) {
		// raw text.
		newPart.clear();
		newPart.subject = tokens.toString( sectencePartFirstToken, tokens.getNumTokens()-1 );
		//newPart.incomplete = true;
		if ( !newPart.subject.isEmpty() )
			parts.push_back( newPart );
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
