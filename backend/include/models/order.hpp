/**
 * @file    order.hpp
 * @brief   订单相关数据模型 — Order, OrderItem, OrderTimeline
 *
 * 对应前端商户管理中的:
 *   - 订单管理 (Orders) — 查询、发货、作废
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace merchant {
namespace model {

/* ============================================================
   订单商品明细 (Order Item)
   ============================================================ */
struct OrderItem {
    int64_t     productId   = 0;        ///< 商品ID
    std::string name;                   ///< 商品名称 (冗余, 防止商品信息变更后历史数据丢失)
    double      price       = 0.0;      ///< 下单时单价
    int32_t     qty         = 0;        ///< 购买数量
    double      subtotal    = 0.0;      ///< 小计 (price * qty)

    OrderItem() = default;
};

/* ============================================================
   订单状态时间线条目 (Timeline Entry)
   ============================================================ */
struct OrderTimelineEntry {
    std::string status;                 ///< 状态描述: 已下单 / 已支付 / 已发货 / 已完成 / 已取消
    std::string time;                   ///< 时间点

    OrderTimelineEntry() = default;
    OrderTimelineEntry(std::string status, std::string time)
        : status(std::move(status)), time(std::move(time)) {}
};

/* ============================================================
   订单 (Order)
   对应前端: Orders 列表 + 详情
   ============================================================ */
struct Order {
    int64_t     id          = 0;        ///< 订单ID (主键)
    std::string orderNo;                ///< 订单编号 (如 ORD20260601001)
    int64_t     userId      = 0;        ///< 会员用户ID
    std::string userName;               ///< 会员用户名 (冗余)
    std::string phone;                  ///< 联系电话 (冗余)
    std::string address;                ///< 收货地址 (冗余)
    int32_t     productCount= 0;        ///< 商品总数量
    double      totalAmount = 0.0;      ///< 订单总金额
    std::string status;                 ///< 订单状态: 待付款/待发货/已发货/已完成/已取消
    std::string payMethod;              ///< 支付方式: 微信支付/支付宝/...
    std::string payTime;                ///< 支付时间 (空=未支付)
    std::string createTime;             ///< 下单时间
    std::string trackingNo;             ///< 快递单号 (已发货后才有)
    std::string courier;                ///< 快递公司 (已发货后才有)

    /// 订单明细行
    std::vector<OrderItem> items;

    /// 状态变化时间线
    std::vector<OrderTimelineEntry> timeline;

    Order() = default;

    /// 是否可发货
    bool canShip() const { return status == "待发货"; }

    /// 是否可作废
    bool canVoid() const { return status == "待付款" || status == "待发货"; }
};

/* ============================================================
   发货请求 (Shipping Request)
   ============================================================ */
struct ShipRequest {
    int64_t     orderId     = 0;
    std::string trackingNo;             ///< 快递单号 (必填)
    std::string courier;                ///< 快递公司

    ShipRequest() = default;
};

/* ============================================================
   订单查询参数
   ============================================================ */
struct OrderQueryParams {
    std::optional<std::string> orderNo;     ///< 按订单编号搜索
    std::optional<std::string> status;      ///< 按状态筛选
    std::optional<std::string> userName;    ///< 按用户名搜索
    std::optional<std::string> startDate;   ///< 下单时间开始
    std::optional<std::string> endDate;     ///< 下单时间结束
    int32_t                    page     = 1;
    int32_t                    pageSize = 10;

    OrderQueryParams() = default;
};

} // namespace model
} // namespace merchant
