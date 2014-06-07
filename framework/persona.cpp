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

#include <stdio.h>
#include <cstring>
#include <ctype.h>

#include "persona.h"
#include "wordtypes.h"

namespace AngelCommunication
{

Persona::Persona()
{
	this->name = "unknown";
	this->gender = GENDER_NONE;
	this->funReplies = true;

	this->lastUpdate = std::clock();
}

void Persona::setName( const String &name )
{
	this->name = name;
	this->namePossesive = name;
	if ( this->name[this->name.getLen()-1] == 's' )
		this->namePossesive.append( "'" );
	else
		this->namePossesive.append( "'s" );
}

void Persona::setGender( Gender gender )
{
	this->gender = gender;
}

void Persona::welcome( Persona &target )
{
	String s;
	s = "Welcome ";
	s.append( target.name );
	s.append( "." );
	say( s );
}

void Persona::tell( Persona &target, String message )
{
	// FIXME: target needs to store who said the tokens... might want to separate lines too?
	target.told( *this, message );
}

void Persona::told( Persona & messenger, String message )
{
	if ( this->waitForReply ) {
		WaitReply waitReply = this->waitForReply;

		this->waitForReply = WR_NONE; // got a reply, stop waiting

		if ( waitReply == WR_COMPLETE_LAST && ( WordType( message ) & (WT_CANCEL_QUEST|WT_FILLER) ) ) {
			this->tokens.clear();
			say( "Okay, whatever. >.>" );
			return;
		}
		else if ( waitReply == WR_AM_I_RIGHT ) {
			int type = WordType( message );

			this->tokens.clear();

			if ( (type & (WT_TRUE|WT_FALSE)) == (WT_TRUE|WT_FALSE) ) {
				say( ":/" );
			} else if ( type & WT_TRUE ) {
				say( "Yay" );
			} else if ( type & WT_FALSE ) {
				say( "Ug, then fix my code or write better!" );
			} else if ( type & (WT_CANCEL_QUEST|WT_FILLER) ) {
				say( "Are you listening to me?" );
				waitForReply = WR_LISTENING_TO_ME;
			} else {
				say( "Guess not..." );
			}
			return;
		}
		else if ( waitReply == WR_LISTENING_TO_ME ) {
			int type = WordType( message );

			this->tokens.clear();

			if ( (type & (WT_TRUE|WT_FALSE)) == (WT_TRUE|WT_FALSE) ) {
				say( "ajskjfajsdhf" );
			} else if ( type & WT_FALSE ) {
				say( "...at least you're honest. ._.;" );
			} else if ( type & WT_TRUE ) {
				say( "Good, now answer my previous question." );
				this->waitForReply = WR_AM_I_RIGHT; // HARD CODE HACK
			} else if ( type & (WT_CANCEL_QUEST|WT_FILLER) ) {
				say( "Answer me." );
				this->waitForReply = waitReply; // press harder!
			} else {
				say( "Guess not..." );
			}
			return;
		}
	}

	this->tokens.parse( message );
}

void Persona::say( const String &message )
{
	printf("%s> %s\n", this->name.c_str(), message.c_str() );
}

bool Persona::checkSubject( int subject ) {

	bool valid = subject < this->tokens.getNumTokens();

	if ( valid && ( ( WordType( this->tokens[subject] ) & WT_FILLER ) || this->tokens[subject] == "?" ) )
		valid = false;

	return valid;
}

static double diffclock(clock_t clock1,clock_t clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks)/(CLOCKS_PER_SEC/1000);
    return diffms;
}

#define STF_YOUCOMPLETEME	1
#define STF_STATEMENT		2
//#define STF_IS_ARE			4 // split at 'is' or 'are', ignore '?' before that token
struct sentenceType_s {
	const char	*text;
	int			subjectStartToken;
	int			flags;
	//int			possiveNoun; // & 1 = singular, & 2 = plural
} sentenceTypes[] = {
	{ "Can ",		1, 0 }, // Can you X, Can your X Y? Can you jump? Can your cat jump?
	{ "Are",		1, 0 }, // ...
	{ "Could ",		1, 0 }, // ...
	{ "How could ",	2, 0 }, // ...
	{ "How did ",	2, 0 }, // ...
	{ "Would ",		1, 0 }, // ...
	{ "How are",	2, STF_YOUCOMPLETEME }, // How are you?
	{ "How is",		2, 0 }, // How is X, How is X Y, How is your X, How is your X Y, How is you? (not sure the last one makes sense. ever.)
	{ "What is ",	2, 0 }, // What is your name?
	{ "What's",		1, 0 }, // ...
	{ "What are ",	2, 0 }, // What are your favorite colors?
	{ "What goes",	2, 0 }, // What goes BOOM?
	{ "What does",	2, 0 }, // What does a cat say?
	//{ "What",		1, STF_IS_ARE }, // What X is Y? What X are Y? What? X is Y?

	// exclimation
	{ "What the ",	2, 0 }, // What the $word [is/are $word]

	// non-questions
	{ "I like ",	2, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ "I love ",	2, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ "I really like ",	3, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ "I really love ",	3, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ NULL, 0 }
};

