#pragma once
#include<string>
#include"main.h"
#include "ThostFtdcMdApi.h"
//using namespace std;
class MdSpi :public CThostFtdcMdSpi {
public:
	MdSpi(CThostFtdcMdApi* mdapi);
	~MdSpi();
	//��������ʱ����
	void OnFrontConnected();

	void ReqUserLogin(std::string brokerId, std::string userId, std::string passwd);
	///��¼������Ӧ
	void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	///�ǳ�������Ӧ
	void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	///��������Ӧ��
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	///ȡ����������Ӧ��
	void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);

	void SubscribeMarketData(char* instIdList);
	void SubscribeMarketData(std::string instIdList);
	//�������еĺ�Լ
	void SubscribeMarketData_All();
	
	
	///�������֪ͨ
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);

	//���ĳֲֺ�Լ����,��stringת��Ϊchar����
	void setInstIdList_Position_MD(std::string& inst_holding);
	//"IF2101,IF2102,IF2103" 
	//����ȫ����Լ
	void set_InstIdList_All(std::string& inst_all);

private:
	CThostFtdcMdApi* mdapi;
	CThostFtdcReqUserLoginField* loginField;
	std::string m_BrokerId;
	std::string m_UserId;
	std::string m_Passwd;
	
	//����������Ҫ���׵ĺ�Լ
	char m_InstId[32];

	//�ֲֵĺ�Լ
	char* m_InstIdList_Position_MD;

	//���еĺ�Լ
	char* m_InstIdList_all;
};