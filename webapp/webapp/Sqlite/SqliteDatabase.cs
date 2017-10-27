using Microsoft.Data.Sqlite;
using System;
using System.Data.Common;
using System.IO;
using System.Threading.Tasks;
using Dapper;

namespace webapp.Sqlite
{
    public static class SqliteDatabase
    {
        private static string DatabaseDirectory => Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "voting-machine");
        private static string DatabaseFileName => Path.Combine(DatabaseDirectory, "voting-machine.sqlite");

        public static async Task<DbConnection> GetConnection()
        {
            var builder = SqliteFactory.Instance.CreateConnectionStringBuilder() as SqliteConnectionStringBuilder;
            Directory.CreateDirectory(DatabaseDirectory);
            var exists = File.Exists(DatabaseFileName);
            builder.DataSource = DatabaseFileName;
            builder.Mode = SqliteOpenMode.ReadWriteCreate;
            var conn = SqliteFactory.Instance.CreateConnection();
            conn.ConnectionString = builder.ConnectionString;
            await conn.OpenAsync();
            if (!exists)
            {
                await SeedDatabase(conn);
            }
            return conn;
        }

        private static async Task SeedDatabase(DbConnection conn)
        {
            await conn.ExecuteAsync(
                @"CREATE TABLE IF NOT EXISTS [Locations] (" +
                    "Id NVARCHAR(32) NOT NULL PRIMARY KEY, " +
                    "Name NVARCHAR(128) NOT NULL, " +
                    "DateCreated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP" +
                ")");
        }
    }
}
