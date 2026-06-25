/**
 * @file    api_router.cpp
 * @brief   ApiRouter 实现 — 路由注册与请求分发
 *
 * 将所有 /api/merchant/* URL 映射到 MerchantController 对应方法。
 */

#include "router/api_router.hpp"

#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace merchant {
namespace router {

/* ============================================================
   路径匹配辅助
   ============================================================ */

/// 提取路径中的 :param 占位符值
/// 如路径 "/api/merchant/orders/5" 匹配模式 "/api/merchant/orders/:id"
/// 返回 {"id": "5"}
static std::unordered_map<std::string, std::string> matchPath(
    const std::string& pattern, const std::string& actual) {

    std::unordered_map<std::string, std::string> params;

    // 将 pattern 和 actual 按 '/' 切分
    auto split = [](const std::string& s) -> std::vector<std::string> {
        std::vector<std::string> parts;
        std::string part;
        for (char c : s) {
            if (c == '/') {
                if (!part.empty()) parts.push_back(part);
                part.clear();
            } else {
                part += c;
            }
        }
        if (!part.empty()) parts.push_back(part);
        return parts;
    };

    auto patternParts = split(pattern);
    auto actualParts  = split(actual);

    if (patternParts.size() != actualParts.size()) return params; // 不匹配

    for (size_t i = 0; i < patternParts.size(); ++i) {
        if (patternParts[i].size() > 1 && patternParts[i][0] == ':') {
            // 占位符 → 提取值
            params[patternParts[i].substr(1)] = actualParts[i];
        } else if (patternParts[i] != actualParts[i]) {
            params.clear();
            return params; // 不匹配
        }
    }

    return params;
}

/* ============================================================
   Constructor
   ============================================================ */
ApiRouter::ApiRouter(MerchantControllerPtr merchantController)
    : m_merchantCtrl(std::move(merchantController)) {}

/* ============================================================
   返回所有商户路由定义
   ============================================================ */
std::vector<RouteEntry> ApiRouter::getMerchantRoutes() const {
    return {
        // 商品分类
        {HttpMethod::GET,     "/api/merchant/categories",         "获取所有分类"},
        {HttpMethod::POST,    "/api/merchant/categories",         "创建分类"},
        {HttpMethod::PUT,     "/api/merchant/categories/:id",     "更新分类"},
        {HttpMethod::DELETE_, "/api/merchant/categories/:id",     "删除分类"},

        // 商品信息
        {HttpMethod::GET,     "/api/merchant/products",           "分页查询商品"},
        {HttpMethod::POST,    "/api/merchant/products",           "创建商品"},
        {HttpMethod::PUT,     "/api/merchant/products/:id",       "更新商品"},
        {HttpMethod::DELETE_, "/api/merchant/products/:id",       "删除商品"},

        // 促销管理
        {HttpMethod::GET,     "/api/merchant/promotions",         "获取所有促销"},
        {HttpMethod::POST,    "/api/merchant/promotions",         "创建促销"},
        {HttpMethod::PUT,     "/api/merchant/promotions/:id",     "更新促销"},
        {HttpMethod::DELETE_, "/api/merchant/promotions/:id",     "删除促销"},
        {HttpMethod::POST,    "/api/merchant/promotions/:id/bind","绑定商品到促销"},

        // 门店管理
        {HttpMethod::GET,     "/api/merchant/stores",             "获取所有门店"},
        {HttpMethod::POST,    "/api/merchant/stores",             "创建门店"},
        {HttpMethod::PUT,     "/api/merchant/stores/:id",         "更新门店"},
        {HttpMethod::DELETE_, "/api/merchant/stores/:id",         "删除门店"},

        // 服务区域
        {HttpMethod::GET,     "/api/merchant/service-areas",       "获取所有区域"},
        {HttpMethod::POST,    "/api/merchant/service-areas",       "创建区域"},
        {HttpMethod::PUT,     "/api/merchant/service-areas/:id",   "更新区域"},
        {HttpMethod::DELETE_, "/api/merchant/service-areas/:id",   "删除区域"},

        // 注册商品 (门店商品绑定)
        {HttpMethod::GET,     "/api/merchant/store-products",      "查询门店商品"},
        {HttpMethod::POST,    "/api/merchant/store-products/bind", "绑定商品到门店"},
        {HttpMethod::PUT,     "/api/merchant/store-products/:id/status", "更新上下架"},
        {HttpMethod::PUT,     "/api/merchant/store-products/:id/stock",  "更新库存"},
        {HttpMethod::DELETE_, "/api/merchant/store-products/:id",  "移除门店商品绑定"},

        // 数据统计
        {HttpMethod::GET,     "/api/merchant/statistics/sales",    "销售排行"},
        {HttpMethod::GET,     "/api/merchant/statistics/visits",   "访问排行"},

        // 订单管理
        {HttpMethod::GET,     "/api/merchant/orders",              "分页查询订单"},
        {HttpMethod::GET,     "/api/merchant/orders/:id",          "订单详情"},
        {HttpMethod::POST,    "/api/merchant/orders/:id/ship",     "发货"},
        {HttpMethod::POST,    "/api/merchant/orders/:id/void",     "作废"},

        // 会员管理
        {HttpMethod::GET,     "/api/merchant/members",             "分页查询会员"},
        {HttpMethod::GET,     "/api/merchant/members/:id",         "会员详情"},
    };
}

/* ============================================================
   请求分发
   根据 HTTP 方法 + 路径, 将请求路由到 MerchantController 对应 handler。
   ============================================================ */
std::string ApiRouter::dispatch(HttpMethod method, const std::string& path,
                                const std::string& body, const std::string& queryParams) {
    if (!m_merchantCtrl) {
        return "{\"error\":true,\"message\":\"控制器未初始化\"}";
    }

    auto routes = getMerchantRoutes();

    // 遍历所有路由定义, 找到匹配的
    for (auto& route : routes) {
        if (route.method != method) continue;

        auto params = matchPath(route.path, path);
        if (params.empty() && route.path.find(':') == std::string::npos) {
            // 精确匹配 (路径不含占位符)
            if (route.path != path) continue;
        } else if (params.empty()) {
            continue; // 含占位符但未匹配
        }

        // 找到匹配路由 → 调用对应的 controller 方法
        // 注意: 这里需要手动映射每条路由到具体的 controller 方法

        // 商品分类
        if (route.path == "/api/merchant/categories") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getCategories();
            if (method == HttpMethod::POST)   return m_merchantCtrl->createCategory(body);
            if (method == HttpMethod::PUT)    return m_merchantCtrl->updateCategory(std::stoll(params["id"]), body);
            if (method == HttpMethod::DELETE_)return m_merchantCtrl->deleteCategory(std::stoll(params["id"]));
        }

        // 商品信息
        if (route.path == "/api/merchant/products") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getProducts(queryParams);
            if (method == HttpMethod::POST)   return m_merchantCtrl->createProduct(body);
            if (method == HttpMethod::PUT)    return m_merchantCtrl->updateProduct(std::stoll(params["id"]), body);
            if (method == HttpMethod::DELETE_)return m_merchantCtrl->deleteProduct(std::stoll(params["id"]));
        }

