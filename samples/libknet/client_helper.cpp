#include "client_helper.h"

// ����ֻ��ʹ�� asio ��ʹ��tcp�������ݷ��� gochart������ʽʵ�֡�

//std::shared_ptr<AsioClient> InitAsioClient(boost::asio::io_context &io_context, const char* ip, const char* port)
//{
//	tcp::resolver resolver(io_context);
//	auto endpoints = resolver.resolve(ip, port);
//	auto client = std::make_shared<AsioClient>(io_context, endpoints);
//	return client;
//}