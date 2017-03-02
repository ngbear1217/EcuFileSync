#pragma once



#define MSG_CONNECT				WM_USER + 700
#define MSG_DISCONNECT			WM_USER + 701
#define MSG_SOCKETCLOSE			WM_USER + 702
#define MSG_CLIENT_INFO			WM_USER + 703

#define MSG_SUBMSG				WM_USER + 704
//#define MSG_SENDFILEINFO		WM_USER + 705
#define MSG_RECVFILEINFO		WM_USER + 707
//#define MSG_FILEUP_ERR			WM_USER + 708
#define MSG_FILEUP_CHK			WM_USER + 709
#define MSG_REMOVECLIENT		WM_USER + 710
//#define MSG_NOFILE				WM_USER + 711
#define MSG_DB_ERR				WM_USER + 712


//#define MSG_MAINMSG				WM_USER + 711
//#define MSG_SOCKETOPEN			WM_USER + 712
//#define MSG_READRTC				WM_USER + 713
//#define MSG_FLASHREADY			WM_USER + 714
//#define MSG_SETFLASHINFO			WM_USER + 715
//#define MSG_FLASHSTATUS			WM_USER + 716
//#define MSG_FLASHRESULT			WM_USER + 717
//#define MSG_RELOAD				WM_USER + 718
//
//#define MSG_AUTOFW				WM_USER + 720
//#define MSG_ECU_INSERT			WM_USER + 721
//#define MSG_OPENFILE_FLASHLOG	WM_USER + 722
//#define MSG_UPLOADFLASHFILE		WM_USER + 723
//#define MSG_FLASH_REULT_STEP1   WM_USER + 724
//#define MSG_SETFLASHTIME		WM_USER + 725
//#define MSG_REWORK_BY_RECONNECT		WM_USER + 726
//
//#define MSG_READFILECNT			WM_USER + 728
//#define MSG_NO_ORDER			WM_USER + 729
//#ifdef VER3_1_3_0
//#define MSG_UI_MESSAGE			WM_USER + 730
//#endif
//



#define MAX_BUFFER 5000
#define SIZE_MAX_DATABUF		4096
#define SIZE_RELEASE_DATE		8
#define SIZE_VER				128
#define SIZE_FLASH_INFO			58


#ifdef VER3_1_2_9_WRITEBUF_1
#define FILE_WRITING_SIZE				1024
#else VER3_1_2_9_WRITEBUF_4
#define FILE_WRITING_SIZE				4096
#endif

#define FILE_READING_SIZE				1024
#define FILE_READING_LENGTH				4

#define SIZE_HEADER 5
#define SIZE_HEADER_REAR 6
#define SIZE_IP	16
#define SIZE_MAC				6
#define SIZE_ID					5 
#define SIZE_VIN				17
#define SIZE_SERIAL				10
#define INTERVAL			70
#define ALIVE_BIT_COUNT			3
#define TIME_UI_MSG_VIEW	3000
#define TIME_FWMODE_ON		15000
#define SIZE_FLASH_RESULT		150
#define MONITORING_TIME		6//sec	
#define PACKETRETURN_TIME   10 //sec





#define NAME_VCI_UPLOAD			_T("FData")
#define FIRMWARE_FILEPATH		_T("Sys/SFlash_FW.vci")
#define BOOTWARE_FILEPATH		_T("Sys/SFlash_BOOT.vci")
#define FIRMWARE_FILEHEADER		_T("FIRMWARE")
#define BOOTWARE_FILEHEADER		_T("BOOTLOADER")
#define INI_PATH				_T("\\FLASH_SETTING.ini")
#define FLASHINI_PATH			_T("D:\\Flash\\Bin\\Data\\MACHINE_INFO\\FLASH_INFO.ini")
#define MAIN_PATH				_T("D:\\FLASH")
#define LOG_PATH				_T("\\Log\\FLASHLOG")
#define FLASH_DATA_PATH			_T("Bin\\Data\\Svr_Flash_Data")
#define VCI_FW_DATA_PATH		_T("Bin\\SysUpdate\\VCI")
#define MULTILANG_PATH			_T("D:\\FLASH\\Bin\\Data\\MasterDB\\ECU_MAINSYSMSG.xml")



typedef struct tagPerIoContext
{

	WSAOVERLAPPED overlapped;
	WSABUF wsaBuf;
	char Buffer[MAX_BUFFER];

	//DWORD dwBytesTransferred; // 현 IO 전송된 바이트 수 
	//DWORD dwTotalBytes;       // 총 얼마만큼 전송되었느냐를 저장하는 바이트 수 

} PerIoContext, *PPerIoContext;

