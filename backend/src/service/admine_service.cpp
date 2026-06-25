/**
 * @file    admine_service.cpp
 * @brief   AdminService 实现 — 管理员后台业务逻辑 (内存mock)
 *
 * 提供:
 *   - 用户管理 CRUD
 *   - 角色管理 (创建/权限更新/删除)
 *   - 操作日志查询
 *
 * 每项写操作自动记录操作日志 (操作人、IP、时间戳)。
 */

#include "service/admine_servoce.hpp"
#include "models/log.hpp"
#include "models/role.hpp"
#include "models/product.hpp"

#include <mutex>
#include <algorithm>
#include <ctime>
#include <stdexcept>
#include <sstream>

namespace merchant {
namespace service {

using namespace merchant::model;

/* ============================================================
   内存数据存储 + 线程安全
   ============================================================ */

namespace {

std::mutex _mutex;

/* --- 用户 --- */
std::vector<User> _users;

/* --- 角色 --- */
std::vector<Role> _roles;

/* --- 操作日志 --- */
std::vector<OperationLog> _operationLogs;

/* --- ID 计数器 --- */
int64_t _nextUserId = 100;
int64_t _nextRoleId = 10;
int64_t _nextLogId  = 500;

/* ============================================================
   辅助: 记录操作日志
   ============================================================ */
void _addLog(const std::string& operatorName, const std::string& type,
             const std::string& module, const std::string& description,
             const std::string& ip) {
    OperationLog log;
    log.id          = _nextLogId++;
    log.operator_   = operatorName;
    log.type        = type;
    log.module      = module;
    log.description = description;
    log.ip          = ip;
    log.time        = nowTimeStr();
    _operationLogs.push_back(log);
}

/* ============================================================
   辅助: 过滤帮助函数
   ============================================================ */

/// 字符串包含检查 (不区分大小写, 用于简单搜索)
bool _contains(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    // 简化: 直接子串查找 (区分大小写, 与前端 mock 行为保持一致)
    return haystack.find(needle) != std::string::npos;
}

/* ============================================================
   初始化种子数据
   ============================================================ */

bool _seeded = false;

void _seedData() {
    if (_seeded) return;
    _seeded = true;

    /* ---- 用户种子数据 (匹配 frontend/js/utils/requests.js _users) ---- */

    // 1. admin — 系统管理员
    {
        User u;
        u.id          = 1;
        u.username    = "admin";
        u.password    = "123456";
        u.realName    = "系统管理员";
        u.role        = "超级管理员";
        u.phone       = "13900000001";
        u.email       = "admin@company.com";
        u.status      = 1;
        u.createdAt   = "2025-01-01 00:00";
        u.lastLogin   = "2025-06-24 09:15";
        u.lastLoginIp = "192.168.1.100";
        u.loginCount  = 128;
        _users.push_back(u);
    }

    // 2. merchant01 — 李明
    {
        User u;
        u.id          = 2;
        u.username    = "merchant01";
        u.password    = "123456";
        u.realName    = "李明";
        u.role        = "商户";
        u.phone       = "13900000002";
        u.email       = "liming@company.com";
        u.status      = 1;
        u.createdAt   = "2025-01-10 09:00";
        u.lastLogin   = "2025-06-24 08:30";
        u.lastLoginIp = "192.168.1.101";
        u.loginCount  = 86;
        _users.push_back(u);
    }

    // 3. operator01 — 王芳
    {
        User u;
        u.id          = 3;
        u.username    = "operator01";
        u.password    = "123456";
        u.realName    = "王芳";
        u.role        = "运营";
        u.phone       = "13900000003";
        u.email       = "wangfang@company.com";
        u.status      = 1;
        u.createdAt   = "2025-02-01 10:00";
        u.lastLogin   = "2025-06-23 16:45";
        u.lastLoginIp = "192.168.1.102";
        u.loginCount  = 54;
        _users.push_back(u);
    }

    // 4. service01 — 张伟
    {
        User u;
        u.id          = 4;
        u.username    = "service01";
        u.password    = "123456";
        u.realName    = "张伟";
        u.role        = "客服";
        u.phone       = "13900000004";
        u.email       = "zhangwei@company.com";
        u.status      = 1;
        u.createdAt   = "2025-02-15 14:00";
        u.lastLogin   = "2025-06-23 11:10";
        u.lastLoginIp = "192.168.1.104";
        u.loginCount  = 32;
        _users.push_back(u);
    }

    // 5. merchant02 — 刘洋
    {
        User u;
        u.id          = 5;
        u.username    = "merchant02";
        u.password    = "123456";
        u.realName    = "刘洋";
        u.role        = "商户";
        u.phone       = "13900000005";
        u.email       = "liuyang@company.com";
        u.status      = 0;
        u.createdAt   = "2025-03-01 08:30";
        u.lastLogin   = "2025-06-20 19:00";
        u.lastLoginIp = "192.168.1.105";
        u.loginCount  = 7;
        _users.push_back(u);
    }

    /* ---- 角色种子数据 (匹配 frontend/js/utils/requests.js _roles) ---- */

    // 1. 超级管理员 — 全权限
    {
        Role r = createSuperAdminRole();
        r.id   = 1;
        _roles.push_back(r);
    }

    // 2. 运营
    {
        Role r = createOperatorRole();
        r.id   = 2;
        _roles.push_back(r);
    }

    // 3. 客服
    {
        Role r = createServiceRole();
        r.id   = 3;
        _roles.push_back(r);
    }

    // 4. 商户
    {
        Role r = createMerchantRole();
        r.id   = 4;
        _roles.push_back(r);
    }

    /* ---- 操作日志种子数据 (匹配 frontend/js/utils/requests.js _logs) ---- */

    {
        OperationLog log;
        log.id          = 1;
        log.operator_   = "admin";
        log.type        = "登录";
        log.module      = "系统管理";
        log.description = "管理员admin登录系统";
        log.ip          = "192.168.1.100";
        log.time        = "2025-06-24 08:00:00";
        _operationLogs.push_back(log);
    }
    {
        OperationLog log;
        log.id          = 2;
        log.operator_   = "admin";
        log.type        = "新增";
        log.module      = "商品管理";
        log.description = "新增商品 [红富士苹果]";
        log.ip          = "192.168.1.100";
        log.time        = "2025-06-24 09:15:00";
        _operationLogs.push_back(log);
    }
    {
        OperationLog log;
        log.id          = 3;
        log.operator_   = "merchant01";
        log.type        = "修改";
        log.module      = "门店管理";
        log.description = "修改门店 [朝阳旗舰店] 营业时间";
        log.ip          = "192.168.1.101";
        log.time        = "2025-06-24 10:30:00";
        _operationLogs.push_back(log);
    }
    {
        OperationLog log;
        log.id          = 4;
        log.operator_   = "operator01";
        log.type        = "删除";
        log.module      = "营销管理";
        log.description = "删除促销活动 [春季特卖]";
        log.ip          = "192.168.1.102";
        log.time        = "2025-06-23 16:45:00";
        _operationLogs.push_back(log);
    }
    {
        OperationLog log;
        log.id          = 5;
        log.operator_   = "admin";
        log.type        = "导出";
        log.module      = "数据统计";
        log.description = "导出6月份销售报表";
        log.ip          = "192.168.1.100";
        log.time        = "2025-06-23 14:20:00";
        _operationLogs.push_back(log);
    }
    {
        OperationLog log;
        log.id          = 6;
        log.operator_   = "service01";
        log.type        = "修改";
        log.module      = "订单管理";
        log.description = "订单 [ORD20260602002] 标记为已发货";
        log.ip          = "192.168.1.104";
        log.time        = "2025-06-23 11:10:00";
        _operationLogs.push_back(log);
    }
    {
        OperationLog log;
        log.id          = 7;
        log.operator_   = "merchant01";
        log.type        = "新增";
        log.module      = "商品管理";
        log.description = "新增商品分类 [熟食烘焙]";
        log.ip          = "192.168.1.101";
        log.time        = "2025-06-22 09:00:00";
        _operationLogs.push_back(log);
    }
    {
        OperationLog log;
        log.id          = 8;
        log.operator_   = "operator01";
        log.type        = "作废";
        log.module      = "订单管理";
        log.description = "作废订单 [ORD20260605005]";
        log.ip          = "192.168.1.102";
        log.time        = "2025-06-21 15:30:00";
        _operationLogs.push_back(log);
    }
    {
        OperationLog log;
        log.id          = 9;
        log.operator_   = "admin";
        log.type        = "新增";
        log.module      = "用户管理";
        log.description = "新增用户 [service01 - 张伟]";
        log.ip          = "192.168.1.100";
        log.time        = "2025-06-20 10:00:00";
        _operationLogs.push_back(log);
    }
    {
        OperationLog log;
        log.id          = 10;
        log.operator_   = "merchant01";
        log.type        = "登录";
        log.module      = "系统管理";
        log.description = "商户merchant01登录系统";
        log.ip          = "192.168.1.101";
        log.time        = "2025-06-20 08:30:00";
        _operationLogs.push_back(log);
    }
}

} // anonymous namespace

/* ============================================================
   AdminServiceImpl — 实现类
   ============================================================ */

class AdminServiceImpl : public AdminService {
public:
    AdminServiceImpl() {
        std::lock_guard<std::mutex> lock(_mutex);
        _seedData();
    }

