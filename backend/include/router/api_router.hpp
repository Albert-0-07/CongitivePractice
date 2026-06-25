/**
 * @file    api_router.hpp
 * @brief   API 路由注册器 — 统一注册所有 HTTP 路由
 *
 * 职责:
 *   - 将 URL 路径与方法映射到各 Controller 的回调
 *   - 由 main.cpp 在应用启动时调用 registerRoutes()
 *
 * 路由设计遵循 RESTful 风格, 所有商户接口统一前缀:
 *   /api/merchant/*
 *
 * 实际注册需配合具体HTTP框架 (Drogon/cpp-httplib/自定义)。
 * 本文件定义路由表结构, 具体绑定逻辑见 src/ 实现。
 */

#pragma once

#include "controller/merchant.hpp"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace merchant {
namespace router {

using namespace merchant::controller;

/**
 * @brief HTTP 方法枚举
 */
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE_  // 避免与 Windows DELETE 宏冲突
};

/**
 * @brief 单条路由定义
 */
struct RouteEntry {
    HttpMethod              method;     ///< HTTP 方法
    std::string             path;       ///< URL 路径 (可含 :param 占位符)
    std::string             description;///< 功能描述
};

/**
 * @class ApiRouter
 * @brief 路由注册器
 *
 * 使用方式 (伪代码):
 * @code
 *   auto router = std::make_shared<ApiRouter>(merchantCtrl);
 *   router->registerRoutes(httpServer);
 * @endcode
 */
class ApiRouter {
public:
    explicit ApiRouter(MerchantControllerPtr merchantController);
    virtual ~ApiRouter() = default;

    /**
     * @brief 返回所有商户路由定义 (供框架适配层使用)
     */
    virtual std::vector<RouteEntry> getMerchantRoutes() const;

    /**
     * @brief 注册所有路由到 HTTP 服务器
     * @param server  目标 HTTP 服务器实例 (依赖具体框架)
     *
     * 具体实现中需将每个路由条目的 path + method 映射到
     * MerchantController 对应的方法调用。
     */
    virtual void registerRoutes() { /* 默认空实现 */ }

    /**
     * @brief 根据路径分发请求到 MerchantController
     * @return JSON 响应字符串
     */
    std::string dispatch(HttpMethod method, const std::string& path,
                         const std::string& body, const std::string& queryParams);

protected:
    MerchantControllerPtr m_merchantCtrl;
};

using ApiRouterPtr = std::shared_ptr<ApiRouter>;

} // namespace router
} // namespace merchant