#define GTF_LEAVING 1
#define GTF_SPECIAL 2 // one does not just say Merry Christmas whenever.
struct greetingType_s {
	const char	*text;
	int flags;
} greetingTypes[] = {
	{ "Hi", 0 },
	{ "Hello", 0 },
	{ "'ello", 0 },
	{ "ello", 0 },
	{ "Hey", 0 }, // not always a greeting...
	{ "ohai", 0 },
	{ "ohayou", 0 }, // Ohayou Gozaimasu
	{ "I acknowledge your existence", 0 },
	{ "Welcome", GTF_SPECIAL }, // not really special but don't want saying a lot
	{ "Good morning", GTF_SPECIAL },
	{ "Good afternoon", GTF_SPECIAL },
	{ "Good evening", GTF_SPECIAL },
	{ "Good night", GTF_LEAVING },
	{ "gn", GTF_LEAVING },
	{ "Merry Christmas", GTF_SPECIAL },

	{ NULL, 0 } // for random, NULL means repeat whatever greeting person said (including special ones).
};

struct statement_s {
	const char	*msg, *reply;
	bool		random;
} statements[] = {
	//{ "Yes", "No", true },
	//{ "No", "Yes", true },
	{ "Marco", "Polo", true },
	{ "You", "What?", false },
	{ "Dumb", "No you", false },
	{ "Your stupid", "*You're stupid. >.>", false },
	{ "You're stupid", "It's not my fault.", false },
	{ "Your dumb", "*You're dumb. <,<", false },
	{ "You're dumb", "It's not my fault.", false },
	{ "Dummy", "It's not my fault.", false },
	{ "Stupid", "It's not my fault.", false },
	{ "make", "You need to quit before you can rebuilt", false },
	{ NULL, NULL, false }
};