    ~AdminServiceImpl() override = default;

    /* ==========================================================
       用户管理 (User Management)
       ========================================================== */

    PageResult<User> getUsers(const UserQueryParams& params) override {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<User> filtered;

        for (const auto& u : _users) {
            if (params.username.has_value() && !params.username->empty()) {
                if (!_contains(u.username, params.username.value())) {
                    continue;
                }
            }
            if (params.role.has_value() && !params.role->empty()) {
                if (u.role != params.role.value()) {
                    continue;
                }
            }
            if (params.status.has_value()) {
                if (u.status != params.status.value()) {
                    continue;
                }
            }
            filtered.push_back(u);
        }

        int64_t total = static_cast<int64_t>(filtered.size());
        int32_t page     = params.page > 0 ? params.page : 1;
        int32_t pageSize = params.pageSize > 0 ? params.pageSize : 10;

        int32_t start = (page - 1) * pageSize;
        int32_t end   = start + pageSize;
        if (start < 0) start = 0;
        if (start > static_cast<int32_t>(filtered.size())) start = static_cast<int32_t>(filtered.size());
        if (end > static_cast<int32_t>(filtered.size())) end = static_cast<int32_t>(filtered.size());

        PageResult<User> result;
        result.total    = total;
        result.page     = page;
        result.pageSize = pageSize;
        result.list.assign(filtered.begin() + start, filtered.begin() + end);

        return result;
    }

