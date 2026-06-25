/**
 * @file    admine_controller.hpp
 * @brief   管理员控制器 — HTTP 请求处理
 *
 * 路由:
 *   GET    /api/admin/users             — 分页查询用户
 *   GET    /api/admin/users/:id          — 获取单用户
 *   POST   /api/admin/users              — 创建用户
 *   PUT    /api/admin/users/:id          — 更新用户
 *   DELETE /api/admin/users/:id          — 删除用户
 *   POST   /api/admin/users/:id/reset-pwd — 重置密码
 *   GET    /api/admin/roles              — 获取所有角色
 *   POST   /api/admin/roles              — 创建角色
 *   PUT    /api/admin/roles/:id          — 更新角色权限
 *   DELETE /api/admin/roles/:id          — 删除角色
 *   GET    /api/admin/operation-logs     — 查询操作日志
 *   GET    /api/admin/operation-logs/:id  — 日志详情
 */

#pragma once

#include "service/admine_servoce.hpp"

#include <memory>
#include <string>

namespace merchant {
namespace controller {

using namespace merchant::service;

class AdminController {
public:
    explicit AdminController(AdminServicePtr service);
    virtual ~AdminController() = default;

    using Request  = std::string;
    using Response = std::string;

    /* ---- 用户管理 ---- */

    Response getUsers(const Request& queryParams);
    Response getUserById(int64_t id);
    Response createUser(const Request& body);
    Response updateUser(int64_t id, const Request& body);
    Response deleteUser(int64_t id, const Request& body);
    Response resetUserPassword(int64_t id, const Request& body);

    /* ---- 角色管理 ---- */

    Response getRoles();
    Response createRole(const Request& body);
    Response updateRolePermissions(int64_t id, const Request& body);
    Response deleteRole(int64_t id, const Request& body);

    /* ---- 操作日志 ---- */

    Response getOperationLogs(const Request& queryParams);
    Response getOperationLogDetail(int64_t id);

private:
    AdminServicePtr m_service;

    static Response success(const std::string& jsonBody);
    static Response error(const std::string& message, int code);
    static Response pageResult(const std::string& jsonArray, int64_t total, int32_t page, int32_t pageSize);
};

using AdminControllerPtr = std::shared_ptr<AdminController>;

} // namespace controller
} // namespace merchant
