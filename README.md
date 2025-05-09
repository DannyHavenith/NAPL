NAPL, a C++ sound processing library.
=====================================

This is a library for audio manipulation. It was originally created as a
scripting language for sound processing. Several utilities have been added later
for specific tasks.

The central concept in the library is a block_producer, which is an interface to
an object that can deliver blocks of audio data. Operations on audio data come
as objects that are themselves block_producers and that might take one or more
block_producers as input.

As the bulk of this code was written decades ago in C++98 for Windows, not all
parts are currently compilable on all platforms.

One application that is reasonably maintained is the `rythmbuilder` program,
which takes a text file that describes a rythm sequence and which can generate a
WAV file with the sounds of that rythm.
