#include "TdSpi.h"
#include<map>
#include<iostream>
#include<mutex>
#include"Strategy.h"
using namespace std;


extern std::map<std::string, std::string> accountConfig_map;//保存账户信息的map
//全局的互斥锁
extern std::mutex m_mutex;
extern Strategy* g_strategy;//策略类指针

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
	cerr << "OnFrontConnected ：" << endl;
	static const char* version = m_pUserTDApi_trade->GetApiVersion();
	cerr << "当前的CTP Api Version：" <<version<< endl;
	ReqAuthenticate();
}



int TdSpi::ReqAuthenticate()
{
	//virtual int ReqAuthenticate(CThostFtdcReqAuthenticateField * pReqAuthenticateField, int nRequestID) = 0;
	CThostFtdcReqAuthenticateField req;
	//初始化
	memset(&req, 0, sizeof(req));


	strcpy(req.AppID, m_AppId.c_str());
	//strcpy(req.AppID, "eiruoejladkfj");
	strcpy(req.AuthCode, m_AuthCode.c_str());
	//strcpy(req.AuthCode, "eiruoejladkfj");
	strcpy(req.BrokerID, m_BrokerId.c_str());
	//strcpy(req.BrokerID,"0000");
	//strcpy(req.UserID, m_UserId.c_str());
	strcpy(req.UserID, "");
	cerr << "请求认证的账户信息：" << endl << " appid: " << req.AppID << " authcode: " << req.AuthCode
		<< " brokerid: " << req.BrokerID << " userId: " << req.UserID << endl;
	
	return m_pUserTDApi_trade->ReqAuthenticate(&req,GetNextRequestID());
}
void TdSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo)
	{
		if (pRspInfo->ErrorID == 0)
		{
			cerr << "穿透测试验证成功！" << "ErrMsg:" << pRspInfo->ErrorMsg << endl;
			ReqUserLogin();
		}

		else
		{
			cerr << "穿透测试验证失败！" << " errorid:" << pRspInfo->ErrorID <<
				"ErrMsg:" << pRspInfo->ErrorMsg << endl;

			//<error id = "AUTH_FAILED" value = "63" prompt = "CTP:客户端认证失败" / >
		}
	}

}
int TdSpi::ReqUserLogin()
{
	cerr << "====ReqUserLogin====,用户登录中..." <<endl;
	//定义一个CThostFtdcReqUserLoginField
	CThostFtdcReqUserLoginField reqUserLogin;
	//初始化为0
	memset(&reqUserLogin, 0, sizeof(reqUserLogin));
	//复制brokerid,userid,passwd
	strcpy(reqUserLogin.BrokerID, m_BrokerId.c_str());
	//strcpy(reqUserLogin.BrokerID, "0000");
	//errorid:3，不合法的登录
	//strcpy(reqUserLogin.UserID, "00000000");
	strcpy(reqUserLogin.UserID, m_UserId.c_str());
	strcpy(reqUserLogin.Password, m_Passwd.c_str());

	//strcpy(reqUserLogin.Password, "00000000");
	
	//登录
	return m_pUserTDApi_trade->ReqUserLogin(&reqUserLogin, GetNextRequestID());

}

void TdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "登录请求回调OnRspUserLogin" << endl;
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
		m_nFrontID = pRspUserLogin->FrontID;
		m_nSessionID = pRspUserLogin->SessionID;
		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);

		sprintf_s(orderRef, sizeof(orderRef), "%d", ++nextOrderRef);

		cout << "前置编号:" << pRspUserLogin->FrontID << endl
			<< "会话编号" << pRspUserLogin->SessionID << endl
			<< "最大报单引用:" << pRspUserLogin->MaxOrderRef << endl
			<< "上期所时间：" << pRspUserLogin->SHFETime << endl
			<< "大商所时间：" << pRspUserLogin->DCETime << endl
			<< "郑商所时间：" << pRspUserLogin->CZCETime << endl
			<< "中金所时间：" << pRspUserLogin->FFEXTime << endl
			<< "能源所时间：" << pRspUserLogin->INETime << endl
			<< "交易日：" << m_pUserTDApi_trade->GetTradingDay() << endl;
		strcpy(m_cTradingDay, m_pUserTDApi_trade->GetTradingDay());//设置交易日期
		
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
	CThostFtdcQryOrderField  QryOrderField;//定义
	memset(&QryOrderField, 0, sizeof(CThostFtdcQryOrderField));//初始化为0
	//brokerid有误
	//strcpy(QryOrderField.BrokerID, "0000");
	strcpy(QryOrderField.BrokerID, m_BrokerId.c_str());
	//InvestorID有误
	//strcpy(QryOrderField.InvestorID, "666666");
	strcpy(QryOrderField.InvestorID, m_UserId.c_str());
	//调用api的ReqQryOrder
	m_pUserTDApi_trade->ReqQryOrder(&QryOrderField, GetNextRequestID());

}
void TdSpi::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "请求查询报单响应：OnRspQryOrder" <<",pOrder  "<<pOrder<< endl;
	if (!IsErrorRspInfo(pRspInfo) && pOrder)
	{
		cerr << "请求查询报单响应：前置编号FrontID：" << pOrder->FrontID << ",会话编号:" << pOrder->SessionID
			<< ",报单引用:  " << pOrder->OrderRef << endl;
			//所有合约
		if (m_QryOrder_Once == true)
		{
			CThostFtdcOrderField* order = new CThostFtdcOrderField();
			memcpy(order, pOrder, sizeof(CThostFtdcOrderField));
			orderList.push_back(order);

			//bIsLast是否是最后一笔回报
			if (bIsLast)
			{
				m_QryOrder_Once = false;
				cerr << "所有合约的报单次数" << orderList.size() << endl;
				cerr << "---------------打印报单开始---------------" << endl;
				for (vector<CThostFtdcOrderField*>::iterator iter = orderList.begin(); iter != orderList.end(); iter++)
				{
					cerr << "经纪公司代码：" << (*iter)->BrokerID << endl << "投资者代码：" << (*iter)->InvestorID << endl
						<< "用户代码：" << (*iter)->UserID << endl << "合约代码：" << (*iter)->InstrumentID << endl
						<< "买卖方向：" << (*iter)->Direction << endl << "组合开平标志：" << (*iter)->CombOffsetFlag << endl
						<< "价格：" << (*iter)->LimitPrice << endl << "数量：" << (*iter)->VolumeTotalOriginal << endl
						<< "报单引用：" << (*iter)->OrderRef << endl << "客户代码：" << (*iter)->ClientID << endl
						<< "报单状态：" << (*iter)->OrderStatus << endl << "委托时间：" << (*iter)->InsertTime << endl
						<< "报单编号：" << (*iter)->OrderStatus << endl << "交易日：" << (*iter)->TradingDay << endl
						<< "报单日期：" << (*iter)->InsertDate << endl;

				}
				cerr << "---------------打印报单完成---------------" << endl;
				cerr << "查询报单正常，将首次查询成交" << endl;
				

			}
		}

	}
	else//查询出错
	{
		m_QryOrder_Once = false;
		cerr << "查询报单出错，或没有成交，将首次查询成交" << endl;
		
	}
	if (bIsLast)
	{
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryTrade();
	}
}


