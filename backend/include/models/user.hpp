/**
 * @file    user.hpp
 * @brief   用户/会员相关数据模型 — User (Member), Role
 *
 * 对应前端:
 *   - 会员管理 / 会员列表 (Members) — 商户端查看注册用户
 *   - 用户管理 (Users) — 管理员端用户CRUD
 *   - 权限管理 (Permissions) — 角色权限矩阵
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <unordered_map>

namespace merchant {
namespace model {

/* ============================================================
   会员/用户 (User / Member)
   对应前端: Members 列表 + 详情, Users CRUD
   ============================================================ */
struct User {
    int64_t     id          = 0;        ///< 用户ID (主键)
    std::string username;               ///< 用户名 (登录用)
    std::string password;               ///< 密码 (仅在创建/重置时使用, 查询不返回)
    std::string realName;               ///< 真实姓名
    std::string gender;                 ///< 性别: 男 / 女
    std::string phone;                  ///< 手机号
    std::string email;                  ///< 邮箱
    std::string avatar;                 ///< 头像 (emoji 或 URL)
    std::string role;                   ///< 角色: 超级管理员 / 运营 / 客服 / 商户 / 会员
    int32_t     status      = 1;        ///< 状态: 1=启用, 0=禁用
    std::string createdAt;              ///< 注册时间
    std::string lastLogin;              ///< 最后登录时间

    /// 会员统计字段 (仅 会员 角色有意义)
    std::vector<std::string> addresses; ///< 收货地址列表
    int32_t     orderCount  = 0;        ///< 历史订单数
    double      totalSpent  = 0.0;      ///< 总消费金额

    /// 管理员扩展字段
    std::string lastLoginIp;            ///< 最后登录IP (仅管理员用户管理)
    int32_t     loginCount  = 0;        ///< 登录次数

    User() = default;
};

/* ============================================================
   会员查询参数
   ============================================================ */
struct MemberQueryParams {
    std::optional<std::string> keyword;     ///< 用户名或手机号搜索
    std::optional<std::string> startDate;   ///< 注册时间开始
    std::optional<std::string> endDate;     ///< 注册时间结束
    int32_t                    page     = 1;
    int32_t                    pageSize = 10;

    MemberQueryParams() = default;
};

/* ============================================================
   用户查询参数 (管理员端)
   ============================================================ */
struct UserQueryParams {
    std::optional<std::string> username;    ///< 按用户名搜索
    std::optional<std::string> role;        ///< 按角色筛选
    std::optional<int32_t>     status;      ///< 按状态筛选
    int32_t                    page     = 1;
    int32_t                    pageSize = 10;

    UserQueryParams() = default;
};

/* ============================================================
   角色权限 (Role & Permissions)
   对应前端: 权限管理 — 权限矩阵
   ============================================================ */

/// 单个模块的权限位
struct ModulePermissions {
    bool view  = false;
    bool add   = false;
    bool edit  = false;
    bool del   = false;
    bool ship  = false;    ///< 仅订单模块: 发货权限
    bool voId  = false;    ///< 仅订单模块: 作废权限

    ModulePermissions() = default;
};

/// 角色定义
struct Role {
    int64_t     id   = 0;
    std::string name;

    /// 各模块权限: key = 模块名 (如 "products", "orders", "members", ...)
    std::unordered_map<std::string, ModulePermissions> permissions;

    Role() = default;
};

/* ============================================================
   操作日志 (Operation Log)
   对应前端: 操作日志 查看
   ============================================================ */
struct OperationLog {
    int64_t     id          = 0;
    std::string operator_;             ///< 操作人用户名
    std::string type;                   ///< 操作类型: 登录/新增/修改/删除/导出/作废
    std::string module;                 ///< 操作模块: 商品管理/订单管理/...
    std::string description;            ///< 操作描述
    std::string ip;                     ///< IP地址
    std::string time;                   ///< 操作时间
    std::string userAgent;              ///< 浏览器UA
    std::string requestParams;          ///< 请求参数 (JSON, 可选)

    OperationLog() = default;
};

/// 日志查询参数
struct LogQueryParams {
    std::optional<std::string> operator_;  ///< 按操作人搜索
    std::optional<std::string> type;       ///< 按操作类型筛选
    std::optional<std::string> startDate;
    std::optional<std::string> endDate;
    int32_t                    page     = 1;
    int32_t                    pageSize = 10;

    LogQueryParams() = default;
};

} // namespace model
} // namespace merchant
