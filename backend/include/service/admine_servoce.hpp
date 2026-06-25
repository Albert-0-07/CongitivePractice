/**
 * @file    admine_servoce.hpp
 * @brief   管理员服务层 — AdminService 接口
 *
 * 负责:
 *   - 用户管理 CRUD (创建/编辑/删除/重置密码)
 *   - 角色实例创建与管理
 *   - 权限分配与查询
 *   - 操作日志查询 (综合所有管理员操作记录)
 *
 * 每项写操作自动记录操作日志 (操作人、IP)。
 */

#pragma once

#include "models/user.hpp"
#include "models/role.hpp"
#include "models/product.hpp"

#include <string>
#include <vector>
#include <memory>

namespace merchant {
namespace service {

using namespace merchant::model;

/**
 * @class AdminService
 * @brief 管理员后台业务逻辑接口
 */
class AdminService {
public:
    AdminService() = default;
    virtual ~AdminService() = default;

    /* ==========================================================
       用户管理 (User Management)
       ========================================================== */

    /// 分页查询所有用户 (管理员/商户/会员)
    virtual PageResult<User> getUsers(const UserQueryParams& params) = 0;

    /// 按 ID 获取用户
    virtual User getUserById(int64_t id) = 0;

    /**
     * @brief 创建新用户
     * @param data          用户数据
     * @param operatorName  操作人用户名 (用于日志)
     * @param operatorIp    操作人IP (用于日志)
     * @return 创建的用户 (含分配ID)
     */
    virtual User createUser(const User& data,
                            const std::string& operatorName,
                            const std::string& operatorIp) = 0;

    /**
     * @brief 更新用户信息
     */
    virtual User updateUser(int64_t id, const User& data,
                            const std::string& operatorName,
                            const std::string& operatorIp) = 0;

    /**
     * @brief 删除用户
     */
    virtual void deleteUser(int64_t id,
                            const std::string& operatorName,
                            const std::string& operatorIp) = 0;

    /**
     * @brief 重置用户密码为默认密码
     */
    virtual std::string resetUserPassword(int64_t id,
                                          const std::string& operatorName,
                                          const std::string& operatorIp) = 0;

    /* ==========================================================
       角色管理 (Role Management) — 创建角色实例
       ========================================================== */

    /// 获取所有角色及其权限
    virtual std::vector<Role> getRoles() = 0;

    /// 按 ID 获取角色
    virtual Role getRoleById(int64_t id) = 0;

    /**
     * @brief 创建新角色实例
     * @param name 角色名称
     * @param permissions 权限映射
     * @return 创建的角色
     */
    virtual Role createRole(const std::string& name,
                            const std::unordered_map<std::string, ModulePermissions>& permissions,
                            const std::string& operatorName,
                            const std::string& operatorIp) = 0;

    /**
     * @brief 更新角色权限
     */
    virtual Role updateRolePermissions(int64_t roleId,
                                       const std::unordered_map<std::string, ModulePermissions>& permissions,
                                       const std::string& operatorName,
                                       const std::string& operatorIp) = 0;

    /**
     * @brief 删除角色
     */
    virtual void deleteRole(int64_t id,
                            const std::string& operatorName,
                            const std::string& operatorIp) = 0;

    /* ==========================================================
       操作日志查询 (Operation Logs)
       ========================================================== */

    /// 分页查询操作日志 (超级管理员操作日志 + 所有后台操作)
    virtual PageResult<OperationLog> getOperationLogs(const LogQueryParams& params) = 0;

    /// 获取单条日志详情
    virtual OperationLog getOperationLogDetail(int64_t id) = 0;
};

/// 工厂
using AdminServicePtr = std::shared_ptr<AdminService>;

/// 创建 AdminService 实例
AdminServicePtr createAdminService();

} // namespace service
} // namespace merchant
