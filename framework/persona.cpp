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

#include "angel.h"
#include "persona.h"
#include "wordtypes.h"

namespace AngelCommunication
{

Persona::Persona()
{
	this->nick = "unknown";
	this->fullName = "unknown";
	this->gender = GENDER_NONE;
	this->autoChat = true;
	this->funReplies = true;

	this->nextUpdateTime = std::time( NULL ) + 2;
}

void Persona::tryNick( const String &nick )
{
	if ( !this->nick.icompareTo( "unknown" ) ) {
		// HACK for initial nick set :/
		updateNick( nick );
	} else {
		String oldnick = this->nick;
		ANGELC_PersonaRename( oldnick.c_str(), nick.c_str() );
	}
}

void Persona::updateNick( const String &nick )
{
	this->nick = nick;
	this->nickPossesive = nick;
	if ( this->nick[this->nick.getLen()-1] == 's' )
		this->nickPossesive.append( "'" );
	else
		this->nickPossesive.append( "'s" );
}

void Persona::setFullName( const String &fullName )
{
	this->fullName = fullName;
}

void Persona::setGender( Gender gender )
{
	this->gender = gender;
}

void Persona::setAutoChat( bool autoChat )
{
	this->autoChat = autoChat;
}

const String &Persona::getNick( void ) const
{
	return this->nick;
}

const String &Persona::getFullName( void ) const
{
	return this->fullName;
}

void Persona::personaConnect( Conversation *con, Persona *persona ) {
	if ( !this->autoChat )
		return;

	String s;
	s = "Hi "; // TODO: use random greeting select here.
	s.append( persona->nick );
	s.append( "." );

	// FIXME: disabled so bots don't get stuck replying to each other from the get go.
	if ( !persona->autoChat )
		con->addMessage( this, s );
}

void Persona::receiveMessage( Conversation *con, Persona *speaker, const String &text, int messageNum, const String &addressee )
{
	if ( !this->autoChat )
		return;

	std::time_t currentTime = std::time( NULL );

	// wait 2 seconds between updates.
	if ( currentTime >= this->nextUpdateTime ) {
		this->nextUpdateTime = time( NULL ) + 2;
	}

	this->messages.push_back( new Message( con, speaker, text, messageNum, addressee ) );
}

float Persona::getSleepTime() {
	std::time_t currentTime = std::time( NULL );

	// waiting for new messages. doesn't care when next update is.
	if ( currentTime >= this->nextUpdateTime )
		return -1;

	return this->nextUpdateTime - currentTime;
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
	{ "I want to ",	3, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ "I want ",	2, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ "I like ",	2, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ "I love ",	2, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ "I really like ",	3, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ "I really love ",	3, STF_YOUCOMPLETEME | STF_STATEMENT },
	{ NULL, 0 }
};

#define GTF_SPECIAL 1 // one does not just say Merry Christmas whenever.
#define GTF_BYE		2
#define GTF_NIGHT	4
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
	{ "Bye",			1,	GTF_BYE },
	{ "Good bye",		2,	GTF_BYE },
	{ "Goodbye",		1,	GTF_BYE },
	{ "Good night",		2,	GTF_NIGHT },
	{ "gn",				1,	GTF_NIGHT },
	{ "sleep",			1,	GTF_NIGHT },
	{ "sleep time",		2,	GTF_NIGHT },
	{ "sleep taim",		2,	GTF_NIGHT },
	{ "Welcome",		1,	GTF_SPECIAL }, // not really special but don't want saying a lot
	{ "Good morning",	2,	GTF_SPECIAL },
	{ "Good afternoon",	2,	GTF_SPECIAL },
	{ "Good evening",	2,	GTF_SPECIAL },
	{ "Merry Christmas",2,	GTF_SPECIAL },

	{ NULL, 0, 0 } // for random, NULL means repeat whatever greeting person said (including special ones).
};

int Persona::GetGreetingAddressee( const Lexer &messageTokens, String &messageAddressee )
{
	String full = messageTokens.toString(); // this kind of sucks.

	messageAddressee = "";

	for (int i = 0; greetingTypes[i].text != NULL; ++i )
	{
		if ( full.icompareTo( greetingTypes[i].text, strlen(greetingTypes[i].text) ) == 0 )
		{
			// Ex: Hi Bob
			if ( greetingTypes[i].subjectStartToken < messageTokens.getNumTokens() ) {
				messageAddressee = messageTokens[greetingTypes[i].subjectStartToken];
			}

			return i;
		}
	}

	return -1;
}

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
	{ "Can I ask a question", "Don't ask to ask. Just ask your question.", false },
	{ "Can I ask you a question", "Don't ask to ask. Just ask your question.", false },
	{ NULL, NULL, false }
};

