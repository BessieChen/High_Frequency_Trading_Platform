#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include"ThostFtdcTraderApi.h"
#include"MdSpi.h"//mdspi.h�Ѿ������������ͷ�ļ�
#include"mystruct.h"
#include<map>
#include<vector>
#include<set>

//#include"ThostFtdcMdApi.h"
class TdSpi :public CThostFtdcTraderSpi {
public:
	//���캯��
	TdSpi(CThostFtdcTraderApi* tdapi, CThostFtdcMdApi* pUserApi_md, MdSpi* pUserSpi_md);

	//���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	void OnFrontConnected();
	
	//�����û���¼
	int ReqUserLogin();

	///�ͻ�����֤����
	int ReqAuthenticate();

	//�����ѯ����
	void ReqQryOrder();

	//�����ѯ�ɽ�
	void ReqQryTrade();

	//�����ѯ�ֲ���ϸ
	void ReqQryInvestorPositionDetail();

	void ReqQryTradingAccount();
	void ReqQryInvestorPosition_All();
	void ReqQryInvestorPosition(char* pInstrument);
	void ReqQryInstrumetAll();
	void ReqQryInstrumet(char * pInstrument);

	///����ȷ�Ͻ��㵥
	void ReqSettlementInfoConfirm();

	void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	
	//�Ƿ���ִ���
	bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);
	
	///��¼������Ӧ
	void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�ǳ�������Ӧ
	void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	
	//�����ѯ������Ϣȷ����Ӧ
	void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;

	//�����ѯͶ���߽�������Ӧ
	void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField* pSettlementInfo,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;

	//Ͷ���߽�����ȷ����Ӧ
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;

	///�û��������������Ӧ
	void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField* pUserPasswordUpdate,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ������Ӧ
	void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//�����ѯͶ���ֲ߳���Ӧ
	void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;
	//�����ѯͶ���ֲ߳���Ӧ
	void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;

	///�����ѯ�ɽ���Ӧ
	void OnRspQryOrder(CThostFtdcOrderField* pOrder,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;

	///�����ѯ�ɽ���Ӧ
	void OnRspQryTrade(CThostFtdcTradeField* pTrade,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;

	///�����ѯ�ֲ���ϸ��Ӧ
	void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pField,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;

	//��ѯ�ʽ��ʻ���Ӧ
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;

	///����֪ͨ
	virtual void OnRtnOrder(CThostFtdcOrderField* pOrder);

	///�ɽ�֪ͨ
	virtual void OnRtnTrade(CThostFtdcTradeField* pTrade);
	
	//make an order;
	void PlaceOrder(const char* pszCode, const char* ExchangeID, int nDirection, int nOpenClose, int nVolume, double fPrice);

	//cancel an order;
	void CancelOrder(CThostFtdcOrderField* pOrder);
	
	// ��ӡ�ֲ���Ϣ
	void ShowPosition();
	
	// ƽ���˻��Ĳ�λ
	void ClosePosition();

	//���ó��򻯲����ܷ񿪲�
	void SetAllowOpen(bool isOk);

	//ͨ��brokerOrderSeq��ȡ����
	CThostFtdcOrderField* GetOrder(int nBrokerOrderSeq);

	//���¶���״̬
	bool UpdateOrder(CThostFtdcOrderField* pOrder);
	protected:
		int GetNextRequestID();
	void	ShowInstMessage();
		
private:
	CThostFtdcTraderApi* m_pUserTDApi_trade;
	CThostFtdcMdApi* m_pUserMDApi_trade;
	MdSpi* m_pUserMDSpi_trade;
	CThostFtdcReqUserLoginField* loginField;
	CThostFtdcReqAuthenticateField* authField;
	int  m_nNextRequestID;
	//��������б���
	std::vector<CThostFtdcOrderField*> orderList;
	//��������гɽ�
	std::vector<CThostFtdcTradeField*> tradeList;
	//δƽ�ֶ൥�ֲ���ϸ
	std::vector<CThostFtdcTradeField*> tradeList_NotClosed_Long;
	//δƽ�ֵĿյ��ֲ���ϸ
	std::vector<CThostFtdcTradeField*> tradeList_NotClosed_Short;

	//�ֲֺ�Լ
	std::vector<std::string> subscribe_inst_vec;
	
	//�ֲ���Ϣ�ṹ���map
	std::map<std::string, position_field*> m_position_field_map;

	//���ڱ��������ڻ���Լ��ӳ��
	std::map<std::string, CThostFtdcInstrumentField*> m_inst_field_map;


	//������ĳɽ�ID����
	std::set<std::string>  m_TradeIDs;
	//������ı�������
	std::map<int, CThostFtdcOrderField*> m_Orders;
	//map<int, CThostFtdcOrderField*> m_CancelingOrders;
	double m_CloseProfit;
	double m_OpenProfit;
	bool m_QryOrder_Once;
	bool m_QryTrade_Once;
	bool m_QryDetail_Once;
	bool m_QryTradingAccount_Once;
	bool m_QryPosition_Once;
	bool m_QryInstrument_Once;
	std::string m_AppId;
	std::string m_AuthCode;
	std::string m_BrokerId;
	std::string m_UserId;
	std::string m_Passwd;
	std::string m_InstId;
	std::string m_Inst_Postion;
	std::string m_Instrument_All;

	int m_nFrontID;
	int m_nSessionID;
	bool m_AllowOpen;
	char orderRef[13];
	//char tradingDate[13];
	TThostFtdcDateType	m_cTradingDay;

};