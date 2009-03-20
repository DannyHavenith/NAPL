function create_ramp( samplerate, samples, bits)
{
	ramp = new Array();
	for (i = 0; i <= samples; ++i)
	{
	    ramp[i] = -1 + 2.0 * i / samples;
	}

	return Napl.func( ramp, 1, bits, samplerate, samples);
}
