using System.Threading.Tasks;
using webapp.Votes.Models;

namespace webapp.Votes.Interfaces
{
    public interface IVoteService
    {
        Task Vote(VoteAddModel vote);
    }
}