    User getUserById(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);

        for (const auto& u : _users) {
            if (u.id == id) {
                return u;
            }
        }
        throw std::runtime_error("用户不存在: id=" + std::to_string(id));
    }

    User createUser(const User& data,
                    const std::string& operatorName,
                    const std::string& operatorIp) override {
        std::lock_guard<std::mutex> lock(_mutex);

        User user;
        user.id        = _nextUserId++;
        user.username  = data.username;
        user.password  = "123456";
        user.realName  = data.realName;
        user.role      = data.role;
        user.phone     = data.phone;
        user.email     = data.email;
        user.avatar    = data.avatar;
        user.status    = 1;
        user.createdAt = nowTimeStr();
        _users.push_back(user);

        std::string desc = "新增用户 [" + user.username + " - " + user.realName + "]";
        _addLog(operatorName, LogType::CREATE, LogModule::USERS, desc, operatorIp);

        return user;
    }

    User updateUser(int64_t id, const User& data,
                    const std::string& operatorName,
                    const std::string& operatorIp) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = std::find_if(_users.begin(), _users.end(),
                               [id](const User& u) { return u.id == id; });
        if (it == _users.end()) {
            throw std::runtime_error("用户不存在: id=" + std::to_string(id));
        }

        std::ostringstream changes;
        changes << "修改用户 [" << it->username << "]";

        if (!data.username.empty() && data.username != it->username) {
            changes << " 用户名: " << it->username << " -> " << data.username;
            it->username = data.username;
        }
        if (!data.realName.empty() && data.realName != it->realName) {
            changes << " 姓名: " << it->realName << " -> " << data.realName;
            it->realName = data.realName;
        }
        if (!data.role.empty() && data.role != it->role) {
            changes << " 角色: " << it->role << " -> " << data.role;
            it->role = data.role;
        }
        if (!data.phone.empty() && data.phone != it->phone) {
            changes << " 手机: " << it->phone << " -> " << data.phone;
            it->phone = data.phone;
        }
        if (!data.email.empty() && data.email != it->email) {
            changes << " 邮箱: " << it->email << " -> " << data.email;
            it->email = data.email;
        }
        if (it->status != data.status) {
            changes << " 状态: " << it->status << " -> " << data.status;
            it->status = data.status;
        }

        _addLog(operatorName, LogType::UPDATE, LogModule::USERS, changes.str(), operatorIp);

        return *it;
    }

    void deleteUser(int64_t id,
                    const std::string& operatorName,
                    const std::string& operatorIp) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = std::find_if(_users.begin(), _users.end(),
                               [id](const User& u) { return u.id == id; });
        if (it == _users.end()) {
            throw std::runtime_error("用户不存在: id=" + std::to_string(id));
        }

        std::string desc = "删除用户 [" + it->username + " - " + it->realName + "]";
        _users.erase(it);

