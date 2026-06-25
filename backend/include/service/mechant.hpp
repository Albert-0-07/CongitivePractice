/**
 * @file    mechant.hpp
 * @brief   商户服务层 — MerchantService 业务逻辑接口
 *
 * 对应前端商户管理全部功能模块:
 *   - 商品分类 CRUD
 *   - 商品信息 CRUD
 *   - 促销管理 CRUD + 商品绑定
 *   - 门店管理 CRUD
 *   - 服务区域管理 CRUD
 *   - 注册商品管理 (门店-商品绑定、库存分配)
 *   - 数据统计 (销售排行、访问排行)
 *   - 订单管理 (查询、发货、作废)
 *   - 会员管理 (列表、详情)
 */

#pragma once

#include "models/product.hpp"
#include "models/store.hpp"
#include "models/order.hpp"
#include "models/user.hpp"

#include <string>
#include <vector>
#include <memory>

namespace merchant {
namespace service {

using namespace merchant::model;

/**
 * @class MerchantService
 * @brief 商户后台业务逻辑层
 *
 * 所有方法返回业务结果; 异常通过 throw 传递错误信息。
 * 具体实现见 src/service/mechant_service.cpp
 */
class MerchantService {
public:
    MerchantService() = default;
    virtual ~MerchantService() = default;

    /* ==========================================================
       商品分类 (Product Categories)
       ========================================================== */

    /// 获取所有分类 (按 sort 排序)
    virtual std::vector<ProductCategory> getCategories() = 0;

    /// 创建分类, 返回带ID的完整对象
    virtual ProductCategory createCategory(const ProductCategory& data) = 0;

    /// 更新分类, 返回更新后的对象
    virtual ProductCategory updateCategory(int64_t id, const ProductCategory& data) = 0;

    /// 删除分类 (需校验该分类下无商品)
    virtual void deleteCategory(int64_t id) = 0;

    /* ==========================================================
       商品信息 (Products)
       ========================================================== */

    /// 分页查询商品列表
    virtual PageResult<Product> getProducts(const ProductQueryParams& params) = 0;

    /// 按ID获取单个商品
    virtual Product getProductById(int64_t id) = 0;

    /// 创建商品
    virtual Product createProduct(const Product& data) = 0;

    /// 更新商品
    virtual Product updateProduct(int64_t id, const Product& data) = 0;

    /// 删除商品
    virtual void deleteProduct(int64_t id) = 0;

    /* ==========================================================
       促销管理 (Promotions)
       ========================================================== */

    /// 获取所有促销列表
    virtual std::vector<Promotion> getPromotions() = 0;

    /// 创建促销
    virtual Promotion createPromotion(const Promotion& data) = 0;

    /// 更新促销
    virtual Promotion updatePromotion(int64_t id, const Promotion& data) = 0;

    /// 删除促销
    virtual void deletePromotion(int64_t id) = 0;

    /// 为促销绑定商品
    virtual void bindProductsToPromotion(int64_t promotionId,
                                         const std::vector<int64_t>& productIds) = 0;

    /// 获取促销已绑定的商品ID列表
    virtual std::vector<int64_t> getPromotionProductIds(int64_t promotionId) = 0;

    /* ==========================================================
       门店管理 (Stores)
       ========================================================== */

    /// 获取所有门店列表
    virtual std::vector<Store> getStores() = 0;

    /// 创建门店
    virtual Store createStore(const Store& data) = 0;

    /// 更新门店
    virtual Store updateStore(int64_t id, const Store& data) = 0;

    /// 删除门店
    virtual void deleteStore(int64_t id) = 0;

    /* ==========================================================
       服务区域管理 (Service Areas)
       ========================================================== */

    /// 获取所有服务区域 (树形结构, 按 sort 排序)
    virtual std::vector<ServiceArea> getServiceAreas() = 0;

    /// 创建区域
    virtual ServiceArea createServiceArea(const ServiceArea& data) = 0;

    /// 更新区域
    virtual ServiceArea updateServiceArea(int64_t id, const ServiceArea& data) = 0;

    /// 删除区域 (需校验该区域下无子区域和门店)
    virtual void deleteServiceArea(int64_t id) = 0;

    /* ==========================================================
       注册商品管理 (Store-Product Binding)
       ========================================================== */

    /// 获取某门店已绑定的商品列表
    virtual std::vector<StoreProduct> getStoreProducts(int64_t storeId) = 0;

    /// 为门店批量绑定商品 (含初始库存)
    /// @param storeId  门店ID
    /// @param bindings 绑定列表: {productId, stock}
    virtual void bindProductsToStore(int64_t storeId,
                                     const std::vector<StoreProduct>& bindings) = 0;

    /// 更新门店商品上下架状态
    virtual void updateStoreProductStatus(int64_t id, int32_t status) = 0;

    /// 更新门店商品库存
    virtual void updateStoreProductStock(int64_t id, int32_t stock) = 0;

    /// 移除门店商品绑定
    virtual void removeStoreProduct(int64_t id) = 0;

    /* ==========================================================
       数据统计 (Statistics)
       ========================================================== */

    /// 商品销售排行
    virtual std::vector<SalesRankItem> getSalesRanking(const StatQueryParams& params) = 0;

    /// 商品访问排行
    virtual std::vector<VisitRankItem> getVisitRanking(const StatQueryParams& params) = 0;

    /* ==========================================================
       订单管理 (Orders)
       ========================================================== */

    /// 分页查询订单列表
    virtual PageResult<Order> getOrders(const OrderQueryParams& params) = 0;

    /// 获取订单详情 (含明细行和时间线)
    virtual Order getOrderDetail(int64_t id) = 0;

    /// 发货: 填写快递信息, 状态 待发货 -> 已发货
    virtual void shipOrder(int64_t id, const ShipRequest& req) = 0;

    /// 作废订单: 状态 待付款/待发货 -> 已取消
    virtual void voidOrder(int64_t id) = 0;

    /* ==========================================================
       会员管理 (Members)
       ========================================================== */

    /// 分页查询会员列表
    virtual PageResult<User> getMembers(const MemberQueryParams& params) = 0;

    /// 获取会员详情
    virtual User getMemberDetail(int64_t id) = 0;
};

/// 工厂函数类型
using MerchantServicePtr = std::shared_ptr<MerchantService>;

/// 创建 MerchantService 实例的工厂函数
MerchantServicePtr createMerchantService();

} // namespace service
} // namespace merchant
