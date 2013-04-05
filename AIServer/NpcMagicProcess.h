#pragma once

class CNpc;
struct _MAGIC_TABLE;

class CNpcMagicProcess  
{
public:
	CNpc*			m_pSrcNpc;	

	BYTE	m_bMagicState;

public:
	CNpcMagicProcess();
	virtual ~CNpcMagicProcess();

	short GetMagicDamage(int tid, int total_hit, int attribute, int dexpoint);
	void ExecuteType9(int magicid);
	void ExecuteType8(int magicid, int tid, int sid, int data1, int data2, int data3);
	void ExecuteType7(int magicid);
	void ExecuteType6(int magicid);
	void ExecuteType5(int magicid);
	void ExecuteType4(int magicid, int tid);
	void ExecuteType3(int magicid, int tid, int data1, int data2, int data3, int moral);
	void ExecuteType2(int magicid, int tid, int data1, int data2, int data3 );
	void ExecuteType1(int magicid, int tid, int data1, int data2, int data3 );	// sequence => type1 or type2

	_MAGIC_TABLE* IsAvailable( int magicid, int tid, BYTE type );
	void MagicPacket(Packet & pkt);
};
