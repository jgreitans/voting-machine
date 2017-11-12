using Microsoft.AspNetCore.Mvc;
using webapp.Votes.Models;
using webapp.Votes.Interfaces;
using System.Threading.Tasks;

namespace webapp.Votes
{
    [Produces("application/json")]
    [Route("api/[controller]")]
    public class VotesController : Controller
    {
        private readonly IVoteService _service;

        public VotesController(IVoteService service)
        {
            _service = service;
        }

        [HttpPost]
        public async Task Post([FromBody]VoteAddModel vote)
        {
            await _service.Vote(vote);
        }

    }
}