void Persona::think() {
	if ( !this->autoChat ) {
		// TODO: drop messages and expectations?
		return;
	}

	if ( getSleepTime() > 0 ) {
		return;
	}

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

bool matchPrase( const Lexer &tokens, const String &expect ) {
	int last;

	last = tokens.getNumTokens()-1;
	if ( tokens[last] == "?" || tokens[last] == "!" || tokens[last] == "." )
	{
		last--;
	}

	if ( !tokens.toString( 0, last ).icompareTo( expect ) )
		return true;

	return false;
}

bool Persona::processMessage( Message *message )
{
	Conversation *con = message->con;
	Persona *from = message->from;
	String full( message->text );
	int messageNum = message->messageNum;
	String addressee = message->addressee;
	bool isAddressedToAnyone = !addressee.icompareTo( "*anybody" );
	bool isAddressedToMe = !this->nick.icompareTo( addressee );
	bool isAddressee = ( isAddressedToAnyone || isAddressedToMe );
	Lexer tokens( full );

	// hmm... these message might be useful for learning about people.
	// all this funtion does is reply so ignore messages not addressed to this bot for now.
	if ( !isAddressee ) {
		return true;
	}

	if ( tokens[0] == this->nick ) {
		bool tookAction = false;
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

		// if one of the above commands
		if ( !s.isEmpty() ) {
			if ( tokens[2] == "fun" ) {
				this->funReplies = enable;
				tookAction = true;
			}

			if ( tookAction ) {
				s.append( " " );
				s.append( tokens.toString( 2 ) );
				s.append( " as you requested." );
				con->addMessage( this, s );
			} else {
				con->addMessage( this, "Hmm? I don't understand" );
				return true;
			}
			return true;
		}

		if ( tokens[1] == "set" ) {
			if ( tokens[2] == "name" || tokens[2] == "nick" ) {
				this->tryNick( tokens.toString( 3 ) );
				tookAction = true;
			}
			else if ( tokens[2] == "gender" ) {
				if ( tolower( tokens[3][0] ) == 'f' ) {
					this->setGender( GENDER_FEMALE );
					tookAction = true;
				} else if ( tolower( tokens[3][0] ) == 'm' ) {
					this->setGender( GENDER_MALE );
					tookAction = true;
				}
			}

			if ( tookAction ) {
				s.append( "Set " );
				s.append( tokens[2] );
				s.append( " to " );
				s.append( tokens.toString( 3 ) );
				s.append( " as you requested." );
				con->addMessage( this, s );
				return true;
			} else {
				con->addMessage( this, "Hmm? I don't understand" );
				return true;
			}
		}

		// just said our name with nothing else or name followed by one of :,?!.~ or something
		if ( tokens.getNumTokens() == 1 || ( tokens.getNumTokens() == 2 && tokens[1].getLen() == 1 ) ) {
			// FIXME: if there is a later message from persona already don't need to respond here. Maybe add a time based expectation?
			//			also, handling this *before* expectations causes badness. Should either drop expections (seems wrong) or ... I'm not sure.
			con->addMessage( this, "Yes?" );
			return true;
		}

		// Ex: "Bob:" or "Bob,"
		// Remove bot name followed by colon or comma as they (usually) are just to show you're addressing "Bob" (which has already been detected)
		if ( tokens[1] == ":" || tokens[1] == "," ) {
			tokens.removeToken( 0 ); // remove name
			tokens.removeToken( 0 ); // remove colon or comma. 0 again because now it's the first token.
			full = tokens.toString();
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

			if ( waitReply == WR_SPECIFIED ) {
				// don't add more statement games
				didStatementGame = true;
			}

			if ( messageNum <= this->expectations[i]->messageNum ) {
				// message is older than expectation. it's not a response.
				++i;
				continue;
			}

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
					freeMessage = false;	// still respone to whatever was said. because I usually don't answer this...
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
					String s(from->getNick());
					s.append(", answer me.");
					con->addMessage( this, s );
					// press harder! (don't release expectation)
					freeExp = false;
					return true;
				} else {
					con->addMessage( this, "Guess not..." );
				}
			} else if ( waitReply == WR_SPECIFIED ) {
				if ( matchPrase( tokens, this->expectations[i]->expstr ) ) {
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
		if ( matchPrase( tokens, statements[i].msg ) )
		{
			con->addMessage( this, statements[i].reply );
			return true;
		}
	}

	// FIXME: bot doesn't actually care about greeting addressee here (but reuses code to get greeting num),
	//		  Conversation::addMessage put addressee in message->addressee which improves handling a lot vs just deciding based on *this* message.
	String greetingAddressee;
	int greetingNum = GetGreetingAddressee( full, greetingAddressee );
	if ( greetingNum != -1 && !isAddressee ) {
		// greeted someone else
		return true;
	}
	else if ( greetingNum != -1 && isAddressee ) {
		// greeted us or /everyone/
		int i = greetingNum;
		bool bye = !!( greetingTypes[i].flags & GTF_BYE );
		bool night = !!( greetingTypes[i].flags & GTF_NIGHT );

		if ( ( greetingTypes[i].flags & GTF_SPECIAL ) && rand() & 1 ) {
			// repeat whatever they said, which could be a special greeting.
			con->addMessage( this, greetingTypes[i].text );
		}
		else
		{
			for ( int n = 0; n < ARRAY_LEN( greetingTypes ); n++ ) {
				int r = rand() / (float)RAND_MAX * ARRAY_LEN( greetingTypes )-1;
				if ( greetingTypes[r].flags & GTF_SPECIAL )
					continue;
				if ( !!( greetingTypes[r].flags & GTF_NIGHT ) != night )
					continue;
				if ( !!( greetingTypes[r].flags & GTF_BYE ) != bye )
					continue;

				String s;

				if ( greetingTypes[r].text == NULL ) {
					// repeat whatever they said, which could be a special greeting.
					s = greetingTypes[i].text;
				} else {
					s = greetingTypes[r].text;
				}

				s.append( " " );
				s.append( from->getNick() );
				con->addMessage( this, s );
				break;
			}
		}

		return true;
	}


	for (int i = 0; sentenceTypes[i].text != NULL; ++i )
	{
		if ( full.icompareTo( sentenceTypes[i].text, strlen(sentenceTypes[i].text) ) == 0 )
		{
			int subject, last;
			bool mine = false, me = false, belongsToSender = false, questionMark;
			bool isAre = ( strstr( sentenceTypes[i].text, "are" ) != NULL );

			subject = sentenceTypes[i].subjectStartToken;

			if ( tokens[subject] == "my" )
			{
				subject++;
				belongsToSender = true;
			}
			else if ( tokens[subject] == "your" || tokens[subject] == this->nickPossesive )
			{
				subject++;
				mine = true;
			}
			else if ( tokens[subject] == "you" || tokens[subject] == this->nick )
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
									s.append(this->fullName);
									s.append(".");
									break;
								case 1:
									s = this->fullName;
									s.append(".");
									break;
								case 2:
									s = this->fullName;
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
							con->addMessage( this, "Good." );
							// Bots get stuck replying with this
							//con->addMessage( this, "Good. How are you?" );
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
						if ( belongsToSender )
							s.append("your ");
						s.append( tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
						s.append(".");
						con->addMessage( this, s );
					}
				}
				return true;
			}

			if ( this->funReplies && !me && !mine && !belongsToSender && (sentenceTypes[i].flags & STF_STATEMENT)) {
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
			else if ( belongsToSender )
				s.append("your ");
			s.append( tokens.toString( w, last ) ); // if use subject instead of w, repeats all the filler words
			s.append( "?" );
			con->addMessage( this, s );

			// create expectation (don't save message text)
			addExpectation( con, from, WR_AM_I_RIGHT );
			return true;
		}
	}

	int subject, last;
	bool mine = false, me = false, belongsToSender = false, questionMark, sarcasmHint = false;
	bool castleTheIsAre = false;

	subject = 0;

	if ( tokens[subject] == "Anyone" && tokens[subject+1] == "know" ) {
		subject += 2;
		// Ex: Anyone know where my shoe is?
		castleTheIsAre = true;
	}

	if ( tokens[subject] == "What" ) {
		subject++;

		// MAGIC HACK for What? ...
		// FIXME: lines are split at "?" now.
		/*if ( tokens[subject] == "?" )
		{
			subject++;
			sarcasmHint = true;
		}*/
	}

	int tokenIs = -1;
	if ( tokens[subject] == "What's" ) {
		tokenIs = subject;
		subject++;
	}

	if ( tokens[subject] == "my" )
	{
		subject++;
		belongsToSender = true;
	}
	else if ( tokens[subject] == "your" || tokens[subject] == this->nickPossesive )
	{
		subject++;
		mine = true;
	}
	else if ( tokens[subject] == "you" || tokens[subject] == this->nick )
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

	tokenIs = tokenIs != -1 ? tokenIs : tokens.findExact( "is" );
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

		// PROFORM THE CASTLING MOVEMENT OF THE IS or ARE IF AT END OF SENTENCE
		if ( castleTheIsAre && firstSplit+1 >= tokens.getNumTokens() ) {
			firstSplit = subject;
			last--;
		}

		subjectB = firstSplit+1;

		if ( tokens[subjectB] == "your" || tokens[subjectB] == this->nickPossesive )
		{
			subjectB++;
			mineB = true;
		}
		else if ( tokens[subjectB] == "you" || tokens[subjectB] == this->nick )
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
			} else if ( belongsToSender ) {
				s.append( "your " );
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
			} else if ( belongsToSender ) {
				s.append( "your " );
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

	if ( isAddressedToMe ) {
		String s( from->getNick() );
		s.append( ", I don't know how to parse that statement." );
		con->addMessage( this, s );
	}
	return true;
}

void Persona::addExpectation( Conversation *c, Persona *f, WaitReply wr )
{
	this->expectations.push_back( new Expectation( c, f, c->getMessageNum(), wr ) );
}

void Persona::addExpectation( Conversation *c, Persona *f, WaitReply wr, const String &str )
{
	this->expectations.push_back( new Expectation( c, f, c->getMessageNum(), wr, str ) );
}

} // end namespace AngelCommunication

