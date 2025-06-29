#pragma once
#include <memory>
#include <string>
#include "core/domain/types.h"
#include "core/ports/task.h"

class IHandler;

class IRouter {
public:
    virtual ~IRouter() = default;

    using handler_ptr = std::shared_ptr<IHandler>;
    virtual void add_route(HTTP::method method,
                            const std::string& endpoint,
                            handler_ptr handler) = 0;
    virtual async_task<void> route(const Request& req, Response& res) = 0;
};