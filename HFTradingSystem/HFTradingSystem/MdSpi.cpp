#include"MdSpi.h"
#include<iostream>
#include<mutex>
#include<vector>


extern std::map<std::string, std::string> accountConfig_map;//�����˻���Ϣ��map


//ȫ�ֵĻ�����
extern std::mutex m_mutex;

//ȫ�ֵ�requestId
extern int g_nRequestID;

using namespace std;

MdSpi::MdSpi(CThostFtdcMdApi* mdapi):mdapi(mdapi)
{
	//�ڻ���˾����
	m_BrokerId = accountConfig_map["brokerId"];
	//�ڻ��˻�
	m_UserId = accountConfig_map["userId"];
	//����
	m_Passwd = accountConfig_map["passwd"];
	//������Ҫ���׵ĺ�Լ
	memset(m_InstId, 0, sizeof(m_InstId));
	strcpy(m_InstId, accountConfig_map["contract"].c_str());

}

MdSpi::~MdSpi()
{
}

void MdSpi::OnFrontConnected()
{
	cerr << "����ǰ���������ϣ������¼" << endl;

	ReqUserLogin(m_BrokerId, m_UserId, m_Passwd);
	
}

void MdSpi::ReqUserLogin(std::string brokerId, std::string userId, std::string passwd)
{
	loginField = new CThostFtdcReqUserLoginField();
	//strcpy(loginField->BrokerID, brokerId.c_str());
	/*strcpy(loginField->UserID, userId.c_str());
	strcpy(loginField->Password, passwd.c_str());*/
	//�����brokerid
	strcpy(loginField->BrokerID, "0000");
	strcpy(loginField->UserID, "");
	strcpy(loginField->Password, "");
	int nResult = mdapi->ReqUserLogin(loginField, GetNextRequestID());
	cerr << "�����¼���飺" << ((nResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}

void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}
//char instIdList[]="IF2012,IF2101,IF2103"
void MdSpi::SubscribeMarketData(char* instIdList)
{

}
//string instIdList="IF2012,IF2101,IF2103"
void MdSpi::SubscribeMarketData(std::string instIdList)
{
	
}
//char* m_InstIdList_all;
void MdSpi::SubscribeMarketData_All()
{

}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
}

//"IF2102,IF2103",string����ת��Ϊ��char *
void MdSpi::setInstIdList_Position_MD(std::string& inst_holding)
{
	m_InstIdList_Position_MD = new char[inst_holding.length()+1];

	memcpy(m_InstIdList_Position_MD, inst_holding.c_str(), inst_holding.length());
	m_InstIdList_Position_MD[inst_holding.length()] = '\0';
}

void MdSpi::set_InstIdList_All(std::string& inst_all)
{
	m_InstIdList_all = new char[inst_all.size() + 1];
	//m_InstIdList_all = new char[inst_all.length()];
	strcpy(m_InstIdList_all, inst_all.c_str());
	m_InstIdList_all[inst_all.length()] = '\0';
}

int MdSpi::GetNextRequestID()
{
	//��m_nNextRequestID���ϻ�����
		/*m_mutex.lock();
		int nNextID = m_nNextRequestID++;
		m_mutex.unlock();*/
		//1ԭ���ڹ��캯������ʹ��m_mutex.lock();
		//��������ʱ��ʹ�ý���m_mutex.unlock();
std:lock_guard<mutex> m_lock(m_mutex);

	int nNextID = g_nRequestID++;

	return g_nRequestID;
}


