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
	this->autoChat = true;
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

void Persona::setAutoChat( bool autoChat )
{
	this->autoChat = autoChat;
}

const String &Persona::getName( void ) const
{
	return this->name;
}

void Persona::personaConnect( Conversation *con, Persona *persona ) {
	if ( !this->autoChat )
		return;

	String s;
	s = "Hi "; // TODO: use random greeting select here.
	s.append( persona->name );
	s.append( "." );

	// FIXME: disabled so bots don't get stuck replying to each other from the get go.
	if ( !persona->autoChat )
		con->addMessage( this, s );
}

void Persona::receiveMessage( Conversation *con, Persona *speaker, const String &text )
{
	if ( !this->autoChat )
		return;

	this->messages.push_back( new Message( con, speaker, text ) );
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
	int			subjectStartToken;
	int			flags;
} greetingTypes[] = {
	{ "Hi",				1,	0 },
	{ "Hello",			1,	0 },
	{ "'ello",			1,	0 },
	{ "ello",			1,	0 },
	{ "Hey",			1,	0 }, // not always a greeting...
	{ "ohai",			1,	0 },
	{ "ohayou",			1,	0 }, // Ohayou Gozaimasu
	{ "I acknowledge your existence", 4, 0 },
	{ "Welcome",		1,	GTF_SPECIAL }, // not really special but don't want saying a lot
	{ "Good morning",	2,	GTF_SPECIAL },
	{ "Good afternoon",	2,	GTF_SPECIAL },
	{ "Good evening",	2,	GTF_SPECIAL },
	{ "Good night",		2,	GTF_LEAVING },
	{ "gn",				1,	GTF_LEAVING },
	{ "Merry Christmas",2,	GTF_SPECIAL },

	{ NULL, 0, 0 } // for random, NULL means repeat whatever greeting person said (including special ones).
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
	if ( !this->autoChat ) {
		// TODO: drop messages and expectations?
		return;
	}

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

	// TODO: limit how fast to process messages?
	size_t numMessages = this->messages.size();

	for ( size_t i = 0; i < numMessages; /**/ ) {
		if ( processMessage( this->messages[i] ) ) {
			delete this->messages[i];
			this->messages.erase( this->messages.begin() + i );
			--numMessages;
		} else {
			++i;
		}
	}
}

