using System.Threading.Tasks;
using webapp.Votes.Interfaces;
using webapp.Votes.Models;

namespace webapp.Votes.Services
{
    public class VoteService : IVoteService
    {
        private readonly IVoteRepository _repository;

        public VoteService(IVoteRepository repository)
        {
            _repository = repository;
        }

        public async Task Vote(VoteAddModel vote)
        {
            await _repository.Vote(vote);
        }
    }
}
