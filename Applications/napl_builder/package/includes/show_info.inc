function show_info( input) 
{
	writeln( "The sample rate is: " + input.sample_rate);
	writeln( "The sample size is: " + input.sample_size + " bits");
	writeln( "The channel count is: " + input.channel_count);
	writeln( "The frame count is: " + input.frame_count);
	writeln( "The frame size  is: " + input.frame_size + " bytes");
};
