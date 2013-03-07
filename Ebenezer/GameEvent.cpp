#include "stdafx.h"
#include "GameEvent.h"
#include "User.h"
#include "GameDefine.h"

CGameEvent::CGameEvent()
{
	m_bType = 0x00;
	m_iCond[0] = 0; m_iCond[1] = 0; m_iCond[2] = 0; m_iCond[3] = 0; m_iCond[4] = 0;
	m_iExec[0] = 0; m_iExec[1] = 0; m_iExec[2] = 0; m_iExec[3] = 0; m_iExec[4] = 0;
}

CGameEvent::~CGameEvent()
{

}

void CGameEvent::RunEvent( CUser *pUser )
{
	switch( m_bType ) {
		case ZONE_CHANGE:
			if( pUser->m_bWarp )
				break;
			pUser->ZoneChange( m_iExec[0], (float)m_iExec[1], (float)m_iExec[2] );
			break;
		case ZONE_TRAP_DEAD:
		//	TRACE("&&& User - zone trap dead ,, name=%s\n", pUser->m_pUserData->m_id);
		//	pUser->Dead();
			break;	
		case ZONE_TRAP_AREA:
			pUser->TrapProcess();
			break;
	}
}