        // 促销管理
        if (route.path == "/api/merchant/promotions") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getPromotions();
            if (method == HttpMethod::POST)   return m_merchantCtrl->createPromotion(body);
            if (method == HttpMethod::PUT)    return m_merchantCtrl->updatePromotion(std::stoll(params["id"]), body);
            if (method == HttpMethod::DELETE_)return m_merchantCtrl->deletePromotion(std::stoll(params["id"]));
        }
        if (route.path == "/api/merchant/promotions/:id/bind") {
            if (method == HttpMethod::POST)   return m_merchantCtrl->bindProductsToPromotion(std::stoll(params["id"]), body);
        }

        // 门店管理
        if (route.path == "/api/merchant/stores") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getStores();
            if (method == HttpMethod::POST)   return m_merchantCtrl->createStore(body);
            if (method == HttpMethod::PUT)    return m_merchantCtrl->updateStore(std::stoll(params["id"]), body);
            if (method == HttpMethod::DELETE_)return m_merchantCtrl->deleteStore(std::stoll(params["id"]));
        }

        // 服务区域
        if (route.path == "/api/merchant/service-areas") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getServiceAreas();
            if (method == HttpMethod::POST)   return m_merchantCtrl->createServiceArea(body);
            if (method == HttpMethod::PUT)    return m_merchantCtrl->updateServiceArea(std::stoll(params["id"]), body);
            if (method == HttpMethod::DELETE_)return m_merchantCtrl->deleteServiceArea(std::stoll(params["id"]));
        }

        // 注册商品 (门店商品绑定)
        if (route.path == "/api/merchant/store-products") {
            if (method == HttpMethod::GET) {
                // 从 queryParams 提取 storeId
                auto eq = queryParams.find("storeId=");
                int64_t storeId = 0;
                if (eq != std::string::npos) {
                    auto amp = queryParams.find('&', eq);
                    storeId = std::stoll(queryParams.substr(eq + 8, amp == std::string::npos ? amp : amp - eq - 8));
                }
                return m_merchantCtrl->getStoreProducts(storeId);
            }
        }
        if (route.path == "/api/merchant/store-products/bind") {
            if (method == HttpMethod::POST)   return m_merchantCtrl->bindProductsToStore(body);
        }
        if (route.path == "/api/merchant/store-products/:id/status") {
            if (method == HttpMethod::PUT)    return m_merchantCtrl->updateStoreProductStatus(std::stoll(params["id"]), body);
        }
        if (route.path == "/api/merchant/store-products/:id/stock") {
            if (method == HttpMethod::PUT)    return m_merchantCtrl->updateStoreProductStock(std::stoll(params["id"]), body);
        }
        if (route.path == "/api/merchant/store-products/:id") {
            if (method == HttpMethod::DELETE_)return m_merchantCtrl->removeStoreProduct(std::stoll(params["id"]));
        }

        // 数据统计
        if (route.path == "/api/merchant/statistics/sales") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getSalesRanking(queryParams);
        }
        if (route.path == "/api/merchant/statistics/visits") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getVisitRanking(queryParams);
        }

        // 订单管理
        if (route.path == "/api/merchant/orders") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getOrders(queryParams);
        }
        if (route.path == "/api/merchant/orders/:id") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getOrderDetail(std::stoll(params["id"]));
        }
        if (route.path == "/api/merchant/orders/:id/ship") {
            if (method == HttpMethod::POST)   return m_merchantCtrl->shipOrder(std::stoll(params["id"]), body);
        }
        if (route.path == "/api/merchant/orders/:id/void") {
            if (method == HttpMethod::POST)   return m_merchantCtrl->voidOrder(std::stoll(params["id"]));
        }

        // 会员管理
        if (route.path == "/api/merchant/members") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getMembers(queryParams);
        }
        if (route.path == "/api/merchant/members/:id") {
            if (method == HttpMethod::GET)    return m_merchantCtrl->getMemberDetail(std::stoll(params["id"]));
        }
    }

    return "{\"error\":true,\"message\":\"路由未找到: " +
           std::string(method == HttpMethod::GET ? "GET" :
                       method == HttpMethod::POST ? "POST" :
                       method == HttpMethod::PUT ? "PUT" : "DELETE") +
           " " + path + "\"}";
}

} // namespace router
} // namespace merchant
