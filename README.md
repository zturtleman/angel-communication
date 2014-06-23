angel-communication
===================

Angel Communication, as of writting, is a "chat bot" with basic semi-working English natural language processing. It doesn't say much other greetings and enough to let me know what it parsed from each line of text. There is a interactive command line program and a IRC client program.

It's planned to be supported on Windows, GNU/Linux, and OS X. It might be broken on Windows and OS X right now.

Example of fun replies mode (enabled by default):

```
User> what is your motivation?
Angel> haha, motivation?
```

## goals

The primary goal is to create interactive characters. For now it's limited to text communication but may expand to include visual repersentations in the future.

  * Persistent identity.
  * Maintain state of personal information and state of conversation.
  * Support direct (user to bot, bot to bot) and group chat.
  * Avoid looking like a script of pre-written blobs of text like a [gamebook](http://en.wikipedia.org/wiki/Gamebook) (mostly passive, but with some interaction).
  * If bot asks a question, follow up on it or possibly drop it, but don't ignore answer and give the same response regardless to what answer is.


## status

I think making a chat bot is split into at least three parts.

  * Interface
  * Convert input into data
  * Process data and respond

I have two working interfaces, using a terminal and through IRC, which seems to be 'good enough' for now. Currently the second two parts are handled together (poorly) and needs to be rewritten.