typedef struct tagPerSocketContext
{

  SOCKET recvSocket;
  SOCKET sendSocket;
  PPerIoContext recvContext;
  PPerIoContext sendContext;
  char* strClientIP[SIZE_IP];
  int ClientObjNo;

} PerSocketContext, *PPerSocketContext;

//VCI packet 형태
typedef struct _BASIC_PACKET
{
	unsigned char cSource;
	unsigned char	cDestination;
	unsigned char	cCmdID;
	BYTE	nLength[2];
	unsigned char	pData[SIZE_MAX_DATABUF];
	unsigned char	cCheckSum;
}BASIC_PACKET, *PBASIC_PACKET;



typedef struct _Close_CompletionKey
{
	char cCloseYn;	
	char* strClientIP[SIZE_IP];
}CLOSE_COMPLETIONKEY, *PCLOSE_COMPLETIONKEY;


typedef struct _FILE_INFO
{
	BYTE	FileName[50];
	BYTE	FileCheckSum[4];
	BYTE	FileSize[4];
} STRT_FILE_INFO, *PSTRT_FILE_INFO;



typedef struct _UPDATE_FILE_INFO
{
	char	FileName[50];
	int	FileSeq;
}STRT_UPDATE_FILE_INFO, *PSTRT_UPDATE_FILE_INFO;

namespace CMDID{
	enum CMDID_E 
	{
		FID_ALL_NAK = 0x007F,
		FID_ALL_ACK = 0x000A,		// 현재 프로토콜상 CmdID는 아님
		FID_PV_SENDFILEINFO		= 0xC2,
		FID_PV_SENDFILESTREAM	= 0xC3,
		FID_PV_SENDFILECHECK	= 0xC4
	};
}

namespace TERMINALID
{
	enum TID 
	{
		SERVER = 0x00F1,
		DEVICE = 0x00F7
	};
}


namespace VCI_FILE_ACCESS
{
	enum VCI_FILE_ACCESS_E 
	{
		FA_OPEN_EXISTING = 0x0000,
		FA_READ = 0x0001,
		FA_WRITE = 0x0002,
		FA_CREATE_NEW = 0x0004,
		FA_CREATE_ALWAYS = 0x0008,
		FA_OPEN_ALWAYS = 0x0010,
		FA_FLASH_LOG = 0x0011
	};
}

namespace VIEW_NO
{
	enum N_VIEW_BOARD
	{
		BOARD1 = 101,
		BOARD2,
		BOARD3,
		BOARD4,
		BOARD5,
		BOARD6,
		BOARD7,
		BOARD8,
		BOARD9,
		BOARD10,
		BOARD11
	};
}

namespace FILE_TYPE
{
	enum N_FILE_TYPE
	{
		IDLE = 1,
		UPDATE_STEP1,
		UPDATE_STEP2,
		UPDATE_STEP3,
		DEAD
	};
}
namespace VCI_STATUS
{
	enum N_VCI_STATUS
	{
		FLASH = 1,
		FWUP,
		FILEUP,
		IDLE,
		NONRESULT
	};
}


namespace FILE_UPLOAD_TYPE
{
	enum N_FILE_UPLOAD_TYPE
	{
		FLASHBIN = 1,
		FIRMWARE,
		BOOTWARE,
		READLOG,
		READFILE
	};
}
namespace CONNECT_TYPE
{
	enum N_CONNECT_TYPE
	{
		NEW_CONNECT = 1,
		RE_CONNECT
	};
}

namespace CLIENT_OBJECT
{
	enum N_CLIENT_OBJECT
	{
		CLIENT_OBJECT1 = 101,
		CLIENT_OBJECT2,
		CLIENT_OBJECT3,
		CLIENT_OBJECT4,
		CLIENT_OBJECT5,
		CLIENT_OBJECT6,
		CLIENT_OBJECT7,
		CLIENT_OBJECT8,
		CLIENT_OBJECT9,
		CLIENT_OBJECT10,
		CLIENT_OBJECT11 = 250
	};
}

namespace RECONNECT_TYPE_tmp
{
	enum N_RECONNECT_TYPE
	{
		INIT = 0,
		FLASHED = 1, //FLASH 진행후에 리컨넥스 된 경우 다시 플래시 하기 위한 플래그
		AUTO_FIRMWARE = 2,
		FILESYNC = 3
	};
}

namespace FILE_COPY_TYPE
{
	enum N_FILE_COPY_TYPE
	{
		ECU_FLASH_FILE = 1,
		LOG_FILE = 2
	};
}


namespace ECU_FILE_TYPE
{
	enum N_ECU_TYPE
	{
		ECU_BIN = 1,
		ECU_INI = 2,
		TCU_BIN = 3, 
		TCU_INI = 4
	};
}