void TdSpi::ReqQryTrade()
{
	CThostFtdcQryTradeField tdField;//定义
	memset(&tdField, 0, sizeof(tdField));//初始化

	strcpy(tdField.BrokerID, m_BrokerId.c_str());
	//strcpy(tdField.BrokerID,"0000");
	strcpy(tdField.InvestorID, m_UserId.c_str());
	//strcpy(tdField.InvestorID, "888888");
	//调用交易api的ReqQryTrade
	m_pUserTDApi_trade->ReqQryTrade(&tdField, GetNextRequestID());
}
void TdSpi::OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{

	cerr << "请求查询成交回报响应：OnRspQryTrade" << " pTrade " << pTrade << endl;

	if (!IsErrorRspInfo(pRspInfo) && pTrade)
	{
		//所有合约
		if (m_QryTrade_Once == true)
		{
			CThostFtdcTradeField* trade = new CThostFtdcTradeField();//创建trade
			memcpy(trade, pTrade, sizeof(CThostFtdcTradeField));//pTrade复制给trade
			tradeList.push_back(trade);

			if (bIsLast)
			{
				m_QryTrade_Once = false;
				cerr << "所有合约的成交次数" << tradeList.size() << endl;
				cerr << "---------------打印成交开始---------------" << endl;
				for (vector<CThostFtdcTradeField*>::iterator iter = tradeList.begin(); iter != tradeList.end(); iter++)
				{
					cerr << "经纪公司代码：" << (*iter)->BrokerID << endl << "投资者代码：" << (*iter)->InvestorID << endl
						<< "用户代码：" << (*iter)->UserID << endl << "成交编号：" << (*iter)->TradeID << endl
						<< "合约代码：" << (*iter)->InstrumentID << endl << "买卖方向：" << (*iter)->Direction << endl
						<< "组合开平标志：" << (*iter)->OffsetFlag << endl << "投机套保标志：" << (*iter)->HedgeFlag << endl
						<< "价格：" << (*iter)->Price << endl << "数量：" << (*iter)->Volume << endl
						<< "报单引用：" << (*iter)->OrderRef << endl << "本地报单编号：" << (*iter)->OrderLocalID << endl
						<< "成交时间：" << (*iter)->TradeTime << endl << "业务单元：" << (*iter)->BusinessUnit << endl
						<< "序号：" << (*iter)->SequenceNo << endl << "经纪公司下单序号：" << (*iter)->BrokerOrderSeq << endl
						<< "交易日：" << (*iter)->TradingDay << endl;

				}
				cerr << "---------------打印成交完成---------------" << endl;
				cerr << "查询报单正常，将首次查询持仓明细" << endl;
				
			}
			

		}

	}
	else//查询出错
	{
		m_QryOrder_Once = false;
		cerr << "查询报单出错，或没有成交，将首次查询成交" << endl;
		
	}
	if (bIsLast)
	{
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryInvestorPositionDetail();
	}

	
}
void TdSpi::ReqQryInvestorPositionDetail()
{
	CThostFtdcQryInvestorPositionDetailField pdField;//创建
	memset(&pdField, 0, sizeof(pdField));//初始化为0
	strcpy(pdField.BrokerID, m_BrokerId.c_str());
	//strcpy(pdField.BrokerID, "0000");
	//strcpy(pdField.InstrumentID, m_InstId.c_str());


	strcpy(pdField.InvestorID, m_UserId.c_str());

	//strcpy(pdField.InvestorID, "0000");
	//调用交易api的ReqQryInvestorPositionDetail
	m_pUserTDApi_trade->ReqQryInvestorPositionDetail(&pdField,GetNextRequestID());
}

void TdSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "请求查询投资者持仓明细回报响应：OnRspQryInvestorPositionDetail" << " pInvestorPositionDetail " << pField << endl;
	if (!IsErrorRspInfo(pRspInfo) && pField)
	{
		//所有合约
		if (m_QryDetail_Once == true)
		{
			//对于所有合约，只保存未平仓的，不保存已经平仓的
			//将程序启动前的持仓记录保存到未平仓容器tradeList_NotClosed_Long和tradeList_NotClosed_Short
			//使用结构体CThostFtdcTradeField，因为它有时间字段，而CThostFtdcInvestorPositionDetailField没有时间字段
			CThostFtdcTradeField* trade = new CThostFtdcTradeField();//创建CThostFtdcTradeField *
			
			strcpy(trade->InvestorID, pField->InvestorID);///投资者代码
			strcpy(trade->InstrumentID, pField->InstrumentID);///合约代码
			strcpy(trade->ExchangeID, pField->ExchangeID);///交易所代码
			trade->Direction = pField->Direction;//买卖方向
			trade->Price = pField->OpenPrice;//价格
			trade->Volume = pField->Volume;//数量
			strcpy(trade->TradeDate, pField->OpenDate);//成交日期
			strcpy(trade->TradeID, pField->TradeID);//*********成交编号********
			if (pField->Volume > 0)//筛选未平仓合约
			{
				if (trade->Direction == '0')//买入方向
					tradeList_NotClosed_Long.push_back(trade);
				else if (trade->Direction == '1')//卖出方向
					tradeList_NotClosed_Short.push_back(trade);
			}
			//收集持仓合约的代码
			bool find_instId = false;
			for (unsigned int i = 0; i < subscribe_inst_vec.size(); i++)
			{
				if (strcmp(subscribe_inst_vec[i].c_str(), trade->InstrumentID) == 0)//合约已存在，已订阅
				{
					find_instId = true;
					break;
				}
			}
			if (!find_instId)//合约未订阅过
			{
				cerr << "---------------------------------------该持仓合约未订阅过，加入订阅列表" << endl;
				subscribe_inst_vec.push_back(trade->InstrumentID);
			}

		}
		//输出所有合约的持仓明细，要在这边进行下一步的查询ReqQryTradingAccount()
		if (bIsLast)
		{
			m_QryDetail_Once = false;
			//持仓的合约
			string inst_holding;
			//
			for (unsigned int i = 0; i < subscribe_inst_vec.size(); i++)
				inst_holding = inst_holding + subscribe_inst_vec[i] + ",";
			//"IF2102,IF2103,"

			inst_holding = inst_holding.substr(0, inst_holding.length() - 1);//去掉最后的逗号，从位置0开始，选取length-1个字符
			//"IF2102,IF2103"

			cerr << "程序启动前的持仓列表:" << inst_holding << ",inst_holding.length()=" << inst_holding.length()
				<< ",subscribe_inst_vec.size()=" << subscribe_inst_vec.size() << endl;

			if (inst_holding.length() > 0)
				m_pUserMDSpi_trade->setInstIdList_Position_MD(inst_holding);//设置程序启动前的留仓，即需要订阅行情的合约

			//size代表笔数，而不是手数
			cerr << "账户所有合约未平仓单笔数（下单笔数，一笔可以对应多手）,多单:" << tradeList_NotClosed_Long.size()
				<< "空单：" << tradeList_NotClosed_Short.size() << endl;


			cerr << "-----------------------------------------未平仓多单明细打印start" << endl;
			for (vector<CThostFtdcTradeField*>::iterator iter = tradeList_NotClosed_Long.begin(); iter != tradeList_NotClosed_Long.end(); iter++)
			{
				cerr << "BrokerID:" << (*iter)->BrokerID << endl << "InvestorID:" << (*iter)->InvestorID << endl
					<< "InstrumentID:" << (*iter)->InstrumentID << endl << "Direction:" << (*iter)->Direction << endl
					<< "OpenPrice:" << (*iter)->Price << endl << "Volume:" << (*iter)->Volume << endl
					<< "TradeDate:" << (*iter)->TradeDate << endl << "TradeID:" << (*iter)->TradeID << endl;
			}

			cerr << "-----------------------------------------未平仓空单明细打印start" << endl;
			for (vector<CThostFtdcTradeField*>::iterator iter = tradeList_NotClosed_Short.begin(); iter != tradeList_NotClosed_Short.end(); iter++)
			{
				cerr << "BrokerID:" << (*iter)->BrokerID << endl << "InvestorID:" << (*iter)->InvestorID << endl
					<< "InstrumentID:" << (*iter)->InstrumentID << endl << "Direction:" << (*iter)->Direction << endl
					<< "OpenPrice:" << (*iter)->Price << endl << "Volume:" << (*iter)->Volume << endl
					<< "TradeDate:" << (*iter)->TradeDate << endl << "TradeID:" << (*iter)->TradeID << endl;
			}
			cerr << "---------------打印持仓明细完成---------------" << endl;
			cerr << "查询持仓明细正常，将首次查询账户资金信息" << endl;
		}
		
	}
	else
	{
		if (m_QryDetail_Once == true)
		{
			m_QryDetail_Once = false;
			cerr << "查询持仓明细出错，或没有持仓明细，将首次查询账户资金" << endl;
		}
	}
	if (bIsLast)
	{
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryTradingAccount();
	}
	
}

void TdSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;//创建req的结构体对象
	memset(&req, 0, sizeof(req));//初始化
	//错误的brokerID
	//strcpy(req.BrokerID, "8888");


	strcpy(req.BrokerID, m_BrokerId.c_str());


	strcpy(req.InvestorID, m_UserId.c_str());
	//strcpy(req.InvestorID, "666666");
	//调用交易api的ReqQryTradingAccount
	int iResult = m_pUserTDApi_trade->ReqQryTradingAccount(&req, GetNextRequestID());
	cerr << "--->>> 请求查询资金账户: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void TdSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "请求查询投资者资金账户回报响应：OnRspQryTradingAccount" << " pTradingAccount " << pTradingAccount << endl;
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount)
	{

		cerr << "投资者编号：" << pTradingAccount->AccountID
			<< "静态权益：期初权益" << pTradingAccount->PreBalance
			<< "动态权益：期货结算准备金" << pTradingAccount->Balance
			<< "可用资金：" << pTradingAccount->Available
			<< "可取资金：" << pTradingAccount->WithdrawQuota
			<< "当前保证金总额：" << pTradingAccount->CurrMargin
			<< "平仓盈亏：" << pTradingAccount->CloseProfit
			<< "持仓盈亏：" << pTradingAccount->PositionProfit
			<< "手续费：" << pTradingAccount->Commission
			<< "冻结保证金：" << pTradingAccount->FrozenCash
			<< endl;
		//所有合约
		if (m_QryTradingAccount_Once == true)
		{
			m_QryTradingAccount_Once = false;
		}

		cerr << "---------------打印资金账户明细完成---------------" << endl;
		cerr << "查询资金账户正常，将首次查询投资者持仓信息" << endl;
	}
	else
	{
		if (m_QryTradingAccount_Once == true)
		{
			m_QryTradingAccount_Once = false;
			cerr << "查询资金账户出错，将首次查询投资者持仓" << endl;
		}
	}
	//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
	std::chrono::milliseconds sleepDuration(3 * 1000);
	std::this_thread::sleep_for(sleepDuration);
	ReqQryInvestorPosition_All();
}


