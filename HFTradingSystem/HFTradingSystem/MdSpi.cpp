#include"MdSpi.h"
#include<iostream>
#include<mutex>
#include<vector>


extern std::map<std::string, std::string> accountConfig_map;//�����˻���Ϣ��map


//ȫ�ֵĻ�����
extern std::mutex m_mutex;

//ȫ�ֵ�requestId
extern int g_nRequestID;

//ȫ�ֵĳֲֺ�Լ
extern std::vector<std::string> subscribe_inst_vec;


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
	if(m_InstIdList_all)
	delete[] m_InstIdList_all;

	if (m_InstIdList_Position_MD)
	delete[] m_InstIdList_Position_MD;

	if (loginField)
		delete loginField;
	

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
	cerr << "OnRspUserLogin �����½�ɹ�! " << endl;;
	cerr << "�����գ�" << mdapi->GetTradingDay() << endl;
	if (pRspInfo->ErrorID == 0) {
		cerr << "����ĵ�½�ɹ�," << "����IDΪ" << nRequestID << endl;
		/***************************************************************/
		cerr << "���Զ�������" << endl;

		////�����Լ�������б�
		InsertInstToSubVec(m_InstId);
		
		//��������
		//if (subscribe_inst_vec.size() > 0)
		//{
		//	//"IF2012,IF2101,IF2103"
		//	cerr << "��������ĺ�Լ������" << subscribe_inst_vec.size() << endl;
		//	cerr << "�гֲ֣���������  " << endl;
		//	SubscribeMarketData(subscribe_inst_vec);
		//
		//}

		
		//SubscribeMarketData("IF2012,IF2101,IF2103");

		//���ġ��������Խ��׵ĺ�Լ������
		//SubscribeMarketData(m_InstId);

		////���ġ��ֲֺ�Լ��������
		//if (m_InstIdList_Position_MD)
		//{
		//	//"IF2012,IF2101,IF2103"
		//	cerr << "m_InstIdList_Position_MD ��С��" << strlen(m_InstIdList_Position_MD) << "," << m_InstIdList_Position_MD << endl;
		//	cerr << "�гֲ֣���������  " << endl;
		//	SubscribeMarketData(m_InstIdList_Position_MD);//�����е�����6��/�룬û�гֲ־Ͳ��ö�������

		//	cerr << "�гֲ֣���������  " << endl;
		//	
		//	
		//												  
		//}


		//else
		//{
		//	cerr << "��ǰû�гֲ�" << endl;
		//}

		//SubscribeMarketData_All();
		SubscribeMarketData("IF2101");
		/*SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");*/
		//����������Ĭ�Ͻ�ֹ����
		cerr << endl << endl << endl << "����Ĭ�Ͻ�ֹ���֣���������ף�������ָ������֣�yxkc,��ֹ���֣�jzkc��" << endl;
		

	}
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "OnRspSubMarketData : Instrument:" <<pSpecificInstrument->InstrumentID<< endl;

	if (pRspInfo)
		cerr << "errorid"<<pRspInfo->ErrorID <<"ErrorMsg:"<< pRspInfo->ErrorMsg << endl;

	
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}
//char instIdList[]="IF2012,IF2101,IF2103"
void MdSpi::SubscribeMarketData(char* instIdList)
{
	//char*��vetor
	vector<char*>list;
	/*   strtok()�������ַ����ָ��һ����Ƭ�Ρ�����sָ�����ָ���ַ���������delim��Ϊ�ָ��ַ����а����������ַ���
		��strtok()�ڲ���s���ַ����з��ֲ���delim�а����ķָ��ַ�ʱ, ��Ὣ���ַ���Ϊ\0 �ַ���
		�ڵ�һ�ε���ʱ��strtok()����������s�ַ���������ĵ����򽫲���s���ó�NULL��
		ÿ�ε��óɹ��򷵻�ָ�򱻷ָ��Ƭ�ε�ָ�롣*/
	char* token = strtok(instIdList, ",");
	while (token != NULL)
	{
		list.push_back(token);
		token = strtok(NULL, ",");
	}
	unsigned int len = list.size();
	char** ppInstrument = new char* [len];
	for (unsigned i = 0; i < len; i++)
	{
		ppInstrument[i] = list[i];//ָ�븳ֵ��û���·����ڴ�ռ�
	}
	//��������api��SubscribeMarketData
	int nRet = mdapi->SubscribeMarketData(ppInstrument, len);
	cerr << "���������飺" << ((nRet == 0) ? "�ɹ�" : "ʧ��") << endl;

	
	delete[] ppInstrument;
}
//string instIdList="IF2012,IF2101,IF2103"
void MdSpi::SubscribeMarketData(std::string instIdList)
{
	
	int len = instIdList.size();
	//����len+1��char�Ŀռ䣬����ᱨ��
	char* pInst = new char[len+1];
	strcpy(pInst, instIdList.c_str());
	//��Ҫ���Ǽ��Ͻ�β��־
	pInst[len] = '\0';
	
	
	//SubscribeMarketData(char* instIdList)
	SubscribeMarketData(pInst);
	delete[]pInst;
}

