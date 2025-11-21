#include "../common/dispatcher.hpp"
#include "requestor.hpp"
#include "rpc_caller.hpp"
#include "rpc_registry.hpp"
#include "rpc_topic.hpp"

namespace rpc
{
    namespace client
    {
        class RegistryClient
        {
        public:
            using ptr = std::shared_ptr<RegistryClient>;
            // 构造函数传入注册中心的地址信息，用于连接注册中心
            RegistryClient(const std::string &ip, int port)
                : _requestor(std::make_shared<Requestor>()),
                  _provider(std::make_shared<client::Provider>(_requestor)),
                  _dispatcher(std::make_shared<Dispatcher>())
            {
                auto rsp_cb = std::bind(&client::Requestor::onResponse, _requestor.get(),
                                        std::placeholders::_1, std::placeholders::_2);
                _dispatcher->registerHandler<BaseMessage>(MType::REQ_SERVICE, rsp_cb);
                auto msg_cb = std::bind(&Dispatcher::onMessage, _dispatcher.get(),
                                        std::placeholders::_1, std::placeholders::_2);
                _client = ClientFactory::create(ip, port);
                _client->setMessageCallback(msg_cb);
                _client->connect();
            }
            // 向外提供的服务注册接口
            bool registryMethdod(const std::string &method, const Address &host)
            {
                return _provider->registryMthod(_client->connection(), method, host);
            }

        private:
            Requestor::ptr _requestor;
            client::Provider::ptr _provider;
            Dispatcher::ptr _dispatcher;
            BaseClient::ptr _client;
        };

        class DiscoveryClient
        {
        public:
            using ptr = std::shared_ptr<DiscoveryClient>;
            // 构造函数传入注册中心的地址信息，用于连接注册中心
            DiscoveryClient(const std::string &ip, int port, const Discoverer::OfflineCallback &cb)
                : _requestor(std::make_shared<Requestor>()),
                  _discoverer(std::make_shared<client::Discoverer>(_requestor, cb)),
                  _dispatcher(std::make_shared<Dispatcher>())
            {
                auto rsp_cb = std::bind(&client::Requestor::onResponse, _requestor.get(),
                                        std::placeholders::_1, std::placeholders::_2);
                _dispatcher->registerHandler<BaseMessage>(MType::RSP_SERVICE, rsp_cb);

                auto req_cb = std::bind(&client::Discoverer::onServiceRequest, _discoverer.get(),
                                        std::placeholders::_1, std::placeholders::_2);
                _dispatcher->registerHandler<ServiceRequest>(MType::REQ_SERVICE, req_cb);
                auto msg_cb = std::bind(&Dispatcher::onMessage, _dispatcher.get(),
                                        std::placeholders::_1, std::placeholders::_2);
                _client = ClientFactory::create(ip, port);
                _client->setMessageCallback(msg_cb);
                _client->connect();
            }
            // 向外提供服务发现接口
            bool serviceDiscovery(const std::string &method, Address &host)
            {
                return _discoverer->serviceDiscovery(_client->connection(), method, host);
            }

        private:
            Requestor::ptr _requestor;
            client::Discoverer::ptr _discoverer;
            Dispatcher::ptr _dispatcher;
            BaseClient::ptr _client;
        };

        class RpcClient
        {
        public:
            using ptr = std::shared_ptr<RpcClient>;

        private:
            struct AddressHash{
                size_t operator()(const Address &host)const{
                    std::string addr=host.first+std::to_string(host.second);
                    return std::hash<std::string>{}(addr);
                }
            };
            bool _enableDiscovery;
            Requestor::ptr _requestor;
            RpcCaller::ptr _caller;
            Dispatcher::ptr _dispatcher;
            BaseClient::ptr _rpc_client;//用于未启用服务发现
            std::mutex _mutex;
            //<"127.0.0.1::8080",client1>
            std::unordered_map<Address,BaseClient::ptr,AddressHash> _rpc_clients;//用于启用服务发现的客户端连接池
        };
    }
}