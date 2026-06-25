/**
 * @file    merchant.hpp
 * @brief   商户控制器 — MerchantController HTTP 请求处理层
 *
 * 对应前端商户管理全部 REST 接口:
 *   GET    /api/merchant/categories
 *   POST   /api/merchant/categories
 *   PUT    /api/merchant/categories/:id
 *   DELETE /api/merchant/categories/:id
 *   GET    /api/merchant/products
 *   POST   /api/merchant/products
 *   PUT    /api/merchant/products/:id
 *   DELETE /api/merchant/products/:id
 *   GET    /api/merchant/promotions
 *   POST   /api/merchant/promotions
 *   PUT    /api/merchant/promotions/:id
 *   DELETE /api/merchant/promotions/:id
 *   POST   /api/merchant/promotions/:id/bind
 *   GET    /api/merchant/stores
 *   POST   /api/merchant/stores
 *   PUT    /api/merchant/stores/:id
 *   DELETE /api/merchant/stores/:id
 *   GET    /api/merchant/service-areas
 *   POST   /api/merchant/service-areas
 *   PUT    /api/merchant/service-areas/:id
 *   DELETE /api/merchant/service-areas/:id
 *   GET    /api/merchant/store-products?storeId=
 *   POST   /api/merchant/store-products/bind
 *   PUT    /api/merchant/store-products/:id/status
 *   PUT    /api/merchant/store-products/:id/stock
 *   DELETE /api/merchant/store-products/:id
 *   GET    /api/merchant/statistics/sales
 *   GET    /api/merchant/statistics/visits
 *   GET    /api/merchant/orders
 *   GET    /api/merchant/orders/:id
 *   POST   /api/merchant/orders/:id/ship
 *   POST   /api/merchant/orders/:id/void
 *   GET    /api/merchant/members
 *   GET    /api/merchant/members/:id
 */

#pragma once

#include "service/mechant.hpp"
#include <memory>
#include <string>
#include <functional>

namespace merchant {
namespace controller {

using namespace merchant::service;

/**
 * @class MerchantController
 * @brief 商户HTTP控制器
 *
 * 职责:
 *   - 解析 HTTP 请求参数
 *   - 调用对应 Service 方法
 *   - 构造 HTTP 响应 (JSON)
 *   - 统一错误处理
 *
 * 具体实现见 src/controller/merchant_controller.cpp
 */
class MerchantController {
public:
    /**
     * @param service  业务逻辑层实例 (依赖注入)
     */
    explicit MerchantController(MerchantServicePtr service);
    virtual ~MerchantController() = default;

    /* ---- 回调类型 ----
     * 每个 handler 接收 JSON 请求体字符串, 返回 JSON 响应体字符串。
     * 实际使用时, 由框架 (如 Drogon / cpp-httplib / 自定义) 适配这些签名。
     */
    using Request  = std::string;   ///< 请求体 (JSON string)
    using Response = std::string;   ///< 响应体 (JSON string)

    /* ==========================================================
       商品分类 (Product Categories)
       ========================================================== */

    Response getCategories();
    Response createCategory(const Request& body);
    Response updateCategory(int64_t id, const Request& body);
    Response deleteCategory(int64_t id);

    /* ==========================================================
       商品信息 (Products)
       ========================================================== */

    Response getProducts(const Request& queryParams);
    Response createProduct(const Request& body);
    Response updateProduct(int64_t id, const Request& body);
    Response deleteProduct(int64_t id);

    /* ==========================================================
       促销管理 (Promotions)
       ========================================================== */

    Response getPromotions();
    Response createPromotion(const Request& body);
    Response updatePromotion(int64_t id, const Request& body);
    Response deletePromotion(int64_t id);
    Response bindProductsToPromotion(int64_t id, const Request& body);

    /* ==========================================================
       门店管理 (Stores)
       ========================================================== */

    Response getStores();
    Response createStore(const Request& body);
    Response updateStore(int64_t id, const Request& body);
    Response deleteStore(int64_t id);

    /* ==========================================================
       服务区域管理 (Service Areas)
       ========================================================== */

    Response getServiceAreas();
    Response createServiceArea(const Request& body);
    Response updateServiceArea(int64_t id, const Request& body);
    Response deleteServiceArea(int64_t id);

    /* ==========================================================
       注册商品管理 (Store-Product Binding)
       ========================================================== */

    Response getStoreProducts(int64_t storeId);
    Response bindProductsToStore(const Request& body);
    Response updateStoreProductStatus(int64_t id, const Request& body);
    Response updateStoreProductStock(int64_t id, const Request& body);
    Response removeStoreProduct(int64_t id);

    /* ==========================================================
       数据统计 (Statistics)
       ========================================================== */

    Response getSalesRanking(const Request& queryParams);
    Response getVisitRanking(const Request& queryParams);

    /* ==========================================================
       订单管理 (Orders)
       ========================================================== */

    Response getOrders(const Request& queryParams);
    Response getOrderDetail(int64_t id);
    Response shipOrder(int64_t id, const Request& body);
    Response voidOrder(int64_t id);

    /* ==========================================================
       会员管理 (Members)
       ========================================================== */

    Response getMembers(const Request& queryParams);
    Response getMemberDetail(int64_t id);

private:
    MerchantServicePtr m_service;

    /// 构造成功响应
    static Response success(const std::string& jsonBody);
    /// 构造错误响应
    static Response error(const std::string& message, int code = 400);
    /// 构造分页响应
    static Response pageResult(const std::string& jsonArray, int64_t total, int32_t page, int32_t pageSize);
};

/// 工厂
using MerchantControllerPtr = std::shared_ptr<MerchantController>;

} // namespace controller
} // namespace merchant