        _addLog(operatorName, LogType::DELETE_, LogModule::USERS, desc, operatorIp);
    }

    std::string resetUserPassword(int64_t id,
                                   const std::string& operatorName,
                                   const std::string& operatorIp) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = std::find_if(_users.begin(), _users.end(),
                               [id](const User& u) { return u.id == id; });
        if (it == _users.end()) {
            throw std::runtime_error("用户不存在: id=" + std::to_string(id));
        }

        it->password = "123456";

        std::string desc = "重置用户 [" + it->username + "] 密码为默认密码";
        _addLog(operatorName, LogType::UPDATE, LogModule::USERS, desc, operatorIp);

        return "123456";
    }

    /* ==========================================================
       角色管理 (Role Management)
       ========================================================== */

    std::vector<Role> getRoles() override {
        std::lock_guard<std::mutex> lock(_mutex);
        return _roles;
    }

    Role getRoleById(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);

        for (const auto& r : _roles) {
            if (r.id == id) {
                return r;
            }
        }
        throw std::runtime_error("角色不存在: id=" + std::to_string(id));
    }

    Role createRole(const std::string& name,
                    const std::unordered_map<std::string, ModulePermissions>& permissions,
                    const std::string& operatorName,
                    const std::string& operatorIp) override {
        std::lock_guard<std::mutex> lock(_mutex);

        Role role;
        role.id          = _nextRoleId++;
        role.name        = name;
        role.permissions = permissions;
        _roles.push_back(role);

        std::string desc = "创建角色 [" + name + "]";
        _addLog(operatorName, LogType::CREATE, LogModule::PERMISSIONS, desc, operatorIp);

        return role;
    }

    Role updateRolePermissions(int64_t roleId,
                                const std::unordered_map<std::string, ModulePermissions>& permissions,
                                const std::string& operatorName,
                                const std::string& operatorIp) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = std::find_if(_roles.begin(), _roles.end(),
                               [roleId](const Role& r) { return r.id == roleId; });
        if (it == _roles.end()) {
            throw std::runtime_error("角色不存在: id=" + std::to_string(roleId));
        }

        it->permissions = permissions;

        std::string desc = "更新角色 [" + it->name + "] 权限";
        _addLog(operatorName, LogType::UPDATE, LogModule::PERMISSIONS, desc, operatorIp);

        return *it;
    }

    void deleteRole(int64_t id,
                     const std::string& operatorName,
                     const std::string& operatorIp) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = std::find_if(_roles.begin(), _roles.end(),
                               [id](const Role& r) { return r.id == id; });
        if (it == _roles.end()) {
            throw std::runtime_error("角色不存在: id=" + std::to_string(id));
        }

        std::string desc = "删除角色 [" + it->name + "]";
        _roles.erase(it);

        _addLog(operatorName, LogType::DELETE_, LogModule::PERMISSIONS, desc, operatorIp);
    }

    /* ==========================================================
       操作日志查询 (Operation Logs)
       ========================================================== */

    PageResult<OperationLog> getOperationLogs(const LogQueryParams& params) override {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<OperationLog> filtered;

        for (const auto& log : _operationLogs) {
            if (params.operator_.has_value() && !params.operator_->empty()) {
                if (!_contains(log.operator_, params.operator_.value())) {
                    continue;
                }
            }
            if (params.type.has_value() && !params.type->empty()) {
                if (log.type != params.type.value()) {
                    continue;
                }
            }
            if (params.startDate.has_value() && !params.startDate->empty()) {
                if (log.time < params.startDate.value()) {
                    continue;
                }
            }
            if (params.endDate.has_value() && !params.endDate->empty()) {
                if (log.time > params.endDate.value()) {
                    continue;
                }
            }
            filtered.push_back(log);
        }

        // 按 id 降序排列 (最新在前, 与前端 mock 一致)
        std::sort(filtered.begin(), filtered.end(),
                  [](const OperationLog& a, const OperationLog& b) {
                      return a.id > b.id;
                  });

        int64_t total = static_cast<int64_t>(filtered.size());
        int32_t page     = params.page > 0 ? params.page : 1;
        int32_t pageSize = params.pageSize > 0 ? params.pageSize : 10;

        int32_t start = (page - 1) * pageSize;
        int32_t end   = start + pageSize;
        if (start < 0) start = 0;
        if (start > static_cast<int32_t>(filtered.size())) start = static_cast<int32_t>(filtered.size());
        if (end > static_cast<int32_t>(filtered.size())) end = static_cast<int32_t>(filtered.size());

        PageResult<OperationLog> result;
        result.total    = total;
        result.page     = page;
        result.pageSize = pageSize;
        result.list.assign(filtered.begin() + start, filtered.begin() + end);

        return result;
    }

    OperationLog getOperationLogDetail(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);

        for (const auto& log : _operationLogs) {
            if (log.id == id) {
                return log;
            }
        }
        throw std::runtime_error("操作日志不存在: id=" + std::to_string(id));
    }
};

/* ============================================================
   工厂函数
   ============================================================ */

AdminServicePtr createAdminService() {
    return std::make_shared<AdminServiceImpl>();
}

} // namespace service
} // namespace merchant