void TdSpi::ReqQryInvestorPosition_All()
{
	CThostFtdcQryInvestorPositionField req;//创建req
	memset(&req, 0, sizeof(req));//初始化为0

	//strcpy(req.BrokerID, "8888");
	strcpy(req.BrokerID, m_BrokerId.c_str());
	strcpy(req.InvestorID, m_UserId.c_str());
	//strcpy(req.InvestorID, "0000");
	//合约为空，则代表查询所有合约的持仓，这个和req为空是一样的
	strcpy(req.InstrumentID, m_InstId.c_str());
	//调用交易api的ReqQryInvestorPosition
	int iResult = m_pUserTDApi_trade->ReqQryInvestorPosition(&req, GetNextRequestID());//req为空，代表查询所有合约的持仓
	cerr << "--->>> 请求查询投资者持仓: " << ((iResult == 0) ? "成功" : "失败") << endl;
}
void TdSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition,
	CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	//cerr << "请求查询持仓响应：OnRspQryInvestorPosition " << ",pInvestorPosition  " << pInvestorPosition << endl;
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition)
	{

		//账户下所有合约
		if (m_QryPosition_Once == true)
		{
			cerr << "请求查询持仓响应：OnRspQryInvestorPosition " << " pInvestorPosition:  "
				<< pInvestorPosition << endl;//会包括已经平仓没有持仓的记录
			cerr << "响应  | 合约 " << pInvestorPosition->InstrumentID << endl
				<< " 持仓多空方向 " << pInvestorPosition->PosiDirection << endl//2多3空
			   // << " 映射后的方向 " << MapDirection(pInvestorPosition->PosiDirection-2,false) << endl
				<< " 总持仓 " << pInvestorPosition->Position << endl
				<< " 今日持仓 " << pInvestorPosition->TodayPosition << endl
				<< " 上日持仓 " << pInvestorPosition->YdPosition << endl
				<< " 保证金 " << pInvestorPosition->UseMargin << endl
				<< " 持仓成本 " << pInvestorPosition->PositionCost << endl
				<< " 开仓量 " << pInvestorPosition->OpenVolume << endl
				<< " 平仓量 " << pInvestorPosition->CloseVolume << endl
				<< " 持仓日期 " << pInvestorPosition->TradingDay << endl
				<< " 平仓盈亏（按昨结） " << pInvestorPosition->CloseProfitByDate << endl
				<< " 持仓盈亏 " << pInvestorPosition->PositionProfit << endl
				<< " 逐日盯市平仓盈亏（按昨结） " << pInvestorPosition->CloseProfitByDate << endl//快期中显示的是这个值
				<< " 逐笔对冲平仓盈亏（按开平合约） " << pInvestorPosition->CloseProfitByTrade << endl//在交易中比较有意义
				<< endl;


			//构造合约对应持仓明细信息的结构体map
			bool  find_trade_message_map = false;
			for (map<string, position_field*>::iterator iter = m_position_field_map.begin(); iter != m_position_field_map.end(); iter++)
			{
				if (strcmp((iter->first).c_str(), pInvestorPosition->InstrumentID) == 0)//合约已存在
				{
					find_trade_message_map = true;
					break;
				}
			}
			if (!find_trade_message_map)//合约不存在
			{
				cerr << "-----------------------没有这个合约，需要构造交易信息结构体" << endl;
				position_field* p_trade_message = new position_field();
				p_trade_message->instId = pInvestorPosition->InstrumentID;
				//构造持仓合约的string
				m_Inst_Postion += pInvestorPosition->InstrumentID ;
				m_Inst_Postion += ",";
				m_position_field_map.insert(pair<string, position_field*>(pInvestorPosition->InstrumentID, p_trade_message));
			}
			if (pInvestorPosition->PosiDirection == '2')//多单
			{
				//昨仓和今仓一次返回
				//获取该合约的持仓明细信息结构体 second; m_map[键]
				position_field* p_tdm = m_position_field_map[pInvestorPosition->InstrumentID];
				p_tdm->LongPosition = p_tdm->LongPosition + pInvestorPosition->Position;
				p_tdm->TodayLongPosition = p_tdm->TodayLongPosition + pInvestorPosition->TodayPosition;
				p_tdm->YdLongPosition = p_tdm->LongPosition - p_tdm->TodayLongPosition;
				p_tdm->LongCloseProfit = p_tdm->LongCloseProfit + pInvestorPosition->CloseProfit;
				p_tdm->LongPositionProfit = p_tdm->LongPositionProfit + pInvestorPosition->PositionProfit;
			}
			else if (pInvestorPosition->PosiDirection == '3')//空单
			{
				//昨仓和今仓一次返回

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
					cerr << "合约代码：" << iter->second->instId << endl
						<< "多单持仓量：" << iter->second->LongPosition << endl
						<< "空单持仓量：" << iter->second->ShortPosition << endl
						<< "多单今日持仓：" << iter->second->TodayLongPosition << endl
						<< "多单上日持仓：" << iter->second->YdLongPosition << endl
						<< "空单今日持仓：" << iter->second->TodayShortPosition << endl
						<< "空单上日持仓：" << iter->second->YdShortPosition << endl
						<< "多单浮动盈亏：" << iter->second->LongPositionProfit << endl
						<< "多单平仓盈亏：" << iter->second->LongCloseProfit << endl
						<< "空单浮动盈亏：" << iter->second->ShortPositionProfit << endl
						<< "空单平仓盈亏：" << iter->second->ShortCloseProfit << endl;

					//账户平仓盈亏
					m_CloseProfit = m_CloseProfit + iter->second->LongCloseProfit + iter->second->ShortCloseProfit;
					//账户浮动盈亏
					m_OpenProfit = m_OpenProfit + iter->second->LongPositionProfit + iter->second->ShortPositionProfit;
				}

				cerr << "账户浮动盈亏 " << m_OpenProfit << endl;
				cerr << "账户平仓盈亏 " << m_CloseProfit << endl;
			}//bisLast


		}
		cerr << "---------------查询投资者持仓完成---------------" << endl;
		cerr << "查询持仓正常，首次查询所有合约代码" << endl;
	}
	else
	{
		if (m_QryPosition_Once == true)
			m_QryPosition_Once = false;
		cerr << "查询投资者持仓出错，或没有持仓，首次查询所有合约" << endl;
	}
	if (bIsLast)
	{
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(10 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryInstrumetAll();
	}
	
}


