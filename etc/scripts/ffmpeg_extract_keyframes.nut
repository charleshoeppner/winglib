
_self.load_module( "ffmpeg", "" );

function _init()
{
	local in_file = "/home/landshark/Desktop/R-033000-034500-13500.avi";
	local out_file = "/home/landshark/Desktop/out.avi";

	local inp = CFfContainer();
	if ( !inp.Open( in_file, CSqMulti() ) )
	{	_self.echo( "failed to open file" );
		return;
	} // end if

	_self.echo( "Video File : " + inp.getWidth() + "x" + inp.getHeight() );

	local dec = CFfDecoder();
	if ( !dec.Create( inp.getVideoCodecId(), inp.getVideoFormat(),
					  inp.getWidth(), inp.getHeight(), 0 ) )
	{	_self.echo( "failed to create decoder" );
		return;
	} // end if

	local out = CFfContainer();
	if ( !out.Create( out_file, "", CSqMulti() ) )
	{	_self.echo( "failed to create output file" );
		return;
	} // end if

	if ( 0 > out.AddVideoStream( inp.getVideoCodecId(), inp.getWidth(), inp.getHeight(), 10 ) )
	{	_self.echo( "failed to add video stream to output file" );
		return;
	} // end if

	if ( !out.InitWrite() )
	{	_self.echo( "failed to initialize output file" );
		return;
	} // end if

	local i = 0, w = 0, e = 0;
	local stream = -1;
	local frame = CSqBinary();
	local frame_info = CSqMulti();
	while ( 0 <= ( stream = inp.ReadFrame( frame, frame_info ) ) )
	{
		// Video?
		if ( inp.getVideoStream() == stream )
		{
			// Is it a key frame?
			if ( frame_info[ "flags" ].toint() )
				e = 5;

			if ( e )
			{
				if ( !out.WriteFrame( frame, CSqMulti( ( e == 5 ) ? "flags=1" : "" ) ) )
				{	_self.echo( "failed to write frame to avi" );
					return;
				} // end if

				e--;
				w++;

			} // end if

			_self.print( "\r" + ( ++i ) + " -> " + w + "    " );
			_self.flush();

		} // end if

	} // end while

}
