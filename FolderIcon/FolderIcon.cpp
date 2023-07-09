/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderIcon.h"
#include "CHelper.h"
#include <atlconv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
CWinApp theApp;

using namespace std;

TCHAR iconPath[] = 
	_T( "C:\\Users\\wtblo\\Documents\\" )
	_T( "VS2015 Image Library\\2015_VSIcon\\Image\\Image.ico" );
const int iconIndex = 0;

/////////////////////////////////////////////////////////////////////////////
// crawl through the directory tree looking for folders to change the icon
void RecursePath( CStdioFile& fOut, LPCTSTR path )
{
	USES_CONVERSION;
	CString csPathname( path );

	fOut.WriteString( csPathname + _T( "\n" ));
	m_csIcon.GetBuffer();

	SHFOLDERCUSTOMSETTINGS fcs = { 0 };
	fcs.dwSize = sizeof( SHFOLDERCUSTOMSETTINGS );
	fcs.dwMask = FCSM_ICONFILE;
	//fcs.pszIconFile = T2W( iconPath );
	fcs.pszIconFile = m_csIcon.GetBuffer();
	fcs.cchIconFile = 0;
	fcs.iIconIndex = iconIndex;
	SHGetSetFolderCustomSettings( &fcs, CT2WEX<>( csPathname ), FCS_FORCEWRITE );
	m_csIcon.ReleaseBuffer();

	csPathname.TrimRight( _T( "\\" ) );
	CString strWildcard;

	strWildcard.Format( _T( "%s\\*.*" ), csPathname );

	// start trolling for files we are interested in
	CFileFind finder;
	BOOL bWorking = finder.FindFile( strWildcard );
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();

		// skip "." and ".." folder names
		if ( finder.IsDots() )
		{
			continue;
		}

		const CString csFolder =
			finder.GetFilePath().TrimRight( _T( "\\" ) );

		// if it's a directory, recursively search it
		if ( finder.IsDirectory() )
		{
			RecursePath( fOut, csFolder + _T( "\\" ) );
		}
	}

	finder.Close();

} // RecursePath

  /////////////////////////////////////////////////////////////////////////////
  // a console application that can crawl through the file
  // and build a sorted list of filenames
