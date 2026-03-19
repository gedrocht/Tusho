#include "tusho/database/sqlite_database_connection.hpp"

#include <stdexcept>

namespace tusho
{
SqliteDatabaseConnection::SqliteDatabaseConnection(const std::filesystem::path &database_file_path)
{
  if (database_file_path.has_parent_path())
  {
    std::filesystem::create_directories(database_file_path.parent_path());
  }

  const int open_result = sqlite3_open(database_file_path.string().c_str(), &native_database_handle_);

  if (open_result != SQLITE_OK || native_database_handle_ == nullptr)
  {
    const std::string failure_message =
      native_database_handle_ == nullptr ? "SQLite failed to open the database." : sqlite3_errmsg(native_database_handle_);

    if (native_database_handle_ != nullptr)
    {
      sqlite3_close(native_database_handle_);
      native_database_handle_ = nullptr;
    }

    throw std::runtime_error(failure_message);
  }

  execute_sql_statement("PRAGMA foreign_keys = ON;");
  execute_sql_statement("PRAGMA journal_mode = WAL;");
  execute_sql_statement("PRAGMA synchronous = NORMAL;");
}

SqliteDatabaseConnection::~SqliteDatabaseConnection()
{
  if (native_database_handle_ != nullptr)
  {
    sqlite3_close(native_database_handle_);
    native_database_handle_ = nullptr;
  }
}

SqliteDatabaseConnection::SqliteDatabaseConnection(SqliteDatabaseConnection &&other_database_connection) noexcept
{
  native_database_handle_ = other_database_connection.native_database_handle_;
  other_database_connection.native_database_handle_ = nullptr;
}

SqliteDatabaseConnection &
SqliteDatabaseConnection::operator=(SqliteDatabaseConnection &&other_database_connection) noexcept
{
  if (this != &other_database_connection)
  {
    if (native_database_handle_ != nullptr)
    {
      sqlite3_close(native_database_handle_);
    }

    native_database_handle_ = other_database_connection.native_database_handle_;
    other_database_connection.native_database_handle_ = nullptr;
  }

  return *this;
}

sqlite3 *SqliteDatabaseConnection::native_handle() const noexcept
{
  return native_database_handle_;
}

void SqliteDatabaseConnection::execute_sql_statement(const std::string &sql_statement_text) const
{
  char *error_message_buffer = nullptr;

  const int execution_result =
    sqlite3_exec(native_database_handle_, sql_statement_text.c_str(), nullptr, nullptr, &error_message_buffer);

  if (execution_result != SQLITE_OK)
  {
    const std::string failure_message =
      error_message_buffer == nullptr ? "SQLite failed to execute a SQL statement." : error_message_buffer;
    sqlite3_free(error_message_buffer);
    throw std::runtime_error(failure_message);
  }
}

sqlite3_stmt *SqliteDatabaseConnection::prepare_statement(const std::string &sql_statement_text) const
{
  sqlite3_stmt *prepared_statement = nullptr;

  const int prepare_result = sqlite3_prepare_v2(
    native_database_handle_,
    sql_statement_text.c_str(),
    static_cast<int>(sql_statement_text.size()),
    &prepared_statement,
    nullptr);

  if (prepare_result != SQLITE_OK)
  {
    throw std::runtime_error(sqlite3_errmsg(native_database_handle_));
  }

  return prepared_statement;
}

std::int64_t SqliteDatabaseConnection::fetch_last_inserted_row_identifier() const
{
  return static_cast<std::int64_t>(sqlite3_last_insert_rowid(native_database_handle_));
}
} // namespace tusho
