/**
 * @file    dp_pool.cpp
 * @brief   数据库连接池 — 占位实现
 *
 * 当前为开发/演示阶段，使用内存数据，无需真实数据库连接。
 * 切换为 MySQL/PostgreSQL 生产环境时，在此实现 ConnectionPool 接口。
 *
 * 参见 include/database/sql_helper.hpp 中定义的:
 *   - SqlParam
 *   - ResultRow / ResultSet
 *   - DbConnection
 *   - ConnectionPool
 *   - WhereBuilder
 *   - CrudHelper<T>
 */

#include "database/sql_helper.hpp"

namespace merchant {
namespace database {

// -------------------------------------------------------------------
// PaginationSql — 占位实现 (返回空串, 内存数据不需要)
// -------------------------------------------------------------------
std::string PaginationSql::mysql(int32_t, int32_t) {
    return "";
}

std::string PaginationSql::postgresql(int32_t, int32_t) {
    return "";
}

// -------------------------------------------------------------------
// WhereBuilder 实现
// -------------------------------------------------------------------
WhereBuilder& WhereBuilder::addIf(const std::string& clause, bool condition,
                                  const SqlParam& param) {
    if (condition) {
        clauses.push_back(clause);
        params.push_back(param);
    }
    return *this;
}

WhereBuilder& WhereBuilder::add(const std::string& clause, const SqlParam& param) {
    clauses.push_back(clause);
    params.push_back(param);
    return *this;
}

std::pair<std::string, std::vector<SqlParam>> WhereBuilder::build() const {
    if (clauses.empty()) return {"", {}};

    std::string sql = "WHERE ";
    for (size_t i = 0; i < clauses.size(); ++i) {
        if (i > 0) sql += " AND ";
        sql += clauses[i];
    }
    return {sql, params};
}

} // namespace database
} // namespace merchant
