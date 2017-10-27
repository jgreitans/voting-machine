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
            using (var conn = await SqliteDatabase.GetConnection())
            {
                return await conn.QueryAsync<LocationModel>(@"SELECT * FROM Locations");
            }
            return await Task.FromResult<IEnumerable<LocationModel>>(
                new List<LocationModel>
                {
                    new LocationModel
                    {
                        Id = "c0800c22-d8ab-4732-aef0-2684e2b7f6e0",
                        Name = "Room 1"
                    },
                    new LocationModel
                    {
                        Id = "b149ba4c-a750-42e8-811d-e299cf9081b5",
                        Name = "Room 2"
                    }
                });
        }

        public async Task<LocationModel> AddLocation(LocationAddModel model)
        {
            using (var conn = await SqliteDatabase.GetConnection())
            {
                var id = Guid.NewGuid().ToString();
                await conn.ExecuteAsync(@"INSERT INTO Locations(Id, Name) VALUES(@Id, @Name)",
                    new {
                        Id = id,
                        Name = model.Name
                    });

                return new LocationModel(id, model.Name);
            }
        }
    }
}
