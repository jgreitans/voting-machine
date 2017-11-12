using Dapper;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using webapp.Locations.Interfaces;
using webapp.Locations.Models;
using webapp.Sqlite;

namespace webapp.Locations.Repositories
{
    public class SqliteLocationRepository : ILocationRepository
    {
        public async Task<IEnumerable<LocationModel>> GetAllLocations()
        {
            using (var conn = SqliteDatabase.GetConnection())
            {
                return await conn.QueryAsync<LocationModel>(@"SELECT * FROM Locations");
            }
        }

        public async Task AddLocation(LocationAddModel model)
        {
            using (var conn = SqliteDatabase.GetConnection())
            {
                var id = Guid.NewGuid().ToString();
                await conn.ExecuteAsync(@"INSERT INTO Locations(Name) VALUES(@Name)",
                    new {
                        Name = model.Name
                    });
            }
        }
    }
}
