/**
 * @file    login_servce.cpp
 * @brief   LoginService 实现 — 登录认证、Token 管理、日志记录
 */

#include "service/login_servce.hpp"
#include "models/log.hpp"

#include <mutex>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <unordered_map>

namespace merchant {
namespace service {

using namespace merchant::model;

/* ============================================================
   LoginServiceImpl
   ============================================================ */

class LoginServiceImpl : public LoginService {
public:
    LoginServiceImpl()
        : _nextId(2000)
    {
        seedLoginLogs();
    }

    ~LoginServiceImpl() override = default;

    /* ----------------------------------------------------------
       login
       ---------------------------------------------------------- */
    LoginResult login(const std::string& username,
                      const std::string& password,
                      const std::string& ip,
                      const std::string& userAgent) override
    {
        std::lock_guard<std::mutex> lock(_mutex);

        LoginResult result;

        // Mock account lookup
        auto it = std::find_if(_mockAccounts.begin(), _mockAccounts.end(),
            [&username](const User& u) { return u.username == username; });

        // Build operation log
        OperationLog opLog;
        opLog.id   = _nextId++;
        opLog.type = LogType::LOGIN;
        opLog.module = LogModule::SYSTEM;
        opLog.ip   = ip;
        opLog.userAgent = userAgent;
        opLog.time = nowTimeStr();
        opLog.operator_ = username;

        // Build login log entry
        LoginLogEntry entry;
        entry.id       = opLog.id;
        entry.username = username;
        entry.ip       = ip;
        entry.userAgent = userAgent;
        entry.loginTime = opLog.time;

        if (it == _mockAccounts.end()) {
            // User not found
            result.success      = false;
            result.errorMessage = "用户不存在";

            opLog.description = "用户 [" + username + "] 登录失败: 用户不存在";
            entry.success     = false;
            entry.failReason  = "用户不存在";
        }
        else if (it->password != password) {
            // Wrong password
            result.success      = false;
            result.errorMessage = "密码错误";

            opLog.description = "用户 [" + username + "] 登录失败: 密码错误";
            entry.success     = false;
            entry.failReason  = "密码错误";
            entry.role        = it->role;
            entry.realName    = it->realName;
        }
        else {
            // Success
            result.success = true;
            result.user    = *it;

            // Generate token
            result.token = "tok_" + it->role + "_" + std::to_string(std::time(nullptr));

            opLog.description = "用户 [" + username + "] 登录成功";
            entry.success     = true;
            entry.role        = it->role;
            entry.realName    = it->realName;

            // Store active token
            _activeTokens[result.token] = *it;

            // Update last login info on the mock account
            it->lastLogin    = opLog.time;
            it->lastLoginIp  = ip;
            it->loginCount  += 1;
        }

        // Record operation log
        _loginLogs.push_back(opLog);

        // Record login log in the appropriate vector
        if (!entry.role.empty() && entry.role != "会员") {
            // admin, merchant, operator roles go to admin logs
            _adminLoginLogs.push_back(entry);
        } else {
            _userLoginLogs.push_back(entry);
        }

        return result;
    }

    /* ----------------------------------------------------------
       validateToken
       ---------------------------------------------------------- */
    std::shared_ptr<User> validateToken(const std::string& token) override
    {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _activeTokens.find(token);
        if (it == _activeTokens.end()) {
            return nullptr;
        }
        return std::make_shared<User>(it->second);
    }

    /* ----------------------------------------------------------
       logout
       ---------------------------------------------------------- */
    void logout(const std::string& token) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _activeTokens.erase(token);
    }

    /* ----------------------------------------------------------
       getAdminLoginLogs
       ---------------------------------------------------------- */
    PageResult<LoginLogEntry> getAdminLoginLogs(const LogQueryParams& params) override
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // Apply filters
        std::vector<LoginLogEntry> filtered;
        for (const auto& entry : _adminLoginLogs) {
            if (params.operator_.has_value() && !params.operator_->empty()) {
                if (entry.username.find(params.operator_.value()) == std::string::npos) {
                    continue;
                }
            }
            if (params.type.has_value() && !params.type->empty()) {
                // type filter in login logs means success/failure
                if (params.type.value() == "成功" && !entry.success) continue;
                if (params.type.value() == "失败" && entry.success) continue;
            }
            if (params.startDate.has_value() && !params.startDate->empty()) {
                if (entry.loginTime < params.startDate.value()) continue;
            }
            if (params.endDate.has_value() && !params.endDate->empty()) {
                if (entry.loginTime > params.endDate.value()) continue;
            }
            filtered.push_back(entry);
        }

        // Paginate
        PageResult<LoginLogEntry> result;
        result.total    = static_cast<int64_t>(filtered.size());
        result.page     = params.page;
        result.pageSize = params.pageSize;

        int32_t start = (params.page - 1) * params.pageSize;
        int32_t end   = std::min(start + params.pageSize, static_cast<int32_t>(filtered.size()));
        if (start < static_cast<int32_t>(filtered.size())) {
            result.list.assign(filtered.begin() + start, filtered.begin() + end);
        }