void TdSpi::ReqQryInvestorPosition(char * pInstrument)
{
	CThostFtdcQryInvestorPositionField req;//创建req
	memset(&req, 0, sizeof(req));//初始化为0

	
	strcpy(req.BrokerID, m_BrokerId.c_str());
	strcpy(req.InvestorID, m_UserId.c_str());
	
	//合约填写具体的合约代码
	strcpy(req.InstrumentID, pInstrument);
	//调用交易api的ReqQryInvestorPosition
	int iResult = m_pUserTDApi_trade->ReqQryInvestorPosition(&req, GetNextRequestID());//req为空，代表查询所有合约的持仓
	cerr << "--->>> 请求查询投资者持仓: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void TdSpi::ReqQryInstrumetAll()
{
	CThostFtdcQryInstrumentField req;//创建req
	memset(&req, 0, sizeof(req));//初始化为0


	//调用交易api的ReqQryInstrument
	int iResult = m_pUserTDApi_trade->ReqQryInstrument(&req, GetNextRequestID());//req结构体为0，查询所有合约
	cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

/// <summary>
/// 查询单个期货合约
/// </summary>
void TdSpi::ReqQryInstrumet(char * pInstrument)
{
	CThostFtdcQryInstrumentField req;//创建req
	memset(&req, 0, sizeof(req));//初始化为0
	strcpy(req.InstrumentID, pInstrument);//合约填写具体的代码，表示查询该合约的信息
	//调用交易api的ReqQryInstrument
	int iResult = m_pUserTDApi_trade->ReqQryInstrument(&req, GetNextRequestID());//
	cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") << endl;
}



void TdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr << "请求查询合约响应：OnRspQryInstrument" << ",pInstrument   " << pInstrument->InstrumentID << endl;
	if (!IsErrorRspInfo(pRspInfo) && pInstrument)
	{

		//账户下所有合约
		if (m_QryInstrument_Once == true)
		{
			m_Instrument_All = m_Instrument_All + pInstrument->InstrumentID + ",";

			//保存所有合约信息到map
			CThostFtdcInstrumentField* pInstField = new CThostFtdcInstrumentField();
			memcpy(pInstField, pInstrument, sizeof(CThostFtdcInstrumentField));
			m_inst_field_map.insert(pair<string, CThostFtdcInstrumentField*>(pInstrument->InstrumentID, pInstField));

			//策略交易的合约
			if (strcmp(m_InstId.c_str(), pInstrument->InstrumentID) == 0)
			{
				cerr << "响应 | 合约：" << pInstrument->InstrumentID
					<< "合约名称：" << pInstrument->InstrumentName
					<< " 合约在交易所代码：" << pInstrument->ExchangeInstID
					<< " 产品代码：" << pInstrument->ProductID
					<< " 产品类型：" << pInstrument->ProductClass
					<< " 多头保证金率：" << pInstrument->LongMarginRatio
					<< " 空头保证金率：" << pInstrument->ShortMarginRatio
					<< " 合约数量乘数：" << pInstrument->VolumeMultiple
					<< " 最小变动价位：" << pInstrument->PriceTick
					<< " 交易所代码：" << pInstrument->ExchangeID
					<< " 交割年份：" << pInstrument->DeliveryYear
					<< " 交割月：" << pInstrument->DeliveryMonth
					<< " 创建日：" << pInstrument->CreateDate
					<< " 到期日：" << pInstrument->ExpireDate
					<< " 上市日：" << pInstrument->OpenDate
					<< " 开始交割日：" << pInstrument->StartDelivDate
					<< " 结束交割日：" << pInstrument->EndDelivDate
					<< " 合约生命周期状态：" << pInstrument->InstLifePhase
					<< " 当前是否交易：" << pInstrument->IsTrading << endl;
			}

			if (bIsLast)
			{
				m_QryInstrument_Once = false;
				m_Instrument_All = m_Instrument_All.substr(0, m_Instrument_All.length() - 1);
				cerr << "m_Instrument_All的大小：" << m_Instrument_All.size() << endl;
				cerr << "map的大小（合约数量）：" << m_inst_field_map.size() << endl;

				//将持仓合约信息设置到mdspi
				//m_pUserMDSpi_trade->setInstIdList_Position_MD(m_Inst_Postion);


				//将合约信息结构体的map复制到策略类
				g_strategy->set_instPostion_map_stgy(m_inst_field_map);
				cerr << "--------------------------输出合约信息map的内容-----------------------" << endl;
				ShowInstMessage();
				//保存全市场合约，在TD进行，需要订阅全市场合约行情时再运行
				m_pUserMDSpi_trade->set_InstIdList_All(m_Instrument_All);
				cerr << "TD初始化完成，启动MD" << endl;
				m_pUserMDApi_trade->Init();
			}
		}
	}
	else
	{
		m_QryInstrument_Once = false;
		cerr << "查询合约失败" << endl;
	}
	

}


void TdSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;//定义
	memset(&req, 0, sizeof(req));//初始化
	strcpy(req.BrokerID, m_BrokerId.c_str());
	//strcpy(req.BrokerID, "0000");
	strcpy(req.InvestorID, m_UserId.c_str());
	//strcpy(req.InvestorID, "000000");
	
	int iResult = m_pUserTDApi_trade->ReqSettlementInfoConfirm(&req, GetNextRequestID());
	cerr << "--->>> 投资者结算结果确认: " << ((iResult == 0) ? "成功" : "失败") << endl;
}


void TdSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	
	if (bIsLast && !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm)
	{
		cerr << "响应 | 结算单..." << pSettlementInfoConfirm->InvestorID
			<< "..." << pSettlementInfoConfirm->ConfirmDate << "," <<
			pSettlementInfoConfirm->ConfirmTime << "...确认" << endl << endl;

		cerr << "结算单确认正常，首次查询报单" << endl;
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		//Sleep(1000);
		ReqQryOrder();
	}
}

bool TdSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
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
	//给m_nNextRequestID加上互斥锁
	/*m_mutex.lock();
	int nNextID = m_nNextRequestID++;
	m_mutex.unlock();*/
	//1原理，在构造函数里面使用m_mutex.lock();
	//在析构的时候使用解锁m_mutex.unlock();
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
		
		cerr << "响应 | 合约：" << pInstrument->InstrumentID
			<< "合约名称：" << pInstrument->InstrumentName
			<< " 合约在交易所代码：" << pInstrument->ExchangeInstID
			<< " 产品代码：" << pInstrument->ProductID
			<< " 产品类型：" << pInstrument->ProductClass
			<< " 多头保证金率：" << pInstrument->LongMarginRatio
			<< " 空头保证金率：" << pInstrument->ShortMarginRatio
			<< " 合约数量乘数：" << pInstrument->VolumeMultiple
			<< " 最小变动价位：" << pInstrument->PriceTick
			<< " 交易所代码：" << pInstrument->ExchangeID
			<< " 交割年份：" << pInstrument->DeliveryYear
			<< " 交割月：" << pInstrument->DeliveryMonth
			<< " 创建日：" << pInstrument->CreateDate
			<< " 到期日：" << pInstrument->ExpireDate
			<< " 上市日：" << pInstrument->OpenDate
			<< " 开始交割日：" << pInstrument->StartDelivDate
			<< " 结束交割日：" << pInstrument->EndDelivDate
			<< " 合约生命周期状态：" << pInstrument->InstLifePhase
			<< " 当前是否交易：" << pInstrument->IsTrading << endl;
	}
}

void TdSpi::PlaceOrder(const char* pszCode, const char* ExchangeID, int nDirection, int nOpenClose, int nVolume, double fPrice)
{
}
