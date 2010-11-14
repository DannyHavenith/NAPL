add_library( napl

	aiff.cpp
	double_dispatch_factory.cpp
	filefact.cpp
	ieee754.cpp
#	lame_wrapper.cpp
	memory_producer.cpp
	memory_writer.cpp
	objfact.cpp
	progress.cpp
	raw_file.cpp
#	resample.cpp
	stdafx.cpp
#	syncwav.cpp
#	test.cpp
	wav.cpp
#	xfade.cpp
#	xxfade.cpp

# header files, just for VS' sake.
	aiff.h
	amplifier.h
	amp_modulator.h
	analyzer.h
	blademp3encdll.h
	constant_producer.h
	convendi.h
	delay.h
	extract_channel.h
	fmodulator.h
	fourier.h
	function_impl.h
	ieee754_.h
	ieee754.h
	lame_wrapper.h
	negator.h
	pan_impl.h
	processor.h
	raw_file.h
	resample.h
	sample_factory_instantiator.h
	samplety.h
	smputils.h
	stdafx.h
	stereomaker.h
	type_converter.h
	uniform_block_converter.h
	wav.h
	xfade.h
	)
