#include "stdafx.h"
#include "NpcMagicProcess.h"
#include "ServerDlg.h"
#include "User.h"
#include "Npc.h"

CNpcMagicProcess::CNpcMagicProcess()
{
	m_pSrcNpc = NULL;
	m_bMagicState = NONE;
}

CNpcMagicProcess::~CNpcMagicProcess()
{

}

void CNpcMagicProcess::MagicPacket(char *pBuf, int len)
{
	int index = 0, send_index = 0, magicid = 0, sid = -1, tid = -1, data1 = 0, data2 = 0, data3 = 0, data4 = 0, data5 = 0, data6 = 0;
	char send_buff[128];
	_MAGIC_TABLE* pTable = NULL;
	
	BYTE command = GetByte( pBuf, index );		// Get the magic status.  
	if( command == MAGIC_FAIL ) {			    // Client indicates that magic failed. Just send back packet.
		SetByte( send_buff, AG_MAGIC_ATTACK_RESULT, send_index );
		SetString( send_buff, pBuf, len-1, send_index );	// len ==> include WIZ_MAGIC_PROCESS command byte. 
		//g_pMain->Send_Region( send_buff, send_index, m_pSrcUser->m_bZone, m_pSrcUser->m_RegionX, m_pSrcUser->m_RegionZ );
		m_bMagicState = NONE;
		return;
	}

	magicid = GetDWORD( pBuf, index );        // Get ID of magic.
	sid = GetShort( pBuf, index );			  // Get ID of source.
	tid = GetShort( pBuf, index );            // Get ID of target.

	data1 = GetShort( pBuf, index );          // ( Remember, you don't definately need this. )
	data2 = GetShort( pBuf, index );		  // ( Only use it when you really feel it's needed. )
	data3 = GetShort( pBuf, index );
	data4 = GetShort( pBuf, index );
	data5 = GetShort( pBuf, index );
	data6 = GetShort( pBuf, index );          // Get data1 ~ data6 (No, I don't know what the hell 'data' is.) 

	pTable = IsAvailable( magicid, tid, command );     // If magic was successful.......
	if( !pTable ) return;

	if( command == MAGIC_EFFECTING )     // Is target another player? 
	{
		//if (tid < -1 || tid >= MAX_USER) return;	

		switch( pTable->bType[0] ) {
		case 1:
			ExecuteType1( pTable->iNum, tid, data1, data2, data3 );
			break;
		case 2:
			ExecuteType2( pTable->iNum, tid, data1, data2, data3 );	
			break;
		case 3:
			ExecuteType3( pTable->iNum, tid, data1, data2, data3, pTable->bMoral );
			break;
		case 4:
			ExecuteType4( pTable->iNum, tid );
			break;
		case 5:
			ExecuteType5( pTable->iNum );
			break;
		case 6:
			ExecuteType6( pTable->iNum );
			break;
		case 7:
			ExecuteType7( pTable->iNum );
			break;
		case 8:
			ExecuteType8( pTable->iNum, tid, sid, data1, data2, data3 );
			break;
		case 9:
			ExecuteType9( pTable->iNum );
			break;
		}

		switch( pTable->bType[1] ) {
		case 1:
			ExecuteType1( pTable->iNum, tid, data4, data5, data6 );
			break;
		case 2:
			ExecuteType2( pTable->iNum, tid, data1, data2, data3 );	
			break;
		case 3:
			ExecuteType3( pTable->iNum, tid, data1, data2, data3, pTable->bMoral );
			break;
		case 4:
			ExecuteType4( pTable->iNum, tid );
			break;
		case 5:
			ExecuteType5( pTable->iNum );
			break;
		case 6:
			ExecuteType6( pTable->iNum );
			break;
		case 7:
			ExecuteType7( pTable->iNum );
			break;
		case 8:
			ExecuteType8( pTable->iNum, tid, sid, data1, data2, data3 );
			break;
		case 9:
			ExecuteType9( pTable->iNum );
			break;
		}
	}
	else if( command == MAGIC_CASTING ) {
		Packet result(AG_MAGIC_ATTACK_RESULT);
		result.append(pBuf, len); // NOTE: used to be len-1, as apparently it "included the opcode". I don't see this behaviour anywhere.
		g_pMain->Send(&result);
	}
}

