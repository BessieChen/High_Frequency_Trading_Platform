#include "TdSpi.h"
#include<map>
#include<iostream>
#include<mutex>
using namespace std;


extern std::map<std::string, std::string> accountConfig_map;//�����˻���Ϣ��map
//ȫ�ֵĻ�����
extern std::mutex m_mutex;


TdSpi::TdSpi(CThostFtdcTraderApi* tdapi, CThostFtdcMdApi* pUserApi_md, MdSpi* pUserSpi_md):
	m_pUserTDApi_trade(tdapi), m_pUserMDApi_trade(pUserApi_md), m_pUserMDSpi_trade(pUserSpi_md)

{
	//appid, simnow_client_test
	//	authcode, 0000000000000000
	//	product, simnow
	//	brokerId, 9999
	//	userId, 159599
	//	passwd, leon1222
	//	contract, ni2101, ta101
	//	MarketFront, tcp://180.168.146.187:10111
	//TradeFront, tcp ://180.168.146.187:10101
	
	//appid,authcode,userid,brokerid
	m_AppId= accountConfig_map["appid"];
	m_AuthCode= accountConfig_map["authcode"];
	m_BrokerId = accountConfig_map["brokerId"];
	m_UserId = accountConfig_map["userId"];
	m_Passwd= accountConfig_map["passwd"];
	m_nNextRequestID=0;
	m_QryOrder_Once = true;

	m_QryTrade_Once = true;
	m_QryDetail_Once = true;
	m_QryTradingAccount_Once = true;
	m_QryPosition_Once = true;
	m_QryInstrument_Once = true;
}

void TdSpi::OnFrontConnected()
{
	cerr << "OnFrontConnected ��" << endl;
	static const char* version = m_pUserTDApi_trade->GetApiVersion();
	cerr << "��ǰ��CTP Api Version��" <<version<< endl;
	ReqAuthenticate();
}



int TdSpi::ReqAuthenticate()
{
	//virtual int ReqAuthenticate(CThostFtdcReqAuthenticateField * pReqAuthenticateField, int nRequestID) = 0;
	CThostFtdcReqAuthenticateField req;
	//��ʼ��
	memset(&req, 0, sizeof(req));


	strcpy(req.AppID, m_AppId.c_str());
	//strcpy(req.AppID, "eiruoejladkfj");
	strcpy(req.AuthCode, m_AuthCode.c_str());
	//strcpy(req.AuthCode, "eiruoejladkfj");
	strcpy(req.BrokerID, m_BrokerId.c_str());
	//strcpy(req.BrokerID,"0000");
	//strcpy(req.UserID, m_UserId.c_str());
	strcpy(req.UserID, "");
	cerr << "������֤���˻���Ϣ��" << endl << " appid: " << req.AppID << " authcode: " << req.AuthCode
		<< " brokerid: " << req.BrokerID << " userId: " << req.UserID << endl;
	
	return m_pUserTDApi_trade->ReqAuthenticate(&req,GetNextRequestID());
}
void TdSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo)
	{
		if (pRspInfo->ErrorID == 0)
		{
			cerr << "��͸������֤�ɹ���" << "ErrMsg:" << pRspInfo->ErrorMsg << endl;
			ReqUserLogin();
		}

		else
		{
			cerr << "��͸������֤ʧ�ܣ�" << " errorid:" << pRspInfo->ErrorID <<
				"ErrMsg:" << pRspInfo->ErrorMsg << endl;

			//<error id = "AUTH_FAILED" value = "63" prompt = "CTP:�ͻ�����֤ʧ��" / >
		}
	}

}
int TdSpi::ReqUserLogin()
{
	cerr << "====ReqUserLogin====,�û���¼��..." <<endl;
	//����һ��CThostFtdcReqUserLoginField
	CThostFtdcReqUserLoginField reqUserLogin;
	//��ʼ��Ϊ0
	memset(&reqUserLogin, 0, sizeof(reqUserLogin));
	//����brokerid,userid,passwd
	strcpy(reqUserLogin.BrokerID, m_BrokerId.c_str());
	//strcpy(reqUserLogin.BrokerID, "0000");
	//errorid:3�����Ϸ��ĵ�¼
	//strcpy(reqUserLogin.UserID, "00000000");
	strcpy(reqUserLogin.UserID, m_UserId.c_str());
	strcpy(reqUserLogin.Password, m_Passwd.c_str());

	//strcpy(reqUserLogin.Password, "00000000");
	
	//��¼
	return m_pUserTDApi_trade->ReqUserLogin(&reqUserLogin, GetNextRequestID());

}

void TdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "��¼����ص�OnRspUserLogin" << endl;
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
		m_nFrontID = pRspUserLogin->FrontID;
		m_nSessionID = pRspUserLogin->SessionID;
		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);

		sprintf_s(orderRef, sizeof(orderRef), "%d", ++nextOrderRef);

		cout << "ǰ�ñ��:" << pRspUserLogin->FrontID << endl
			<< "�Ự���" << pRspUserLogin->SessionID << endl
			<< "��󱨵�����:" << pRspUserLogin->MaxOrderRef << endl
			<< "������ʱ�䣺" << pRspUserLogin->SHFETime << endl
			<< "������ʱ�䣺" << pRspUserLogin->DCETime << endl
			<< "֣����ʱ�䣺" << pRspUserLogin->CZCETime << endl
			<< "�н���ʱ�䣺" << pRspUserLogin->FFEXTime << endl
			<< "��Դ��ʱ�䣺" << pRspUserLogin->INETime << endl
			<< "�����գ�" << m_pUserTDApi_trade->GetTradingDay() << endl;
		strcpy(m_cTradingDay, m_pUserTDApi_trade->GetTradingDay());//���ý�������
		
		cout << "--------------------------------------------" << endl << endl;
	}

	ReqSettlementInfoConfirm();

}


void TdSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{


}
void TdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}
void TdSpi::ReqQryOrder()
{
	CThostFtdcQryOrderField  QryOrderField;//����
	memset(&QryOrderField, 0, sizeof(CThostFtdcQryOrderField));//��ʼ��Ϊ0
	//brokerid����
	//strcpy(QryOrderField.BrokerID, "0000");
	strcpy(QryOrderField.BrokerID, m_BrokerId.c_str());
	//InvestorID����
	strcpy(QryOrderField.InvestorID, "666666");
	//strcpy(QryOrderField.InvestorID, m_UserId.c_str());
	//����api��ReqQryOrder
	m_pUserTDApi_trade->ReqQryOrder(&QryOrderField, GetNextRequestID());

}
void TdSpi::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "�����ѯ������Ӧ��OnRspQryOrder" <<",pOrder  "<<pOrder<< endl;
	if (!IsErrorRspInfo(pRspInfo) && pOrder)
	{
		cerr << "�����ѯ������Ӧ��ǰ�ñ��FrontID��" << pOrder->FrontID << ",�Ự���:" << pOrder->SessionID
			<< ",��������:  " << pOrder->OrderRef << endl;
			//���к�Լ
		if (m_QryOrder_Once == true)
		{
			CThostFtdcOrderField* order = new CThostFtdcOrderField();
			memcpy(order, pOrder, sizeof(CThostFtdcOrderField));
			orderList.push_back(order);

			//bIsLast�Ƿ������һ�ʻر�
			if (bIsLast)
			{
				m_QryOrder_Once = false;
				cerr << "���к�Լ�ı�������" << orderList.size() << endl;
				cerr << "---------------��ӡ������ʼ---------------" << endl;
				for (vector<CThostFtdcOrderField*>::iterator iter = orderList.begin(); iter != orderList.end(); iter++)
				{
					cerr << "���͹�˾���룺" << (*iter)->BrokerID << endl << "Ͷ���ߴ��룺" << (*iter)->InvestorID << endl
						<< "�û����룺" << (*iter)->UserID << endl << "��Լ���룺" << (*iter)->InstrumentID << endl
						<< "��������" << (*iter)->Direction << endl << "��Ͽ�ƽ��־��" << (*iter)->CombOffsetFlag << endl
						<< "�۸�" << (*iter)->LimitPrice << endl << "������" << (*iter)->VolumeTotalOriginal << endl
						<< "�������ã�" << (*iter)->OrderRef << endl << "�ͻ����룺" << (*iter)->ClientID << endl
						<< "����״̬��" << (*iter)->OrderStatus << endl << "ί��ʱ�䣺" << (*iter)->InsertTime << endl
						<< "������ţ�" << (*iter)->OrderStatus << endl << "�����գ�" << (*iter)->TradingDay << endl
						<< "�������ڣ�" << (*iter)->InsertDate << endl;

				}
				cerr << "---------------��ӡ�������---------------" << endl;
				cerr << "��ѯ�������������״β�ѯ�ɽ�" << endl;
				//�߳�����3�룬��ctp��̨�г������Ӧʱ�䣬Ȼ���ٽ��в�ѯ����
				std::chrono::milliseconds sleepDuration(3 * 1000);
				std::this_thread::sleep_for(sleepDuration);
				ReqQryTrade();

			}
		}

	}
	else//��ѯ����
	{
		m_QryOrder_Once = false;
		cerr << "��ѯ����������û�гɽ������״β�ѯ�ɽ�" << endl;
		//�߳�����3�룬��ctp��̨�г������Ӧʱ�䣬Ȼ���ٽ��в�ѯ����
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryTrade();
	}
	
}