        return result;
    }

    /* ----------------------------------------------------------
       getUserLoginLogs
       ---------------------------------------------------------- */
    PageResult<LoginLogEntry> getUserLoginLogs(const LogQueryParams& params) override
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // Apply filters
        std::vector<LoginLogEntry> filtered;
        for (const auto& entry : _userLoginLogs) {
            if (params.operator_.has_value() && !params.operator_->empty()) {
                if (entry.username.find(params.operator_.value()) == std::string::npos) {
                    continue;
                }
            }
            if (params.type.has_value() && !params.type->empty()) {
                if (params.type.value() == "成功" && !entry.success) continue;
                if (params.type.value() == "失败" && entry.success) continue;
            }
            if (params.startDate.has_value() && !params.startDate->empty()) {
                if (entry.loginTime < params.startDate.value()) continue;
            }
            if (params.endDate.has_value() && !params.endDate->empty()) {
                if (entry.loginTime > params.endDate.value()) continue;
            }
            filtered.push_back(entry);
        }

        // Paginate
        PageResult<LoginLogEntry> result;
        result.total    = static_cast<int64_t>(filtered.size());
        result.page     = params.page;
        result.pageSize = params.pageSize;

        int32_t start = (params.page - 1) * params.pageSize;
        int32_t end   = std::min(start + params.pageSize, static_cast<int32_t>(filtered.size()));
        if (start < static_cast<int32_t>(filtered.size())) {
            result.list.assign(filtered.begin() + start, filtered.begin() + end);
        }

        return result;
    }

    /* ----------------------------------------------------------
       getOperationLogs
       ---------------------------------------------------------- */
    PageResult<OperationLog> getOperationLogs(const LogQueryParams& params) override
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // Apply filters
        std::vector<OperationLog> filtered;
        for (const auto& log : _loginLogs) {
            if (params.operator_.has_value() && !params.operator_->empty()) {
                if (log.operator_.find(params.operator_.value()) == std::string::npos) {
                    continue;
                }
            }
            if (params.type.has_value() && !params.type->empty()) {
                if (log.type != params.type.value()) continue;
            }
            if (params.startDate.has_value() && !params.startDate->empty()) {
                if (log.time < params.startDate.value()) continue;
            }
            if (params.endDate.has_value() && !params.endDate->empty()) {
                if (log.time > params.endDate.value()) continue;
            }
            filtered.push_back(log);
        }

        // Paginate
        PageResult<OperationLog> result;
        result.total    = static_cast<int64_t>(filtered.size());
        result.page     = params.page;
        result.pageSize = params.pageSize;

        int32_t start = (params.page - 1) * params.pageSize;
        int32_t end   = std::min(start + params.pageSize, static_cast<int32_t>(filtered.size()));
        if (start < static_cast<int32_t>(filtered.size())) {
            result.list.assign(filtered.begin() + start, filtered.begin() + end);
        }

        return result;
    }

