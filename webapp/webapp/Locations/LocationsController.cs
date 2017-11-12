using System.Collections.Generic;
using Microsoft.AspNetCore.Mvc;
using System.Threading.Tasks;
using webapp.Locations.Interfaces;
using webapp.Locations.Models;

namespace webapp.Locations
{
    [Produces("application/json")]
    [Route("api/[controller]")]
    public class LocationsController : Controller
    {
        private readonly ILocationService _locationService;

        public LocationsController(ILocationService locationService)
        {
            _locationService = locationService;
        }

        [HttpGet]
        public async Task<IEnumerable<LocationModel>> GetAll()
        {
            return await _locationService.GetAllLocations();
        }

        [HttpPost]
        public async Task Add([FromBody]LocationAddModel model)
        {
            await _locationService.AddLocation(model);
        }
        
        [HttpPut("{id}")]
        public void Edit(string id, [FromBody]LocationAddModel value)
        {
        }
        
        [HttpDelete("{id}")]
        public void Delete(string id)
        {
        }
    }
}
