/**
 * @file    login_servce.hpp
 * @brief   登录认证服务层 — LoginService 接口
 *
 * 负责:
 *   - 商户/管理员登录验证
 *   - 用户端(会员)登录验证
 *   - 登录日志记录 (记录每次登录的时间、IP、User-Agent)
 *   - Token 生成与校验
 */

#pragma once

#include "models/user.hpp"
#include "models/product.hpp"

#include <string>
#include <vector>
#include <memory>

namespace merchant {
namespace service {

using namespace merchant::model;

/**
 * @brief 登录结果
 */
struct LoginResult {
    bool        success     = false;
    std::string token;
    User        user;
    std::string errorMessage;
};

/**
 * @brief 登录日志条目 (前端展示用)
 */
struct LoginLogEntry {
    int64_t     id          = 0;
    std::string username;       ///< 登录用户名
    std::string realName;       ///< 真实姓名
    std::string role;           ///< 角色
    std::string loginTime;      ///< 登录时间
    std::string ip;             ///< 登录IP
    std::string userAgent;      ///< 浏览器UA
    bool        success     = true;  ///< 是否登录成功
    std::string failReason;     ///< 失败原因 (仅失败时)
};

/**
 * @class LoginService
 * @brief 登录认证业务逻辑接口
 */
class LoginService {
public:
    LoginService() = default;
    virtual ~LoginService() = default;

    /**
     * @brief 用户登录验证
     * @param username  用户名
     * @param password  密码
     * @param ip        客户端IP (用于日志)
     * @param userAgent 浏览器UA (用于日志)
     * @return 登录结果
     *
     * 验证成功后自动记录登录日志。
     * 验证失败也记录日志(失败原因)。
     */
    virtual LoginResult login(const std::string& username,
                              const std::string& password,
                              const std::string& ip,
                              const std::string& userAgent) = 0;

    /**
     * @brief 校验 Token 有效性
     * @return 如果有效返回对应用户, 否则返回空
     */
    virtual std::shared_ptr<User> validateToken(const std::string& token) = 0;

    /**
     * @brief 登出 (使 Token 失效)
     */
    virtual void logout(const std::string& token) = 0;

    /**
     * @brief 查询登录日志 (管理员/商户登录记录)
     * @param params 查询参数: 可按用户名筛选
     * @return 分页的登录日志列表
     */
    virtual PageResult<LoginLogEntry> getAdminLoginLogs(const LogQueryParams& params) = 0;

    /**
     * @brief 查询用户端登录日志 (会员登录记录)
     * @param params 查询参数
     * @return 分页的登录日志列表
     */
    virtual PageResult<LoginLogEntry> getUserLoginLogs(const LogQueryParams& params) = 0;

    /**
     * @brief 查询所有操作日志 (通用, 供管理员查看)
     */
    virtual PageResult<OperationLog> getOperationLogs(const LogQueryParams& params) = 0;
};

/// 工厂
using LoginServicePtr = std::shared_ptr<LoginService>;

/// 创建 LoginService 实例
LoginServicePtr createLoginService();

} // namespace service
} // namespace merchant
