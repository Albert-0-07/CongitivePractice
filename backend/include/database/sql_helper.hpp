/**
 * @file    sql_helper.hpp
 * @brief   数据库操作辅助 — SQL 构建器、连接池、结果映射
 *
 * 为商户管理各模块提供统一的数据库访问接口:
 *   - 参数化查询构建
 *   - 分页 SQL 生成
 *   - CRUD 模板方法
 *   - 结果集到 Model 的映射
 *
 * 具体实现见 src/database/dp_pool.cpp (数据库连接池) 及
 * src/models/*.cpp (各模型的数据访问对象 DAO)。
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <functional>
#include <unordered_map>

namespace merchant {
namespace database {

/* ============================================================
   SQL 参数绑定
   ============================================================ */
struct SqlParam {
    enum class Type { INT, INT64, DOUBLE, STRING, BOOL, NULL_VAL };

    Type        type    = Type::NULL_VAL;
    int64_t     intVal  = 0;
    double      dblVal  = 0.0;
    std::string strVal;

    SqlParam() = default;

    static SqlParam fromInt(int32_t v)   { SqlParam p; p.type = Type::INT;    p.intVal = v; return p; }
    static SqlParam fromInt64(int64_t v) { SqlParam p; p.type = Type::INT64;  p.intVal = v; return p; }
    static SqlParam fromDouble(double v) { SqlParam p; p.type = Type::DOUBLE; p.dblVal = v; return p; }
    static SqlParam fromString(const std::string& v) { SqlParam p; p.type = Type::STRING; p.strVal = v; return p; }
    static SqlParam fromBool(bool v)     { SqlParam p; p.type = Type::BOOL;   p.intVal = v ? 1 : 0; return p; }
    static SqlParam null()               { return SqlParam(); }
};

/* ============================================================
   查询结果行
   ============================================================ */
class ResultRow {
public:
    virtual ~ResultRow() = default;

    virtual int32_t     getInt(const std::string& col) const = 0;
    virtual int64_t     getInt64(const std::string& col) const = 0;
    virtual double      getDouble(const std::string& col) const = 0;
    virtual std::string getString(const std::string& col) const = 0;
    virtual bool        getBool(const std::string& col) const = 0;
    virtual bool        isNull(const std::string& col) const = 0;
};

/* ============================================================
   查询结果集
   ============================================================ */
class ResultSet {
public:
    virtual ~ResultSet() = default;

    virtual bool              next() = 0;     ///< 游标移到下一行
    virtual const ResultRow&  current() const = 0;
    virtual int32_t           rowCount() const = 0;
};

/* ============================================================
   数据库连接
   ============================================================ */
class DbConnection {
public:
    virtual ~DbConnection() = default;

    /// 执行查询, 返回结果集
    virtual std::unique_ptr<ResultSet> query(
        const std::string& sql,
        const std::vector<SqlParam>& params = {}) = 0;

    /// 执行写操作 (INSERT/UPDATE/DELETE), 返回影响行数
    virtual int32_t execute(
        const std::string& sql,
        const std::vector<SqlParam>& params = {}) = 0;

    /// 获取最后插入的自增ID
    virtual int64_t lastInsertId() = 0;

    /// 开启事务
    virtual void beginTransaction() = 0;

    /// 提交事务
    virtual void commit() = 0;

    /// 回滚事务
    virtual void rollback() = 0;
};

/* ============================================================
   连接池
   ============================================================ */
class ConnectionPool {
public:
    virtual ~ConnectionPool() = default;

    /// 从池中获取一个连接 (用完后自动归还, 或通过 RAII guard)
    virtual std::unique_ptr<DbConnection> acquire() = 0;

    /// 获取当前活跃连接数
    virtual int32_t activeCount() const = 0;

    /// 获取空闲连接数
    virtual int32_t idleCount() const = 0;
};

/* ============================================================
   SQL 构建器
   ============================================================ */

/**
 * @brief 分页 SQL 生成器
 *
 * 根据数据库类型生成对应的 LIMIT/OFFSET 子句
 */
struct PaginationSql {
    static std::string mysql(int32_t page, int32_t pageSize);
    static std::string postgresql(int32_t page, int32_t pageSize);
};

