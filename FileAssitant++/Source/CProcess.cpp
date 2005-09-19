/***********************************************************************************

  Module :	CProcess.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 03 August 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CProcess.h"
#include "CFrameWork.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************
bool			CProcess::s_bPaused( false );
CProcessList	CProcess::s_ProcessList;
CProcessList	CProcess::s_NewProcessList;
CProcessList	CProcess::s_DeletedProcessList;
CProcessList	CProcess::s_MoveToBackProcessList;
CProcessList	CProcess::s_MoveToFrontProcessList;

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
void	CProcess::Open()
{
	s_bPaused = false;
}

//*************************************************************************************
//	
//*************************************************************************************
void	CProcess::Close()
{
	while ( s_NewProcessList.empty() == false )
	{
		CProcess *	p_process( s_NewProcessList.front() );

		delete p_process;
	}

	while ( s_ProcessList.empty() == false )
	{
		CProcess *	p_process( s_ProcessList.front() );

		delete p_process;
	}
}

//*************************************************************************************
//	
//*************************************************************************************
void	CProcess::ProcessList()
{
	//
	//	Delete any dead processes
	//
 	for ( CProcessList::iterator it = s_DeletedProcessList.begin(); it != s_DeletedProcessList.end(); ++it )
	{
		delete ( *it );
	}
	s_DeletedProcessList.clear();

	//
	//	Add the new processes
	//
	for ( CProcessList::iterator it = s_NewProcessList.begin(); it != s_NewProcessList.end(); ++it )
	{
		s_ProcessList.push_back( *it );
	}
	s_NewProcessList.clear();

	//
	//	Move any processes to the back of the list
	//
	for ( CProcessList::iterator it = s_MoveToBackProcessList.begin(); it != s_MoveToBackProcessList.end(); ++it )
	{
		s_ProcessList.remove( *it );
		s_ProcessList.push_back( *it );
	}
	s_MoveToBackProcessList.clear();

	//
	//	Move any processes to the front of the list
	//
	for ( CProcessList::iterator it = s_MoveToFrontProcessList.begin(); it != s_MoveToFrontProcessList.end(); ++it )
	{
		s_ProcessList.remove( *it );
		s_ProcessList.push_front( *it );
	}
	s_MoveToFrontProcessList.clear();

	//
	//	If we're paused don't process any thing
	//
	if ( s_bPaused == false )
	{
		//
		//	Process all the processes
		//
		for ( CProcessList::iterator it = s_ProcessList.begin(); it != s_ProcessList.end(); ++it )
		{
			( *it )->Process();
		}

		//
		//	Process the input for all processes
		//
		for ( CProcessList::iterator it = s_ProcessList.begin(); it != s_ProcessList.end(); ++it )
		{
			( *it )->ProcessInput();
		}
	}
}

//**********************************************************************************
//	
//**********************************************************************************
void	CProcess::Pause( bool pause )
{
	s_bPaused = pause;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CProcess::IsPaused()
{
	return s_bPaused;
}

//*************************************************************************************
//	
//*************************************************************************************
CProcess::CProcess()
{
	s_NewProcessList.push_back( this );
}

//*************************************************************************************
//	
//*************************************************************************************
CProcess::~CProcess()
{
	s_ProcessList.remove( this );
	s_NewProcessList.remove( this );
	s_MoveToBackProcessList.remove( this );
	s_MoveToFrontProcessList.remove( this );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CProcess::Delete()
{
	s_DeletedProcessList.remove( this );
	s_DeletedProcessList.push_back( this );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CProcess::MoveToBackOfList()
{
	s_NewProcessList.remove( this );
	s_MoveToBackProcessList.remove( this );
	s_MoveToBackProcessList.push_back( this );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CProcess::MoveToFrontOfList()
{
	s_NewProcessList.remove( this );
	s_MoveToFrontProcessList.remove( this );
	s_MoveToFrontProcessList.push_back( this );
}

//*******************************  END OF FILE  ************************************
