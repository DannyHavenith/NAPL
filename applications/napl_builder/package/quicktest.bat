@echo off
rem *** first generate four tones of an Amaj7 chord

function.ns a.wav 0
function.ns cis.wav 4
function.ns e.wav 7
function.ns g.wav 10

REM *** now construct a 4-channel Amaj7 chord
make_stereo.ns a.wav cis.wav a-cis.wav
make_stereo.ns e.wav g.wav e-g.wav
make_stereo.ns a-cis.wav e-g.wav Amaj7.wav

REM *** separate 4 channel wav into separate channels
extract_channel.ns Amaj7.wav

REM *** resampling
resampling.ns Amaj7.wav
resampling.ns a-cis.wav

cat.ns a-cis.wav e-g.wav cat.wav

iterator.ns a-cis.wav

cut_at_sample.ns a.wav 1000

test_includes.ns


pause





