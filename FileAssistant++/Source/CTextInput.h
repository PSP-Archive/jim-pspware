/***********************************************************************************

  Module :	CTextInput.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 13 August 2005 71M

***********************************************************************************/

#ifndef CTEXTINPUT_H_
#define CTEXTINPUT_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CWindow.h"
#include "CKeyboard.h"

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
class CTextInput : public CWindow, public CKeyboardListener
{
	public:

		enum eReturnCode
		{
			RC_OK,
			RC_CANCEL,
		};

		CTextInput( const CString & title, const CString & text );
		~CTextInput();

		virtual void	Render();

		eReturnCode		Show();

		const CString &	GetText() const;

		virtual void	Message( CKeyboardListener::eMessage message, const void * const p_data );

	private:

		CString			m_szText;

		CKeyboard *		m_pKeyboard;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CTEXTINPUT_H_ */