int _tmain( int argc, TCHAR* argv[], TCHAR* envp[] )
{
	HMODULE hModule = ::GetModuleHandle( NULL );
	if ( hModule == NULL )
	{
		_tprintf( _T( "Fatal Error: GetModuleHandle failed\n" ) );
		return 1;
	}

	// initialize MFC and error on failure
	if ( !AfxWinInit( hModule, NULL, ::GetCommandLine(), 0 ) )
	{
		_tprintf( _T( "Fatal Error: MFC initialization failed\n " ) );
		return 2;
	}

	// do some common command line argument corrections
	vector<CString> arrArgs = CHelper::CorrectedCommandLine( argc, argv );
	size_t nArgs = arrArgs.size();

	// this stream can be redirected from the command line to allow the 
	// output you are interested in to be captured into another file
	CStdioFile fOut( stdout );

	// this stream is not redirected; it only shows up on the console and
	// will not affect the output file that is being redirected to
	CStdioFile fErr( stderr );

	CString csMessage;

	// display the number of arguments if not 1 to help the user 
	// understand what went wrong if there is an error in the
	// command line syntax
	if ( nArgs != 1 )
	{
		fErr.WriteString( _T( ".\n" ) );
		csMessage.Format( _T( "The number of parameters are %d\n.\n" ), nArgs - 1 );
		fErr.WriteString( csMessage );

		// display the arguments
		for ( int i = 1; i < nArgs; i++ )
		{
			csMessage.Format( _T( "Parameter %d is %s\n.\n" ), i, arrArgs[ i ] );
			fErr.WriteString( csMessage );
		}
	}

	// two or three arguments expected
	if ( !( nArgs == 2 || nArgs == 3 ) )
	{
		fErr.WriteString( _T( ".\n" ) );
		fErr.WriteString
		(
			_T( "FolderIcon, Copyright (c) 2023, " )
			_T( "by W. T. Block.\n" )
		);

		fErr.WriteString
		(
			_T( ".\n" )
			_T( "A Windows command line program to change folder icons in\n" )
			_T( "  the given directory tree.\n" )
			_T( ".\n" )
		);

		fErr.WriteString
		(
			_T( ".\n" )
			_T( "Usage:\n" )
			_T( ".\n" )
			_T( ".  FolderIcon pathname [icon_name]\n" )
			_T( ".\n" )
			_T( "Where:\n" )
			_T( ".\n" )
		);

		fErr.WriteString
		(
			_T( ".  pathname is the root of the tree to be scanned\n" )
			_T( ".\n" )
			_T( ".  icon_name is the optional name to an icon file.\n" )
			_T( ".  defaults to 'image.ico' in the target folder.\n" )
			_T( ".\n" )

		);

		return 3;
	}

	// display the executable path
	//csMessage.Format( _T( "Executable pathname: %s\n" ), arrArgs[ 0 ] );
	//fErr.WriteString( _T( ".\n" ) );
	//fErr.WriteString( csMessage );
	//fErr.WriteString( _T( ".\n" ) );

	// retrieve the pathname
	CString csPath = arrArgs[ 1 ];

	// test for current folder character (a period)
	bool bExists = csPath == _T( "." );


	// if it is a period, add a wild card of *.* to retrieve
	// all folders and files
	if ( bExists )
	{
		csPath = _T( ".\\*.*" );
		TCHAR pStr[ MAX_PATH ];
		::GetCurrentDirectory( MAX_PATH, pStr );
		m_csRoot = pStr;
		m_csRoot.TrimRight( _T( "\\" ));
		m_csRoot += _T( "\\" );
	}
	else // if it is not a period, test to see if the folder exists
	{
		CString csFolder;
		if ( ::PathIsDirectory( csPath ) )
		{
			csFolder = csPath;
		}
		else
		{
			csFolder = CHelper::GetFolder( csPath );
		}

		csFolder.TrimRight( _T( "\\" ));
		csFolder += _T( "\\" );

		if ( ::PathFileExists( csFolder ) )
		{
			bExists = true;
			m_csRoot = csFolder;
		}
	}

	if ( !bExists )
	{
		csMessage.Format( _T( "Invalid pathname:\n\t%s\n" ), csPath );
		fErr.WriteString( _T( ".\n" ) );
		fErr.WriteString( csMessage );
		fErr.WriteString( _T( ".\n" ) );
		return 4;

	}
	else
	{
		csMessage.Format( _T( "Given pathname:\n\t%s\n" ), csPath );
		fErr.WriteString( _T( ".\n" ) );
		fErr.WriteString( csMessage );
	}


	// the path to the icon
	m_csIcon = m_csRoot + _T( "image.ico" );
	if ( nArgs == 3 )
	{
		if ( ::PathFileExists( m_csRoot + arrArgs[ 2 ] ) )
		{
			m_csIcon = m_csRoot + arrArgs[ 2 ];
		}
	}

	// test to see if the icon exists
	bool bIcon = false;
	if ( ::PathFileExists( m_csIcon ) )
	{
		bIcon = true;
	}

	if ( !bIcon )
	{
		csMessage.Format( _T( "Invalid icon path:\n\t%s\n" ), m_csIcon );
		fErr.WriteString( _T( ".\n" ) );
		fErr.WriteString( csMessage );
		fErr.WriteString( _T( ".\n" ) );
		return 5;

	}
	else
	{
		csMessage.Format( _T( "Given icon path:\n\t%s\n" ), m_csIcon );
		fErr.WriteString( _T( ".\n" ) );
		fErr.WriteString( csMessage );
	}

	// crawl through directory tree defined by the command line
	// parameter trolling for folders
	RecursePath( fOut, m_csRoot );

	// all is good
	return 0;

} // _tmain

  /////////////////////////////////////////////////////////////////////////////