void Persona::think() {
#if 0 // ZTM: doesn't work if using select sleep in CLI main.cpp ...
	std::clock_t time = std::clock();

	// msec
	double sinceLastUpdate = diffclock(time, this->lastUpdate);

	// wait 2 seconds between updates.
	if ( sinceLastUpdate < 2 ) {
		printf("skip think, frametime=%f\n", sinceLastUpdate );
		return;
	}

	this->lastUpdate = time;

	printf("frametime=%f\n", sinceLastUpdate );
#endif

	if ( this->waitForReply != WR_NONE || !this->tokens.getNumTokens() ) {
		return;
	}

#if 0
	std::vector<String> sentencesTypes;
	sentencesTypes.push_back("Can $noun $verb?");
	sentencesTypes.push_back("Could $noun $verb?");
	sentencesTypes.push_back("Would $noun $verb?");
	sentencesTypes.push_back("What the $noun $verb?");
	sentencesTypes.push_back("What is $noun?"); // posesive your/Angel's
	sentencesTypes.push_back("What are $noun?"); // posesive your/Angel's
#endif

	String full = this->tokens.toString();

	if ( this->tokens[0] == this->name ) {
		bool enable = false;
		String s;

		if ( this->tokens[1] == "stop" ) {
			s = "Stopped";
			enable = false;
		} else if ( this->tokens[1] == "disable" ) {
			s = "Diabled";
			enable = false;
		} else if ( this->tokens[1] == "enable" ) {
			s = "Enabled";
			enable = true;
		}

		if ( this->tokens[2] == "fun" ) {
			this->funReplies = enable;
		}

		if ( !s.isEmpty() ) {
			s.append( " " );
			s.append( this->tokens.toString( 2 ) );
			s.append( " as you requested." );
			say( s );
			this->tokens.clear();
			return;
		}

		if ( this->tokens[1] == "set" ) {
			if ( this->tokens[2] == "name" ) {
				this->setName( this->tokens.toString( 3 ) );
			}
			if ( this->tokens[2] == "gender" ) {
				if ( tolower( this->tokens[3][0] ) == 'f' ) {
					this->setGender( GENDER_FEMALE );
				} else if ( tolower( this->tokens[3][0] ) == 'm' ) {
					this->setGender( GENDER_MALE );
				}
			}

			s.append( "Set " );
			s.append( this->tokens[2] );
			s.append( " to " );
			s.append( this->tokens.toString( 3 ) );
			s.append( " as you requested." );
			say( s );
			this->tokens.clear();
			return;
		}
	}

	bool didStatementGame = false;
	if ( !this->messageBate.isEmpty() ) {
		didStatementGame = true;
		if ( this->messageBate == full ) {
			say("yay!");
			this->messageBate = "";
			this->tokens.clear();
			return;
		} else {
			say("._.");
			this->messageBate = "";
			// keep going instead of ignoring me!
		}
	}

	for (int i = 0; statements[i].msg != NULL; ++i )
	{
		if ( full == statements[i].msg )
		{
			say( statements[i].reply );
			this->tokens.clear();
			return;
		}
	}

	for (int i = 0; greetingTypes[i].text != NULL; ++i )
	{
		if ( full.icompareTo( greetingTypes[i].text, strlen(greetingTypes[i].text) ) == 0 )
		{
			bool leaving = ( greetingTypes[i].flags & GTF_LEAVING );
			if ( ( greetingTypes[i].flags & GTF_SPECIAL ) && rand() & 1 ) {
				// repeat whatever they said, which could be a special greeting.
				say( greetingTypes[i].text );
			}
			else
#if 1
			while ( 1 ) {
				int r = rand() / (float)RAND_MAX * ARRAY_LEN( greetingTypes )-1;
				if ( greetingTypes[r].flags & GTF_SPECIAL )
					continue;
				if ( ( greetingTypes[r].flags & GTF_LEAVING ) != leaving )
					continue;
				if ( greetingTypes[r].text == NULL ) {
					// repeat whatever they said, which could be a special greeting.
					say( greetingTypes[i].text );
				} else {
					say( greetingTypes[r].text );
				}
				break;
			}
#else
			{
				static int greetings = 0; // this should be based on time since last greeting too.

				switch (greetings)
				{
					case 0:
						say( "Hello! ^_^" );	
						break;
						
					case 1:
						say( "Hello.. -.-" );	
						break;
						
					case 2:
						say( ">.>" );	
						break;
						
					case 3:
						say( "-.-" );	
						break;
						
					default:
						say( "..." );
						break;
				}
				
				greetings++;
			}
#endif
			this->tokens.clear();
			return;
		}
	}

	for (int i = 0; sentenceTypes[i].text != NULL; ++i )
	{
		if ( full.icompareTo( sentenceTypes[i].text, strlen(sentenceTypes[i].text) ) == 0 )
		{
			int subject, last;
			bool mine = false, me = false, questionMark, sarcasmHint = false;
			bool isAre = ( strstr( sentenceTypes[i].text, "are" ) != NULL );

			subject = sentenceTypes[i].subjectStartToken;

			// MAGIC HACK for What? ...
			if ( this->tokens[subject] == "?" )
			{
				subject++;
				sarcasmHint = true;
			}

			if ( this->tokens[subject] == "your" || this->tokens[subject] == this->namePossesive )
			{
				subject++;
				mine = true;
			}
			else if ( this->tokens[subject] == "you" || this->tokens[subject] == this->name )
			{
				subject++;
				me = true;
			}

			last = this->tokens.getNumTokens()-1;
			questionMark = ( this->tokens[last] == "?" );
			if ( this->tokens[last] == "?" || this->tokens[last] == "!" || this->tokens[last] == "." )
			{
				last--;
			}

			// check if it's all non-sense filler words
			int w;
			for ( w = subject; w <= last; w++ ) {
				if ( !( WordType( this->tokens[w] ) & WT_FILLER ) ) {
					break;
				}
			}

			// Ex: I like you
			if ( !isAre && me && (sentenceTypes[i].flags & STF_YOUCOMPLETEME)) {
				// nothing after 'you'?
				if ( subject > last ) {
					String s( "I ");
					s.append( this->tokens.toString( 1, sentenceTypes[i].subjectStartToken-1 ) );
					s.append( " you too" );
					s.append( questionMark ? "?" : "." );
					if ( rand() % 3 == 0 ) {
						s.append( " >_<" );
					}
					say( s );
					this->tokens.clear();
					return;
				} else {
					// Ex: I like you face. reply as if user said "your" instead of "you"
					me = false;
					mine = true;
				}
			}

			// Ex: I like your
			if (!isAre && mine && (sentenceTypes[i].flags & STF_YOUCOMPLETEME)) {
				// nothing after 'your'?
				if ( subject > last ) {
					String s( "You ");
					s.append( this->tokens.toString( 1, sentenceTypes[i].subjectStartToken-1 ) );
					s.append( " my what?" );
					say( s );
					this->waitForReply = WR_COMPLETE_LAST;
					// don't this->tokens.clear(); so that we know what reply context is. maybe a temporary hack?
				} else {
					//if ( !complimentItem( this->tokens.toString( subject, last ) )) {
						String s( "I might ");
						s.append( this->tokens.toString( 1, sentenceTypes[i].subjectStartToken-1 ) );
						s.append( " my " );
						s.append( this->tokens.toString( w, last ) );
						s.append( " if I knew what it was." );
					//}
					say( s );
					this->tokens.clear();
				}
				return;
			}

			if ( w > last && !(sentenceTypes[i].flags & STF_YOUCOMPLETEME) ) {
				say("What are you talking about?");
				this->waitForReply = WR_COMPLETE_LAST;
				// don't this->tokens.clear(); so that we know what reply context is. maybe a temporary hack?
				return;
			}

			// TODO: figure out how many tokens are the noun. >.>
			//verb = subject + 1;

			if ( this->funReplies && ( !(sentenceTypes[i].flags & STF_STATEMENT) || questionMark ) ) {
				if ( me || mine ) {
					if ( !isAre ) {
						if ( this->tokens[w].icompareTo( "name" ) == 0 )
						{
							static int toldTimes = 0;
							String s;
							switch ( toldTimes )
							{
								case 0:
									s = "My name is ";
									s.append(this->name);
									s.append(".");
									say( s );
									break;
								case 1:
									s = this->name;
									s.append(".");
									say( s );
									break;
								case 2:
									s = this->name;
									s.append("...");
									say( s );
									break;
								default:
									//TODO: set mood annoyed?
									say( "..." );
									break;
							}
							toldTimes++;
						}
						else if ( this->tokens[w].icompareTo( "favorite" ) == 0 )
						{
							// if subject is "?" remove it so reply is parsed instead next time
							if ( this->tokens[w+1] == "?" ) {
								this->tokens.removeToken( w+1 );
								last--;
							}

							if ( w+1 > last ) {
								say( "Favorite what?" );
								this->waitForReply = WR_COMPLETE_LAST;
								// don't this->tokens.clear(); so that we know what reply context is. maybe a temporary hack?
								// need to kill the bad token though
								return;
							}

							String s("I don't have a favorite ");
							s.append( this->tokens.toString( w+1, last ) ); // if use subject instead of w, repeats all the filler words
							s.append(".");
							say( s );
						}
						else
						{
							String s("haha ");
							s.append( this->tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
							s.append("?");
							say( s );
						}
					} else {
						// Ex: How are you?
						if (this->tokens[subject] == "?")
						{
							say( "Good, busy. How are you?" );
						}
						else
						{
							String s("Hmm, I don't know about ");
							s.append(this->tokens[subject]);
							if (this->tokens[subject] == "a" || this->tokens[subject] == "an" )
							{
								s.append( " " );
								s.append(this->tokens[subject+1]);
							}
							s.append(".");
							say( s );
						}
					}
				}
				else
				{
					{
						// if not talking about me, then I don't know.
						String s("Let's talk about me instead of ");
						s.append( this->tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
						s.append(", okay?");
						say( s );
					}
				}
				this->tokens.clear();
				return;
			}

			if ( this->funReplies && !me && !mine && (sentenceTypes[i].flags & STF_STATEMENT)) {
				String s( "Let's talk about me instead of ");
				if ( rand() % 3 == 0 ) {
					s.append( "boring old " );
				}
				s.append( this->tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
				s.append( "! :)" );
				say( s );
				this->tokens.clear();
				return;
			}

			// TODO: be able to talk about whatever this is.
			String s( "I think you're talking about ");
			if ( mine )
				s.append("my ");
			else if ( me )
				s.append("me "); // not proper american English... not sure what it should be.
			s.append( this->tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
			s.append( "?" );
			say( s );

			this->waitForReply = WR_AM_I_RIGHT;
			this->tokens.clear();
			return;
		}
	}

	int subject, last;
	bool mine = false, me = false, questionMark, sarcasmHint = false;

	subject = 0;

	if ( this->tokens[subject] == "What" ) {
		subject++;

		// MAGIC HACK for What? ...
		if ( this->tokens[subject] == "?" )
		{
			subject++;
			sarcasmHint = true;
		}
	}

	if ( this->tokens[subject] == "your" || this->tokens[subject] == this->namePossesive )
	{
		subject++;
		mine = true;
	}
	else if ( this->tokens[subject] == "you" || this->tokens[subject] == this->name )
	{
		subject++;
		me = true;
	}

	last = this->tokens.getNumTokens()-1;
	questionMark = ( this->tokens[last] == "?" );
	if ( this->tokens[last] == "?" || this->tokens[last] == "!" || this->tokens[last] == "." )
	{
		last--;
	}

	// check if it's all non-sense filler words
	int w;
	for ( w = subject; w <= last; w++ ) {
		if ( !( WordType( this->tokens[w] ) & WT_FILLER ) ) {
			break;
		}
	}

	int tokenIs = tokens.findExact( "is" );
	int tokenAre = tokens.findExact( "are" );
	// Ex: What game is fun? / What games are fun? / What? Games are fun?
	if ( tokenIs >= 0 || tokenAre >= 0 ) {
		int firstSplit;
		int subjectB;
		bool mineB = false, meB = false;

		firstSplit = tokenIs;
		if ( tokenIs < 0 || ( tokenAre >= 0 && tokenAre < tokenIs ) ) {
			firstSplit = tokenAre;
		}

		subjectB = firstSplit+1;

		if ( this->tokens[subjectB] == "your" || this->tokens[subjectB] == this->namePossesive )
		{
			subjectB++;
			mineB = true;
		}
		else if ( this->tokens[subjectB] == "you" || this->tokens[subjectB] == this->name )
		{
			subjectB++;
			meB = true;
		}

		String partA = this->tokens.toString( w, firstSplit-1 ); // if use subject instead of w, repeats all the filler words

		// check if it's all non-sense filler words
		int w2;
		for ( w2 = subjectB; w2 <= last; w2++ ) {
			if ( !( WordType( this->tokens[w2] ) & WT_FILLER ) ) {
				break;
			}
		}

		if ( w2 > last ) {
			say("What are you talking about?");
			this->waitForReply = WR_COMPLETE_LAST;
			// don't this->tokens.clear(); so that we know what reply context is. maybe a temporary hack?
			return;
		}

		String partB = this->tokens.toString( subjectB, last ); // if use w2 instead of subject2, removes the filler words

		// TODO: be able to talk about whatever this is.
		String s( "I think you're talking about ");

		// oh shit Ex: What time is it?
		if ( partB == "it" ) {
			if ( mine || mineB || me || meB ) {
				if ( firstSplit == subject ) // Ex: You are smart. Say: Me
					s.append( "me being " );
				else
					s.append( "my " );
			} else if ( firstSplit == tokenIs && w2 == subjectB ) { // if 'is' and no filler words
				s.append( "the " );
			}
			// Ex: You are it?
			if ( firstSplit == subject ) {

			} else {
				s.append( partA );
				s.append( " " );
			}

			// replace 'it' (aka partB) with...
			if ( mine || mineB || me || meB ) {
				if ( firstSplit != subject ) // if haven't already said 'being'
					s.append( "being " );
			} else {
				s.append( "of " );
			}
			s.append( "something" );

			s.append( "?" );
			if ( sarcasmHint )
				s.append( " With maybe a hint of sarcasm?" );
		} else {
			if ( mine || mineB || me || meB ) {
				if ( firstSplit == subject ) // Ex: You are smart. Say: Me
					s.append( "me being " );
				else
					s.append( "my " );
			} else if ( firstSplit == tokenIs && w2 == subjectB ) { // if 'is' and no filler words
				s.append( "a " );
			}
			s.append( partB );
			// Ex: You are smart. Don't say: my smart _are_.
			if ( firstSplit != subject ) {
				s.append( " " );
				s.append( partA );
			}
			s.append( "?" );
			if ( sarcasmHint )
				s.append( " With maybe a hint of sarcasm?" );
		}
		say( s );

		this->waitForReply = WR_AM_I_RIGHT;
		this->tokens.clear();
		return;
	}

	if ( !didStatementGame && this->funReplies ) {
		for ( int i = 0; i < ARRAY_LEN( statements ); i++ ) {
			// fail to find anything to say, so just mess with them.
			int st = rand() / (float)RAND_MAX * ARRAY_LEN( statements )-1;
			if ( !statements[st].random )
				continue;
			say( statements[st].msg );
			this->messageBate.setData( statements[st].reply );
			this->tokens.clear();
			return;
		}
	}

	say( "I don't know how to parse that statement, sorry." );
	this->tokens.clear();
}

} // end namespace AngelCommunication

