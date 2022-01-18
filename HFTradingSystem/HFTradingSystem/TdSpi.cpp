#include "TdSpi.h"
#include<map>
#include<iostream>
#include<mutex>
#include"Strategy.h"
using namespace std;


extern std::map<std::string, std::string> accountConfig_map;//�����˻���Ϣ��map
//ȫ�ֵĻ�����
extern std::mutex m_mutex;
extern Strategy* g_strategy;//������ָ��

extern int g_nRequestID;

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
	m_InstId= accountConfig_map["contract"];
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
	//strcpy(QryOrderField.InvestorID, "666666");
	strcpy(QryOrderField.InvestorID, m_UserId.c_str());
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
				

			}
		}

	}
	else//��ѯ����
	{
		m_QryOrder_Once = false;
		cerr << "��ѯ����������û�гɽ������״β�ѯ�ɽ�" << endl;
		
	}
	if (bIsLast)
	{
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
				
			}
			

		}

	}
	else//��ѯ����
	{
		m_QryOrder_Once = false;
		cerr << "��ѯ����������û�гɽ������״β�ѯ�ɽ�" << endl;
		
	}
	if (bIsLast)
	{
		//�߳�����3�룬��ctp��̨�г������Ӧʱ�䣬Ȼ���ٽ��в�ѯ����
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryInvestorPositionDetail();
	}

	
}
void TdSpi::ReqQryInvestorPositionDetail()
{
	CThostFtdcQryInvestorPositionDetailField pdField;//����
	memset(&pdField, 0, sizeof(pdField));//��ʼ��Ϊ0
	strcpy(pdField.BrokerID, m_BrokerId.c_str());
	//strcpy(pdField.BrokerID, "0000");
	//strcpy(pdField.InstrumentID, m_InstId.c_str());


	strcpy(pdField.InvestorID, m_UserId.c_str());

	//strcpy(pdField.InvestorID, "0000");
	//���ý���api��ReqQryInvestorPositionDetail
	m_pUserTDApi_trade->ReqQryInvestorPositionDetail(&pdField,GetNextRequestID());
}

void TdSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "�����ѯͶ���ֲ߳���ϸ�ر���Ӧ��OnRspQryInvestorPositionDetail" << " pInvestorPositionDetail " << pField << endl;
	if (!IsErrorRspInfo(pRspInfo) && pField)
	{
		//���к�Լ
		if (m_QryDetail_Once == true)
		{
			//�������к�Լ��ֻ����δƽ�ֵģ��������Ѿ�ƽ�ֵ�
			//����������ǰ�ĳֲּ�¼���浽δƽ������tradeList_NotClosed_Long��tradeList_NotClosed_Short
			//ʹ�ýṹ��CThostFtdcTradeField����Ϊ����ʱ���ֶΣ���CThostFtdcInvestorPositionDetailFieldû��ʱ���ֶ�
			CThostFtdcTradeField* trade = new CThostFtdcTradeField();//����CThostFtdcTradeField *
			
			strcpy(trade->InvestorID, pField->InvestorID);///Ͷ���ߴ���
			strcpy(trade->InstrumentID, pField->InstrumentID);///��Լ����
			strcpy(trade->ExchangeID, pField->ExchangeID);///����������
			trade->Direction = pField->Direction;//��������
			trade->Price = pField->OpenPrice;//�۸�
			trade->Volume = pField->Volume;//����
			strcpy(trade->TradeDate, pField->OpenDate);//�ɽ�����
			strcpy(trade->TradeID, pField->TradeID);//*********�ɽ����********
			if (pField->Volume > 0)//ɸѡδƽ�ֺ�Լ
			{
				if (trade->Direction == '0')//���뷽��
					tradeList_NotClosed_Long.push_back(trade);
				else if (trade->Direction == '1')//��������
					tradeList_NotClosed_Short.push_back(trade);
			}
			//�ռ��ֲֺ�Լ�Ĵ���
			bool find_instId = false;
			for (unsigned int i = 0; i < subscribe_inst_vec.size(); i++)
			{
				if (strcmp(subscribe_inst_vec[i].c_str(), trade->InstrumentID) == 0)//��Լ�Ѵ��ڣ��Ѷ���
				{
					find_instId = true;
					break;
				}
			}
			if (!find_instId)//��Լδ���Ĺ�
			{
				cerr << "---------------------------------------�óֲֺ�Լδ���Ĺ������붩���б�" << endl;
				subscribe_inst_vec.push_back(trade->InstrumentID);
			}

		}
		//������к�Լ�ĳֲ���ϸ��Ҫ����߽�����һ���Ĳ�ѯReqQryTradingAccount()
		if (bIsLast)
		{
			m_QryDetail_Once = false;
			//�ֲֵĺ�Լ
			string inst_holding;
			//
			for (unsigned int i = 0; i < subscribe_inst_vec.size(); i++)
				inst_holding = inst_holding + subscribe_inst_vec[i] + ",";
			//"IF2102,IF2103,"

			inst_holding = inst_holding.substr(0, inst_holding.length() - 1);//ȥ�����Ķ��ţ���λ��0��ʼ��ѡȡlength-1���ַ�
			//"IF2102,IF2103"

			cerr << "��������ǰ�ĳֲ��б�:" << inst_holding << ",inst_holding.length()=" << inst_holding.length()
				<< ",subscribe_inst_vec.size()=" << subscribe_inst_vec.size() << endl;

			if (inst_holding.length() > 0)
				m_pUserMDSpi_trade->setInstIdList_Position_MD(inst_holding);//���ó�������ǰ�����֣�����Ҫ��������ĺ�Լ

			//size�������������������
			cerr << "�˻����к�Լδƽ�ֵ��������µ�������һ�ʿ��Զ�Ӧ���֣�,�൥:" << tradeList_NotClosed_Long.size()
				<< "�յ���" << tradeList_NotClosed_Short.size() << endl;


			cerr << "-----------------------------------------δƽ�ֶ൥��ϸ��ӡstart" << endl;
			for (vector<CThostFtdcTradeField*>::iterator iter = tradeList_NotClosed_Long.begin(); iter != tradeList_NotClosed_Long.end(); iter++)
			{
				cerr << "BrokerID:" << (*iter)->BrokerID << endl << "InvestorID:" << (*iter)->InvestorID << endl
					<< "InstrumentID:" << (*iter)->InstrumentID << endl << "Direction:" << (*iter)->Direction << endl
					<< "OpenPrice:" << (*iter)->Price << endl << "Volume:" << (*iter)->Volume << endl
					<< "TradeDate:" << (*iter)->TradeDate << endl << "TradeID:" << (*iter)->TradeID << endl;
			}

			cerr << "-----------------------------------------δƽ�ֿյ���ϸ��ӡstart" << endl;
			for (vector<CThostFtdcTradeField*>::iterator iter = tradeList_NotClosed_Short.begin(); iter != tradeList_NotClosed_Short.end(); iter++)
			{
				cerr << "BrokerID:" << (*iter)->BrokerID << endl << "InvestorID:" << (*iter)->InvestorID << endl
					<< "InstrumentID:" << (*iter)->InstrumentID << endl << "Direction:" << (*iter)->Direction << endl
					<< "OpenPrice:" << (*iter)->Price << endl << "Volume:" << (*iter)->Volume << endl
					<< "TradeDate:" << (*iter)->TradeDate << endl << "TradeID:" << (*iter)->TradeID << endl;
			}
			cerr << "---------------��ӡ�ֲ���ϸ���---------------" << endl;
			cerr << "��ѯ�ֲ���ϸ���������״β�ѯ�˻��ʽ���Ϣ" << endl;
		}
		
	}
	else
	{
		if (m_QryDetail_Once == true)
		{
			m_QryDetail_Once = false;
			cerr << "��ѯ�ֲ���ϸ������û�гֲ���ϸ�����״β�ѯ�˻��ʽ�" << endl;
		}
	}
	if (bIsLast)
	{
		//�߳�����3�룬��ctp��̨�г������Ӧʱ�䣬Ȼ���ٽ��в�ѯ����
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryTradingAccount();
	}
	
}

void TdSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;//����req�Ľṹ�����
	memset(&req, 0, sizeof(req));//��ʼ��
	//�����brokerID
	//strcpy(req.BrokerID, "8888");


	strcpy(req.BrokerID, m_BrokerId.c_str());


	strcpy(req.InvestorID, m_UserId.c_str());
	//strcpy(req.InvestorID, "666666");
	//���ý���api��ReqQryTradingAccount
	int iResult = m_pUserTDApi_trade->ReqQryTradingAccount(&req, GetNextRequestID());
	cerr << "--->>> �����ѯ�ʽ��˻�: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}

void TdSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "�����ѯͶ�����ʽ��˻��ر���Ӧ��OnRspQryTradingAccount" << " pTradingAccount " << pTradingAccount << endl;
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount)
	{

		cerr << "Ͷ���߱�ţ�" << pTradingAccount->AccountID
			<< "��̬Ȩ�棺�ڳ�Ȩ��" << pTradingAccount->PreBalance
			<< "��̬Ȩ�棺�ڻ�����׼����" << pTradingAccount->Balance
			<< "�����ʽ�" << pTradingAccount->Available
			<< "��ȡ�ʽ�" << pTradingAccount->WithdrawQuota
			<< "��ǰ��֤���ܶ" << pTradingAccount->CurrMargin
			<< "ƽ��ӯ����" << pTradingAccount->CloseProfit
			<< "�ֲ�ӯ����" << pTradingAccount->PositionProfit
			<< "�����ѣ�" << pTradingAccount->Commission
			<< "���ᱣ֤��" << pTradingAccount->FrozenCash
			<< endl;
		//���к�Լ
		if (m_QryTradingAccount_Once == true)
		{
			m_QryTradingAccount_Once = false;
		}

		cerr << "---------------��ӡ�ʽ��˻���ϸ���---------------" << endl;
		cerr << "��ѯ�ʽ��˻����������״β�ѯͶ���ֲ߳���Ϣ" << endl;
	}
	else
	{
		if (m_QryTradingAccount_Once == true)
		{
			m_QryTradingAccount_Once = false;
			cerr << "��ѯ�ʽ��˻��������״β�ѯͶ���ֲ߳�" << endl;
		}
	}
	//�߳�����3�룬��ctp��̨�г������Ӧʱ�䣬Ȼ���ٽ��в�ѯ����
	std::chrono::milliseconds sleepDuration(3 * 1000);
	std::this_thread::sleep_for(sleepDuration);
	ReqQryInvestorPosition_All();
}


