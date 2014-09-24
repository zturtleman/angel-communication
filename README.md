angel-communication
===================

Angel Communication is a "chat bot" with basic semi-working English natural language processing. It doesn't say much other greetings and enough to let me know what it parsed from each line of text. There is a interactive command line program and a IRC client program.

Most of the development has been on building everything besides a chat bot. Interactive terminal, IRC client, sentence parsing, conversation message handling, etc.

It's been tested on Windows, GNU/Linux, and OS X. Though it might not always be working on all of them.

Example chat:

```
Angel Communication CLI
Type 'quit' to exit Angel Communication.
Angel> Hi User.
User> What is the sky?
Angel> Let's talk about me instead of sky.
User> what is your motivation?
Angel> haha motivation?
User> bye
Angel> Good bye User
```

To add a second bot, launch the CLI program or IRC client with "--two" argument.

The IRC client currently has the server and channel name hard coded in irc/irc_main.cpp.

## compiling

Use [CMake](http://www.cmake.org) to generate build files.

On GNU/Linux and OS X if you have cmake installed you can run `mkdir build && cd build && cmake .. && make`.

On GNU/Linux you can cross-compile for Windows using `mkdir build-mingw && cd build-mingw && cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-cross-mingw32-linux.cmake && make`.

## goals

The primary goal is to create interactive characters (opposed to information retrieval, etc). For now it's limited to text communication but may expand to include visual repersentations in the future.

  * Persistent identity.
  * Maintain state of personal information and state of conversation.
  * Support direct (user to bot, bot to bot) and group chat.
  * Avoid looking like a script of pre-written blobs of text like a [gamebook](http://en.wikipedia.org/wiki/Gamebook) (mostly passive, but with some interaction).
  * If bot asks a question, follow up on it or possibly drop it, but don't ignore answer and give the same response regardless to what answer is.


## status

I think making a chat bot is split into at least three parts.

### Interface

I have two working interfaces, using a terminal and through IRC. They seems to be good enough for now.

### Convert input into data

Currently sentence parsing is an ugly mess mixed in with the processing and response code.
It does create "expectation" events so bot follows up to questions it asks though.

I started rewriting the sentence parsing in framework/sentence.cpp.

### Process data and respond

Needs to be split from the parsing code after sentence parsing rewrite can replace current messy code.

