#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/create/handler.hpp"
#include "handlers/delete/handler.hpp"
#include "handlers/get/handler.hpp"
#include "handlers/update/handler.hpp"
#include "middlewares/auth_middleware.hpp"
#include "middlewares/protected_handler_pipeline_builder.hpp"
#include "repositories/file_repository.hpp"

int main(int argc, char *argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                              .Append<userver::server::handlers::Ping>()
                              .Append<userver::components::TestsuiteSupport>()
                              .AppendComponentList(userver::clients::http::ComponentList())
                              .Append<userver::clients::dns::Component>()
                              .Append<userver::server::handlers::TestsControl>()
                              .Append<Handlers::CreateHandler>()
                              .Append<Handlers::GetHandler>()
                              .Append<Handlers::UpdateHandler>()
                              .Append<Handlers::DeleteHandler>()
                              .Append<Repositories::FileComponent>()
                              .Append<Auth::JwtCredentials>()
                              .Append<Middlewares::AuthMiddlewareFactory>()
                              .Append<Middlewares::ProtectedHandlerPipelineBuilder>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
