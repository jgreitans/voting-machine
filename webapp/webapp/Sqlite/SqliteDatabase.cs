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

        public static DbConnection GetConnection()
        {
            var builder = SqliteFactory.Instance.CreateConnectionStringBuilder() as SqliteConnectionStringBuilder;
            Directory.CreateDirectory(DatabaseDirectory);
            var exists = File.Exists(DatabaseFileName);
            builder.DataSource = DatabaseFileName;
            builder.Mode = SqliteOpenMode.ReadWriteCreate;
            var conn = SqliteFactory.Instance.CreateConnection();
            conn.ConnectionString = builder.ConnectionString;
            conn.Open();
            if (!exists)
            {
                SeedDatabase(conn);
            }
            return conn;
        }

        private static void SeedDatabase(DbConnection conn)
        {
            conn.Execute(
                @"CREATE TABLE IF NOT EXISTS [Locations] (" +
                    "Id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    "Name NVARCHAR(128) NOT NULL, " +
                    "Ts TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP" +
                ")");
            conn.Execute(
                @"CREATE TABLE IF NOT EXISTS [Votes] (" +
                    "Id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    "Value SMALLINT NOT NULL, " +
                    "Ts TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP" +
                ")");
        }
    }
}