private:
    /* ----------------------------------------------------------
       Seed initial login logs
       ---------------------------------------------------------- */
    void seedLoginLogs()
    {
        // Seed operation logs
        _loginLogs = {
            makeLogEntry(2001, "admin",     LogType::LOGIN, LogModule::SYSTEM,
                         "用户 [admin] 登录成功", "192.168.1.100", "Mozilla/5.0 Chrome/120",
                         "2025-12-01 08:30:00"),
            makeLogEntry(2002, "merchant",  LogType::LOGIN, LogModule::SYSTEM,
                         "用户 [merchant] 登录成功", "192.168.1.101", "Mozilla/5.0 Edge/120",
                         "2025-12-01 09:15:00"),
            makeLogEntry(2003, "zhangsan",  LogType::LOGIN, LogModule::SYSTEM,
                         "用户 [zhangsan] 登录成功", "112.97.87.23", "Mozilla/5.0 Safari/17",
                         "2025-12-01 10:00:00"),
            makeLogEntry(2004, "operator",  LogType::LOGIN, LogModule::SYSTEM,
                         "用户 [operator] 登录成功", "192.168.1.102", "Mozilla/5.0 Firefox/121",
                         "2025-12-01 10:30:00"),
            makeLogEntry(2005, "admin",     LogType::LOGIN, LogModule::SYSTEM,
                         "用户 [admin] 登录成功", "192.168.1.100", "Mozilla/5.0 Chrome/120",
                         "2025-12-02 08:05:00"),
            makeLogEntry(2006, "lisi",      LogType::LOGIN, LogModule::SYSTEM,
                         "用户 [lisi] 登录成功", "58.213.47.12", "Mozilla/5.0 Chrome/119",
                         "2025-12-02 11:20:00"),
            makeLogEntry(2007, "unknown",   LogType::LOGIN, LogModule::SYSTEM,
                         "用户 [testuser] 登录失败: 用户不存在", "10.0.0.55", "curl/7.88",
                         "2025-12-02 14:45:00"),
            makeLogEntry(2008, "zhangsan",  LogType::LOGIN, LogModule::SYSTEM,
                         "用户 [zhangsan] 登录失败: 密码错误", "112.97.87.23", "Mozilla/5.0 Safari/17",
                         "2025-12-03 07:55:00"),
        };

        // Seed admin login logs
        _adminLoginLogs = {
            makeLoginLogEntry(2001, "admin",    "系统管理员", "超级管理员",
                              "2025-12-01 08:30:00", "192.168.1.100", "Mozilla/5.0 Chrome/120", true, ""),
            makeLoginLogEntry(2002, "merchant", "李明",       "商户",
                              "2025-12-01 09:15:00", "192.168.1.101", "Mozilla/5.0 Edge/120",  true, ""),
            makeLoginLogEntry(2004, "operator", "王芳",       "运营",
                              "2025-12-01 10:30:00", "192.168.1.102", "Mozilla/5.0 Firefox/121", true, ""),
            makeLoginLogEntry(2005, "admin",    "系统管理员", "超级管理员",
                              "2025-12-02 08:05:00", "192.168.1.100", "Mozilla/5.0 Chrome/120", true, ""),
        };

        // Seed user login logs
        _userLoginLogs = {
            makeLoginLogEntry(2003, "zhangsan", "张三", "会员",
                              "2025-12-01 10:00:00", "112.97.87.23", "Mozilla/5.0 Safari/17", true, ""),
            makeLoginLogEntry(2006, "lisi",     "李四", "会员",
                              "2025-12-02 11:20:00", "58.213.47.12", "Mozilla/5.0 Chrome/119", true, ""),
            makeLoginLogEntry(2008, "zhangsan", "张三", "会员",
                              "2025-12-03 07:55:00", "112.97.87.23", "Mozilla/5.0 Safari/17", false, "密码错误"),
        };
    }

    // Helper to create an OperationLog entry
    static OperationLog makeLogEntry(int64_t id,
                                     const std::string& operatorName,
                                     const std::string& type,
                                     const std::string& module,
                                     const std::string& description,
                                     const std::string& ip,
                                     const std::string& userAgent,
                                     const std::string& timeStr)
    {
        OperationLog log;
        log.id          = id;
        log.operator_   = operatorName;
        log.type        = type;
        log.module      = module;
        log.description = description;
        log.ip          = ip;
        log.userAgent   = userAgent;
        log.time        = timeStr;
        return log;
    }

    // Helper to create a LoginLogEntry
    static LoginLogEntry makeLoginLogEntry(int64_t id,
                                           const std::string& username,
                                           const std::string& realName,
                                           const std::string& role,
                                           const std::string& loginTime,
                                           const std::string& ip,
                                           const std::string& userAgent,
                                           bool success,
                                           const std::string& failReason)
    {
        LoginLogEntry entry;
        entry.id        = id;
        entry.username  = username;
        entry.realName  = realName;
        entry.role      = role;
        entry.loginTime = loginTime;
        entry.ip        = ip;
        entry.userAgent = userAgent;
        entry.success   = success;
        entry.failReason = failReason;
        return entry;
    }

    /* ----------------------------------------------------------
       Member data
       ---------------------------------------------------------- */
    std::mutex _mutex;
    int64_t    _nextId;

    // Mock accounts
    std::vector<User> _mockAccounts = []() {
        std::vector<User> accounts;

        User admin;
        admin.id       = 1;
        admin.username = "admin";
        admin.password = "admin123";
        admin.realName = "系统管理员";
        admin.role     = "超级管理员";
        admin.status   = 1;
        accounts.push_back(admin);

        User merchant;
        merchant.id       = 2;
        merchant.username = "merchant";
        merchant.password = "merchant123";
        merchant.realName = "李明";
        merchant.role     = "商户";
        merchant.status   = 1;
        accounts.push_back(merchant);

        User oper;
        oper.id       = 3;
        oper.username = "operator";
        oper.password = "oper123";
        oper.realName = "王芳";
        oper.role     = "运营";
        oper.status   = 1;
        accounts.push_back(oper);

        User zhangsan;
        zhangsan.id       = 4;
        zhangsan.username = "zhangsan";
        zhangsan.password = "123456";
        zhangsan.realName = "张三";
        zhangsan.role     = "会员";
        zhangsan.status   = 1;
        accounts.push_back(zhangsan);

        User lisi;
        lisi.id       = 5;
        lisi.username = "lisi";
        lisi.password = "123456";
        lisi.realName = "李四";
        lisi.role     = "会员";
        lisi.status   = 1;
        accounts.push_back(lisi);

        return accounts;
    }();

    // Active tokens: token -> User
    std::unordered_map<std::string, User> _activeTokens;

    // Operation logs (combined, for getOperationLogs)
    std::vector<OperationLog> _loginLogs;

    // Login logs by level
    std::vector<LoginLogEntry> _adminLoginLogs;  // admin / merchant / operator
    std::vector<LoginLogEntry> _userLoginLogs;   // members
};

/* ============================================================
   Factory function
   ============================================================ */

LoginServicePtr createLoginService()
{
    return std::make_shared<LoginServiceImpl>();
}

} // namespace service
} // namespace merchant