void TdSpi::ReqQryTrade()
{
	CThostFtdcQryTradeField tdField;//����
	memset(&tdField, 0, sizeof(tdField));//��ʼ��

	strcpy(tdField.BrokerID, m_BrokerId.c_str());
	//strcpy(tdField.BrokerID,"0000");
	strcpy(tdField.InvestorID, m_UserId.c_str());
	//strcpy(tdField.InvestorID, "888888");
	//���ý���api��ReqQryTrade
	m_pUserTDApi_trade->ReqQryTrade(&tdField, GetNextRequestID());
}
void TdSpi::OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{

	cerr << "�����ѯ�ɽ��ر���Ӧ��OnRspQryTrade" << " pTrade " << pTrade << endl;

	if (!IsErrorRspInfo(pRspInfo) && pTrade)
	{
		//���к�Լ
		if (m_QryTrade_Once == true)
		{
			CThostFtdcTradeField* trade = new CThostFtdcTradeField();//����trade
			memcpy(trade, pTrade, sizeof(CThostFtdcTradeField));//pTrade���Ƹ�trade
			tradeList.push_back(trade);

			if (bIsLast)
			{
				m_QryTrade_Once = false;
				cerr << "���к�Լ�ĳɽ�����" << tradeList.size() << endl;
				cerr << "---------------��ӡ�ɽ���ʼ---------------" << endl;
				for (vector<CThostFtdcTradeField*>::iterator iter = tradeList.begin(); iter != tradeList.end(); iter++)
				{
					cerr << "���͹�˾���룺" << (*iter)->BrokerID << endl << "Ͷ���ߴ��룺" << (*iter)->InvestorID << endl
						<< "�û����룺" << (*iter)->UserID << endl << "�ɽ���ţ�" << (*iter)->TradeID << endl
						<< "��Լ���룺" << (*iter)->InstrumentID << endl << "��������" << (*iter)->Direction << endl
						<< "��Ͽ�ƽ��־��" << (*iter)->OffsetFlag << endl << "Ͷ���ױ���־��" << (*iter)->HedgeFlag << endl
						<< "�۸�" << (*iter)->Price << endl << "������" << (*iter)->Volume << endl
						<< "�������ã�" << (*iter)->OrderRef << endl << "���ر�����ţ�" << (*iter)->OrderLocalID << endl
						<< "�ɽ�ʱ�䣺" << (*iter)->TradeTime << endl << "ҵ��Ԫ��" << (*iter)->BusinessUnit << endl
						<< "��ţ�" << (*iter)->SequenceNo << endl << "���͹�˾�µ���ţ�" << (*iter)->BrokerOrderSeq << endl
						<< "�����գ�" << (*iter)->TradingDay << endl;

				}
				cerr << "---------------��ӡ�ɽ����---------------" << endl;
				cerr << "��ѯ�������������״β�ѯ�ֲ���ϸ" << endl;
				//�߳�����3�룬��ctp��̨�г������Ӧʱ�䣬Ȼ���ٽ��в�ѯ����
				std::chrono::milliseconds sleepDuration(3 * 1000);
				std::this_thread::sleep_for(sleepDuration);
				ReqQryInvestorPositionDetail();
			}
			

		}

	}
	else//��ѯ����
	{
		m_QryOrder_Once = false;
		cerr << "��ѯ����������û�гɽ������״β�ѯ�ɽ�" << endl;
		//�߳�����3�룬��ctp��̨�г������Ӧʱ�䣬Ȼ���ٽ��в�ѯ����
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryInvestorPositionDetail();
	}

	
}
void TdSpi::ReqQryInvestorPositionDetail()
{
}