void TdSpi::ReqQryInvestorPosition_All()
{
	CThostFtdcQryInvestorPositionField req;//����req
	memset(&req, 0, sizeof(req));//��ʼ��Ϊ0

	//strcpy(req.BrokerID, "8888");
	strcpy(req.BrokerID, m_BrokerId.c_str());
	strcpy(req.InvestorID, m_UserId.c_str());
	//strcpy(req.InvestorID, "0000");
	//��ԼΪ�գ�������ѯ���к�Լ�ĳֲ֣������reqΪ����һ����
	strcpy(req.InstrumentID, m_InstId.c_str());
	//���ý���api��ReqQryInvestorPosition
	int iResult = m_pUserTDApi_trade->ReqQryInvestorPosition(&req, GetNextRequestID());//reqΪ�գ������ѯ���к�Լ�ĳֲ�
	cerr << "--->>> �����ѯͶ���ֲ߳�: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}
void TdSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition,
	CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	//cerr << "�����ѯ�ֲ���Ӧ��OnRspQryInvestorPosition " << ",pInvestorPosition  " << pInvestorPosition << endl;
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition)
	{

		//�˻������к�Լ
		if (m_QryPosition_Once == true)
		{
			cerr << "�����ѯ�ֲ���Ӧ��OnRspQryInvestorPosition " << " pInvestorPosition:  "
				<< pInvestorPosition << endl;//������Ѿ�ƽ��û�гֲֵļ�¼
			cerr << "��Ӧ  | ��Լ " << pInvestorPosition->InstrumentID << endl
				<< " �ֲֶ�շ��� " << pInvestorPosition->PosiDirection << endl//2��3��
			   // << " ӳ���ķ��� " << MapDirection(pInvestorPosition->PosiDirection-2,false) << endl
				<< " �ֲܳ� " << pInvestorPosition->Position << endl
				<< " ���ճֲ� " << pInvestorPosition->TodayPosition << endl
				<< " ���ճֲ� " << pInvestorPosition->YdPosition << endl
				<< " ��֤�� " << pInvestorPosition->UseMargin << endl
				<< " �ֲֳɱ� " << pInvestorPosition->PositionCost << endl
				<< " ������ " << pInvestorPosition->OpenVolume << endl
				<< " ƽ���� " << pInvestorPosition->CloseVolume << endl
				<< " �ֲ����� " << pInvestorPosition->TradingDay << endl
				<< " ƽ��ӯ��������ᣩ " << pInvestorPosition->CloseProfitByDate << endl
				<< " �ֲ�ӯ�� " << pInvestorPosition->PositionProfit << endl
				<< " ���ն���ƽ��ӯ��������ᣩ " << pInvestorPosition->CloseProfitByDate << endl//��������ʾ�������ֵ
				<< " ��ʶԳ�ƽ��ӯ��������ƽ��Լ�� " << pInvestorPosition->CloseProfitByTrade << endl//�ڽ����бȽ�������
				<< endl;


			//�����Լ��Ӧ�ֲ���ϸ��Ϣ�Ľṹ��map
			bool  find_trade_message_map = false;
			for (map<string, position_field*>::iterator iter = m_position_field_map.begin(); iter != m_position_field_map.end(); iter++)
			{
				if (strcmp((iter->first).c_str(), pInvestorPosition->InstrumentID) == 0)//��Լ�Ѵ���
				{
					find_trade_message_map = true;
					break;
				}
			}
			if (!find_trade_message_map)//��Լ������
			{
				cerr << "-----------------------û�������Լ����Ҫ���콻����Ϣ�ṹ��" << endl;
				position_field* p_trade_message = new position_field();
				p_trade_message->instId = pInvestorPosition->InstrumentID;
				//����ֲֺ�Լ��string
				m_Inst_Postion += pInvestorPosition->InstrumentID ;
				m_Inst_Postion += ",";
				m_position_field_map.insert(pair<string, position_field*>(pInvestorPosition->InstrumentID, p_trade_message));
			}
			if (pInvestorPosition->PosiDirection == '2')//�൥
			{
				//��ֺͽ��һ�η���
				//��ȡ�ú�Լ�ĳֲ���ϸ��Ϣ�ṹ�� second; m_map[��]
				position_field* p_tdm = m_position_field_map[pInvestorPosition->InstrumentID];
				p_tdm->LongPosition = p_tdm->LongPosition + pInvestorPosition->Position;
				p_tdm->TodayLongPosition = p_tdm->TodayLongPosition + pInvestorPosition->TodayPosition;
				p_tdm->YdLongPosition = p_tdm->LongPosition - p_tdm->TodayLongPosition;
				p_tdm->LongCloseProfit = p_tdm->LongCloseProfit + pInvestorPosition->CloseProfit;
				p_tdm->LongPositionProfit = p_tdm->LongPositionProfit + pInvestorPosition->PositionProfit;
			}
			else if (pInvestorPosition->PosiDirection == '3')//�յ�
			{
				//��ֺͽ��һ�η���

				position_field* p_tdm = m_position_field_map[pInvestorPosition->InstrumentID];
				p_tdm->ShortPosition = p_tdm->ShortPosition + pInvestorPosition->Position;
				p_tdm->TodayShortPosition = p_tdm->TodayShortPosition + pInvestorPosition->TodayPosition;
				p_tdm->YdShortPosition = p_tdm->ShortPosition - p_tdm->TodayShortPosition;
				p_tdm->ShortCloseProfit = p_tdm->ShortCloseProfit + pInvestorPosition->CloseProfit;
				p_tdm->ShortPositionProfit = p_tdm->ShortPositionProfit + pInvestorPosition->PositionProfit;
			}

			if (bIsLast)
			{
				m_QryPosition_Once = false;
				m_Inst_Postion = m_Inst_Postion.substr(0, m_Inst_Postion.length() - 1);
				
				//m_pUserMDSpi_trade->setInstIdList_Position_MD(m_Inst_Postion);
				
				for (map<string, position_field*>::iterator iter = m_position_field_map.begin(); iter != m_position_field_map.end(); iter++)
				{
					cerr << "��Լ���룺" << iter->second->instId << endl
						<< "�൥�ֲ�����" << iter->second->LongPosition << endl
						<< "�յ��ֲ�����" << iter->second->ShortPosition << endl
						<< "�൥���ճֲ֣�" << iter->second->TodayLongPosition << endl
						<< "�൥���ճֲ֣�" << iter->second->YdLongPosition << endl
						<< "�յ����ճֲ֣�" << iter->second->TodayShortPosition << endl
						<< "�յ����ճֲ֣�" << iter->second->YdShortPosition << endl
						<< "�൥����ӯ����" << iter->second->LongPositionProfit << endl
						<< "�൥ƽ��ӯ����" << iter->second->LongCloseProfit << endl
						<< "�յ�����ӯ����" << iter->second->ShortPositionProfit << endl
						<< "�յ�ƽ��ӯ����" << iter->second->ShortCloseProfit << endl;

					//�˻�ƽ��ӯ��
					m_CloseProfit = m_CloseProfit + iter->second->LongCloseProfit + iter->second->ShortCloseProfit;
					//�˻�����ӯ��
					m_OpenProfit = m_OpenProfit + iter->second->LongPositionProfit + iter->second->ShortPositionProfit;
				}

				cerr << "�˻�����ӯ�� " << m_OpenProfit << endl;
				cerr << "�˻�ƽ��ӯ�� " << m_CloseProfit << endl;
			}//bisLast


		}
		cerr << "---------------��ѯͶ���ֲ߳����---------------" << endl;
		cerr << "��ѯ�ֲ��������״β�ѯ���к�Լ����" << endl;
	}
	else
	{
		if (m_QryPosition_Once == true)
			m_QryPosition_Once = false;
		cerr << "��ѯͶ���ֲֳ߳�����û�гֲ֣��״β�ѯ���к�Լ" << endl;
	}
	if (bIsLast)
	{
		//�߳�����3�룬��ctp��̨�г������Ӧʱ�䣬Ȼ���ٽ��в�ѯ����
		std::chrono::milliseconds sleepDuration(10 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryInstrumetAll();
	}
	
}


