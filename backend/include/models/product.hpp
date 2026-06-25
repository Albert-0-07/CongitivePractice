/**
 * @file    product.hpp
 * @brief   商品相关数据模型 — Product, Category, Promotion
 *
 * 对应前端商户管理中的:
 *   - 商品分类 (ProductCategory)
 *   - 商品信息管理 (Product)
 *   - 促销管理 (Promotion)
 *
 * 该模块与 store.hpp 配合完成 注册商品管理 功能。
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace merchant {
namespace model {

/* ============================================================
   商品分类 (Product Category)
   对应前端: Categories CRUD
   ============================================================ */
struct ProductCategory {
    int64_t     id          = 0;        ///< 分类ID (主键)
    std::string name;                   ///< 分类名称
    std::string description;            ///< 分类描述
    int32_t     sort        = 0;        ///< 排序值 (越小越靠前)
    int32_t     status      = 1;        ///< 状态: 1=启用, 0=禁用
    std::string createdAt;              ///< 创建时间 (ISO-8601 格式)

    ProductCategory() = default;
};

/* ============================================================
   商品 (Product)
   对应前端: Products CRUD
   ============================================================ */
struct Product {
    int64_t     id          = 0;        ///< 商品ID (主键)
    std::string name;                   ///< 商品名称
    int64_t     categoryId  = 0;        ///< 所属分类ID -> ProductCategory::id
    std::string image;                  ///< 商品图片 (emoji 或 URL)
    std::string description;            ///< 商品简介
    double      price       = 0.0;      ///< 单价
    int32_t     stock       = 0;        ///< 总库存
    std::string unit;                   ///< 单位 (斤/个/箱/盒/...)
    int32_t     status      = 1;        ///< 状态: 1=上架, 0=下架
    std::string createdAt;              ///< 创建时间

    Product() = default;
};

/* ============================================================
   促销 (Promotion)
   对应前端: Promotions CRUD + 绑定商品
   ============================================================ */
struct Promotion {
    int64_t     id          = 0;        ///< 促销ID (主键)
    std::string name;                   ///< 促销名称
    std::string type;                   ///< 促销类型: 限时折扣 / 满减 / 买赠 / 优惠券
    std::string startTime;              ///< 开始时间 (datetime)
    std::string endTime;                ///< 结束时间 (datetime)
    double      discount    = 0.0;      ///< 折扣值 (仅 限时折扣 有效, 1-10)
    std::string description;            ///< 促销描述
    std::string status;                 ///< 状态: 进行中 / 未开始 / 已结束
    std::vector<int64_t> productIds;    ///< 绑定的商品ID列表
    std::string fullDiscountCondition;  ///< 满减条件说明 (仅 满减 有效)
    std::string giftDescription;        ///< 赠品说明 (仅 买赠 有效)
    std::string couponDescription;      ///< 优惠券使用说明 (仅 优惠券 有效)

    Promotion() = default;
};

/* ============================================================
   门店商品关联 (Store-Product Binding)
   对应前端: StoreProducts — 注册商品管理
   ============================================================ */
struct StoreProduct {
    int64_t     id          = 0;        ///< 记录ID (主键)
    int64_t     storeId     = 0;        ///< 门店ID -> Store::id
    int64_t     productId   = 0;        ///< 商品ID -> Product::id
    int32_t     stock       = 0;        ///< 该门店分配库存
    int32_t     status      = 1;        ///< 上架状态: 1=已上架, 0=已下架
    std::string bindTime;               ///< 绑定时间

    StoreProduct() = default;
};

/* ============================================================
   统计 — 销售排行条目 (Statistics - Sales Ranking)
   ============================================================ */
struct SalesRankItem {
    int64_t     id          = 0;
    std::string name;
    int64_t     categoryId  = 0;
    std::string image;
    int32_t     salesCount  = 0;        ///< 销量
    double      salesAmount = 0.0;      ///< 销售额
    double      avgPrice    = 0.0;      ///< 均价

    SalesRankItem() = default;
};

/* ============================================================
   统计 — 访问排行条目 (Statistics - Visit Ranking)
   ============================================================ */
struct VisitRankItem {
    int64_t     id              = 0;
    std::string name;
    int64_t     categoryId      = 0;
    std::string image;
    int32_t     visitCount      = 0;    ///< 访问量 (PV)
    int32_t     uniqueVisitors  = 0;    ///< 独立访客 (UV)
    std::string conversionRate;         ///< 转化率 (字符串, 如 "24.5%")

    VisitRankItem() = default;
};

/* ============================================================
   统计查询参数
   ============================================================ */
struct StatQueryParams {
    std::optional<std::string> startDate;   ///< 开始日期
    std::optional<std::string> endDate;     ///< 结束日期
    std::optional<int64_t>     categoryId;  ///< 分类筛选

    StatQueryParams() = default;
};

/* ============================================================
   商品查询参数 (分页 + 过滤)
   ============================================================ */
struct ProductQueryParams {
    std::optional<std::string> name;        ///< 按名称模糊搜索
    std::optional<int64_t>     categoryId;  ///< 按分类筛选
    std::optional<int32_t>     status;      ///< 按状态筛选
    int32_t                    page     = 1;
    int32_t                    pageSize = 10;

    ProductQueryParams() = default;
};

/* ============================================================
   通用分页结果模板
   ============================================================ */
template <typename T>
struct PageResult {
    std::vector<T> list;        ///< 当前页数据
    int64_t        total    = 0;  ///< 总记录数
    int32_t        page     = 1;  ///< 当前页码
    int32_t        pageSize = 10; ///< 每页条数

    PageResult() = default;
};

} // namespace model
} // namespace merchant