void MdSpi::SubscribeMarketData(std::vector<std::string>& subscribeVec)
{

	int nLen = subscribeVec.size();

	if (nLen > 0)
	{
		char  * * ppInst  = new char * [nLen];
		for ( int i = 0; i<nLen; i++)
		{
			ppInst[i] = new char[31]{ 0 };
			//memcpy(ppInst[i], subscribeVec[i].c_str(), 31);
			strcpy(ppInst[i], subscribeVec[i].c_str());
			
		}

		int nResult=mdapi->SubscribeMarketData(ppInst, nLen);
		cerr << "�������� " << (nResult == 0 ? ("�ɹ�") : ("ʧ��")) << endl;

		for (int i = 0; i < nLen; i++)
			delete[] ppInst[i];

		delete[] ppInst;
	}
		
}
//char* m_InstIdList_all;
void MdSpi::SubscribeMarketData_All()
{
	SubscribeMarketData(m_InstIdList_all);
}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
	/*cout << "===========================================" << endl;
	cout << "�������" << endl;
	cout << " ������:" << pDepthMarketData->TradingDay << endl
		<< "��Լ����:" << pDepthMarketData->InstrumentID << endl
		<< "���¼�:" << pDepthMarketData->LastPrice << endl
		<< "�ϴν����:" << pDepthMarketData->PreSettlementPrice << endl
		<< "������:" << pDepthMarketData->PreClosePrice << endl
		<< "����:" << pDepthMarketData->Volume << endl
		<< "��ֲ���:" << pDepthMarketData->PreOpenInterest << endl
		<< "������ʱ��" << pDepthMarketData->UpdateTime << endl
		<< "�����º���" << pDepthMarketData->UpdateMillisec << endl
	<< "�����һ��" << pDepthMarketData->BidPrice1 << endl
	<< "������һ:" << pDepthMarketData->BidVolume1 << endl
	<< "������һ:" << pDepthMarketData->AskPrice1 << endl
	<< "������һ:" << pDepthMarketData->AskVolume1 << endl
	<< "�����̼�:" << pDepthMarketData->ClosePrice << endl
	<< "���վ���:" << pDepthMarketData->AveragePrice << endl
	<< "���ν���۸�:" << pDepthMarketData->SettlementPrice << endl
	<< "�ɽ����:" << pDepthMarketData->Turnover << endl
	<< "�ֲ���:" << pDepthMarketData->OpenInterest << endl;*/
}

//"IF2102,IF2103",string����ת��Ϊ��char *
void MdSpi::setInstIdList_Position_MD(std::string& inst_holding)
{
	int len = inst_holding.length();
	m_InstIdList_Position_MD = new char[len+1];

	memcpy(m_InstIdList_Position_MD, inst_holding.c_str(), len);
	m_InstIdList_Position_MD[len] = '\0';
}

void MdSpi::InsertInstToSubVec(char* Inst)
{
	//�����ݲ��뵽vector����  subscribe_inst_vec;
	bool findflag = false;
	int len = subscribe_inst_vec.size();
	for (int i = 0; i < len; i++)
	{
		if (strcmp(subscribe_inst_vec[i].c_str(), Inst) == 0)
		{
			findflag = true;
			break;
		}
	}
	//���û���ҵ��Ͳ��붩�ĺ�Լ��vector��
	if (!findflag)
		subscribe_inst_vec.push_back(Inst);


}
/// <summary>
/// ��string���͵����к�Լת��Ϊchar *����
/// </summary>
/// <param name="inst_all"></param>
///string inst_all="IF2012,IF2101,IF2103";
void MdSpi::set_InstIdList_All(std::string& inst_all)
{
	//�ַ����ĳ���
	int nLen = inst_all.size();
	//
	m_InstIdList_all = new char[nLen + 1];
	strcpy(m_InstIdList_all, inst_all.c_str());
	//ǧ���ס���Ͻ�����־����Ҫ����������
	m_InstIdList_all[nLen] = '\0';
	//delete [] m_InstIdList_all;д���������������
}

int MdSpi::GetNextRequestID()
{
	//��g_nRequestID���ϻ�����
		/*m_mutex.lock();
		int nNextID = g_nRequestID++;
		m_mutex.unlock();*/
		//1ԭ���ڹ��캯������ʹ��m_mutex.lock();
		//��������ʱ��ʹ�ý���m_mutex.unlock();
std:lock_guard<mutex> m_lock(m_mutex);

	int nNextID = g_nRequestID++;

	return g_nRequestID;
}