void TdSpi::ReqQryInvestorPosition(char * pInstrument)
{
	CThostFtdcQryInvestorPositionField req;//����req
	memset(&req, 0, sizeof(req));//��ʼ��Ϊ0

	
	strcpy(req.BrokerID, m_BrokerId.c_str());
	strcpy(req.InvestorID, m_UserId.c_str());
	
	//��Լ��д����ĺ�Լ����
	strcpy(req.InstrumentID, pInstrument);
	//���ý���api��ReqQryInvestorPosition
	int iResult = m_pUserTDApi_trade->ReqQryInvestorPosition(&req, GetNextRequestID());//reqΪ�գ������ѯ���к�Լ�ĳֲ�
	cerr << "--->>> �����ѯͶ���ֲ߳�: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}

void TdSpi::ReqQryInstrumetAll()
{
	CThostFtdcQryInstrumentField req;//����req
	memset(&req, 0, sizeof(req));//��ʼ��Ϊ0


	//���ý���api��ReqQryInstrument
	int iResult = m_pUserTDApi_trade->ReqQryInstrument(&req, GetNextRequestID());//req�ṹ��Ϊ0����ѯ���к�Լ
	cerr << "--->>> �����ѯ��Լ: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}

/// <summary>
/// ��ѯ�����ڻ���Լ
/// </summary>
void TdSpi::ReqQryInstrumet(char * pInstrument)
{
	CThostFtdcQryInstrumentField req;//����req
	memset(&req, 0, sizeof(req));//��ʼ��Ϊ0
	strcpy(req.InstrumentID, pInstrument);//��Լ��д����Ĵ��룬��ʾ��ѯ�ú�Լ����Ϣ
	//���ý���api��ReqQryInstrument
	int iResult = m_pUserTDApi_trade->ReqQryInstrument(&req, GetNextRequestID());//
	cerr << "--->>> �����ѯ��Լ: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}



void TdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr << "�����ѯ��Լ��Ӧ��OnRspQryInstrument" << ",pInstrument   " << pInstrument->InstrumentID << endl;
	if (!IsErrorRspInfo(pRspInfo) && pInstrument)
	{

		//�˻������к�Լ
		if (m_QryInstrument_Once == true)
		{
			m_Instrument_All = m_Instrument_All + pInstrument->InstrumentID + ",";

			//�������к�Լ��Ϣ��map
			CThostFtdcInstrumentField* pInstField = new CThostFtdcInstrumentField();
			memcpy(pInstField, pInstrument, sizeof(CThostFtdcInstrumentField));
			m_inst_field_map.insert(pair<string, CThostFtdcInstrumentField*>(pInstrument->InstrumentID, pInstField));

			//���Խ��׵ĺ�Լ
			if (strcmp(m_InstId.c_str(), pInstrument->InstrumentID) == 0)
			{
				cerr << "��Ӧ | ��Լ��" << pInstrument->InstrumentID
					<< "��Լ���ƣ�" << pInstrument->InstrumentName
					<< " ��Լ�ڽ��������룺" << pInstrument->ExchangeInstID
					<< " ��Ʒ���룺" << pInstrument->ProductID
					<< " ��Ʒ���ͣ�" << pInstrument->ProductClass
					<< " ��ͷ��֤���ʣ�" << pInstrument->LongMarginRatio
					<< " ��ͷ��֤���ʣ�" << pInstrument->ShortMarginRatio
					<< " ��Լ����������" << pInstrument->VolumeMultiple
					<< " ��С�䶯��λ��" << pInstrument->PriceTick
					<< " ���������룺" << pInstrument->ExchangeID
					<< " ������ݣ�" << pInstrument->DeliveryYear
					<< " �����£�" << pInstrument->DeliveryMonth
					<< " �����գ�" << pInstrument->CreateDate
					<< " �����գ�" << pInstrument->ExpireDate
					<< " �����գ�" << pInstrument->OpenDate
					<< " ��ʼ�����գ�" << pInstrument->StartDelivDate
					<< " ���������գ�" << pInstrument->EndDelivDate
					<< " ��Լ��������״̬��" << pInstrument->InstLifePhase
					<< " ��ǰ�Ƿ��ף�" << pInstrument->IsTrading << endl;
			}

			if (bIsLast)
			{
				m_QryInstrument_Once = false;
				m_Instrument_All = m_Instrument_All.substr(0, m_Instrument_All.length() - 1);
				cerr << "m_Instrument_All�Ĵ�С��" << m_Instrument_All.size() << endl;
				cerr << "map�Ĵ�С����Լ��������" << m_inst_field_map.size() << endl;

				//���ֲֺ�Լ��Ϣ���õ�mdspi
				//m_pUserMDSpi_trade->setInstIdList_Position_MD(m_Inst_Postion);


				//����Լ��Ϣ�ṹ���map���Ƶ�������
				g_strategy->set_instPostion_map_stgy(m_inst_field_map);
				cerr << "--------------------------�����Լ��Ϣmap������-----------------------" << endl;
				ShowInstMessage();
				//����ȫ�г���Լ����TD���У���Ҫ����ȫ�г���Լ����ʱ������
				m_pUserMDSpi_trade->set_InstIdList_All(m_Instrument_All);
				cerr << "TD��ʼ����ɣ�����MD" << endl;
				m_pUserMDApi_trade->Init();
			}
		}
	}
	else
	{
		m_QryInstrument_Once = false;
		cerr << "��ѯ��Լʧ��" << endl;
	}
	

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
	
	int nNextID = g_nRequestID++;

	return g_nRequestID;
}

void TdSpi::ShowInstMessage()
{
	//std::map<std::string, CThostFtdcInstrumentField*> m_inst_field_map;

	for (std::map<std::string, CThostFtdcInstrumentField*>::iterator iter = m_inst_field_map.begin(); iter != m_inst_field_map.end(); iter++)
	{
		CThostFtdcInstrumentField* pInstrument = iter->second;
		
		cerr << "��Ӧ | ��Լ��" << pInstrument->InstrumentID
			<< "��Լ���ƣ�" << pInstrument->InstrumentName
			<< " ��Լ�ڽ��������룺" << pInstrument->ExchangeInstID
			<< " ��Ʒ���룺" << pInstrument->ProductID
			<< " ��Ʒ���ͣ�" << pInstrument->ProductClass
			<< " ��ͷ��֤���ʣ�" << pInstrument->LongMarginRatio
			<< " ��ͷ��֤���ʣ�" << pInstrument->ShortMarginRatio
			<< " ��Լ����������" << pInstrument->VolumeMultiple
			<< " ��С�䶯��λ��" << pInstrument->PriceTick
			<< " ���������룺" << pInstrument->ExchangeID
			<< " ������ݣ�" << pInstrument->DeliveryYear
			<< " �����£�" << pInstrument->DeliveryMonth
			<< " �����գ�" << pInstrument->CreateDate
			<< " �����գ�" << pInstrument->ExpireDate
			<< " �����գ�" << pInstrument->OpenDate
			<< " ��ʼ�����գ�" << pInstrument->StartDelivDate
			<< " ���������գ�" << pInstrument->EndDelivDate
			<< " ��Լ��������״̬��" << pInstrument->InstLifePhase
			<< " ��ǰ�Ƿ��ף�" << pInstrument->IsTrading << endl;
	}
}

void TdSpi::PlaceOrder(const char* pszCode, const char* ExchangeID, int nDirection, int nOpenClose, int nVolume, double fPrice)
{
}
