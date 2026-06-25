/**
 * @file    main.cpp
 * @brief   应用入口 — HTTP 服务器, 静态文件, API 路由
 *
 * 商户管理系统后端服务:
 *   1. 加载配置
 *   2. 初始化 Service / Controller / Router
 *   3. 注册 API 路由 + 前端静态文件
 *   4. 启动 HTTP 监听 (cpp-httplib)
 */

#include <iostream>
#include <memory>
#include <csignal>
#include <string>
#include <fstream>

#include "httplib.h"
#include "config/config.hpp"
#include "service/mechant.hpp"
#include "service/login_servce.hpp"
#include "service/admine_servoce.hpp"
#include "controller/merchant.hpp"
#include "controller/login_controller.hpp"
#include "controller/admine_controller.hpp"
#include "router/api_router.hpp"

using namespace merchant;

// 全局
static volatile bool         g_running   = true;
static httplib::Server*      g_server    = nullptr;

static void signalHandler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        std::cout << "\n[INFO] 收到退出信号, 正在关闭..." << std::endl;
        g_running = false;
        if (g_server) g_server->stop();
    }
}

// 简易 URL 解码
static std::string urlDecode(const std::string& src) {
    std::string ret;
    for (size_t i = 0; i < src.size(); ++i) {
        if (src[i] == '%' && i + 2 < src.size()) {
            int hi = src[i+1], lo = src[i+2];
            auto hex = [](char c) -> int {
                if (c>='0'&&c<='9') return c-'0';
                if (c>='A'&&c<='F') return c-'A'+10;
                if (c>='a'&&c<='f') return c-'a'+10;
                return 0;
            };
            ret += static_cast<char>((hex(hi) << 4) | hex(lo));
            i += 2;
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}

// 将查询参数 map 转为 &key=value 字符串
static std::string toQueryString(const httplib::Params& params) {
    std::string qs;
    for (auto& p : params) {
        if (!qs.empty()) qs += "&";
        qs += p.first + "=" + p.second;
    }
    return qs;
}

// 为所有 API 响应添加 CORS 头 (开发阶段, 方便前后端分离调试)
static void addCors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin",  "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    // ---- 配置 ----
    auto& cfg = config::AppConfig::instance();
    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--help" || arg1 == "-h") {
            std::cout << "商户管理系统 — Merchant Management Server\n"
                      << "用法: " << argv[0] << " [config.json] [选项]\n"
                      << "选项:\n"
                      << "  --port N    监听端口 (默认 8080)\n"
                      << "  --static P  静态文件路径 (默认 ../frontend)\n"
                      << "  --help      显示帮助\n\n"
                      << "测试账号: admin/admin123  merchant/merchant123\n";
            return 0;
        }
        if (arg1.rfind(".json") != std::string::npos)
            cfg.loadFromFile(arg1);
    }
    cfg.applyCommandLine(argc, argv);

    std::string staticPath = cfg.server.staticPath;
    uint16_t    port       = cfg.server.port;
    std::string host       = cfg.server.host;

    std::cout << "╔══════════════════════════════════════╗\n"
              << "║   🏪 商户管理系统 v1.0.0             ║\n"
              << "╠══════════════════════════════════════╣\n"
              << "║  HTTP:  " << host << ":" << port
              << std::string(24 - std::to_string(port).size(), ' ') << "║\n"
              << "║  静态:  " << staticPath
              << std::string(22 - staticPath.size(), ' ') << "║\n"
              << "╠══════════════════════════════════════╣\n"
              << "║  测试账号:                           ║\n"
              << "║  管理员: admin / admin123            ║\n"
              << "║  商户:   merchant / merchant123      ║\n"
              << "╚══════════════════════════════════════╝\n";

    // ---- 实例化 Service 层 ----
    auto merchantService = service::createMerchantService();
    auto loginService    = service::createLoginService();
    auto adminService    = service::createAdminService();

    if (!merchantService || !loginService || !adminService) {
        std::cerr << "[FATAL] 服务初始化失败" << std::endl;
        return 1;
    }

    // ---- 实例化 Controller 层 ----
    auto merchantCtrl = std::make_shared<controller::MerchantController>(merchantService);
    auto loginCtrl    = std::make_shared<controller::LoginController>(loginService);
    auto adminCtrl    = std::make_shared<controller::AdminController>(adminService);

    // ---- 实例化 Router ----
    auto apiRouter = std::make_shared<router::ApiRouter>(merchantCtrl);

    // ---- HTTP 服务器 ----
    httplib::Server svr;
    g_server = &svr;

    // CORS 预检
    svr.Options(R"(/api/.*)", [](const httplib::Request&, httplib::Response& res) {
        addCors(res);
        res.status = 204;
    });

    /* ============================================================
       API 路由 — Merchant
       每一条路由提取路径参数和 query string,
       构造 Request 传给 Controller, 将 Response 返回给客户端。
       ============================================================ */

    // 对需要 body 的 POST/PUT, 直接传 req.body
    // 对 GET, 传 query string
    // id 参数从 URL 路径中提取

    auto getQ = [](const httplib::Request& r) { return toQueryString(r.params); };

    // ---- 商品分类 ----
    svr.Get("/api/merchant/categories", [&](const httplib::Request&, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getCategories(), "application/json");
    });
    svr.Post("/api/merchant/categories", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->createCategory(req.body), "application/json");
    });
    svr.Put("/api/merchant/categories/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->updateCategory(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Delete("/api/merchant/categories/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->deleteCategory(std::stoll(req.matches[1])), "application/json");
    });

    // ---- 商品信息 ----
    svr.Get("/api/merchant/products", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getProducts(getQ(req)), "application/json");
    });
    svr.Post("/api/merchant/products", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->createProduct(req.body), "application/json");
    });
    svr.Put("/api/merchant/products/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->updateProduct(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Delete("/api/merchant/products/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->deleteProduct(std::stoll(req.matches[1])), "application/json");
    });

    // ---- 促销管理 ----
    svr.Get("/api/merchant/promotions", [&](const httplib::Request&, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getPromotions(), "application/json");
    });
    svr.Post("/api/merchant/promotions", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->createPromotion(req.body), "application/json");
    });
    svr.Put("/api/merchant/promotions/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->updatePromotion(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Delete("/api/merchant/promotions/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->deletePromotion(std::stoll(req.matches[1])), "application/json");
    });
    svr.Post("/api/merchant/promotions/(\\d+)/bind", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->bindProductsToPromotion(std::stoll(req.matches[1]), req.body), "application/json");
    });

    // ---- 门店管理 ----
    svr.Get("/api/merchant/stores", [&](const httplib::Request&, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getStores(), "application/json");
    });
    svr.Post("/api/merchant/stores", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->createStore(req.body), "application/json");
    });
    svr.Put("/api/merchant/stores/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->updateStore(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Delete("/api/merchant/stores/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->deleteStore(std::stoll(req.matches[1])), "application/json");
    });

    // ---- 服务区域 ----
    svr.Get("/api/merchant/service-areas", [&](const httplib::Request&, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getServiceAreas(), "application/json");
    });
    svr.Post("/api/merchant/service-areas", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->createServiceArea(req.body), "application/json");
    });
    svr.Put("/api/merchant/service-areas/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->updateServiceArea(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Delete("/api/merchant/service-areas/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->deleteServiceArea(std::stoll(req.matches[1])), "application/json");
    });

    // ---- 注册商品 (门店商品绑定) ----
    svr.Get("/api/merchant/store-products", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res);
        int64_t storeId = 0;
        if (req.has_param("storeId")) storeId = std::stoll(req.get_param_value("storeId"));
        res.set_content(merchantCtrl->getStoreProducts(storeId), "application/json");
    });
    svr.Post("/api/merchant/store-products/bind", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->bindProductsToStore(req.body), "application/json");
    });
    svr.Put("/api/merchant/store-products/(\\d+)/status", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->updateStoreProductStatus(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Put("/api/merchant/store-products/(\\d+)/stock", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->updateStoreProductStock(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Delete("/api/merchant/store-products/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->removeStoreProduct(std::stoll(req.matches[1])), "application/json");
    });

    // ---- 数据统计 ----
    svr.Get("/api/merchant/statistics/sales", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getSalesRanking(getQ(req)), "application/json");
    });
    svr.Get("/api/merchant/statistics/visits", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getVisitRanking(getQ(req)), "application/json");
    });

    // ---- 订单管理 ----
    svr.Get("/api/merchant/orders", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getOrders(getQ(req)), "application/json");
    });
    svr.Get("/api/merchant/orders/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getOrderDetail(std::stoll(req.matches[1])), "application/json");
    });
    svr.Post("/api/merchant/orders/(\\d+)/ship", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->shipOrder(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Post("/api/merchant/orders/(\\d+)/void", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->voidOrder(std::stoll(req.matches[1])), "application/json");
    });

    // ---- 会员管理 ----
    svr.Get("/api/merchant/members", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getMembers(getQ(req)), "application/json");
    });
    svr.Get("/api/merchant/members/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(merchantCtrl->getMemberDetail(std::stoll(req.matches[1])), "application/json");
    });

    /* ============================================================
       API 路由 — Login & Token
       ============================================================ */
    svr.Post("/api/login", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res);
        try {
            std::string body = req.body;
            // 安全注入 IP: 只在请求体不为空且确实需要时注入
            if (!body.empty() && body.find("\"ip\"") == std::string::npos) {
                auto pos = body.rfind('}');
                if (pos != std::string::npos && pos > 0) {
                    body.insert(pos, ",\"ip\":\"" + req.remote_addr + "\"");
                }
            }
            res.set_content(loginCtrl->login(body), "application/json");
        } catch (const std::exception& e) {
            res.set_content("{\"error\":true,\"message\":\"" + std::string(e.what()) + "\"}", "application/json");
        } catch (...) {
            res.set_content("{\"error\":true,\"message\":\"内部服务器错误\"}", "application/json");
        }
    });
    svr.Post("/api/logout", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(loginCtrl->logout(req.body), "application/json");
    });
    svr.Get("/api/check-token", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(loginCtrl->checkToken(toQueryString(req.params)), "application/json");
    });

    /* ============================================================
       API 路由 — Admin (用户/角色/日志)
       ============================================================ */
    svr.Get("/api/admin/users", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->getUsers(getQ(req)), "application/json");
    });
    svr.Get("/api/admin/users/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->getUserById(std::stoll(req.matches[1])), "application/json");
    });
    svr.Post("/api/admin/users", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->createUser(req.body), "application/json");
    });
    svr.Put("/api/admin/users/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->updateUser(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Delete("/api/admin/users/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->deleteUser(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Post("/api/admin/users/(\\d+)/reset-pwd", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->resetUserPassword(std::stoll(req.matches[1]), req.body), "application/json");
    });

    svr.Get("/api/admin/roles", [&](const httplib::Request&, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->getRoles(), "application/json");
    });
    svr.Post("/api/admin/roles", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->createRole(req.body), "application/json");
    });
    svr.Put("/api/admin/roles/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->updateRolePermissions(std::stoll(req.matches[1]), req.body), "application/json");
    });
    svr.Delete("/api/admin/roles/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->deleteRole(std::stoll(req.matches[1]), req.body), "application/json");
    });

    svr.Get("/api/admin/operation-logs", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->getOperationLogs(getQ(req)), "application/json");
    });
    svr.Get("/api/admin/operation-logs/(\\d+)", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(adminCtrl->getOperationLogDetail(std::stoll(req.matches[1])), "application/json");
    });

    svr.Get("/api/admin/login-logs", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(loginCtrl->getAdminLoginLogs(getQ(req)), "application/json");
    });
    svr.Get("/api/admin/user-logs", [&](const httplib::Request& req, httplib::Response& res) {
        addCors(res); res.set_content(loginCtrl->getUserLoginLogs(getQ(req)), "application/json");
    });

    /* ============================================================
       静态文件 — 前端 SPA
       ============================================================ */
    // 首页
    svr.Get("/", [&](const httplib::Request&, httplib::Response& res) {
        // 直接读取 index.html 返回，避免 mount_point 404 触发 error_handler 重定向死循环
        std::string indexPath = staticPath + "/index.html";
        std::ifstream f(indexPath);
        if (f.good()) {
            std::string content((std::istreambuf_iterator<char>(f)),
                                std::istreambuf_iterator<char>());
            res.set_content(content, "text/html; charset=utf-8");
        } else {
            res.status = 404;
            res.set_content("<h1>404</h1><p>找不到 index.html，请检查静态文件路径: " + indexPath + "</p>", "text/html; charset=utf-8");
        }
    });

    // 挂载整个 frontend 目录 (CSS, JS, HTML 等)
    svr.set_mount_point("/", staticPath);

    // 404 处理 — 非 API 路径返回简单的错误信息，不再重定向
    svr.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        if (req.path.find("/api/") == 0) {
            res.set_content("{\"error\":true,\"message\":\"API not found\"}", "application/json");
        } else {
            res.status = 404;
            res.set_content("<h1>404 Not Found</h1><p>资源不存在: " + req.path + "</p>", "text/html; charset=utf-8");
        }
    });

    // ---- 启动 ----
    std::cout << "[INFO] 服务启动中..." << std::endl;
    std::cout << "[INFO] 打开浏览器访问: http://" << host << ":" << port << std::endl;
    std::cout << "[INFO] 按 Ctrl+C 停止服务" << std::endl;

    svr.listen(host.c_str(), port);

    std::cout << "[INFO] 服务已停止" << std::endl;
    return 0;
}
