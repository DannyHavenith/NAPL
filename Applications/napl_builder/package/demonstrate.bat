@echo off

REM ***** cut and truncate truncate_me.wav
truncate.ns truncate_me.wav truncated.wav 2 3.0 4.0

resampling.ns truncated.wav

REM ***** show some info about the output
show_info.ns truncate_me.wav truncated.wav fast_truncated.wav resampled_truncated.wav

REM **** wait for input
pause


