#ifndef __XDR_MG_MSGINFO_H__
#define __XDR_MG_MSGINFO_H__

enum Procedure_Status
{
	success = 0, //成功；
	failed = 1, //失败；
	timeout = 255 //超时
};

enum XDR_TYPE
{
	XDR_TYPE_Uu = 1,
	XDR_TYPE_X2 = 2,
	XDR_TYPE_UE_MR = 3,
	XDR_TYPE_Cell_MR = 4,
	XDR_TYPE_S1_MME = 5,
	XDR_TYPE_S6a = 6,
	XDR_TYPE_S11 = 7,
	XDR_TYPE_S10 = 8,
	XDR_TYPE_SGs = 9,
	XDR_TYPE_S5_S8 = 10,
	XDR_TYPE_S1_U = 11,
	XDR_TYPE_Gn_C = 12
};


enum XDR_PROCEDURE
{
	Attach = 1,
	Sevice_Request = 2,
	Paging = 3,
	TAU = 4,
	Detach = 5,
	PDN_connectivity = 6,
	PDN_disconnection = 7,
	EPS_resource_allocation = 8,
	EPS_resource_modify = 9,
	EPS_context_deactivation = 10,
	EPS_context_modification = 11,
	EPS_context_activation = 12,
	X2_handover = 13,
	S1_handover = 14,
	UE_Context_Release = 15,
	EPS_Release = 16,
	CSFB = 17
};

enum S1_MME_PROCEDURE
{
	s1_Attach = 1,	
	s1_Sevice_Request = 2,	
	s1_Extended_Service_Request = 3,	
	s1_Paging = 4,	
	s1_TAU = 5,	
	s1_Detach = 6,	
	s1_PDN_connectivity = 7,	
	s1_PDN_disconnection = 8,	
	s1_EPS_resource_allocation = 9,
	s1_EPS_resource_modify = 10,	
	s1_EPS_context_deactivation = 11,
	s1_EPS_context_modification = 12,	
	s1_EPS_context_activation = 13,	
	s1_X2_change = 14,
	s1_S1_out = 15,
	s1_S1_in = 16,
	s1_Initial_context_setup = 17,
	s1_UE_context_modification = 18,	
	s1_UE_context_release = 19,
	s1_E_RAB_release = 20,
	s1_Reset = 21,	
	s1_Error_indication = 22,
	s1_S1_setup = 23,
	s1_ENB_configuration_update = 24,
	s1_MME_configuration_update = 25,
	s1_Overload_start = 26,
	s1_Overload_stop = 27,
	s1_Identity_Acquisition = 28,	
	s1_Authentication = 29,	
	s1_Security_Activation = 30	
};

enum UU_PROCEDURE
{
	RRC_CONN_STP = 1, //RRC连接建立 
	RRC_SMC = 2, //安全模式激活 
	RRC_RE_CFG = 3, //RRC连接重配 
	RRC_RE_EST = 4, //RRC连接重建 
	RRC_REL = 5, //RRC连接释放 
	RRC_HO_intraCELL = 6, //小区内部切换 
	RRC_HO_intraENB = 7, //基站内切换 
	RRC_HO_interENB = 8, //基站间切换 
	RRC_HO_IN = 9, //从其它RAT切入 
	RRC_HO_OUT_RAT = 10, //切出至其他RAT 
	RRC_PAGING_PS = 11, //PS寻呼 
	RRC_PAGING_CS = 12, //CS寻呼 
	RRC_OTHER = 13 //其它事件类型 
};

enum X2_PROCEDURE
{
	handover = 1,
	handover_cancel = 2,
	setup = 3,
	x2_Reset = 4,
	update = 5,
	reporting = 6,
	change = 7,
	activation = 8, 
	Load = 9,
	Error = 10
};

//enum S1_MME_PROCEDURE
//{
//	Attach = 1,	
//	Sevice_Request = 2,	
//	Extended_Service_Request = 3,	
//	Paging = 4,	
//	TAU = 5,	
//	Detach = 6,	
//	PDN_connectivity = 7,	
//	PDN_disconnection = 8,	
//	EPS_resource_allocation = 9,
//	EPS_resource_modify = 10,	
//	EPS_context_deactivation = 11,
//	EPS_context_modification = 12,	
//	EPS_context_activation = 13,	
//	X2_change = 14,
//	S1_out = 15,
//	S1_in = 16,
//	Initial_context_setup = 17,
//	UE_context_modification = 18,	
//	UE_context_release = 19,
//	E_RAB_release = 20,
//	Reset = 21,	
//	Error_indication = 22,
//	S1_setup = 23,
//	ENB_configuration_update = 24,
//	MME_configuration_update = 25,
//	Overload_start = 26,
//	Overload_stop = 27,
//	Identity_Acquisition = 28,	
//	Authentication = 29,	
//	Security_Activation = 30	
//};

enum S6A_PROCEDURE
{
	Update_Location = 1,
	Cancel_Location = 2,
	Purge_UE = 3,
	Insert_Subscriber_Data = 4,
	Delete_Subscriber_Data = 5,
	Authentication_Information = 6,
	Reset = 7,
	Notification = 8

};

enum SGS_PROCEDURE
{
	PAGING = 1,
	SERVICE_REQUEST = 2,
	DOWNLINK_UNITDATA = 3,
	UPLINK_UNITDATA = 4,
	LOCATION_UPDATE = 5,
	TMSI_REALLOCATION = 6,
	ALERT = 7,
	UE_ACTIVITY_INDICATION = 8,
	EPS_DETACH = 9,
	IMSI_DETACH = 10,
	RESET = 11,
	SERVICE_ABORT = 12,
	MM_INFORMATION = 13,
	RELEASE = 14,
	STATUS = 15,
	UE_UNREACHABLE = 16

};


#endif //__XDR_MG_MSGINFO_H__