bool Persona::processMessage( Message *message )
{
	Conversation *con = message->con;
	Persona *from = message->from;
	String full( message->text );
	Lexer tokens( full );

	// ignore pointless auto chat (otherwise bots get stuck repeating it...)
	if ( full == "I don't know how to parse that statement, sorry." )
		return true;

	// TODO: ignore messages that start with someone elses name?

	if ( tokens[0] == this->name ) {
		bool enable = false;
		String s;

		if ( tokens[1] == "stop" ) {
			s = "Stopped";
			enable = false;
		} else if ( tokens[1] == "disable" ) {
			s = "Diabled";
			enable = false;
		} else if ( tokens[1] == "enable" ) {
			s = "Enabled";
			enable = true;
		}

		if ( tokens[2] == "fun" ) {
			this->funReplies = enable;
		}

		if ( !s.isEmpty() ) {
			s.append( " " );
			s.append( tokens.toString( 2 ) );
			s.append( " as you requested." );
			con->addMessage( this, s );
			return true;
		}

		if ( tokens[1] == "set" ) {
			if ( tokens[2] == "name" ) {
				this->setName( tokens.toString( 3 ) );
			}
			if ( tokens[2] == "gender" ) {
				if ( tolower( tokens[3][0] ) == 'f' ) {
					this->setGender( GENDER_FEMALE );
				} else if ( tolower( tokens[3][0] ) == 'm' ) {
					this->setGender( GENDER_MALE );
				}
			}

			s.append( "Set " );
			s.append( tokens[2] );
			s.append( " to " );
			s.append( tokens.toString( 3 ) );
			s.append( " as you requested." );
			con->addMessage( this, s );
			return true;
		}
	}

	bool didStatementGame = false;

	// check if expecting something from this persona
	size_t numExp = this->expectations.size();
	for ( size_t i = 0; i < numExp; /**/ ) {

		// compare pointers
		if ( this->expectations[i]->con == con && this->expectations[i]->from == from )
		{
			WaitReply waitReply = this->expectations[i]->waitForReply;
			bool freeExp = true;
			bool freeMessage = true;

			if ( waitReply == WR_COMPLETE_LAST ) {
				if ( ( WordType( full ) & (WT_CANCEL_QUEST|WT_FILLER) ) ) {
					con->addMessage( this, "Okay, whatever. >.>" );
					// free exp and message
				} else {
					String mergedText = this->expectations[i]->expstr;
					mergedText.append( full );
					full = mergedText;

					tokens.parse( full );

					// free exp, but still use message reply code
					freeMessage = false;
				}
			}
			else if ( waitReply == WR_AM_I_RIGHT ) {
				int type = WordType( full );

				if ( (type & (WT_TRUE|WT_FALSE)) == (WT_TRUE|WT_FALSE) ) {
					con->addMessage( this, ":/" );
				} else if ( type & WT_TRUE ) {
					con->addMessage( this, "Yay" );
				} else if ( type & WT_FALSE ) {
					con->addMessage( this, "Ug, then fix my code or write better!" );
				} else if ( type & (WT_CANCEL_QUEST|WT_FILLER) ) {
					con->addMessage( this, "Are you listening to me?" );
					// mutate the expectation
					this->expectations[i]->waitForReply = WR_LISTENING_TO_ME;
					freeExp = false;
				} else {
					con->addMessage( this, "Guess not..." );
				}
			}
			else if ( waitReply == WR_LISTENING_TO_ME ) {
				int type = WordType( full );

				if ( (type & (WT_TRUE|WT_FALSE)) == (WT_TRUE|WT_FALSE) ) {
					con->addMessage( this, "ajskjfajsdhf" );
				} else if ( type & WT_FALSE ) {
					con->addMessage( this, "...at least you're honest. ._.;" );
				} else if ( type & WT_TRUE ) {
					con->addMessage( this, "Good, now answer my previous question." );
					// mutate the expectation
					this->expectations[i]->waitForReply = WR_AM_I_RIGHT; // HARD CODE HACK
					freeExp = false;
					return true;
				} else if ( type & (WT_CANCEL_QUEST|WT_FILLER) ) {
					String s(from->getName());
					s.append(", answer me.");
					con->addMessage( this, s );
					// press harder! (don't release expectation)
					freeExp = false;
					return true;
				} else {
					con->addMessage( this, "Guess not..." );
				}
			} else if ( waitReply == WR_SPECIFIED ) {
				didStatementGame = true;
				if ( this->expectations[i]->expstr == full ) {
					con->addMessage( this, "yay!" );
				} else {
					con->addMessage( this, "._." );
					// keep going instead of ignoring message
					freeMessage = false;
				}
			}

			if ( freeExp ) {
				delete this->expectations[i];
				this->expectations.erase( this->expectations.begin() + i );
				--numExp;
			} else {
				++i;
			}

			if ( freeMessage ) {
				return true;
			} else {
				// FIXME: what if there are multiple expectation?
				break;
			}
		} else {
			++i;
		}
	}

	for (int i = 0; statements[i].msg != NULL; ++i )
	{
		if ( full == statements[i].msg )
		{
			con->addMessage( this, statements[i].reply );
			return true;
		}
	}

	for (int i = 0; greetingTypes[i].text != NULL; ++i )
	{
		if ( full.icompareTo( greetingTypes[i].text, strlen(greetingTypes[i].text) ) == 0 )
		{
			bool leaving = ( greetingTypes[i].flags & GTF_LEAVING );

			// Ex: Hi Bob
			if ( greetingTypes[i].subjectStartToken < tokens.getNumTokens() ) {
				if ( tokens[greetingTypes[i].subjectStartToken].icompareTo( this->name ) != 0 ) {
					// greeted someone else, ignore.
					return true;
				}
			}

			if ( ( greetingTypes[i].flags & GTF_SPECIAL ) && rand() & 1 ) {
				// repeat whatever they said, which could be a special greeting.
				con->addMessage( this, greetingTypes[i].text );
			}
			else
#if 1
			while ( 1 ) {
				int r = rand() / (float)RAND_MAX * ARRAY_LEN( greetingTypes )-1;
				if ( greetingTypes[r].flags & GTF_SPECIAL )
					continue;
				if ( ( greetingTypes[r].flags & GTF_LEAVING ) != leaving )
					continue;

				String s;

				if ( greetingTypes[r].text == NULL ) {
					// repeat whatever they said, which could be a special greeting.
					s = greetingTypes[i].text;
				} else {
					s = greetingTypes[r].text;
				}

				s.append( " " );
				s.append( from->getName() );
				con->addMessage( this, s );
				break;
			}
#else
			{
				static int greetings = 0; // this should be based on time since last greeting too.

				switch (greetings)
				{
					case 0:
						con->addMessage( this, "Hello! ^_^" );
						break;
						
					case 1:
						con->addMessage( this, "Hello.. -.-" );
						break;
						
					case 2:
						con->addMessage( this, ">.>" );
						break;
						
					case 3:
						con->addMessage( this, "-.-" );
						break;
						
					default:
						con->addMessage( this, "..." );
						break;
				}
				
				greetings++;
			}
#endif
			return true;
		}
	}

	for (int i = 0; sentenceTypes[i].text != NULL; ++i )
	{
		if ( full.icompareTo( sentenceTypes[i].text, strlen(sentenceTypes[i].text) ) == 0 )
		{
			int subject, last;
			bool mine = false, me = false, questionMark;
			bool isAre = ( strstr( sentenceTypes[i].text, "are" ) != NULL );

			subject = sentenceTypes[i].subjectStartToken;

			// MAGIC HACK for What? ...
			/*if ( tokens[subject] == "?" )
			{
				subject++;
			}*/

			if ( tokens[subject] == "your" || tokens[subject] == this->namePossesive )
			{
				subject++;
				mine = true;
			}
			else if ( tokens[subject] == "you" || tokens[subject] == this->name )
			{
				subject++;
				me = true;
			}

			last = tokens.getNumTokens()-1;
			questionMark = ( tokens[last] == "?" );
			if ( tokens[last] == "?" || tokens[last] == "!" || tokens[last] == "." )
			{
				last--;
			}

			// check if it's all non-sense filler words
			int w;
			for ( w = subject; w <= last; w++ ) {
				if ( !( WordType( tokens[w] ) & WT_FILLER ) ) {
					break;
				}
			}

			// Ex: I like you
			if ( !isAre && me && (sentenceTypes[i].flags & STF_YOUCOMPLETEME)) {
				// nothing after 'you'?
				if ( subject > last ) {
					String s( "I ");
					s.append( tokens.toString( 1, sentenceTypes[i].subjectStartToken-1 ) );
					s.append( " you too" );
					s.append( questionMark ? "?" : "." );
					if ( rand() % 3 == 0 ) {
						s.append( " >_<" );
					}
					con->addMessage( this, s );
					return true;
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
					s.append( tokens.toString( 1, sentenceTypes[i].subjectStartToken-1 ) );
					s.append( " my what?" );
					con->addMessage( this, s );
					// create expectation (save message text)
					addExpectation( con, from, WR_COMPLETE_LAST, full );
				} else {
					//if ( !complimentItem( tokens.toString( subject, last ) )) {
						String s( "I might ");
						s.append( tokens.toString( 1, sentenceTypes[i].subjectStartToken-1 ) );
						s.append( " my " );
						s.append( tokens.toString( w, last ) );
						s.append( " if I knew what it was." );
					//}
					con->addMessage( this, s );
				}
				return true;
			}

			if ( w > last && !(sentenceTypes[i].flags & STF_YOUCOMPLETEME) ) {
				con->addMessage( this, "What are you talking about?" );
				// create expectation (save message text)
				addExpectation( con, from, WR_COMPLETE_LAST, full );
				return true;
			}

			// TODO: figure out how many tokens are the noun. >.>
			//verb = subject + 1;

			if ( this->funReplies && ( !(sentenceTypes[i].flags & STF_STATEMENT) || questionMark ) ) {
				if ( me || mine ) {
					if ( !isAre ) {
						if ( tokens[w].icompareTo( "name" ) == 0 )
						{
							static int toldTimes = 0;
							String s;
							switch ( toldTimes )
							{
								case 0:
									s = "My name is ";
									s.append(this->name);
									s.append(".");
									break;
								case 1:
									s = this->name;
									s.append(".");
									break;
								case 2:
									s = this->name;
									s.append("...");
									break;
								default:
									//TODO: set mood annoyed?
									s = "...";
									break;
							}
							con->addMessage( this, s );
							toldTimes++;
						}
						else if ( tokens[w].icompareTo( "favorite" ) == 0 )
						{
							// if subject is "?" remove it so reply is parsed instead next time
							if ( tokens[w+1] == "?" ) {
								tokens.removeToken( w+1 );
								last--;
							}

							if ( w+1 > last ) {
								con->addMessage( this, "Favorite what?" );

								// create expectation (save message text)
								addExpectation( con, from, WR_COMPLETE_LAST, tokens.toString() );
								return true;
							}

							String s("I don't have a favorite ");
							s.append( tokens.toString( w+1, last ) ); // if use subject instead of w, repeats all the filler words
							s.append(".");
							con->addMessage( this, s );
						}
						else
						{
							String s("haha ");
							s.append( tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
							s.append("?");
							con->addMessage( this, s );
						}
					} else {
						// Ex: How are you?
						if (tokens[subject] == "?")
						{
							con->addMessage( this, "Good, busy. How are you?" );
						}
						else
						{
							String s("Hmm, I don't know about ");
							s.append(tokens[subject]);
							if (tokens[subject] == "a" || tokens[subject] == "an" )
							{
								s.append( " " );
								s.append(tokens[subject+1]);
							}
							s.append(".");
							con->addMessage( this, s );
						}
					}
				}
				else
				{
					{
						// if not talking about me, then I don't know.
						String s("Let's talk about me instead of ");
						s.append( tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
						s.append(", okay?");
						con->addMessage( this, s );
					}
				}
				return true;
			}

			if ( this->funReplies && !me && !mine && (sentenceTypes[i].flags & STF_STATEMENT)) {
				String s( "Let's talk about me instead of ");
				if ( rand() % 3 == 0 ) {
					s.append( "boring old " );
				}
				s.append( tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
				s.append( "! :)" );
				con->addMessage( this, s );
				return true;
			}

			// TODO: be able to talk about whatever this is.
			String s( "I think you're talking about ");
			if ( mine )
				s.append("my ");
			else if ( me )
				s.append("me "); // not proper american English... not sure what it should be.
			s.append( tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
			s.append( "?" );
			con->addMessage( this, s );

			// create expectation (don't save message text)
			addExpectation( con, from, WR_AM_I_RIGHT );
			return true;
		}
	}

	int subject, last;
	bool mine = false, me = false, questionMark, sarcasmHint = false;

	subject = 0;

	if ( tokens[subject] == "What" ) {
		subject++;

		// MAGIC HACK for What? ...
		if ( tokens[subject] == "?" )
		{
			subject++;
			sarcasmHint = true;
		}
	}

	if ( tokens[subject] == "your" || tokens[subject] == this->namePossesive )
	{
		subject++;
		mine = true;
	}
	else if ( tokens[subject] == "you" || tokens[subject] == this->name )
	{
		subject++;
		me = true;
	}

	last = tokens.getNumTokens()-1;
	questionMark = ( tokens[last] == "?" );
	if ( tokens[last] == "?" || tokens[last] == "!" || tokens[last] == "." )
	{
		last--;
	}

	// check if it's all non-sense filler words
	int w;
	for ( w = subject; w <= last; w++ ) {
		if ( !( WordType( tokens[w] ) & WT_FILLER ) ) {
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

		if ( tokens[subjectB] == "your" || tokens[subjectB] == this->namePossesive )
		{
			subjectB++;
			mineB = true;
		}
		else if ( tokens[subjectB] == "you" || tokens[subjectB] == this->name )
		{
			subjectB++;
			meB = true;
		}

		String partA = tokens.toString( w, firstSplit-1 ); // if use subject instead of w, repeats all the filler words

		// check if it's all non-sense filler words
		int w2;
		for ( w2 = subjectB; w2 <= last; w2++ ) {
			if ( !( WordType( tokens[w2] ) & WT_FILLER ) ) {
				break;
			}
		}

		if ( w2 > last ) {
			con->addMessage( this, "What are you talking about?" );

			// create expectation (save message text)
			addExpectation( con, from, WR_COMPLETE_LAST, full );
			return true;
		}

		String partB = tokens.toString( subjectB, last ); // if use w2 instead of subject2, removes the filler words

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
		con->addMessage( this, s );

		// create expectation (don't save message text)
		addExpectation( con, from, WR_AM_I_RIGHT );
		return true;
	}

	if ( con->numPersonas() == 2 && !didStatementGame && this->funReplies ) {
		for ( int i = 0; i < ARRAY_LEN( statements ); i++ ) {
			// fail to find anything to say, so just mess with them.
			int st = rand() / (float)RAND_MAX * ARRAY_LEN( statements )-1;
			if ( !statements[st].random )
				continue;
			con->addMessage( this, statements[st].msg );
			addExpectation( con, from, WR_SPECIFIED, statements[st].reply );
			return true;
		}
	}

	con->addMessage( this, "I don't know how to parse that statement, sorry." );
	return true;
}

void Persona::addExpectation( Conversation *c, Persona *f, WaitReply wr )
{
	this->expectations.push_back( new Expectation( c, f, wr ) );
}

void Persona::addExpectation( Conversation *c, Persona *f, WaitReply wr, const String &str )
{
	this->expectations.push_back( new Expectation( c, f, wr, str ) );
}

} // end namespace AngelCommunication

