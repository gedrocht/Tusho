#ifndef TUSHO_DATABASE_SQLITE_DATABASE_CONNECTION_HPP
#define TUSHO_DATABASE_SQLITE_DATABASE_CONNECTION_HPP

#include <cstdint>
#include <filesystem>
#include <sqlite3.h>
#include <string>

namespace tusho
{
/**
 * @brief Small RAII wrapper around a SQLite database connection.
 *
 * "RAII" means "Resource Acquisition Is Initialization". In practical terms,
 * this class opens the database in its constructor and guarantees that the
 * database is closed in its destructor. That pattern helps prevent resource
 * leaks and keeps the calling code tidy.
 */
class SqliteDatabaseConnection
{
public:
  explicit SqliteDatabaseConnection(const std::filesystem::path &database_file_path);
  ~SqliteDatabaseConnection();

  SqliteDatabaseConnection(const SqliteDatabaseConnection &) = delete;
  SqliteDatabaseConnection &operator=(const SqliteDatabaseConnection &) = delete;

  SqliteDatabaseConnection(SqliteDatabaseConnection &&other_database_connection) noexcept;
  SqliteDatabaseConnection &operator=(SqliteDatabaseConnection &&other_database_connection) noexcept;

  sqlite3 *native_handle() const noexcept;

  void execute_sql_statement(const std::string &sql_statement_text) const;
  sqlite3_stmt *prepare_statement(const std::string &sql_statement_text) const;
  std::int64_t fetch_last_inserted_row_identifier() const;

private:
  sqlite3 *native_database_handle_ = nullptr;
};
} // namespace tusho

#endif
