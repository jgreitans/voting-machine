using Dapper;
using System.Threading.Tasks;
using webapp.Sqlite;
using webapp.Votes.Interfaces;
using webapp.Votes.Models;

namespace webapp.Votes.Repositories
{
    public class SqliteVoteRepository : IVoteRepository
    {
        public async Task Vote(VoteAddModel vote)
        {
            using (var conn = SqliteDatabase.GetConnection())
            {
                await conn.ExecuteAsync(@"INSERT INTO Votes(Value) VALUES(@Value)",
                    new
                    {
                        Value = vote.Value
                    });
            }
        }
    }
}
