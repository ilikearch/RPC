#include "../common/dispatcher.hpp"
#include "../client/rpc_client.hpp"
#include "rpc_router.hpp"
#include "rpc_registry.hpp"
// #include "rpc_topic.hpp"

namespace rpc
{
    namespace server
    {
        class RegistryServer
        {
        public:
            using ptr = std::shared_ptr<RegistryServer>;

        private:
            PDManager::ptr _pd_manager;
            Dispatcher::ptr _dispather;
            BaseServer::ptr _server;
        };
    }

}
