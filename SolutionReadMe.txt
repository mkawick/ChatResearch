Building requires Boost

Make sure that you add the Boost directory to your 
Tools/options/Projects and solutions/VC++ Directories

For me, this reads:

Includes:
C:\projects\Mber\boost

This is an incomplete set of boost 1.53 because we do not need the entire set of libs too.

//-------------------------------------------------------------------

Also, you will need to add libevent to your path:
Includes:
C:\projects\mber\libevent\include
C:\projects\mber\libevent\WIN32-Code (win 32 only)
Libraries:
C:\projects\mBer\libevent