void TdSpi::ReqQryTradingAccount()
{
}

void TdSpi::ReqQryInvestorPosition_All()
{
}

void TdSpi::ReqQryInvestorPosition()
{
}

void TdSpi::ReqQryInstrumetAll()
{
}

void TdSpi::ReqQryInstrumet()
{
}

void TdSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;//����
	memset(&req, 0, sizeof(req));//��ʼ��
	strcpy(req.BrokerID, m_BrokerId.c_str());
	//strcpy(req.BrokerID, "0000");
	strcpy(req.InvestorID, m_UserId.c_str());
	//strcpy(req.InvestorID, "000000");
	
	int iResult = m_pUserTDApi_trade->ReqSettlementInfoConfirm(&req, GetNextRequestID());
	cerr << "--->>> Ͷ���߽�����ȷ��: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}


void TdSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	
	if (bIsLast && !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm)
	{
		cerr << "��Ӧ | ���㵥..." << pSettlementInfoConfirm->InvestorID
			<< "..." << pSettlementInfoConfirm->ConfirmDate << "," <<
			pSettlementInfoConfirm->ConfirmTime << "...ȷ��" << endl << endl;

		cerr << "���㵥ȷ���������״β�ѯ����" << endl;
		//�߳�����3�룬��ctp��̨�г������Ӧʱ�䣬Ȼ���ٽ��в�ѯ����
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		//Sleep(1000);
		ReqQryOrder();
	}
}

bool TdSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	return bResult;
	
}




void TdSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField* pSettlementInfo, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}


void TdSpi::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField* pUserPasswordUpdate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}



void TdSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
}

void TdSpi::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
}



void TdSpi::CancelOrder(CThostFtdcOrderField* pOrder)
{
}

void TdSpi::ShowPosition()
{
}

void TdSpi::ClosePosition()
{
}

void TdSpi::SetAllowOpen(bool isOk)
{
}

CThostFtdcOrderField* TdSpi::GetOrder(int nBrokerOrderSeq)
{
	return nullptr;
}

bool TdSpi::UpdateOrder(CThostFtdcOrderField* pOrder)
{
	return false;
}

int TdSpi::GetNextRequestID()
{
	//��m_nNextRequestID���ϻ�����
	/*m_mutex.lock();
	int nNextID = m_nNextRequestID++;
	m_mutex.unlock();*/
	//1ԭ���ڹ��캯������ʹ��m_mutex.lock();
	//��������ʱ��ʹ�ý���m_mutex.unlock();
std:lock_guard<mutex> m_lock(m_mutex);
	
	int nNextID = m_nNextRequestID++;

	return m_nNextRequestID;
}

void TdSpi::PlaceOrder(const char* pszCode, const char* ExchangeID, int nDirection, int nOpenClose, int nVolume, double fPrice)
{
}
