/***********************************************************************************

  Module :	CRenderable.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 06 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CRenderable.h"
#include "CGfx.h"

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
CRenderCallbackList	CRenderable::s_RenderList[ MAX_RENDER_ORDER ];

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
void	CRenderable::Open()
{
}

//**********************************************************************************
//	
//**********************************************************************************
void	CRenderable::Close()
{
	for ( u32 i = 0; i < MAX_RENDER_ORDER; ++i )
	{
		s_RenderList[ i ].clear();
	}
}

//*************************************************************************************
//	
//*************************************************************************************
void	CRenderable::Render()
{
	CGfx::BeginRender();
	CGfx::ClearScreen( 0x00000000 );

	for ( u32 i = 0; i < MAX_RENDER_ORDER; ++i )
	{
		for ( CRenderCallbackList::iterator it = s_RenderList[ i ].begin(); it != s_RenderList[ i ].end(); ++it )
		{
			( *it )();
		}
	}

	CGfx::EndRender();
	CGfx::SwapBuffers();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CRenderable::Register( eRenderOrder order, RenderCallback callback )
{
	s_RenderList[ order ].remove( callback );
	s_RenderList[ order ].push_back( callback );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CRenderable::UnRegister( eRenderOrder order, RenderCallback callback )
{
	s_RenderList[ order ].remove( callback );
}

//*******************************  END OF FILE  ************************************
