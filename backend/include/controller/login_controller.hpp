/**
 * @file    login_controller.hpp
 * @brief   登录控制器 — HTTP 请求处理
 *
 * 路由:
 *   POST /api/login              — 管理员/商户/会员登录
 *   POST /api/logout             — 登出
 *   GET  /api/admin/login-logs   — 查询管理端登录日志
 *   GET  /api/admin/user-logs    — 查询用户端登录日志
 *   GET  /api/admin/operation-logs — 查询操作日志
 */

#pragma once

#include "service/login_servce.hpp"

#include <memory>
#include <string>

namespace merchant {
namespace controller {

using namespace merchant::service;

class LoginController {
public:
    explicit LoginController(LoginServicePtr service);
    virtual ~LoginController() = default;

    using Request  = std::string;
    using Response = std::string;

    /**
     * @brief 登录
     * POST /api/login
     * Body: { "username": "...", "password": "...", "ip": "...", "userAgent": "..." }
     */
    Response login(const Request& body);

    /**
     * @brief 登出
     * POST /api/logout
     * Body: { "token": "..." }
     */
    Response logout(const Request& body);

    /**
     * @brief 验证 Token
     * GET /api/check-token?token=...
     */
    Response checkToken(const Request& queryParams);

    /**
     * @brief 管理员登录日志
     * GET /api/admin/login-logs?page=1&pageSize=10&operator=...
     */
    Response getAdminLoginLogs(const Request& queryParams);

    /**
     * @brief 用户端登录日志
     * GET /api/admin/user-logs?page=1&pageSize=10
     */
    Response getUserLoginLogs(const Request& queryParams);

    /**
     * @brief 操作日志 (已有, 也可通过 LoginService 查询)
     */
    Response getOperationLogs(const Request& queryParams);

private:
    LoginServicePtr m_service;

    static Response success(const std::string& jsonBody);
    static Response error(const std::string& message, int code);
    static Response pageResult(const std::string& jsonArray, int64_t total, int32_t page, int32_t pageSize);
};

using LoginControllerPtr = std::shared_ptr<LoginController>;

} // namespace controller
} // namespace merchant
