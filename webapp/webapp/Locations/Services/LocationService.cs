using System.Collections.Generic;
using System.Threading.Tasks;
using webapp.Locations.Interfaces;
using webapp.Locations.Models;

namespace webapp.Locations.Services
{
    public class LocationService : ILocationService
    {
        private readonly ILocationRepository _locationRepository;

        public LocationService(ILocationRepository locationRepository)
        {
            _locationRepository = locationRepository;
        }

        public async Task<IEnumerable<LocationModel>> GetAllLocations()
        {
            return await _locationRepository.GetAllLocations();
        }

        public async Task AddLocation(LocationAddModel model)
        {
            await _locationRepository.AddLocation(model);
        }
    }
}
