/***********************************************************************************

  Module :	CTextFileHandler.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 07 September 2005 71M

***********************************************************************************/

#ifndef CTEXTFILEHANDLER_H_
#define CTEXTFILEHANDLER_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CFileHandler.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CTextFileHandler
{
	public:

		static bool							Execute( const CString & file );
		static const sFileExtensionInfo &	Information( const CString & file );

	private:

		static void							Render();
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CTEXTFILEHANDLER_H_ */
