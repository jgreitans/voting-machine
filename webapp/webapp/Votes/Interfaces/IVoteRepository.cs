using System.Threading.Tasks;
using webapp.Votes.Models;

namespace webapp.Votes.Interfaces
{
    public interface IVoteRepository
    {
        Task Vote(VoteAddModel vote);
    }
}