_MAGIC_TABLE* CNpcMagicProcess::IsAvailable(int magicid, int tid, BYTE type )
{
	CUser* pUser = NULL;
	CNpc* pNpc = NULL;
	_MAGIC_TABLE* pTable = NULL;

	int modulator = 0, Class = 0, moral = 0;

	if( !m_pSrcNpc ) return FALSE;

	pTable = g_pMain->m_MagictableArray.GetData( magicid );     // Get main magic table.
	if( !pTable ) goto fail_return;            

	if( tid >= 0 && tid < MAX_USER )     // Compare morals between source and target character.       
	{
		pUser = g_pMain->GetUserPtr(tid);
		if( !pUser || pUser->m_bLive == USER_DEAD ) goto fail_return;
		moral = pUser->m_bNation;
	}
	else if( tid >= NPC_BAND )     // Compare morals between source and target NPC.            
	{
		pNpc = g_pMain->m_arNpc.GetData(tid - NPC_BAND);
		if( !pNpc || pNpc->m_NpcState == NPC_DEAD ) goto fail_return;
		moral = pNpc->m_byGroup;
	}
	else if ( tid == -1) {  // Party Moral check routine.
		if (pTable->bMoral == MORAL_AREA_ENEMY) {
			if( m_pSrcNpc->m_byGroup == 0) {	// Switch morals. �۾��Ұ� : ���ʹ� ������� ������ ���� ������.. ���߿� NPC�� �� ������ ����ϸ� ������ ��
				moral = 2 ;
			}
			else {
				moral = 1 ;
			}
		}
		else {
			moral = m_pSrcNpc->m_byGroup;	
		}
	}
	else moral = m_pSrcNpc->m_byGroup ;
	
	switch( pTable->bMoral ) {	// tid >= 0 case only
	case MORAL_SELF:
		if( tid != (m_pSrcNpc->m_sNid+NPC_BAND) ) goto fail_return;
		break;
	case MORAL_FRIEND_WITHME:
		if( m_pSrcNpc->m_byGroup != moral ) goto fail_return;
		break;
	case MORAL_FRIEND_EXCEPTME:
		if( m_pSrcNpc->m_byGroup != moral ) goto fail_return;
		if( tid == (m_pSrcNpc->m_sNid+NPC_BAND) ) goto fail_return;
		break;
	case MORAL_PARTY:
	case MORAL_PARTY_ALL:
		
		break;
	case MORAL_NPC:
		if( !pNpc ) goto fail_return;
		if( pNpc->m_byGroup != moral ) goto fail_return;
		break;
	case MORAL_ENEMY:
		if( m_pSrcNpc->m_byGroup == moral ) goto fail_return;
		break;
	}

	return pTable;      // Magic was successful! 

fail_return:    // In case the magic failed. 
	Packet result(AG_MAGIC_ATTACK_RESULT, uint8(MAGIC_FAIL));
	result	<< magicid << uint16(m_pSrcNpc->m_sNid+NPC_BAND)
			<< uint16(tid)
			<< uint16(type == MAGIC_CASTING ? -100 : 0) << uint16(0)
			<< uint16(0) << uint16(0)
			<< uint16(0) << uint16(0);
	g_pMain->Send(&result);

	m_bMagicState = NONE;

	return NULL;     // Magic was a failure!
}

void CNpcMagicProcess::ExecuteType1(int magicid, int tid, int data1, int data2, int data3)   // Applied to an attack skill using a weapon.
{	
	
}

void CNpcMagicProcess::ExecuteType2(int magicid, int tid, int data1, int data2, int data3)
{		

}