/**
 * @brief 动态 WHERE 条件构建
 *
 * 用于构建带可选参数的查询, 如:
 * @code
 *   WhereBuilder w;
 *   w.addIf("name LIKE ?",    !name.empty(),    SqlParam::fromString("%" + name + "%"));
 *   w.addIf("categoryId = ?", categoryId != 0,  SqlParam::fromInt64(categoryId));
 *   w.addIf("status = ?",     status.has_value(), SqLParam::fromInt(status.value()));
 *   auto [clause, params] = w.build();
 * @endcode
 */
class WhereBuilder {
public:
    WhereBuilder() = default;

    /// 条件成立时添加子句
    WhereBuilder& addIf(const std::string& clause, bool condition, const SqlParam& param);

    /// 无条件添加子句
    WhereBuilder& add(const std::string& clause, const SqlParam& param);

    /// 构建 WHERE 子句和参数列表
    /// @return pair{ WHERE clause string, 参数列表 }
    std::pair<std::string, std::vector<SqlParam>> build() const;

    bool empty() const { return clauses.empty(); }

private:
    std::vector<std::string>       clauses;
    std::vector<SqlParam>          params;
};

/* ============================================================
   CRUD 模板工具
   ============================================================ */

/**
 * @brief 通用 CRUD 操作工具
 *
 * 提供模板化的增删改查操作, 减少各模块重复代码。
 * 每个模型需要提供自己的 表名、字段列表、结果集→模型映射函数。
 *
 * 使用示例:
 * @code
 *   // 定义映射
 *   auto mapper = [](const ResultRow& row) -> Product {
 *       Product p;
 *       p.id = row.getInt64("id");
 *       p.name = row.getString("name");
 *       // ...
 *       return p;
 *   };
 *
 *   CrudHelper<Product> helper("products", {
 *       "name", "category_id", "price", "stock", "status"
 *   });
 *
 *   auto result = helper.getById(conn, 1, mapper);
 * @endcode
 */
template <typename T>
class CrudHelper {
public:
    CrudHelper(const std::string& tableName,
               const std::vector<std::string>& columns)
        : m_table(tableName), m_columns(columns) {}

    /// 按 ID 查询单条
    T getById(DbConnection& conn, int64_t id,
              std::function<T(const ResultRow&)> mapper) const;

    /// 查询全部
    std::vector<T> getAll(DbConnection& conn,
                          std::function<T(const ResultRow&)> mapper) const;

    /// 分页查询
    std::pair<std::vector<T>, int64_t> getPage(
        DbConnection& conn, int32_t page, int32_t pageSize,
        const std::string& whereClause,
        const std::vector<SqlParam>& whereParams,
        std::function<T(const ResultRow&)> mapper) const;

    /// 插入
    int64_t insert(DbConnection& conn,
                   const std::vector<std::string>& values,
                   const std::vector<SqlParam>& params) const;

    /// 更新
    int32_t update(DbConnection& conn, int64_t id,
                   const std::vector<std::string>& setClauses,
                   const std::vector<SqlParam>& params) const;

    /// 删除
    int32_t remove(DbConnection& conn, int64_t id) const;

    const std::string& tableName() const { return m_table; }
    const std::vector<std::string>& columns() const { return m_columns; }

private:
    std::string              m_table;
    std::vector<std::string> m_columns;
};

/* ============================================================
   商户系统预定义 SQL 表名常量
   ============================================================ */
namespace tables {
    constexpr const char* CATEGORIES     = "product_categories";
    constexpr const char* PRODUCTS       = "products";
    constexpr const char* PROMOTIONS     = "promotions";
    constexpr const char* PROMO_PRODUCTS = "promotion_products";
    constexpr const char* STORES         = "stores";
    constexpr const char* SERVICE_AREAS  = "service_areas";
    constexpr const char* STORE_PRODUCTS = "store_products";
    constexpr const char* ORDERS         = "orders";
    constexpr const char* ORDER_ITEMS    = "order_items";
    constexpr const char* ORDER_TIMELINE = "order_timeline";
    constexpr const char* USERS          = "users";
    constexpr const char* ROLES          = "roles";
    constexpr const char* ROLE_PERMS     = "role_permissions";
    constexpr const char* OPERATION_LOGS = "operation_logs";
}

} // namespace database
} // namespace merchant