void CNpcMagicProcess::ExecuteType3(int magicid, int tid, int data1, int data2, int data3, int moral )  // Applied when a magical attack, healing, and mana restoration is done.
{	
	int damage = 0, attack_type = 0; 
	BOOL bResult = 1;
	_MAGIC_TYPE3* pType = NULL;
	CNpc* pNpc = NULL ;      // Pointer initialization!
	int dexpoint = 0;

	_MAGIC_TABLE* pMagic = NULL;
	pMagic = g_pMain->m_MagictableArray.GetData( magicid );   // Get main magic table.
	if( !pMagic ) return; 

	if(tid == -1)	{	// ���� ����,, ������ ���������� ���Ӽ������� ó���Ѵ�.. �������� ����..
		goto packet_send;
	}

	pNpc = g_pMain->m_arNpc.GetData(tid-NPC_BAND);
	if(pNpc == NULL || pNpc->m_NpcState == NPC_DEAD || pNpc->m_iHP == 0)	{
		bResult = 0;
		goto packet_send;
	}
	
	pType = g_pMain->m_Magictype3Array.GetData( magicid );      // Get magic skill table type 3.
	if( !pType ) return;
	
	damage = GetMagicDamage(tid, pType->sFirstDamage, pType->bAttribute, dexpoint);
//	if(damage == 0)	damage = -1;

	//TRACE("magictype3 ,, magicid=%d, damage=%d\n", magicid, damage);
	
	if (pType->bDuration == 0)    { // Non-Durational Spells.
		if (pType->bDirectType == 1) {    // Health Point related !
			if(damage > 0)	{
				bResult = pNpc->SetHMagicDamage(damage);
			}
			else	{
				damage = abs(damage);
/*				if(pType->bAttribute == 3)   attack_type = 3; // ������Ű�� �����̶��.....
				else attack_type = magicid;

				if(pNpc->SetDamage(attack_type, damage, m_pSrcUser->m_strUserID, m_pSrcUser->m_iUserId + USER_BAND) == FALSE)	{
					// Npc�� ���� ���,,
					pNpc->SendExpToUserList(); // ����ġ �й�!!
					pNpc->SendDead();
					m_pSrcUser->SendAttackSuccess(tid, MAGIC_ATTACK_TARGET_DEAD, damage, pNpc->m_iHP, MAGIC_ATTACK);
				}
				else	{
					// ���� ��� ����
					m_pSrcUser->SendAttackSuccess(tid, ATTACK_SUCCESS, damage, pNpc->m_iHP, MAGIC_ATTACK);
				}	*/
			}
		}
	}
	else if (pType->bDuration != 0)   {  // Durational Spells! Remember, durational spells only involve HPs.
	} 

packet_send:
	//if ( pMagic->bType[1] == 0 || pMagic->bType[1] == 3 ) 
	{
		Packet result(AG_MAGIC_ATTACK_RESULT, uint8(MAGIC_EFFECTING));
		result	<< magicid << uint16(m_pSrcNpc->m_sNid+NPC_BAND)
				<< uint16(tid)
				<< uint16(data1) << uint16(bResult) << uint16(data3)
				<< uint16(moral) << uint16(0) << uint16(0);
		g_pMain->Send(&result);
	}
}

void CNpcMagicProcess::ExecuteType4(int magicid, int tid)
{
	
	return;
}

void CNpcMagicProcess::ExecuteType5(int magicid)
{
	return;
}

void CNpcMagicProcess::ExecuteType6(int magicid)
{
	return;
}

void CNpcMagicProcess::ExecuteType7(int magicid)
{
	return;
}

void CNpcMagicProcess::ExecuteType8(int magicid, int tid, int sid, int data1, int data2, int data3 )	// Warp, resurrection, and summon spells.
{	
	
}

void CNpcMagicProcess::ExecuteType9(int magicid)
{
	return;
}

short CNpcMagicProcess::GetMagicDamage(int tid, int total_hit, int attribute, int dexpoint)
{
	short damage = 0, temp_hit = 0 ; 
	int random = 0, total_r = 0 ;
	BYTE result;
	BOOL bSign = TRUE;			// FALSE�̸� -, TRUE�̸� +

	if( tid < NPC_BAND || tid > INVALID_BAND) return 0;     // Check if target id is valid.

	CNpc* pNpc = NULL;              
	pNpc = g_pMain->m_arNpc.GetData(tid-NPC_BAND);
	if(pNpc == NULL || pNpc->m_NpcState == NPC_DEAD || pNpc->m_iHP == 0)	return 0;
	if(pNpc->m_proto->m_tNpcType == NPC_ARTIFACT || pNpc->m_proto->m_tNpcType == NPC_PHOENIX_GATE || pNpc->m_proto->m_tNpcType == NPC_GATE_LEVER || pNpc->m_proto->m_tNpcType == NPC_SPECIAL_GATE ) return 0;
	
	//result = m_pSrcUser->GetHitRate(m_pSrcUser->m_fHitrate / pNpc->m_sEvadeRate ); 
	result = SUCCESS;
		
	if (result != FAIL) {		// In case of SUCCESS (and SUCCESS only!) .... 
		switch (attribute) {
			case FIRE_R	:
				total_r = pNpc->m_byFireR;
				break;
			case COLD_R :
				total_r = pNpc->m_byColdR;
				break;
			case LIGHTNING_R :
				total_r = pNpc->m_byLightningR ;
				break;
			case MAGIC_R :
				total_r = pNpc->m_byMagicR ;
				break;
			case DISEASE_R :
				total_r = pNpc->m_byDiseaseR ;
				break;
			case POISON_R :			
				total_r = pNpc->m_byPoisonR ;
				break;
		}	

		total_hit = (total_hit * (dexpoint  + 20)) / 170;

		if(total_hit < 0)	{
			total_hit = abs(total_hit);
			bSign = FALSE;
		}
		
		damage = (short)(total_hit - (0.7f * total_hit * total_r / 200)) ;
		random = myrand (0, damage) ;
		damage = (short)((0.7f * (total_hit - (0.9f * total_hit * total_r / 200))) + 0.2f * random);
	}
	else damage = 0 ;

	if(bSign == FALSE && damage != 0)	{
		damage = - damage;
	}
		
	return damage ;		
}